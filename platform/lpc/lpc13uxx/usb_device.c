/*
 * usb_device.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/delay.h>
#include <halm/generic/pointer_queue.h>
#include <halm/platform/lpc/lpc13uxx/usb_base.h>
#include <halm/platform/lpc/lpc13uxx/usb_defs.h>
#include <halm/platform/lpc/usb_device.h>
#include <halm/usb/usb_control.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
#include <xcore/accel.h>
#include <assert.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
struct SbUsbEndpoint
{
  struct UsbEndpointBase base;

  /* Parent device */
  struct UsbDevice *device;
  /* Queued requests */
  PointerQueue requests;
  /* Address inside the packet buffer memory */
  uint16_t position;
  /* Max request size */
  uint16_t size;
  /* Logical address */
  uint8_t address;
};

struct UsbDevice
{
  struct UsbBase base;

  /* Array of registered endpoints */
  struct UsbEndpoint *endpoints[USB_EP_NUMBER];
  /* Control message handler */
  struct UsbControl *control;

  /* The last allocated address inside the packet buffer memory */
  uint16_t position;
  /* Device is configured */
  bool configured;
  /* Device is enabled */
  bool enabled;


  uint8_t scheduledAddress;
};
/*----------------------------------------------------------------------------*/
static void applyAddress(struct UsbDevice *, uint8_t);
static void interruptHandler(void *);
static void resetDevice(struct UsbDevice *);
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
static UsbStringIndex devStringAppend(void *, struct UsbString);
static void devStringErase(void *, struct UsbString);

