/*
 * usb_device.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <string.h>
#include <xcore/containers/queue.h>
#include <xcore/memory.h>
#include <halm/delay.h>
#include <halm/platform/nxp/lpc13xx/usb_base.h>
#include <halm/platform/nxp/lpc13xx/usb_defs.h>
#include <halm/platform/nxp/usb_device.h>
#include <halm/usb/usb_control.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
/*----------------------------------------------------------------------------*/
struct UsbSieEndpoint
{
  struct UsbEndpoint base;

  /* Parent device */
  struct UsbDevice *device;
  /* Queued requests */
  struct Queue requests;
  /* Logical address */
  uint8_t address;
};

struct UsbDevice
{
  struct UsbBase base;

  /* Array of registered endpoints */
  struct UsbEndpoint *endpoints[10];
  /* Control message handler */
  struct UsbControl *control;

  /* Device is configured */
  bool configured;
  /* Device is enabled */
  bool enabled;
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static void resetDevice(struct UsbDevice *);
static void usbCommand(struct UsbDevice *, uint8_t);
static uint8_t usbCommandRead(struct UsbDevice *, uint8_t);
static void usbCommandWrite(struct UsbDevice *, uint8_t, uint16_t);
static void waitForInt(struct UsbDevice *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum Result devInit(void *, const void *);
static void *devCreateEndpoint(void *, uint8_t);
static uint8_t devGetInterface(const void *);
static void devSetAddress(void *, uint8_t);
static void devSetConnected(void *, bool);
static enum Result devBind(void *, void *);
static void devUnbind(void *, const void *);
static void devSetPower(void *, uint16_t);
static enum UsbSpeed devGetSpeed(const void *);
static enum Result devStringAppend(void *, struct UsbString);
static void devStringErase(void *, struct UsbString);

#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *);
#else
#define devDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static const struct UsbDeviceClass devTable = {
    .size = sizeof(struct UsbDevice),
    .init = devInit,
    .deinit = devDeinit,

    .createEndpoint = devCreateEndpoint,
    .getInterface = devGetInterface,
    .setAddress = devSetAddress,
    .setConnected = devSetConnected,

    .bind = devBind,
    .unbind = devUnbind,

    .setPower = devSetPower,
    .getSpeed = devGetSpeed,

    .stringAppend = devStringAppend,
    .stringErase = devStringErase
};
/*----------------------------------------------------------------------------*/
const struct UsbDeviceClass * const UsbDevice = &devTable;
/*----------------------------------------------------------------------------*/
static void epHandler(struct UsbSieEndpoint *, uint8_t);
static enum Result epReadData(struct UsbSieEndpoint *, uint8_t *,
    size_t, size_t *);
static void epWriteData(struct UsbSieEndpoint *, const uint8_t *, size_t);
/*----------------------------------------------------------------------------*/
static enum Result epInit(void *, const void *);
static void epDeinit(void *);
static void epClear(void *);
static void epDisable(void *);
static void epEnable(void *, uint8_t, uint16_t);
static enum Result epEnqueue(void *, struct UsbRequest *);
static bool epIsStalled(void *);
static void epSetStalled(void *, bool);
/*----------------------------------------------------------------------------*/
static const struct UsbEndpointClass epTable = {
    .size = sizeof(struct UsbSieEndpoint),
    .init = epInit,
    .deinit = epDeinit,

    .clear = epClear,
    .disable = epDisable,
    .enable = epEnable,
    .enqueue = epEnqueue,
    .isStalled = epIsStalled,
    .setStalled = epSetStalled
};
/*----------------------------------------------------------------------------*/
const struct UsbEndpointClass * const UsbSieEndpoint = &epTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct UsbDevice * const device = object;
  LPC_USB_Type * const reg = device->base.reg;
  const uint32_t intStatus = reg->USBDevIntSt;

  reg->USBDevIntClr = intStatus;

