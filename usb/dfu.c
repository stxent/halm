/*
 * dfu.c
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/usb/dfu.h>
#include <halm/usb/dfu_defs.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
#include <halm/usb/usb_trace.h>
#include <xcore/memory.h>
#include <assert.h>
#include <limits.h>
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
static inline void toLittleEndian24(uint8_t *, uint32_t);
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
    0
};
/*----------------------------------------------------------------------------*/
static void deviceDescriptor(const void *object __attribute__((unused)),
    struct UsbDescriptor *header, void *payload)
{
  header->length = sizeof(struct UsbDeviceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_DEVICE;

  if (payload)
  {
    struct UsbDeviceDescriptor * const descriptor = payload;

    descriptor->usb = TO_LITTLE_ENDIAN_16(0x0200);
    descriptor->deviceClass = USB_CLASS_PER_INTERFACE;
    descriptor->maxPacketSize = TO_LITTLE_ENDIAN_16(DFU_CONTROL_EP_SIZE);
    descriptor->device = TO_LITTLE_ENDIAN_16(0x0100);
    descriptor->numConfigurations = 1;
  }
}
/*----------------------------------------------------------------------------*/
static void configDescriptor(const void *object __attribute__((unused)),
    struct UsbDescriptor *header, void *payload)
{
  header->length = sizeof(struct UsbConfigurationDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CONFIGURATION;

  if (payload)
  {
    struct UsbConfigurationDescriptor * const descriptor = payload;

    descriptor->totalLength = TO_LITTLE_ENDIAN_16(
        sizeof(struct UsbConfigurationDescriptor)
        + sizeof(struct UsbInterfaceDescriptor)
        + sizeof(struct DfuFunctionalDescriptor));
    descriptor->numInterfaces = 1;
    descriptor->configurationValue = 1;
  }
}
/*----------------------------------------------------------------------------*/
static void interfaceDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct Dfu * const driver = object;

  header->length = sizeof(struct UsbInterfaceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_INTERFACE;

  if (payload)
  {
    struct UsbInterfaceDescriptor * const descriptor = payload;

    descriptor->interfaceNumber = driver->interfaceIndex;
    descriptor->numEndpoints = 0;
    descriptor->interfaceClass = USB_CLASS_APP_SPEC;
    descriptor->interfaceSubClass = APP_SPEC_SUBCLASS_DFU;
    descriptor->interfaceProtocol = APP_SPEC_PROTOCOL_DFU_MODE;
  }
}
/*----------------------------------------------------------------------------*/
static void functionalDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct Dfu * const driver = object;

  header->length = sizeof(struct DfuFunctionalDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_DFU_FUNCTIONAL;

  if (payload)
  {
    struct DfuFunctionalDescriptor * const descriptor = payload;

    descriptor->attributes = DFU_MANIFESTATION_TOLERANT;
    if (driver->onDetachRequest)
      descriptor->attributes |= DFU_WILL_DETACH;
    if (driver->onDownloadRequest)
      descriptor->attributes |= DFU_CAN_DNLOAD;
    if (driver->onUploadRequest)
      descriptor->attributes |= DFU_CAN_UPLOAD;

    descriptor->detachTimeout = TO_LITTLE_ENDIAN_16(USHRT_MAX);
    descriptor->dfuVersion = TO_LITTLE_ENDIAN_16(0x0110);

    descriptor->transferSize = toLittleEndian16(driver->transferSize);
  }
}
/*----------------------------------------------------------------------------*/
static void onTimerOverflow(void *argument)
{
  struct Dfu * const driver = argument;

  timerDisable(driver->timer);

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
  if (!driver->onDownloadRequest)
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
  assert(driver->timer || !driver->timeout);

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
  const bool enableTimer = driver->timer && driver->timeout;

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

  struct DfuGetStatusResponse * const statusResponse = response;

  toLittleEndian24(statusResponse->pollTimeout, driver->timeout);
  statusResponse->status = driver->status;
  statusResponse->state = driver->state;
  statusResponse->string = 0;
  *responseLength = sizeof(struct DfuGetStatusResponse);

  usbTrace("dfu at %u: state %u, status %u, timeout %u",
      driver->interfaceIndex, driver->state, driver->status, driver->timeout);

  if (enableTimer)
  {
    timerSetOverflow(driver->timer, driver->timeout);
    timerSetValue(driver->timer, 0);
    timerEnable(driver->timer);
  }
}
/*----------------------------------------------------------------------------*/
static enum Result processUploadRequest(struct Dfu *driver,
    uint16_t requestedLength, void *response, uint16_t *responseLength)
{
  if (!driver->onUploadRequest)
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
static inline void toLittleEndian24(uint8_t *output, uint32_t input)
{
  output[0] = (uint8_t)input;
  output[1] = (uint8_t)(input >> 8);
  output[2] = (uint8_t)(input >> 16);
}
/*----------------------------------------------------------------------------*/
static enum Result driverInit(void *object, const void *configBase)
{
  const struct DfuConfig * const config = configBase;
  assert(config);
  assert(config->device);
  assert(config->transferSize);

  struct Dfu * const driver = object;

  driver->device = config->device;
  driver->timer = config->timer;
  driver->callbackArgument = 0;
  driver->onDetachRequest = 0;
  driver->onDownloadRequest = 0;
  driver->onUploadRequest = 0;
  driver->transferSize = config->transferSize;

  if (driver->timer)
    timerSetCallback(driver->timer, onTimerOverflow, driver);

  resetDriver(driver);

  driver->interfaceIndex = usbDevGetInterface(driver->device);
  return usbDevBind(driver->device, driver);
}
/*----------------------------------------------------------------------------*/
static void driverDeinit(void *object)
{
  struct Dfu * const driver = object;

  usbDevUnbind(driver->device, driver);

  if (driver->timer)
    timerSetCallback(driver->timer, 0, 0);
}
/*----------------------------------------------------------------------------*/
static enum Result driverControl(void *object,
    const struct UsbSetupPacket *packet, void *buffer,
    uint16_t *responseLength,
    uint16_t maxResponseLength __attribute__((unused)))
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

      if (driver->onDetachRequest)
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
static const UsbDescriptorFunctor *driverDescribe(const void *object
    __attribute__((unused)))
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
