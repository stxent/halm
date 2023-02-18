/*
 * usb_device.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/delay.h>
#include <halm/generic/pointer_queue.h>
#include <halm/platform/numicro/usb_base.h>
#include <halm/platform/numicro/usb_defs.h>
#include <halm/platform/numicro/usb_device.h>
#include <halm/usb/usb_control.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
#include <assert.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#define CONTROL_IN      1
#define CONTROL_OUT     0
#define ENDPOINT_FIRST  2
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
  /* Physical index */
  uint8_t index;
};

struct UsbDevice
{
  struct UsbBase base;

  /* Array of registered endpoints */
  struct UsbEndpoint *endpoints[CONFIG_PLATFORM_USB_DEVICE_EP_COUNT];
  /* Control message handler */
  struct UsbControl *control;

  /* The last allocated address inside the packet buffer memory */
  uint16_t position;
  /* The address to be set after the status stage of the control transaction */
  uint8_t scheduledAddress;
  /* Device is configured */
  bool configured;
  /* Device is enabled */
  bool enabled;
};
/*----------------------------------------------------------------------------*/
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
static enum UsbSpeed devGetSpeed(const void *);
static void devSetPower(void *, uint16_t);
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

    .getSpeed = devGetSpeed,
    .setPower = devSetPower,

    .stringAppend = devStringAppend,
    .stringErase = devStringErase
};
/*----------------------------------------------------------------------------*/
static inline void epCopyData(uint32_t *, const uint32_t *, size_t);
static inline void epEnqueueRequest(struct UsbEndpoint *,
    const struct UsbRequest *);