  /* Device status interrupt */
  if (intStatus & USBDevInt_DEV_STAT)
  {
    const uint8_t deviceStatus = usbCommandRead(device,
        USB_CMD_GET_DEVICE_STATUS);

    if (deviceStatus & DEVICE_STATUS_RST)
    {
      resetDevice(device);
      usbControlEvent(device->control, USB_DEVICE_EVENT_RESET);
    }

    if (deviceStatus & DEVICE_STATUS_SUS_CH)
    {
      const bool suspended = (deviceStatus & DEVICE_STATUS_SUS) != 0;

      usbControlEvent(device->control, suspended ?
          USB_DEVICE_EVENT_SUSPEND : USB_DEVICE_EVENT_RESUME);
    }
  }

  /* Endpoint interrupt */
  if (intStatus & USBDevInt_EP_MASK)
  {
    struct UsbEndpoint ** const endpointArray = device->endpoints;
    uint32_t epIntStatus = reverseBits32((intStatus >> 1) & 0xFF);

    do
    {
      const unsigned int index = countLeadingZeros32(epIntStatus);
      const uint8_t status = usbCommandRead(device,
          USB_CMD_CLEAR_INTERRUPT | index);

      epIntStatus -= (1UL << 31) >> index;
      epHandler((struct UsbSieEndpoint *)endpointArray[index], status);
    }
    while (epIntStatus);
  }
}
/*----------------------------------------------------------------------------*/
static void resetDevice(struct UsbDevice *device)
{
  LPC_USB_Type * const reg = device->base.reg;

  /* Set inactive configuration */
  device->configured = false;

  /* Configure and clear interrupts */
  reg->USBDevIntEn = USBDevInt_DEV_STAT;
  reg->USBDevIntClr = 0xFFFFFFFF;
}
/*----------------------------------------------------------------------------*/
static void usbCommand(struct UsbDevice *device, uint8_t command)
{
  LPC_USB_Type * const reg = device->base.reg;

  /* Clear command and data interrupt flags */
  reg->USBDevIntClr = USBDevInt_CCEMPTY | USBDevInt_CDFULL;
  while (reg->USBDevIntSt & USBDevInt_CDFULL);

  /* Write command code and wait for completion */
  reg->USBCmdCode = USBCmdCode_CMD_PHASE(USB_CMD_PHASE_COMMAND)
      | USBCmdCode_CMD_CODE(command);
  waitForInt(device, USBDevInt_CCEMPTY);
}
/*----------------------------------------------------------------------------*/
static uint8_t usbCommandRead(struct UsbDevice *device, uint8_t command)
{
  LPC_USB_Type * const reg = device->base.reg;

  /* Write command code */
  usbCommand(device, command);

  /* Send read request and wait for data */
  reg->USBCmdCode = USBCmdCode_CMD_PHASE(USB_CMD_PHASE_READ)
      | USBCmdCode_CMD_CODE(command);
  waitForInt(device, USBDevInt_CDFULL);

  return reg->USBCmdData;
}
/*----------------------------------------------------------------------------*/
static void usbCommandWrite(struct UsbDevice *device, uint8_t command,
    uint16_t data)
{
  LPC_USB_Type * const reg = device->base.reg;

  /* Write command code */
  usbCommand(device, command);

  /* Write data and wait for completion */
  reg->USBCmdCode = USBCmdCode_CMD_PHASE(USB_CMD_PHASE_WRITE)
      | USBCmdCode_CMD_WDATA(data);
  waitForInt(device, USBDevInt_CCEMPTY);
}
/*----------------------------------------------------------------------------*/
static void waitForInt(struct UsbDevice *device, uint32_t mask)
{
  LPC_USB_Type * const reg = device->base.reg;

  /* Wait for specific interrupt */
  while ((reg->USBDevIntSt & mask) != mask);
  /* Clear pending interrupt flags */
  reg->USBDevIntClr = mask;
}
/*----------------------------------------------------------------------------*/
static enum Result devInit(void *object, const void *configBase)
{
  const struct UsbDeviceConfig * const config = configBase;
  assert(config);

  const struct UsbBaseConfig baseConfig = {
      .dm = config->dm,
      .dp = config->dp,
      .connect = config->connect,
      .vbus = config->vbus,
      .channel = config->channel
  };
  const struct UsbControlConfig controlConfig = {
      .parent = object,
      .vid = config->vid,
      .pid = config->pid
  };
  struct UsbDevice * const device = object;
  enum Result res;

  /* Call base class constructor */
  res = UsbBase->init(object, &baseConfig);
  if (res != E_OK)
    return res;

