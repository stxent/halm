/*
 * usb_device.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/delay.h>
#include <halm/generic/pointer_queue.h>
#include <halm/platform/lpc/lpc13xx/usb_base.h>
#include <halm/platform/lpc/lpc13xx/usb_defs.h>
#include <halm/platform/lpc/usb_device.h>
#include <halm/usb/usb_control.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
#include <xcore/accel.h>
#include <assert.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
struct UsbEndpoint
{
  struct UsbEndpointBase base;

  /* Parent device */
  struct UsbDevice *device;
  /* Queued requests */
  PointerQueue requests;
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
static void usbRunCommand(LPC_USB_Type *, enum UsbCommandPhase, uint32_t,
    uint32_t);
/*----------------------------------------------------------------------------*/
static enum Result devInit(void *, const void *);
static void *devCreateEndpoint(void *, uint8_t);
static uint8_t devGetInterface(const void *);
static void devSetAddress(void *, uint8_t);
static void devSetConnected(void *, bool);
static enum Result devBind(void *, void *);
static void devUnbind(void *, const void *);
static enum UsbSpeed devGetSpeed(const void *);
static void devSetPower(void *, uint16_t);
static UsbStringIndex devStringAppend(void *, struct UsbString);
static void devStringErase(void *, struct UsbString);

