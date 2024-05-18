/*
 * dfu.c
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/usb/dfu.h>
#include <halm/usb/dfu_defs.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
#include <halm/usb/usb_trace.h>
#include <xcore/memory.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static void deviceDescriptor(const void *, struct UsbDescriptor *, void *);
static void configDescriptor(const void *, struct UsbDescriptor *, void *);
static void interfaceDescriptor(const void *, struct UsbDescriptor *, void *);
static void functionalDescriptor(const void *, struct UsbDescriptor *, void *);
/*----------------------------------------------------------------------------*/
static void onTimerOverflow(void *);
static enum Result processDownloadRequest(struct Dfu *, const void *, uint16_t);
static void processGetStatusRequest(struct Dfu *, void *, uint16_t *);
static enum Result processUploadRequest(struct Dfu *, uint16_t, void *,
    uint16_t *);
static void resetDriver(struct Dfu *);
static void setStatus(struct Dfu *, enum DfuStatus);
/*----------------------------------------------------------------------------*/
static enum Result driverInit(void *, const void *);
static void driverDeinit(void *);
static enum Result driverControl(void *, const struct UsbSetupPacket *,
    void *, uint16_t *, uint16_t);
static const UsbDescriptorFunctor *driverDescribe(const void *);
static void driverNotify(void *, unsigned int);
/*----------------------------------------------------------------------------*/
const struct UsbDriverClass * const Dfu = &(const struct UsbDriverClass){
    .size = sizeof(struct Dfu),
    .init = driverInit,
    .deinit = driverDeinit,

    .control = driverControl,
    .describe = driverDescribe,
    .notify = driverNotify
};
/*----------------------------------------------------------------------------*/
static const UsbDescriptorFunctor deviceDescriptorTable[] = {
    deviceDescriptor,
    configDescriptor,
    interfaceDescriptor,
    functionalDescriptor,
    NULL
};
/*----------------------------------------------------------------------------*/
static void deviceDescriptor(const void *, struct UsbDescriptor *header,
    void *payload)
{
  header->length = sizeof(struct UsbDeviceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_DEVICE;

  if (payload != NULL)
  {
    static const struct UsbDeviceDescriptor descriptor = {
        .length = sizeof(struct UsbDeviceDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_DEVICE,
        .usb = TO_LITTLE_ENDIAN_16(0x0200),
        .deviceClass = USB_CLASS_PER_INTERFACE,
        .deviceSubClass = 0,
        .deviceProtocol = 0,
        .maxPacketSize = TO_LITTLE_ENDIAN_16(DFU_CONTROL_EP_SIZE),
        .idVendor = 0,
        .idProduct = 0,
        .device = TO_LITTLE_ENDIAN_16(0x0100),
        .manufacturer = 0,
        .product = 0,
        .serialNumber = 0,
        .numConfigurations = 1
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void configDescriptor(const void *, struct UsbDescriptor *header,
    void *payload)
{
  header->length = sizeof(struct UsbConfigurationDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CONFIGURATION;

  if (payload != NULL)
  {
    static const struct UsbConfigurationDescriptor descriptor = {
        .length = sizeof(struct UsbConfigurationDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_CONFIGURATION,
        .totalLength = TO_LITTLE_ENDIAN_16(
            sizeof(struct UsbConfigurationDescriptor)
            + sizeof(struct UsbInterfaceDescriptor)
            + sizeof(struct DfuFunctionalDescriptor)),
        .numInterfaces = 1,
        .configurationValue = 1,
        .configuration = 0,
        .attributes = 0,
        .maxPower = 0
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void interfaceDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct Dfu * const driver = object;

  header->length = sizeof(struct UsbInterfaceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_INTERFACE;

  if (payload != NULL)
  {
    const struct UsbInterfaceDescriptor descriptor = {
        .length = sizeof(struct UsbInterfaceDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_INTERFACE,
        .interfaceNumber = driver->interfaceIndex,
        .alternateSettings = 0,
        .numEndpoints = 0,
        .interfaceClass = USB_CLASS_APP_SPEC,
        .interfaceSubClass = APP_SPEC_SUBCLASS_DFU,
        .interfaceProtocol = APP_SPEC_PROTOCOL_DFU_MODE,
        .interface = 0
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void functionalDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct Dfu * const driver = object;

  header->length = sizeof(struct DfuFunctionalDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_DFU_FUNCTIONAL;

  if (payload != NULL)
  {
    struct DfuFunctionalDescriptor descriptor = {
        .length = sizeof(struct DfuFunctionalDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_DFU_FUNCTIONAL,
        .attributes = 0,
        .detachTimeout = TO_LITTLE_ENDIAN_16(USHRT_MAX),
        .transferSize = toLittleEndian16(driver->transferSize),
        .dfuVersion = TO_LITTLE_ENDIAN_16(0x0110)
    };

    descriptor.attributes = DFU_MANIFESTATION_TOLERANT;
    if (driver->onDetachRequest != NULL)
      descriptor.attributes |= DFU_WILL_DETACH;
    if (driver->onDownloadRequest != NULL)
      descriptor.attributes |= DFU_CAN_DNLOAD;
    if (driver->onUploadRequest != NULL)
      descriptor.attributes |= DFU_CAN_UPLOAD;

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void onTimerOverflow(void *argument)
{
  struct Dfu * const driver = argument;

  switch ((enum DfuState)driver->state)
  {
    case STATE_DFU_DNBUSY:
      driver->state = STATE_DFU_DNLOAD_SYNC;
      break;

    case STATE_DFU_MANIFEST:
      driver->state = STATE_DFU_MANIFEST_SYNC;
      break;

    default:
      break;
  }

  usbTrace("dfu at %u: timer event", driver->interfaceIndex);
}
/*----------------------------------------------------------------------------*/
static enum Result processDownloadRequest(struct Dfu *driver,
    const void *payload, uint16_t payloadLength)
{
  if (driver->onDownloadRequest == NULL)
  {
    setStatus(driver, DFU_STATUS_ERR_STALLEDPKT);
    return E_INVALID;
  }

  if (driver->state == STATE_DFU_IDLE && payloadLength)
  {
    driver->position = 0;
  }
  else if (driver->state != STATE_DFU_DNLOAD_IDLE)
  {
    setStatus(driver, DFU_STATUS_ERR_STALLEDPKT);
    usbTrace("dfu at %u: incorrect sequence", driver->interfaceIndex);
    return E_ERROR;
  }

  const size_t written = driver->onDownloadRequest(driver->callbackArgument,
      driver->position, payload, payloadLength, &driver->timeout);

  /*
   * User-space code must not use timeouts when the timer is not
   * initialized.
   */
  assert(driver->timer != NULL || !driver->timeout);

  if (written == payloadLength)
  {
    /* Write succeeded, update internal position */
    driver->position += written;
    driver->state = written ? STATE_DFU_DNLOAD_SYNC : STATE_DFU_MANIFEST_SYNC;
    return E_OK;
  }
  else
  {
    /* Write failed */
    setStatus(driver, DFU_STATUS_ERR_WRITE);
    usbTrace("dfu at %u: write failed at %u",
        driver->interfaceIndex, driver->position);
    return E_INTERFACE;
  }
}
/*----------------------------------------------------------------------------*/
static void processGetStatusRequest(struct Dfu *driver, void *response,
    uint16_t *responseLength)
{
  const bool enableTimer = driver->timer != NULL && driver->timeout;

  switch ((enum DfuState)driver->state)
  {
    case STATE_DFU_DNLOAD_SYNC:
      driver->state = enableTimer ? STATE_DFU_DNBUSY : STATE_DFU_DNLOAD_IDLE;
      break;

    case STATE_DFU_MANIFEST_SYNC:
      driver->state = enableTimer ? STATE_DFU_MANIFEST : STATE_DFU_IDLE;
      break;

    default:
      break;
  }

  const struct DfuGetStatusResponse statusResponse = {
      .status = driver->status,
      .pollTimeout = {
            driver->timeout,
            driver->timeout >> 8,
            driver->timeout >> 16
      },
      .state = driver->state,
      .string = 0
  };

  memcpy(response, &statusResponse, sizeof(statusResponse));
  *responseLength = sizeof(statusResponse);

  usbTrace("dfu at %u: state %u, status %u, timeout %u",
      driver->interfaceIndex, driver->state, driver->status, driver->timeout);

  if (enableTimer)
  {
    const uint32_t frequency = timerGetFrequency(driver->timer);
    const uint64_t timeout = (driver->timeout * (1ULL << 32)) / 1000;
    const uint32_t overflow = (frequency * timeout + ((1ULL << 32) - 1)) >> 32;

    timerSetOverflow(driver->timer, overflow);
    timerEnable(driver->timer);
  }
}
/*----------------------------------------------------------------------------*/
static enum Result processUploadRequest(struct Dfu *driver,
    uint16_t requestedLength, void *response, uint16_t *responseLength)
{
  if (driver->onUploadRequest == NULL)
  {
    setStatus(driver, DFU_STATUS_ERR_STALLEDPKT);
    return E_INVALID;
  }

  if (driver->state == STATE_DFU_IDLE)
  {
    driver->position = 0;
  }
  else if (driver->state != STATE_DFU_UPLOAD_IDLE)
  {
    setStatus(driver, DFU_STATUS_ERR_STALLEDPKT);
    usbTrace("dfu at %u: incorrect upload sequence", driver->interfaceIndex);
    return E_ERROR;
  }

  const size_t read = driver->onUploadRequest(driver->callbackArgument,
      driver->position, response, requestedLength);

  driver->position += read;
  driver->state = read == requestedLength ?
      STATE_DFU_UPLOAD_IDLE : STATE_DFU_IDLE;

  *responseLength = (uint16_t)read;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void resetDriver(struct Dfu *driver)
{
  driver->position = 0;
  driver->state = STATE_DFU_IDLE;
  driver->status = DFU_STATUS_OK;
  driver->timeout = 0;
}
/*----------------------------------------------------------------------------*/
static void setStatus(struct Dfu *driver, enum DfuStatus status)
{
  if (status != DFU_STATUS_OK)
    driver->state = STATE_DFU_ERROR;
  driver->status = status;
}
/*----------------------------------------------------------------------------*/
static enum Result driverInit(void *object, const void *configBase)
{
  const struct DfuConfig * const config = configBase;
  assert(config != NULL);
  assert(config->device != NULL);
  assert(config->transferSize);

  struct Dfu * const driver = object;

  driver->device = config->device;
  driver->callbackArgument = NULL;
  driver->onDetachRequest = NULL;
  driver->onDownloadRequest = NULL;
  driver->onUploadRequest = NULL;
  driver->transferSize = config->transferSize;

  if (config->timer != NULL)
  {
    driver->timer = config->timer;
    timerSetAutostop(driver->timer, true);
    timerSetCallback(driver->timer, onTimerOverflow, driver);
  }
  else
    driver->timer = NULL;

  resetDriver(driver);

  driver->interfaceIndex = usbDevGetInterface(driver->device);
  return usbDevBind(driver->device, driver);
}
/*----------------------------------------------------------------------------*/
static void driverDeinit(void *object)
{
  struct Dfu * const driver = object;

  usbDevUnbind(driver->device, driver);

  if (driver->timer != NULL)
    timerSetCallback(driver->timer, NULL, NULL);
}
/*----------------------------------------------------------------------------*/
static enum Result driverControl(void *object,
    const struct UsbSetupPacket *packet, void *buffer, uint16_t *responseLength,
    uint16_t)
{
  struct Dfu * const driver = object;
  const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);

  if (type != REQUEST_TYPE_CLASS)
    return E_INVALID;
  if (packet->index != driver->interfaceIndex)
    return E_INVALID;

  enum Result res;

  switch (packet->request)
  {
    case DFU_REQUEST_DNLOAD:
    {
      res = processDownloadRequest(driver, buffer, packet->length);
      break;
    }

    case DFU_REQUEST_UPLOAD:
    {
      res = processUploadRequest(driver, packet->length, buffer,
          responseLength);
      break;
    }

    case DFU_REQUEST_GETSTATUS:
    {
      processGetStatusRequest(driver, buffer, responseLength);
      res = E_OK;
      break;
    }

    case DFU_REQUEST_CLRSTATUS:
    {
      if ((enum DfuState)driver->state == STATE_DFU_ERROR)
      {
        usbTrace("dfu at %u: status cleared", driver->interfaceIndex);

        driver->status = DFU_STATUS_OK;
        driver->state = STATE_DFU_IDLE;
        res = E_OK;
      }
      else
      {
        usbTrace("dfu at %u: clear failed", driver->interfaceIndex);

        setStatus(driver, DFU_STATUS_ERR_STALLEDPKT);
        res = E_INVALID;
      }

      break;
    }

    case DFU_REQUEST_ABORT:
    {
      switch ((enum DfuState)driver->state)
      {
        case STATE_DFU_IDLE:
        case STATE_DFU_DNLOAD_SYNC:
        case STATE_DFU_DNLOAD_IDLE:
        case STATE_DFU_MANIFEST_SYNC:
        case STATE_DFU_UPLOAD_IDLE:
          usbTrace("dfu at %u: aborted", driver->interfaceIndex);

          driver->state = STATE_DFU_IDLE;
          res = E_OK;
          break;

        default:
          usbTrace("dfu at %u: abort failed", driver->interfaceIndex);

          setStatus(driver, DFU_STATUS_ERR_STALLEDPKT);
          res = E_INVALID;
          break;
      }

      break;
    }

    case DFU_REQUEST_DETACH:
    {
      usbTrace("dfu at %u: detach request", driver->interfaceIndex);

      if (driver->onDetachRequest != NULL)
      {
        driver->onDetachRequest(driver->callbackArgument, packet->value);
        driver->state = STATE_APP_IDLE;
        res = E_OK;
      }
      else
      {
        setStatus(driver, DFU_STATUS_ERR_STALLEDPKT);
        res = E_INVALID;
      }
      break;
    }

    case DFU_REQUEST_GETSTATE:
    {
      *(uint8_t *)buffer = driver->state;
      *responseLength = 1;
      res = E_OK;
      break;
    }

    default:
    {
      usbTrace("dfu at %u: unknown request 0x%02X",
          driver->interfaceIndex, packet->request);

      setStatus(driver, DFU_STATUS_ERR_STALLEDPKT);
      res = E_INVALID;
      break;
    }
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static const UsbDescriptorFunctor *driverDescribe(const void *)
{
  return deviceDescriptorTable;
}
/*----------------------------------------------------------------------------*/
static void driverNotify(void *object, unsigned int event)
{
  struct Dfu * const driver = object;

  if (event == USB_DEVICE_EVENT_RESET)
    resetDriver(driver);
}
/*----------------------------------------------------------------------------*/
void dfuOnDownloadCompleted(struct Dfu *driver, bool status)
{
  driver->timeout = 0;

  switch ((enum DfuState)driver->state)
  {
    case STATE_DFU_DNLOAD_SYNC:
    case STATE_DFU_DNBUSY:
      if (!status)
        driver->status = DFU_STATUS_ERR_WRITE;
      break;

    default:
      usbTrace("dfu at %u: unexpected call", driver->interfaceIndex);
      break;
  }
}
/*----------------------------------------------------------------------------*/
void dfuSetDetachRequestCallback(struct Dfu *driver,
    void (*callback)(void *, uint16_t))
{
  driver->onDetachRequest = callback;
}
/*----------------------------------------------------------------------------*/
void dfuSetDownloadRequestCallback(struct Dfu *driver,
    size_t (*callback)(void *, size_t, const void *, size_t, uint16_t *))
{
  driver->onDownloadRequest = callback;
}
/*----------------------------------------------------------------------------*/
void dfuSetUploadRequestCallback(struct Dfu *driver,
    size_t (*callback)(void *, size_t, void *, size_t))
{
  driver->onUploadRequest = callback;
}
/*----------------------------------------------------------------------------*/
void dfuSetCallbackArgument(struct Dfu *driver, void *argument)
{
  driver->callbackArgument = argument;
}