#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *);
#else
#define devDeinit deletedDestructorTrap
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

    .setPower = devSetPower,
    .getSpeed = devGetSpeed,

    .stringAppend = devStringAppend,
    .stringErase = devStringErase
};
/*----------------------------------------------------------------------------*/
static void epHandler(struct SbUsbEndpoint *, bool);
static void epPrimeOut(struct SbUsbEndpoint *);
static bool epReadData(struct SbUsbEndpoint *, struct UsbRequest *);
static bool epReadSetupPacket(struct SbUsbEndpoint *, struct UsbRequest *);
static bool epWriteDataAndPrime(struct SbUsbEndpoint *,
    const struct UsbRequest *);
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
static const struct UsbEndpointClass * const SbUsbEndpoint =
    &(const struct UsbEndpointClass){
    .size = sizeof(struct SbUsbEndpoint),
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
static void applyAddress(struct UsbDevice *device, uint8_t address)
{
  LPC_USB_Type * const reg = device->base.reg;

  reg->DEVCMDSTAT = (reg->DEVCMDSTAT & ~DEVCMDSTAT_DEV_ADDR_MASK)
      | DEVCMDSTAT_DEV_ADDR(address);
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct UsbDevice * const device = object;
  LPC_USB_Type * const reg = device->base.reg;
  const uint32_t devStatus = reg->DEVCMDSTAT;
  const uint32_t intStatus = reg->INTSTAT;

  reg->INTSTAT = intStatus;

  /* Device interrupts */
  if (intStatus & INTSTAT_DEV_INT)
  {
    /* Device reset */
    if (devStatus & DEVCMDSTAT_DRES_C)
    {
      reg->DEVCMDSTAT |= DEVCMDSTAT_DRES_C;

      resetDevice(device);
      usbControlNotify(device->control, USB_DEVICE_EVENT_RESET);
    }

    /* Connect Change event on VBus disappearance */
    if (devStatus & DEVCMDSTAT_DCON_C)
    {
      reg->DEVCMDSTAT |= DEVCMDSTAT_DCON_C;
    }

    /*
     * Suspend Change event:
     *   - The device goes in the suspended state.
     *   - The device is disconnected.
     *   - The device receives resume signaling on its upstream port.
     */
    if (devStatus & DEVCMDSTAT_DSUS_C)
    {
      reg->DEVCMDSTAT |= DEVCMDSTAT_DSUS_C;

      usbControlNotify(device->control, ((devStatus & DEVCMDSTAT_DSUS) ?
          USB_DEVICE_EVENT_SUSPEND : USB_DEVICE_EVENT_RESUME));
    }
  }

  uint32_t epStatus = intStatus & INTSTAT_EP_INT_MASK;

  /* EP interrupts */
  if (epStatus)
  {
    epStatus = reverseBits32(epStatus);

    struct UsbEndpoint ** const epArray = device->endpoints;
    const bool setup = (devStatus & DEVCMDSTAT_SETUP) != 0;

    do
    {
      const unsigned int index = countLeadingZeros32(epStatus);
      const bool controlEpSetup = (index == 0) && setup;

      epStatus -= (1UL << 31) >> index;
      epHandler((struct SbUsbEndpoint *)epArray[index], controlEpSetup);

      if (controlEpSetup)
        reg->DEVCMDSTAT |= DEVCMDSTAT_SETUP;
    }
    while (epStatus);
  }
}
/*----------------------------------------------------------------------------*/
static void resetDevice(struct UsbDevice *device)
{
  LPC_USB_Type * const reg = device->base.reg;

  device->position = (uint32_t)device->base.endpointList
      + epcsAlignSize(USB_EP_LIST_SIZE);
  memset(device->base.endpointList, 0, USB_EP_LIST_SIZE);

  reg->EPINUSE = 0;
  reg->EPSKIP = EPSKIP_MASK;
  reg->EPBUFCFG = 0;

  reg->DEVCMDSTAT &= ~DEVCMDSTAT_DEV_ADDR_MASK;
  reg->DEVCMDSTAT |= DEVCMDSTAT_DEV_EN;
  /* Clear all interrupts */
  reg->INTSTAT = INTSTAT_MASK;
  /* Enable device interrupt only */
  reg->INTEN = INTEN_DEV_INT_EN;

  devSetAddress(device, 0);
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

  /* Call base class constructor */
  const enum Result res = UsbBase->init(device, &baseConfig);
  if (res != E_OK)
    return res;

  device->base.handler = interruptHandler;
  device->enabled = false;
  device->position = 0; /* Will be initialized during device reset */
  memset(device->endpoints, 0, sizeof(device->endpoints));

  /* Initialize control message handler after endpoint initialization */
  device->control = init(UsbControl, &controlConfig);
  if (!device->control)
    return E_ERROR;

  /* Configure interrupts and reset system variables */
  resetDevice(device);

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

    device->endpoints[index] = init(SbUsbEndpoint, &config);
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
  device->scheduledAddress = address;

  if (address != 0)
    applyAddress(device, 0);
}
/*----------------------------------------------------------------------------*/
static void devSetConnected(void *object, bool state)
{
  struct UsbDevice * const device = object;
  LPC_USB_Type * const reg = device->base.reg;

  device->enabled = state;

  if (state)
    reg->DEVCMDSTAT |= DEVCMDSTAT_DCON;
  else
    reg->DEVCMDSTAT &= ~DEVCMDSTAT_DCON;
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
static void epHandler(struct SbUsbEndpoint *ep, bool setup)
{
  if (pointerQueueEmpty(&ep->requests))
    return;

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    if (!USB_EP_LOGICAL_ADDRESS(ep->address))
    {
      struct UsbDevice * const device = ep->device;

      if (device->scheduledAddress != 0)
      {
        /*
         * Set a previously saved device address after the status stage
         * of the control transaction.
         */
        applyAddress(device, device->scheduledAddress);
        device->scheduledAddress = 0;
      }
    }

    if (!epIsStalled(ep))
    {
      struct UsbRequest * const request = pointerQueueFront(&ep->requests);
      const bool ok = epWriteDataAndPrime(ep, request);

      pointerQueuePopFront(&ep->requests);
      request->callback(request->argument, request,
          ok ? USB_REQUEST_COMPLETED : USB_REQUEST_ERROR);
    }
  }
  else
  {
    struct UsbRequest * const request = pointerQueueFront(&ep->requests);
    enum UsbRequestStatus status = USB_REQUEST_ERROR;

    if (setup)
    {
      if (epReadSetupPacket(ep, request))
        status = USB_REQUEST_SETUP;
    }
    else
    {
      if (epReadData(ep, request))
        status = USB_REQUEST_COMPLETED;
    }

    if (status != USB_REQUEST_ERROR)
    {
      pointerQueuePopFront(&ep->requests);
      request->callback(request->argument, request, status);
    }

    if (!pointerQueueEmpty(&ep->requests))
      epPrimeOut(ep);
  }
}
/*----------------------------------------------------------------------------*/
static void epPrimeOut(struct SbUsbEndpoint *ep)
{
  const unsigned int index = EP_TO_INDEX(ep->address);
  struct EpListEntry * const entry = &ep->device->base.endpointList[index];

  if (index == 0)
    entry->b[1] = epcsSetupTransfer(0, 0, ep->position + ep->size);

  entry->b[0] = epcsSetupTransfer(entry->b[0], ep->size, ep->position);
  entry->b[0] |= EPCS_A;
}
/*----------------------------------------------------------------------------*/
static bool epReadData(struct SbUsbEndpoint *ep,
    struct UsbRequest *request)
{
  struct UsbDevice * const device = ep->device;
  const unsigned int index = EP_TO_INDEX(ep->address);
  const struct EpListEntry * const entry = &device->base.endpointList[index];
  const uint32_t read = ep->size - EPCS_NBytes_VALUE(entry->b[0]);

  if (request->capacity < read)
    return false;

  const LPC_USB_Type * const reg = device->base.reg;
  const uint32_t address = reg->DATABUFSTART + ep->position;

  memcpy(request->buffer, (void *)address, read);
  request->length = read;

  return true;
}
/*----------------------------------------------------------------------------*/
static bool epReadSetupPacket(struct SbUsbEndpoint *ep,
    struct UsbRequest *request)
{
  struct UsbDevice * const device = ep->device;
  struct EpListEntry * const entry = &device->base.endpointList[0];

  if (request->capacity < sizeof(struct UsbSetupPacket))
    return false;

  const LPC_USB_Type * const reg = device->base.reg;
  const uint32_t address = reg->DATABUFSTART + ep->position + ep->size;

  memcpy(request->buffer, (const void *)address, sizeof(struct UsbSetupPacket));
  request->length = sizeof(struct UsbSetupPacket);

  /* Clear stall conditions on both control endpoints */
  entry[0].b[0] &= ~EPCS_S;
  entry[1].b[0] &= ~EPCS_S;

  return true;
}
/*----------------------------------------------------------------------------*/
static bool epWriteDataAndPrime(struct SbUsbEndpoint *ep,
    const struct UsbRequest *request)
{
  struct UsbDevice * const device = ep->device;
  const unsigned int index = EP_TO_INDEX(ep->address);
  struct EpListEntry * const entry = &device->base.endpointList[index];

  if (request->length > ep->size)
    return false;

  const LPC_USB_Type * const reg = device->base.reg;
  const uint32_t address = reg->DATABUFSTART + ep->position;

  memcpy((void *)address, request->buffer, request->length);
  entry->b[0] = epcsSetupTransfer(entry->b[0], request->length, ep->position);
  entry->b[0] |= EPCS_A;

  return true;
}
/*----------------------------------------------------------------------------*/
static enum Result epInit(void *object, const void *configBase)
{
  const struct UsbEndpointConfig * const config = configBase;
  struct SbUsbEndpoint * const ep = object;

  if (pointerQueueInit(&ep->requests, CONFIG_PLATFORM_USB_DEVICE_EP_REQUESTS))
  {
    ep->address = config->address;
    ep->device = config->parent;
    ep->position = 0;
    ep->size = 0;

    return E_OK;
  }
  else
    return E_MEMORY;
}
/*----------------------------------------------------------------------------*/
static void epDeinit(void *object)
{
  struct SbUsbEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;
  const unsigned int index = EP_TO_INDEX(ep->address);

  /* Disable interrupts and remove pending requests */
  epDisable(ep);
  epClear(ep);

  const IrqState state = irqSave();
  device->endpoints[index] = 0;
  irqRestore(state);

  assert(pointerQueueEmpty(&ep->requests));
  pointerQueueDeinit(&ep->requests);
}
/*----------------------------------------------------------------------------*/
static void epClear(void *object)
{
  struct SbUsbEndpoint * const ep = object;

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
  struct SbUsbEndpoint * const ep = object;
  LPC_USB_Type * const reg = ep->device->base.reg;
  const uint32_t mask = ~(1UL << EP_TO_INDEX(ep->address));

  reg->INTSTAT &= mask;
  reg->INTEN &= mask;
}
/*----------------------------------------------------------------------------*/
static void epEnable(void *object, uint8_t type __attribute__((unused)),
    uint16_t size)
{
  struct SbUsbEndpoint * const ep = object;
  const unsigned int index = EP_TO_INDEX(ep->address);
  struct UsbDevice * const device = ep->device;

  ep->position = device->position;
  ep->size = epcsAlignSize(size);

  device->position += ep->size;
  if (index == 0)
    device->position += epcsAlignSize(sizeof(struct UsbSetupPacket));

  LPC_USB_Type * const reg = device->base.reg;
  const uint32_t mask = 1UL << index;

  reg->INTSTAT |= mask;
  reg->INTEN |= mask;

  struct EpListEntry * const entry = &device->base.endpointList[index];
  const uint32_t value = (type != ENDPOINT_TYPE_ISOCHRONOUS) ? 0 : EPCS_T;

  entry->b[0] = value;
  entry->b[1] = value;

//  switch (type)
//  {
//    case ENDPOINT_TYPE_ISOCHRONOUS:
//    case ENDPOINT_TYPE_BULK:
//#ifdef CONFIG_PLATFORM_USB_DOUBLE_BUFFERING
//      ep->subclass = DbUsbEndpoint;
//      break;
//#endif
//
//    default:
//      ep->subclass = SbUsbEndpoint;
//      break;
//  }
//
//  ep->subclass->enable(ep, type, size);
}
/*----------------------------------------------------------------------------*/
static enum Result epEnqueue(void *object, struct UsbRequest *request)
{
  assert(request);
  assert(request->callback);

  struct SbUsbEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;
  const unsigned int index = EP_TO_INDEX(ep->address);

  if (index >= 2 && !ep->device->configured)
    return E_IDLE;

  assert(ep->position); /* Device should be initialized */

  const IrqState state = irqSave();
  enum Result res = E_FULL;

  if (!pointerQueueFull(&ep->requests))
  {
    bool invokeHandler = false;

    if (ep->address & USB_EP_DIRECTION_IN)
    {
      if (pointerQueueEmpty(&ep->requests))
      {
        if (!(device->base.endpointList[index].b[0] & EPCS_A))
          invokeHandler = true;
      }
    }
    else if (pointerQueueEmpty(&ep->requests))
    {
      epPrimeOut(ep);
    }
    pointerQueuePushBack(&ep->requests, request);

    if (invokeHandler)
    {
      LPC_USB_Type * const reg = device->base.reg;
      reg->INTSETSTAT |= INTSETSTAT_EP_SET_INT(index);
    }

    res = E_OK;
  }

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
static bool epIsStalled(void *object)
{
  const struct SbUsbEndpoint * const ep = object;
  const unsigned int index = EP_TO_INDEX(ep->address);

  return (ep->device->base.endpointList[index].b[0] & EPCS_S) != 0;
}
/*----------------------------------------------------------------------------*/
static void epSetStalled(void *object, bool stalled)
{
  struct SbUsbEndpoint * const ep = object;
  const unsigned int index = EP_TO_INDEX(ep->address);
  struct EpListEntry * const entry = &ep->device->base.endpointList[index];

  if (stalled)
  {
    entry->b[0] |= EPCS_S;
//    entry->b[1] |= EPCS_S; // TODO
  }
  else
  {
    entry->b[0] &= ~EPCS_S;

//    const LPC_USB_Type * const reg = device->base.reg; // TODO
//
//    entry->b[0] &= ~EPCS_S;
//    entry->b[1] &= ~EPCS_S;
//
//    if (reg->EPINUSE & (1UL << index))
//      entry->b[1] |= (entry->b[1] & ~EPCS_RF_TV) | EPCS_TR;
//    else
//      entry->b[0] |= (entry->b[0] & ~EPCS_RF_TV) | EPCS_TR;

    /* Write pending IN request to the endpoint buffer */
    if ((ep->address & USB_EP_DIRECTION_IN)
        && !pointerQueueEmpty(&ep->requests))
    {
      struct UsbRequest * const request = pointerQueueFront(&ep->requests);
      const bool ok = epWriteDataAndPrime(ep, request);

      pointerQueuePopFront(&ep->requests);
      request->callback(request->argument, request,
          ok ? USB_REQUEST_COMPLETED : USB_REQUEST_ERROR);
    }
  }
}