#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *);
#else
#  define devDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct UsbDeviceClass * const UsbDevice =
    &(const struct UsbDeviceClass){
    .size = sizeof(struct UsbDevice),
    .init = devInit,
    .deinit = devDeinit,

    .createEndpoint = devCreateEndpoint,
    .getInterface = devGetInterface,
    .setAddress = devSetAddress,
    .setConnected = devSetConnected,

    .bind = devBind,
    .unbind = devUnbind,

    .getSpeed = devGetSpeed,
    .setPower = devSetPower,

    .stringAppend = devStringAppend,
    .stringErase = devStringErase
};
/*----------------------------------------------------------------------------*/
static void epHandler(struct UsbEndpoint *, uint8_t);
static bool epReadData(struct UsbEndpoint *, uint8_t *, size_t, size_t *);
static void epReadPacketMemory(LPC_USB_Type *, uint8_t *, size_t);
static void epWriteData(struct UsbEndpoint *, const uint8_t *, size_t);
static void epWritePacketMemory(LPC_USB_Type *, const uint8_t *, size_t);
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
static const struct UsbEndpointClass * const UsbEndpoint =
    &(const struct UsbEndpointClass){
    .size = sizeof(struct UsbEndpoint),
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
static void interruptHandler(void *object)
{
  struct UsbDevice * const device = object;
  LPC_USB_Type * const reg = device->base.reg;
  const uint32_t intStatus = reg->USBDevIntSt;

  reg->USBDevIntClr = intStatus;

  /* Device status interrupt */
  if (intStatus & USBDevInt_DEV_STAT)
  {
    const uint8_t devStatus = usbCommandRead(device,
        USB_CMD_GET_DEVICE_STATUS);

    if (devStatus & DEVICE_STATUS_RST)
    {
      resetDevice(device);
      usbControlNotify(device->control, USB_DEVICE_EVENT_RESET);
    }

    if (devStatus & DEVICE_STATUS_SUS_CH)
    {
      usbControlNotify(device->control, (devStatus & DEVICE_STATUS_SUS) ?
          USB_DEVICE_EVENT_SUSPEND : USB_DEVICE_EVENT_RESUME);
    }
  }

  /* Endpoint interrupt */
  if (intStatus & USBDevInt_EP_MASK)
  {
    struct UsbEndpoint ** const epArray = device->endpoints;
    uint32_t epIntStatus = reverseBits32((intStatus >> 1) & 0xFF);

    do
    {
      const unsigned int index = countLeadingZeros32(epIntStatus);
      const uint8_t status = usbCommandRead(device,
          USB_CMD_CLEAR_INTERRUPT | index);

      epIntStatus -= (1UL << 31) >> index;
      epHandler((struct UsbEndpoint *)epArray[index], status);
    }
    while (epIntStatus);
  }
}
/*----------------------------------------------------------------------------*/
static void resetDevice(struct UsbDevice *device)
{
  LPC_USB_Type * const reg = device->base.reg;

  /* Configure and clear interrupts */
  reg->USBDevIntClr = 0xFFFFFFFFUL;
  reg->USBDevIntEn = USBDevInt_DEV_STAT;

  devSetAddress(device, 0);

  /* Reset all enabled endpoints except for Control Endpoints */
  for (size_t index = 2; index < ARRAY_SIZE(device->endpoints); ++index)
    device->endpoints[index] = NULL;
}
/*----------------------------------------------------------------------------*/
static void usbCommand(struct UsbDevice *device, uint8_t command)
{
  LPC_USB_Type * const reg = device->base.reg;

  reg->USBDevIntClr = USBDevInt_CCEMPTY;
  usbRunCommand(reg, USB_CMD_PHASE_COMMAND, command, USBDevInt_CCEMPTY);
}
/*----------------------------------------------------------------------------*/
static uint8_t usbCommandRead(struct UsbDevice *device, uint8_t command)
{
  LPC_USB_Type * const reg = device->base.reg;

  reg->USBDevIntClr = USBDevInt_CCEMPTY;
  usbRunCommand(reg, USB_CMD_PHASE_COMMAND, command, USBDevInt_CCEMPTY);

  reg->USBDevIntClr = USBDevInt_CCEMPTY | USBDevInt_CDFULL;
  usbRunCommand(reg, USB_CMD_PHASE_READ, command, USBDevInt_CDFULL);

  return reg->USBCmdData;
}
/*----------------------------------------------------------------------------*/
static void usbCommandWrite(struct UsbDevice *device, uint8_t command,
    uint16_t data)
{
  LPC_USB_Type * const reg = device->base.reg;

  reg->USBDevIntClr = USBDevInt_CCEMPTY;
  usbRunCommand(reg, USB_CMD_PHASE_COMMAND, command, USBDevInt_CCEMPTY);

  reg->USBDevIntClr = USBDevInt_CCEMPTY;
  usbRunCommand(reg, USB_CMD_PHASE_WRITE, data, USBDevInt_CCEMPTY);
}
/*----------------------------------------------------------------------------*/
static void usbRunCommand(LPC_USB_Type *reg, enum UsbCommandPhase phase,
    uint32_t payload, uint32_t mask)
{
  reg->USBCmdCode = USBCmdCode_CMD_PHASE(phase) | USBCmdCode_CMD_CODE(payload);
  while (!(reg->USBDevIntSt & (mask | USBDevInt_DEV_STAT)));
}
/*----------------------------------------------------------------------------*/
static enum Result devInit(void *object, const void *configBase)
{
  const struct UsbDeviceConfig * const config = configBase;
  assert(config != NULL);

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

  /* Call base class constructor */
  const enum Result res = UsbBase->init(device, &baseConfig);
  if (res != E_OK)
    return res;

  device->base.handler = interruptHandler;
  device->enabled = false;

  for (size_t index = 0; index < ARRAY_SIZE(device->endpoints); ++index)
    device->endpoints[index] = NULL;

  /* Initialize control message handler after endpoint initialization */
  device->control = init(UsbControl, &controlConfig);
  if (device->control == NULL)
    return E_ERROR;

  /* Reset device address */
  devSetAddress(device, 0);
  /* By default, only ACKs generate interrupts */
  usbCommandWrite(device, USB_CMD_SET_MODE, 0);

  LPC_USB_Type * const reg = device->base.reg;

  /* Configure interrupts in USB core */
  reg->USBDevIntEn = 0;
  /* Configure interrupts in NVIC */
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
  const unsigned int index = EP_TO_INDEX(address);
  struct UsbDevice * const device = object;

  assert(index < ARRAY_SIZE(device->endpoints));

  const struct UsbEndpointConfig config = {
      .parent = device,
      .address = address
  };
  struct UsbEndpoint * const ep = init(UsbEndpoint, &config);

  if (index < 2)
  {
    /* Set Control Endpoints immediately after creation */
    assert(device->endpoints[index] == NULL);
    device->endpoints[index] = ep;
  }

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
  LPC_USB_Type * const reg = device->base.reg;

  device->enabled = state;

  if (device->enabled)
  {
    reg->USBDevIntClr = USBDevInt_DEV_STAT;
    reg->USBDevIntEn = USBDevInt_DEV_STAT;

    usbCommandWrite(device, USB_CMD_SET_DEVICE_STATUS, DEVICE_STATUS_CON);
  }
  else
  {
    reg->USBDevIntEn = 0;
    usbCommandWrite(device, USB_CMD_SET_DEVICE_STATUS, 0);
  }
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
static enum UsbSpeed devGetSpeed(const void *object __attribute__((unused)))
{
  return USB_FS;
}
/*----------------------------------------------------------------------------*/
static void devSetPower(void *object, uint16_t current)
{
  struct UsbDevice * const device = object;
  usbControlSetPower(device->control, current);
}
/*----------------------------------------------------------------------------*/
static UsbStringIndex devStringAppend(void *object, struct UsbString string)
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
static void epHandler(struct UsbEndpoint *ep, uint8_t status)
{
  /* Double-buffering availability mask */
  static const uint32_t dbAvailable = 0x000003C0UL;

  if (pointerQueueEmpty(&ep->requests))
    return;

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    if (status & SELECT_ENDPOINT_ST)
    {
      /* The endpoint is stalled */
      return;
    }

    const unsigned int index = EP_TO_INDEX(ep->address);
    const size_t pending = pointerQueueSize(&ep->requests);
    size_t count = 0;

    if (!(status & SELECT_ENDPOINT_B1FULL))
      ++count;
    if ((dbAvailable & (1UL << index)) && !(status & SELECT_ENDPOINT_B2FULL))
      ++count;
    count = MIN(count, pending);

    while (count--)
    {
      struct UsbRequest * const request = pointerQueueFront(&ep->requests);
      pointerQueuePopFront(&ep->requests);

      epWriteData(ep, request->buffer, request->length);
      request->callback(request->argument, request, USB_REQUEST_COMPLETED);
    }
  }
  else
  {
    struct UsbRequest * const request = pointerQueueFront(&ep->requests);
    size_t read;

    if (epReadData(ep, request->buffer, request->capacity, &read))
    {
      const enum UsbRequestStatus requestStatus = status & SELECT_ENDPOINT_STP ?
          USB_REQUEST_SETUP : USB_REQUEST_COMPLETED;

      pointerQueuePopFront(&ep->requests);
      request->length = read;
      request->callback(request->argument, request, requestStatus);
    }
  }
}
/*----------------------------------------------------------------------------*/
static bool epReadData(struct UsbEndpoint *ep, uint8_t *buffer,
    size_t length, size_t *read)
{
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);
  size_t packetLength;

  /* Enable data read mode for the endpoint */
  reg->USBCtrl = USBCtrl_RD_EN | USBCtrl_LOG_ENDPOINT(number);

  /*
   * User Manual says that it takes 3 clock cycle to fetch the packet length
   * from the RAM after programming the control register.
   */
  __dsb();
  packetLength = reg->USBRxPLen;

  /* Check packet validity */
  if (!(packetLength & USBRxPLen_DV))
    return false;
  /* Extract length */
  packetLength = USBRxPLen_PKT_LNGTH_VALUE(packetLength);
  /* Check for buffer overflow */
  if (packetLength > length)
    return false;

  /* Read data from the internal packet memory */
  *read = packetLength;
  epReadPacketMemory(reg, buffer, packetLength);

  /* Clear endpoint number and read enable bit */
  reg->USBCtrl = 0;

  /* Select endpoint and clear buffer */
  usbCommand(ep->device, USB_CMD_SELECT_ENDPOINT | EP_TO_INDEX(ep->address));
  usbCommand(ep->device, USB_CMD_CLEAR_BUFFER);

  return true;
}
/*----------------------------------------------------------------------------*/
static void epReadPacketMemory(LPC_USB_Type *reg, uint8_t *buffer,
    size_t length)
{
  uint32_t *start = (uint32_t *)buffer;
  uint32_t * const end = start + (length >> 2);

  buffer = (uint8_t *)end;
  length &= 3;

  while (start < end)
    *start++ = reg->USBRxData;

  if (length)
  {
    uint32_t lastWord = reg->USBRxData;

    switch (length)
    {
      case 3:
        *buffer++ = (uint8_t)lastWord;
        lastWord >>= 8;
        /* Falls through */
      case 2:
        *buffer++ = (uint8_t)lastWord;
        lastWord >>= 8;
        /* Falls through */
      case 1:
        *buffer = (uint8_t)lastWord;
        break;
    }
  }
}
/*----------------------------------------------------------------------------*/
static void epWriteData(struct UsbEndpoint *ep, const uint8_t *buffer,
    size_t length)
{
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  /* Enable data write mode for the endpoint */
  reg->USBCtrl = USBCtrl_WR_EN | USBCtrl_LOG_ENDPOINT(number);
  /* Set packet length */
  reg->USBTxPLen = length;

  /* Write data into the internal packet memory */
  epWritePacketMemory(reg, buffer, length);

  /* Wait until the write enable bit becomes cleared */
  while (reg->USBCtrl & USBCtrl_WR_EN);
  /* Clear the endpoint number */
  reg->USBCtrl = 0;

  /* Select endpoint and validate buffer */
  usbCommand(ep->device, USB_CMD_SELECT_ENDPOINT | EP_TO_INDEX(ep->address));
  usbCommand(ep->device, USB_CMD_VALIDATE_BUFFER);
}
/*----------------------------------------------------------------------------*/
static void epWritePacketMemory(LPC_USB_Type *reg, const uint8_t *buffer,
    size_t length)
{
  if (length)
  {
    const uint32_t *start = (const uint32_t *)buffer;
    const uint32_t * const end = start + (length >> 2);

    buffer = (const uint8_t *)end;
    length &= 3;

    while (start < end)
    {
      reg->USBTxData = *start++;
    }

    if (length)
    {
      uint32_t lastWord = 0;

      switch (length)
      {
        case 3:
          lastWord = buffer[2] << 16;
          /* Falls through */
        case 2:
          lastWord |= buffer[1] << 8;
          /* Falls through */
        case 1:
          lastWord |= buffer[0];
          break;
      }

      reg->USBTxData = lastWord;
    }
  }
  else
  {
    reg->USBTxData = 0;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result epInit(void *object, const void *configBase)
{
  const struct UsbEndpointConfig * const config = configBase;
  struct UsbEndpoint * const ep = object;
  size_t size;

  if (USB_EP_LOGICAL_ADDRESS(ep->address) == 0)
    size = CONFIG_USB_DEVICE_CONTROL_REQUESTS;
  else
    size = CONFIG_PLATFORM_USB_DEVICE_EP_REQUESTS;

  if (pointerQueueInit(&ep->requests, size))
  {
    ep->address = config->address;
    ep->device = config->parent;

    return E_OK;
  }
  else
    return E_MEMORY;
}
/*----------------------------------------------------------------------------*/
static void epDeinit(void *object)
{
  struct UsbEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;
  const unsigned int index = EP_TO_INDEX(ep->address);

  /* Disable interrupts and remove pending requests */
  epDisable(ep);
  epClear(ep);

  if (index < 2)
  {
    assert(device->endpoints[index] == ep);
    device->endpoints[index] = NULL;
  }

  assert(pointerQueueEmpty(&ep->requests));
  pointerQueueDeinit(&ep->requests);
}
/*----------------------------------------------------------------------------*/
static void epClear(void *object)
{
  struct UsbEndpoint * const ep = object;

  while (!pointerQueueEmpty(&ep->requests))
  {
    struct UsbRequest * const request = pointerQueueFront(&ep->requests);
    pointerQueuePopFront(&ep->requests);

    request->callback(request->argument, request, USB_REQUEST_CANCELLED);
  }
}
/*----------------------------------------------------------------------------*/
static void epDisable(void *object)
{
  struct UsbEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;
  LPC_USB_Type * const reg = device->base.reg;
  const unsigned int index = EP_TO_INDEX(ep->address);
  const uint32_t mask = 1UL << (index + 1);

  reg->USBDevIntEn &= ~mask;

  usbCommandWrite(device, USB_CMD_SET_ENDPOINT_STATUS | index,
      SET_ENDPOINT_STATUS_DA);

  if (index >= 2 && device->endpoints[index] == ep)
    device->endpoints[index] = NULL;
}
/*----------------------------------------------------------------------------*/
static void epEnable(void *object, uint8_t type __attribute__((unused)),
    uint16_t size __attribute__((unused)))
{
  struct UsbEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;
  const unsigned int index = EP_TO_INDEX(ep->address);

  if (index >= 2)
  {
    assert(device->endpoints[index] == NULL);
    device->endpoints[index] = ep;
  }

  LPC_USB_Type * const reg = device->base.reg;
  const uint32_t mask = 1UL << (index + 1);

  // TODO Check whether SIE clear is needed
  reg->USBDevIntClr = mask;
  reg->USBDevIntEn |= mask;

  usbCommandWrite(device, USB_CMD_SET_ENDPOINT_STATUS | index, 0);
}
/*----------------------------------------------------------------------------*/
static enum Result epEnqueue(void *object, struct UsbRequest *request)
{
  assert(request != NULL);
  assert(request->callback != NULL);

  struct UsbEndpoint * const ep = object;
  const unsigned int index = EP_TO_INDEX(ep->address);

  /*
   * Additional checks should be performed for data endpoints
   * to avoid USB controller hanging.
   */
  if (index >= 2 && !ep->device->configured)
    return E_IDLE;

  const IrqState state = irqSave();
  enum Result res = E_FULL;

  if (!pointerQueueFull(&ep->requests))
  {
    const uint8_t epCode = USB_CMD_SELECT_ENDPOINT | index;
    const uint8_t epStatus = usbCommandRead(ep->device, epCode);
    bool invokeHandler = false;

    if (ep->address & USB_EP_DIRECTION_IN)
    {
      static const uint8_t mask = SELECT_ENDPOINT_ST
          | SELECT_ENDPOINT_B1FULL | SELECT_ENDPOINT_B2FULL;

      invokeHandler = !(epStatus & mask) && pointerQueueEmpty(&ep->requests);
    }
    else if (epStatus & SELECT_ENDPOINT_FE)
    {
      invokeHandler = true;
    }

    pointerQueuePushBack(&ep->requests, request);

    if (invokeHandler)
    {
      LPC_USB_Type * const reg = ep->device->base.reg;
      const uint32_t mask = 1UL << (index + 1);

      /* Schedule interrupt */
      reg->USBDevIntSet = mask;
    }

    res = E_OK;
  }

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
static bool epIsStalled(void *object)
{
  struct UsbEndpoint * const ep = object;
  const unsigned int index = EP_TO_INDEX(ep->address);
  const uint8_t status = usbCommandRead(ep->device,
      USB_CMD_SELECT_ENDPOINT | index);

  return (status & SELECT_ENDPOINT_ST) != 0;
}
/*----------------------------------------------------------------------------*/
static void epSetStalled(void *object, bool stalled)
{
  struct UsbEndpoint * const ep = object;
  const unsigned int index = EP_TO_INDEX(ep->address);

  usbCommandWrite(ep->device, USB_CMD_SET_ENDPOINT_STATUS | index,
      stalled ? SET_ENDPOINT_STATUS_ST : 0);

  /* Write pending IN request to the endpoint buffer */
  if (!stalled && (ep->address & USB_EP_DIRECTION_IN)
      && !pointerQueueEmpty(&ep->requests))
  {
    struct UsbRequest * const request = pointerQueueFront(&ep->requests);
    pointerQueuePopFront(&ep->requests);

    epWriteData(ep, request->buffer, request->length);
    request->callback(request->argument, request, USB_REQUEST_COMPLETED);
  }
}
