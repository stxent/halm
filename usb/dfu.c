/*
 * dfu.c
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <limits.h>
#include <xcore/memory.h>
#include <halm/usb/dfu.h>
#include <halm/usb/dfu_defs.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
#include <halm/usb/usb_trace.h>
/*----------------------------------------------------------------------------*/
static void deviceDescriptor(const void *, struct UsbDescriptor *, void *);
static void configDescriptor(const void *, struct UsbDescriptor *, void *);
static void interfaceDescriptor(const void *, struct UsbDescriptor *, void *);
static void functionalDescriptor(const void *, struct UsbDescriptor *, void *);
/*----------------------------------------------------------------------------*/
static void onTimerOverflow(void *);
static enum result processDownloadRequest(struct Dfu *, const void *, uint16_t);
static void processGetStatusRequest(struct Dfu *, void *, uint16_t *);
static enum result processUploadRequest(struct Dfu *, uint16_t, void *,
    uint16_t *);
static void resetDriver(struct Dfu *);
static void setStatus(struct Dfu *, enum dfuStatus);
static inline void toLittleEndian24(uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *, const void *);
static void driverDeinit(void *);
static enum result driverConfigure(void *, const struct UsbSetupPacket *,
    const void *, uint16_t, void *, uint16_t *, uint16_t);
static const usbDescriptorFunctor *driverDescribe(const void *);
static void driverEvent(void *, unsigned int);
/*----------------------------------------------------------------------------*/
static const struct UsbDriverClass driverTable = {
    .size = sizeof(struct Dfu),
    .init = driverInit,
    .deinit = driverDeinit,

    .configure = driverConfigure,
    .describe = driverDescribe,
    .event = driverEvent
};
/*----------------------------------------------------------------------------*/
const struct UsbDriverClass * const Dfu = &driverTable;
/*----------------------------------------------------------------------------*/
static const usbDescriptorFunctor deviceDescriptorTable[] = {
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

  timerSetEnabled(driver->timer, false);

  switch ((enum dfuState)driver->state)
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
static enum result processDownloadRequest(struct Dfu *driver,
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
  else if (driver->state == STATE_DFU_IDLE
      || driver->state != STATE_DFU_DNLOAD_IDLE)
  {
    setStatus(driver, DFU_STATUS_ERR_STALLEDPKT);
    usbTrace("dfu at %u: incorrect sequence", driver->interfaceIndex);
    return E_ERROR;
  }

  const size_t written = driver->onDownloadRequest(driver->position,
      payload, payloadLength, &driver->timeout);

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

  switch ((enum dfuState)driver->state)
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
    timerSetEnabled(driver->timer, true);
  }
}
/*----------------------------------------------------------------------------*/
static enum result processUploadRequest(struct Dfu *driver,
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

  const size_t read = driver->onUploadRequest(driver->position, response,
      requestedLength);

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
static void setStatus(struct Dfu *driver, enum dfuStatus status)
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
static enum result driverInit(void *object, const void *configBase)
{
  const struct DfuConfig * const config = configBase;
  assert(config);
  assert(config->device);
  assert(config->transferSize);

  struct Dfu * const driver = object;
  enum result res;

  driver->device = config->device;
  driver->timer = config->timer;
  driver->onDownloadRequest = 0;
  driver->onUploadRequest = 0;
  driver->transferSize = config->transferSize;

  if (driver->timer)
    timerSetCallback(driver->timer, onTimerOverflow, driver);

  resetDriver(driver);

  driver->interfaceIndex = usbDevGetInterface(driver->device);
  if ((res = usbDevBind(driver->device, driver)) != E_OK)
    return res;

  return E_OK;
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
static enum result driverConfigure(void *object,
    const struct UsbSetupPacket *packet, const void *payload,
    uint16_t payloadLength, void *response, uint16_t *responseLength,
    uint16_t maxResponseLength __attribute__((unused)))
{
  struct Dfu * const driver = object;
  const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);

  if (type != REQUEST_TYPE_CLASS)
    return E_INVALID;
  if (packet->index != driver->interfaceIndex)
    return E_VALUE;

  switch (packet->request)
  {
    case DFU_REQUEST_DNLOAD:
      if (packet->length == payloadLength)
        return processDownloadRequest(driver, payload, packet->length);
      else
        return E_VALUE;

    case DFU_REQUEST_UPLOAD:
      return processUploadRequest(driver, packet->length, response,
          responseLength);

    case DFU_REQUEST_GETSTATUS:
      processGetStatusRequest(driver, response, responseLength);
      return E_OK;

    case DFU_REQUEST_CLRSTATUS:
    {
      switch ((enum dfuState)driver->state)
      {
        case STATE_DFU_ERROR:
          driver->status = DFU_STATUS_OK;
          driver->state = STATE_DFU_IDLE;
          usbTrace("dfu at %u: status cleared", driver->interfaceIndex);
          return E_OK;

        default:
          setStatus(driver, DFU_STATUS_ERR_STALLEDPKT);
          usbTrace("dfu at %u: clear failed", driver->interfaceIndex);
          return E_INVALID;
      }
    }

    case DFU_REQUEST_ABORT:
    {
      switch (driver->state)
      {
        case STATE_DFU_IDLE:
        case STATE_DFU_DNLOAD_SYNC:
        case STATE_DFU_DNLOAD_IDLE:
        case STATE_DFU_MANIFEST_SYNC:
        case STATE_DFU_UPLOAD_IDLE:
          driver->state = STATE_DFU_IDLE;
          usbTrace("dfu at %u: aborted", driver->interfaceIndex);
          return E_OK;

        default:
          setStatus(driver, DFU_STATUS_ERR_STALLEDPKT);
          usbTrace("dfu at %u: abort failed", driver->interfaceIndex);
          return E_INVALID;
      }
    }

    case DFU_REQUEST_DETACH:
      setStatus(driver, DFU_STATUS_ERR_STALLEDPKT);
      usbTrace("dfu at %u: detach request", driver->interfaceIndex);
      return E_INVALID;

    case DFU_REQUEST_GETSTATE:
      ((uint8_t *)response)[0] = driver->state;
      *responseLength = 1;
      return E_OK;

    default:
      setStatus(driver, DFU_STATUS_ERR_STALLEDPKT);
      usbTrace("dfu at %u: unknown request 0x%02X",
          driver->interfaceIndex, packet->request);
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static const usbDescriptorFunctor *driverDescribe(const void *object
    __attribute__((unused)))
{
  return deviceDescriptorTable;
}
/*----------------------------------------------------------------------------*/
static void driverEvent(void *object, unsigned int event)
{
  struct Dfu * const driver = object;

  if (event == USB_DEVICE_EVENT_RESET)
    resetDriver(driver);
}
/*----------------------------------------------------------------------------*/
void dfuOnDownloadCompleted(struct Dfu *driver, bool status)
{
  driver->timeout = 0;

  switch ((enum dfuState)driver->state)
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
void dfuSetDownloadRequestCallback(struct Dfu *driver,
    size_t (*callback)(size_t, const void *, size_t, uint16_t *))
{
  driver->onDownloadRequest = callback;
}
/*----------------------------------------------------------------------------*/
void dfuSetUploadRequestCallback(struct Dfu *driver,
    size_t (*callback)(size_t, void *, size_t))
{
  driver->onUploadRequest = callback;
}