static inline NM_USBD_EP_Type *epGetChannel(struct UsbEndpoint *);
static void epHandlePacket(struct UsbEndpoint *);
static void epHandleSetupPacket(struct UsbEndpoint *);
static bool epReadData(struct UsbEndpoint *, uint8_t *, size_t, size_t *);
static inline void epSetDataType(struct UsbEndpoint *, uint8_t);
static void epWriteData(struct UsbEndpoint *, const uint8_t *, size_t);
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
  NM_USBD_Type * const reg = device->base.reg;
  const uint32_t devStatus = reg->ATTR;
  const uint32_t intStatus = reg->INTSTS;

  reg->INTSTS = intStatus;

  /* VBUS detection interrupt */
  if (intStatus & INTSTS_VBDETIF)
  {
    if (reg->VBUSDET & VBUSDET_VBUSDET)
    {
      reg->ATTR |= ATTR_USBEN;
    }
    else
    {
      reg->ATTR &= ~ATTR_USBEN;
    }
  }

  /* Device status interrupt */
  if (intStatus & INTSTS_BUSIF)
  {
    if (devStatus & ATTR_USBRST)
    {
      resetDevice(device);
      usbControlNotify(device->control, USB_DEVICE_EVENT_RESET);
    }

    if (devStatus & ATTR_RESUME)
    {
      usbControlNotify(device->control, USB_DEVICE_EVENT_RESUME);
    }

    if (devStatus & ATTR_SUSPEND)
    {
      usbControlNotify(device->control, USB_DEVICE_EVENT_SUSPEND);
    }
  }

  /* Endpoint interrupt */
  if (intStatus & INTSTS_USBIF)
  {
    if (intStatus & INTSTS_SETUP)
    {
      epSetDataType(device->endpoints[CONTROL_IN], 1);
      epSetDataType(device->endpoints[CONTROL_OUT], 1);

      epHandleSetupPacket(device->endpoints[CONTROL_OUT]);
    }

    usbEpFlagsIterate(epHandlePacket, device->endpoints,
        INTSTS_EPEVT_VALUE(intStatus));
  }

  if (intStatus & INTSTS_SOFIF)
  {
    // TODO SOF handling
    usbControlNotify(device->control, USB_DEVICE_EVENT_FRAME);
  }

  if (intStatus & INTSTS_NEVWKIF)
  {
    // TODO USB wakeup handling
  }
}
/*----------------------------------------------------------------------------*/
static void resetDevice(struct UsbDevice *device)
{
  NM_USBD_Type * const reg = device->base.reg;

  for (size_t index = 0; index < CONFIG_PLATFORM_USB_DEVICE_EP_COUNT; ++index)
    reg->EP[index].CFG = 0;

  /* Allocate buffer for setup packet inside the USB SRAM */
  device->position = STBUFSEG_SIZE;
  reg->STBUFSEG = 0;

  device->scheduledAddress = 0;
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
  device->scheduledAddress = 0;
  device->configured = false;
  device->enabled = false;
  memset(device->endpoints, 0, sizeof(device->endpoints));

  /* Initialize control message handler after endpoint initialization */
  device->control = init(UsbControl, &controlConfig);
  if (!device->control)
    return E_ERROR;

  /* Reset system variables and configure interrupts */
  NM_USBD_Type * const reg = device->base.reg;

  /* Disable interrupts and clear pending interrupt flags */
  reg->INTEN = 0;
  reg->INTSTS = INTSTS_BUSIF | INTSTS_USBIF | INTSTS_VBDETIF | INTSTS_NEVWKIF;

  reg->ATTR = ATTR_PWRDN;
  reg->FADDR = 0;
  reg->SE0 = SE0_SE0;

  for (size_t index = 0; index < CONFIG_PLATFORM_USB_DEVICE_EP_COUNT; ++index)
    reg->EP[index].CFG = 0;

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
  struct UsbEndpoint *ep = 0;
  size_t channel;

  /* Lock access to endpoint list */
  const IrqState state = irqSave();

  if (USB_EP_LOGICAL_ADDRESS(address) == 0)
  {
    channel = (address & USB_EP_DIRECTION_IN) ? CONTROL_IN : CONTROL_OUT;
  }
  else
  {
    /* Allocate free endpoint */
    channel = ENDPOINT_FIRST;

    while (channel < CONFIG_PLATFORM_USB_DEVICE_EP_COUNT)
    {
      struct UsbEndpoint * const current = device->endpoints[channel];

      if (!current)
        break;

      if (current->address == address)
      {
        ep = current;
        break;
      }

      ++channel;
    }
  }

  if (!ep && channel != CONFIG_PLATFORM_USB_DEVICE_EP_COUNT)
  {
    /* Driver should be disabled during initialization */
    assert(!device->enabled);

    const struct UsbEndpointConfig config = {
        .parent = device,
        .address = address,
        .index = channel
    };

    device->endpoints[channel] = init(UsbEndpoint, &config);
    ep = device->endpoints[channel];
  }

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
  NM_USBD_Type * const reg = device->base.reg;

  device->configured = address != 0;
  device->scheduledAddress = address;

  if (address == 0)
    reg->FADDR = address;
}
/*----------------------------------------------------------------------------*/
static void devSetConnected(void *object, bool state)
{
  struct UsbDevice * const device = object;
  NM_USBD_Type * const reg = device->base.reg;

  device->enabled = state;

  if (state)
  {
    /* Clear pending interrupt flags and enable interrupts */
    reg->INTSTS = INTSTS_BUSIF | INTSTS_USBIF | INTSTS_VBDETIF
        | INTSTS_NEVWKIF;
    reg->INTEN = INTEN_BUSIEN | INTEN_USBIEN | INTEN_VBDETIEN
        | INTEN_NEVWKIEN | INTEN_WKEN;

    reg->ATTR |= ATTR_PHYEN | ATTR_DPPUEN;
    reg->SE0 = 0;
  }
  else
  {
    /* Disable interrupts */
    reg->INTEN = 0;

    reg->ATTR &= ~(ATTR_PHYEN | ATTR_USBEN | ATTR_DPPUEN);
    reg->SE0 = SE0_SE0;
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
static inline void epCopyData(uint32_t *destination, const uint32_t *source,
    size_t length)
{
  const uint32_t * const end = destination + (length >> 2);

  while (destination != end)
    *destination++ = *source++;
  length &= 3;

  if (length)
  {
    const uint8_t * const buffer = (const uint8_t *)source;
    uint32_t word = 0;

    switch (length)
    {
      case 3:
        word = buffer[2] << 16;
        /* Falls through */
      case 2:
        word |= buffer[1] << 8;
        /* Falls through */
      case 1:
        word |= buffer[0];
        break;
    }

    *destination = word;
  }
}
/*----------------------------------------------------------------------------*/
static inline void epEnqueueRequest(struct UsbEndpoint *ep,
    const struct UsbRequest *request)
{
  NM_USBD_EP_Type * const channel = epGetChannel(ep);
  channel->MXPLD = request->capacity;
}
/*----------------------------------------------------------------------------*/
static inline NM_USBD_EP_Type *epGetChannel(struct UsbEndpoint *ep)
{
  return ((NM_USBD_Type *)ep->device->base.reg)->EP + ep->index;
}
/*----------------------------------------------------------------------------*/
static void epHandlePacket(struct UsbEndpoint *ep)
{
  if (pointerQueueEmpty(&ep->requests))
    return;

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    if (ep->address == USB_EP_DIRECTION_IN)
    {
      /* Control IN endpoint */
      struct UsbDevice * const device = ep->device;

      if (device->scheduledAddress != 0)
      {
        NM_USBD_Type * const reg = device->base.reg;

        /*
         * Set a previously saved device address after the status stage
         * of the control transaction.
         */
        reg->FADDR = device->scheduledAddress;
        device->scheduledAddress = 0;
      }
    }

    struct UsbRequest *req = pointerQueueFront(&ep->requests);
    pointerQueuePopFront(&ep->requests);

    req->callback(req->argument, req, USB_REQUEST_COMPLETED);

    if (!pointerQueueEmpty(&ep->requests))
    {
      req = pointerQueueFront(&ep->requests);
      epWriteData(ep, req->buffer, req->length);
    }
  }
  else
  {
    struct UsbRequest *req = pointerQueueFront(&ep->requests);
    size_t read;

    if (epReadData(ep, req->buffer, req->capacity, &read))
    {
      pointerQueuePopFront(&ep->requests);
      req->length = read;
      req->callback(req->argument, req, USB_REQUEST_COMPLETED);
    }

    /* Trigger reception of a next packet */
    if (!pointerQueueEmpty(&ep->requests))
    {
      req = pointerQueueFront(&ep->requests);
      epEnqueueRequest(ep, req);
    }
  }
}
/*----------------------------------------------------------------------------*/
static void epHandleSetupPacket(struct UsbEndpoint *ep)
{
  NM_USBD_Type * const reg = ep->device->base.reg;

  if (!pointerQueueEmpty(&ep->requests))
  {
    struct UsbRequest * const req = pointerQueueFront(&ep->requests);
    const uint32_t offset = reg->STBUFSEG & STBUFSEG_ADDRESS_MASK;

    pointerQueuePopFront(&ep->requests);

    epCopyData(req->buffer, (const void *)(reg->SRAM + offset), STBUFSEG_SIZE);
    req->length = STBUFSEG_SIZE;
    req->callback(req->argument, req, USB_REQUEST_SETUP);
  }
}
/*----------------------------------------------------------------------------*/
static bool epReadData(struct UsbEndpoint *ep, uint8_t *buffer,
    size_t length, size_t *read)
{
  const NM_USBD_Type * const reg = ep->device->base.reg;
  const NM_USBD_EP_Type * const channel = &reg->EP[ep->index];
  const uint32_t available = channel->MXPLD;
  const uint32_t offset = channel->BUFSEG & BUFSEG_ADDRESS_MASK;

  if (available > length)
    return false;

  epCopyData((uint32_t *)buffer, (const void *)(reg->SRAM + offset), available);
  *read = available;

  return true;
}
/*----------------------------------------------------------------------------*/
static inline void epSetDataType(struct UsbEndpoint *ep, uint8_t type)
{
  NM_USBD_EP_Type * const channel = epGetChannel(ep);

  channel->CFGP |= CFGP_CLRRDY;

  if (type)
    channel->CFG |= CFG_DSQSYNC;
  else
    channel->CFG &= ~CFG_DSQSYNC;
}
/*----------------------------------------------------------------------------*/
static void epWriteData(struct UsbEndpoint *ep, const uint8_t *buffer,
    size_t length)
{
  NM_USBD_Type * const reg = ep->device->base.reg;
  NM_USBD_EP_Type * const channel = &reg->EP[ep->index];
  const uint32_t offset = channel->BUFSEG & BUFSEG_ADDRESS_MASK;

  epCopyData((void *)(reg->SRAM + offset), (const uint32_t *)buffer, length);
  channel->MXPLD = length;
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
    ep->index = config->index;

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

  /* Disable interrupts and remove pending requests */
  epDisable(ep);
  epClear(ep);

  const IrqState state = irqSave();
  device->endpoints[ep->index] = 0;
  irqRestore(state);

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
  NM_USBD_EP_Type * const channel = epGetChannel(ep);
  uint32_t cfg = channel->CFG & ~CFG_STATE_MASK;

  cfg |= CFG_STATE(STATE_EP_DISABLED);
  channel->CFG = cfg;
}
/*----------------------------------------------------------------------------*/
static void epEnable(void *object, uint8_t type, uint16_t size)
{
  struct UsbEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;
  NM_USBD_EP_Type * const channel = epGetChannel(ep);

  channel->BUFSEG = device->position & BUFSEG_ADDRESS_MASK;
  channel->CFGP = 0;

  device->position += (size + (BUFSEG_ADDRESS_ALIGNMENT - 1))
      & ~(BUFSEG_ADDRESS_ALIGNMENT - 1);

  uint32_t cfg = CFG_EPNUM(EP_TO_NUMBER(ep->address));

  if (USB_EP_LOGICAL_ADDRESS(ep->address) == 0)
    cfg |= CFG_CSTALL;
  if (type == ENDPOINT_TYPE_ISOCHRONOUS)
    cfg |= CFG_ISOCH;

  if (ep->address & USB_EP_DIRECTION_IN)
    cfg |= CFG_STATE(STATE_EP_IN);
  else
    cfg |= CFG_STATE(STATE_EP_OUT);

  channel->CFG = cfg;
}
/*----------------------------------------------------------------------------*/
static enum Result epEnqueue(void *object, struct UsbRequest *request)
{
  assert(request);
  assert(request->callback);

  struct UsbEndpoint * const ep = object;
  const IrqState state = irqSave();
  enum Result res = E_FULL;

  if (!pointerQueueFull(&ep->requests))
  {
    if (ep->address & USB_EP_DIRECTION_IN)
    {
      /* Send packet when the queue is empty */
      if (pointerQueueEmpty(&ep->requests))
        epWriteData(ep, request->buffer, request->length);
    }
    else
    {
      /* Trigger reception when the queue is empty */
      if (pointerQueueEmpty(&ep->requests))
        epEnqueueRequest(ep, request);
    }

    pointerQueuePushBack(&ep->requests, request);
    res = E_OK;
  }

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
static bool epIsStalled(void *object)
{
  struct UsbEndpoint * const ep = object;
  const NM_USBD_EP_Type * const channel = epGetChannel(ep);

  return (channel->CFGP & CFGP_SSTALL) != 0;
}
/*----------------------------------------------------------------------------*/
static void epSetStalled(void *object, bool stalled)
{
  struct UsbEndpoint * const ep = object;
  NM_USBD_EP_Type * const channel = epGetChannel(ep);

  if (stalled)
    channel->CFGP |= CFGP_SSTALL;
  else
    channel->CFGP &= ~CFGP_SSTALL;

  /* Write pending IN request to the endpoint buffer */
  if (!stalled && (ep->address & USB_EP_DIRECTION_IN)
      && !pointerQueueEmpty(&ep->requests))
  {
    struct UsbRequest * const request = pointerQueueFront(&ep->requests);
    epWriteData(ep, request->buffer, request->length);
  }
}