  device->base.handler = interruptHandler;
  device->enabled = false;
  memset(device->endpoints, 0, sizeof(device->endpoints));

  /* Initialize control message handler */
  device->control = init(UsbControl, &controlConfig);
  if (!device->control)
    return E_ERROR;

  /* Configure interrupts and reset system variables */
  resetDevice(device);
  /* By default, only ACKs generate interrupts */
  usbCommandWrite(device, USB_CMD_SET_MODE, 0);

  irqSetPriority(device->base.irq, config->priority);
  irqEnable(device->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *object)
{
  struct UsbDevice * const device = object;

  irqDisable(device->base.irq);

  deinit(device->control);
  UsbBase->deinit(device);
}
#endif
/*----------------------------------------------------------------------------*/
static void *devCreateEndpoint(void *object, uint8_t address)
{
  struct UsbDevice * const device = object;
  const unsigned int index = EP_TO_INDEX(address);

  assert(index < ARRAY_SIZE(device->endpoints));

  struct UsbEndpoint *ep = 0;
  const IrqState state = irqSave();

  if (!device->endpoints[index])
  {
    /* Initialization of endpoint is only available before the driver starts */
    assert(!device->enabled);

    const struct UsbEndpointConfig config = {
        .parent = device,
        .address = address
    };

    device->endpoints[index] = init(UsbSieEndpoint, &config);
  }
  ep = device->endpoints[index];

  irqRestore(state);
  return ep;
}
/*----------------------------------------------------------------------------*/
static uint8_t devGetInterface(const void *object __attribute__((unused)))
{
  return 0;
}
/*----------------------------------------------------------------------------*/
static void devSetAddress(void *object, uint8_t address)
{
  struct UsbDevice * const device = object;

  device->configured = address != 0;

  usbCommandWrite(device, USB_CMD_SET_ADDRESS,
      SET_ADDRESS_DEV_EN | SET_ADDRESS_DEV_ADDR(address));
  usbCommandWrite(device, USB_CMD_CONFIGURE_DEVICE,
      device->configured ? CONFIGURE_DEVICE_CONF_DEVICE : 0);
}
/*----------------------------------------------------------------------------*/
static void devSetConnected(void *object, bool state)
{
  struct UsbDevice * const device = object;

  usbCommandWrite(device, USB_CMD_SET_DEVICE_STATUS,
      state ? DEVICE_STATUS_CON : 0);
  device->enabled = state;
}
/*----------------------------------------------------------------------------*/
static enum Result devBind(void *object, void *driver)
{
  struct UsbDevice * const device = object;
  return usbControlBindDriver(device->control, driver);
}
/*----------------------------------------------------------------------------*/
static void devUnbind(void *object, const void *driver __attribute__((unused)))
{
  struct UsbDevice * const device = object;
  usbControlUnbindDriver(device->control);
}
/*----------------------------------------------------------------------------*/
static void devSetPower(void *object, uint16_t current)
{
  struct UsbDevice * const device = object;
  usbControlSetPower(device->control, current);
}
/*----------------------------------------------------------------------------*/
static enum UsbSpeed devGetSpeed(const void *object __attribute__((unused)))
{
  return USB_FS;
}
/*----------------------------------------------------------------------------*/
static enum Result devStringAppend(void *object, struct UsbString string)
{
  struct UsbDevice * const device = object;
  return usbControlStringAppend(device->control, string);
}
/*----------------------------------------------------------------------------*/
static void devStringErase(void *object, struct UsbString string)
{
  struct UsbDevice * const device = object;
  usbControlStringErase(device->control, string);
}
/*----------------------------------------------------------------------------*/
static void epHandler(struct UsbSieEndpoint *ep, uint8_t status)
{
  if (queueEmpty(&ep->requests))
    return;

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    const unsigned int index = EP_TO_INDEX(ep->address);

    while (!queueEmpty(&ep->requests))
    {
      const uint8_t epCode = USB_CMD_SELECT_ENDPOINT | index;
      const uint8_t epStatus = usbCommandRead(ep->device, epCode);

      if (!(epStatus & (SELECT_ENDPOINT_FE | SELECT_ENDPOINT_ST)))
      {
        struct UsbRequest *request;

        queuePop(&ep->requests, &request);
        epWriteData(ep, request->buffer, request->length);
        request->callback(request->callbackArgument, request,
            USB_REQUEST_COMPLETED);
      }
      else
        break;
    }
  }
  else
  {
    struct UsbRequest *request;
    size_t read;

    queuePeek(&ep->requests, &request);

    if (epReadData(ep, request->buffer, request->capacity, &read) == E_OK)
    {
      queuePop(&ep->requests, 0);

      const enum UsbRequestStatus requestStatus = status & SELECT_ENDPOINT_STP ?
          USB_REQUEST_SETUP : USB_REQUEST_COMPLETED;

      request->length = read;
      request->callback(request->callbackArgument, request, requestStatus);
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum Result epReadData(struct UsbSieEndpoint *ep, uint8_t *buffer,
    size_t length, size_t *read)
{
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int index = EP_TO_INDEX(ep->address);

  /* Set read enable bit for specific endpoint */
  reg->USBCtrl =
      USBCtrl_RD_EN | USBCtrl_LOG_ENDPOINT(USB_EP_LOGICAL_ADDRESS(ep->address));

  /*
   * User Manual says that it takes 3 clock cycle to fetch the packet length
   * from the RAM. It seems that this delay should be a little bit higher.
   */
  delayTicks(4);

  size_t packetLength = reg->USBRxPLen;

  /* Check packet validity */
  if (!(packetLength & USBRxPLen_DV))
    return E_INTERFACE;
  /* Extract length */
  packetLength = USBRxPLen_PKT_LNGTH_VALUE(packetLength);
  /* Check for buffer overflow */
  if (packetLength > length)
    return E_VALUE;

  *read = packetLength;

  /* Read data from internal buffer */
  uint32_t word = 0;

  for (size_t position = 0; position < packetLength; ++position)
  {
    if (!(position & 0x03))
      word = reg->USBRxData;

    buffer[position] = (uint8_t)word;
    word >>= 8;
  }

  /* Clear read enable bit */
  reg->USBCtrl = 0;

  /* Select endpoint and clear buffer */
  usbCommand(ep->device, USB_CMD_SELECT_ENDPOINT | index);
  usbCommand(ep->device, USB_CMD_CLEAR_BUFFER);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void epWriteData(struct UsbSieEndpoint *ep, const uint8_t *buffer,
    size_t length)
{
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int index = EP_TO_INDEX(ep->address);

  /* Set write enable for specific endpoint */
  reg->USBCtrl =
      USBCtrl_WR_EN | USBCtrl_LOG_ENDPOINT(USB_EP_LOGICAL_ADDRESS(ep->address));
  /* Set packet length */
  reg->USBTxPLen = length;

  if (length == 0)
  {
    /* To send an empty packet a single write operation has to be performed */
    reg->USBTxData = 0;
  }
  else
  {
    /* Write data */
    size_t position = 0;
    uint32_t word = 0;

    while (position < length)
    {
      word |= buffer[position] << ((position & 0x03) << 3);
      ++position;

      if (!(position & 0x03) || position == length)
      {
        reg->USBTxData = word;
        word = 0;
      }
    }
  }

  /* Clear write enable bit */
  reg->USBCtrl = 0;

  /* Select endpoint and validate buffer */
  usbCommand(ep->device, USB_CMD_SELECT_ENDPOINT | index);
  usbCommand(ep->device, USB_CMD_VALIDATE_BUFFER);
}
/*----------------------------------------------------------------------------*/
static enum Result epInit(void *object, const void *configBase)
{
  const struct UsbEndpointConfig * const config = configBase;
  struct UsbDevice * const device = config->parent;
  struct UsbSieEndpoint * const ep = object;

  const enum Result res = queueInit(&ep->requests,
      sizeof(struct UsbRequest *), CONFIG_PLATFORM_USB_DEVICE_EP_REQUESTS);

  if (res == E_OK)
  {
    ep->address = config->address;
    ep->device = device;
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static void epDeinit(void *object)
{
  struct UsbSieEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;

  /* Disable interrupts and remove pending requests */
  epDisable(ep);
  epClear(ep);

  const unsigned int index = EP_TO_INDEX(ep->address);

  const IrqState state = irqSave();
  device->endpoints[index] = 0;
  irqRestore(state);

  assert(queueEmpty(&ep->requests));
  queueDeinit(&ep->requests);
}
/*----------------------------------------------------------------------------*/
static void epClear(void *object)
{
  struct UsbSieEndpoint * const ep = object;

  while (!queueEmpty(&ep->requests))
  {
    struct UsbRequest *request;

    queuePop(&ep->requests, &request);
    request->callback(request->callbackArgument, request,
        USB_REQUEST_CANCELLED);
  }
}
/*----------------------------------------------------------------------------*/
static void epDisable(void *object)
{
  struct UsbSieEndpoint * const ep = object;
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int index = EP_TO_INDEX(ep->address);
  const uint32_t mask = 1UL << (index + 1);

  reg->USBDevIntEn &= ~mask;

  usbCommandWrite(ep->device, USB_CMD_SET_ENDPOINT_STATUS | index,
      SET_ENDPOINT_STATUS_DA);
}
/*----------------------------------------------------------------------------*/
static void epEnable(void *object, uint8_t type __attribute__((unused)),
    uint16_t size __attribute__((unused)))
{
  struct UsbSieEndpoint * const ep = object;
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int index = EP_TO_INDEX(ep->address);
  const uint32_t mask = 1UL << (index + 1);

  // TODO Check whether SIE clear is needed
  reg->USBDevIntClr = mask;
  reg->USBDevIntEn |= mask;

  usbCommandWrite(ep->device, USB_CMD_SET_ENDPOINT_STATUS | index, 0);
}
/*----------------------------------------------------------------------------*/
static enum Result epEnqueue(void *object, struct UsbRequest *request)
{
  assert(request->callback);

  struct UsbSieEndpoint * const ep = object;
  const unsigned int index = EP_TO_INDEX(ep->address);

  /*
   * Additional checks should be performed for data endpoints
   * to avoid USB controller hanging.
   */
  if (index >= 2 && !ep->device->configured)
    return E_IDLE;

  irqDisable(ep->device->base.irq);
  assert(!queueFull(&ep->requests));

  const uint8_t epCode = USB_CMD_SELECT_ENDPOINT | index;
  const uint8_t epStatus = usbCommandRead(ep->device, epCode);
  bool invokeHandler = false;

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    static const uint8_t mask = SELECT_ENDPOINT_ST
        | SELECT_ENDPOINT_B1FULL | SELECT_ENDPOINT_B2FULL;

    invokeHandler = !(epStatus & mask) && !queueSize(&ep->requests);
  }
  else if (epStatus & SELECT_ENDPOINT_FE)
  {
    invokeHandler = true;
  }

  queuePush(&ep->requests, &request);

  if (invokeHandler)
  {
    LPC_USB_Type * const reg = ep->device->base.reg;
    const uint32_t mask = 1UL << (index + 1);

    /* Schedule interrupt */
    reg->USBDevIntSet = mask;
  }

  irqEnable(ep->device->base.irq);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static bool epIsStalled(void *object)
{
  struct UsbSieEndpoint * const ep = object;
  const unsigned int index = EP_TO_INDEX(ep->address);
  const uint8_t status = usbCommandRead(ep->device,
      USB_CMD_SELECT_ENDPOINT | index);

  return (status & SELECT_ENDPOINT_ST) != 0;
}
/*----------------------------------------------------------------------------*/
static void epSetStalled(void *object, bool stalled)
{
  struct UsbSieEndpoint * const ep = object;
  const unsigned int index = EP_TO_INDEX(ep->address);

  usbCommandWrite(ep->device, USB_CMD_SET_ENDPOINT_STATUS | index,
      stalled ? SET_ENDPOINT_STATUS_ST : 0);

  /* Write pending IN request to the endpoint buffer */
  if (!stalled && (ep->address & USB_EP_DIRECTION_IN)
      && !queueEmpty(&ep->requests))
  {
    struct UsbRequest *request;

    queuePop(&ep->requests, &request);
    epWriteData(ep, request->buffer, request->length);
    request->callback(request->callbackArgument, request,
        USB_REQUEST_COMPLETED);
  }
}
