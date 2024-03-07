/*
 * usb_device.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/pointer_queue.h>
#include <halm/platform/stm32/clocking.h>
#include <halm/platform/stm32/gen_2/usb_defs.h>
#include <halm/platform/stm32/usb_base.h>
#include <halm/platform/stm32/usb_device.h>
#include <halm/usb/usb_control.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
#include <xcore/accel.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct UsbEndpoint;

/* Endpoint subclass descriptor */
struct DataEndpointClass
{
  enum Result (*enqueue)(struct UsbEndpoint *, struct UsbRequest *);
  void (*isr)(struct UsbEndpoint *, size_t, bool);
};

struct UsbEndpoint
{
  struct UsbEndpointBase base;

  /* Endpoint subclass */
  const struct DataEndpointClass *subclass;
  /* Parent device */
  struct UsbDevice *device;
  /* Queued requests */
  PointerQueue requests;
  /* Maximum size of a packet */
  uint16_t size;
  /* Logical address */
  uint8_t address;
};

struct UsbDevice
{
  struct UsbBase base;

  /* Array of registered endpoints */
  struct UsbEndpoint **endpoints;
  /* Control message handler */
  struct UsbControl *control;

  /* The last allocated address inside the packet buffer memory */
  uint16_t position;
  /* Device is configured */
  bool configured;
  /* Device is enabled */
  bool enabled;
  /* Device is suspended */
  bool suspended;
};
/*----------------------------------------------------------------------------*/
static void configPeriphLatency(struct UsbDevice *);
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
// static void disableEpIn(struct UsbEndpoint *); // TODO
static void disableEpOut(struct UsbEndpoint *);
static void enableEpIn(struct UsbEndpoint *, size_t);
static void enableEpOut(struct UsbEndpoint *);
static uint32_t encodeEp0Size(size_t);
static void flushEpInFifo(struct UsbEndpoint *);

#ifdef CONFIG_PLATFORM_USB_DMA
static enum Result dmaEpEnqueue(struct UsbEndpoint *, struct UsbRequest *);
static void dmaEpHandler(struct UsbEndpoint *, size_t, bool);
#endif

static enum Result sieEpEnqueue(struct UsbEndpoint *, struct UsbRequest *);
static void sieEpHandler(struct UsbEndpoint *, size_t, bool);
static bool sieEpReadData(struct UsbEndpoint *, uint8_t *, size_t, size_t,
    size_t *);
static void sieEpWriteData(struct UsbEndpoint *, const uint8_t *, size_t);
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
#ifdef CONFIG_PLATFORM_USB_DMA
static const struct DataEndpointClass * const DmaUsbEndpoint =
    &(const struct DataEndpointClass){
    .enqueue = dmaEpEnqueue,
    .isr = dmaEpHandler
};
#endif

static const struct DataEndpointClass * const SieUsbEndpoint =
    &(const struct DataEndpointClass){
    .enqueue = sieEpEnqueue,
    .isr = sieEpHandler
};

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
static void configPeriphLatency(struct UsbDevice *device)
{
  STM_USB_OTG_Type * const reg = device->base.reg;
  uint32_t value = reg->GLOBAL.GUSBCFG & ~GUSBCFG_TRDT_MASK;

  if (!device->base.hs)
  {
    const uint32_t ahbFrequency = clockFrequency(MainClock);

    // TODO Implement TRDT table
    if (ahbFrequency >= 32000000)
      value |= GUSBCFG_TRDT(0x6);
    else
      value |= GUSBCFG_TRDT(0xF);
  }
  else
  {
    value |= GUSBCFG_TRDT(0x9);
  }

  reg->GLOBAL.GUSBCFG = value;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct UsbDevice * const device = object;
  STM_USB_OTG_Type * const reg = device->base.reg;
  const uint32_t intStatus = reg->GLOBAL.GINTSTS;

  if (intStatus & GINTSTS_USBRST)
  {
    reg->GLOBAL.GINTSTS = GINTSTS_USBRST;
    resetDevice(device);
    usbControlNotify(device->control, USB_DEVICE_EVENT_RESET);
  }

  if (intStatus & GINTSTS_ENUMDNE)
  {
    reg->GLOBAL.GINTSTS = GINTSTS_ENUMDNE;
    usbControlNotify(device->control, USB_DEVICE_EVENT_PORT_CHANGE);
  }

  if (intStatus & GINTSTS_USBSUSP)
  {
    reg->GLOBAL.GINTSTS = GINTSTS_USBSUSP;
    if (!device->suspended)
      usbControlNotify(device->control, USB_DEVICE_EVENT_SUSPEND);
    device->suspended = true;
  }

  if (intStatus & GINTSTS_WKUPINT)
  {
    reg->GLOBAL.GINTSTS = GINTSTS_WKUPINT;
    if (device->suspended)
      usbControlNotify(device->control, USB_DEVICE_EVENT_RESUME);
    device->suspended = false;
  }

  if (intStatus & GINTSTS_RXFLVL)
  {
    const uint32_t fifoStatus = reg->GLOBAL.GRXSTSP;
    const uint8_t epNumber = GRXSTSR_EPNUM_VALUE(fifoStatus);
    const uint8_t packetStatus = GRXSTSR_PKTSTS_VALUE(fifoStatus);
    const size_t packetLength = GRXSTSR_BCNT_VALUE(fifoStatus);

    if (packetStatus == PKTSTS_SETUP_PACKET_RECEIVED
        || packetStatus == PKTSTS_OUT_PACKET_RECEIVED)
    {
      struct UsbEndpoint * const ep = device->endpoints[EP_TO_INDEX(epNumber)];
      const bool setup = packetStatus == PKTSTS_SETUP_PACKET_RECEIVED;

      /* New SETUP packet is received, flush endpoint 0 IN FIFO */
      if (setup && (reg->DEV_EP_IN[0].DIEPTSIZ & DIEPTSIZ0_PKTCNT_MASK))
        flushEpInFifo(device->endpoints[EP_TO_INDEX(USB_EP_DIRECTION_IN)]);

      ep->subclass->isr(ep, packetLength, setup);
    }
  }

  if (intStatus & GINTSTS_IEPINT)
  {
    uint32_t epIntStatus = reverseBits32(DAINT_IEPINT_VALUE(reg->DEV.DAINT));

    while (epIntStatus)
    {
      const unsigned int index = countLeadingZeros32(epIntStatus);
      struct UsbEndpoint * const ep =
          device->endpoints[EP_TO_INDEX(index | USB_EP_DIRECTION_IN)];

      epIntStatus -= (1UL << 31) >> index;
      reg->DEV_EP_IN[index].DIEPINT = DIEPINT_XFRC;

      ep->subclass->isr(ep, 0, false);
    }
  }
}
/*----------------------------------------------------------------------------*/
static void resetDevice(struct UsbDevice *device)
{
  STM_USB_OTG_Type * const reg = device->base.reg;

  /* Set inactive configuration */
  device->suspended = false;
  device->position = MIN(device->base.memoryCapacity >> 1, 1024);

  /* Disable interrupts for all endpoints */
  reg->DEV.DAINTMSK = 0;

  /* Disable all enabled endpoints */
  for (size_t index = 1; index < device->base.numberOfEndpoints >> 1; ++index)
  {
    if (reg->DEV_EP_IN[index].DIEPCTL & DIEPCTL_EPENA)
      reg->DEV_EP_IN[index].DIEPCTL |= DIEPCTL_EPDIS;

    if (reg->DEV_EP_OUT[index].DOEPCTL & DOEPCTL_EPENA)
      reg->DEV_EP_OUT[index].DOEPCTL |= DOEPCTL_EPDIS;

    reg->GLOBAL.DIEPTXF[index - 1] = 0;
  }

  /* Flush all FIFO */
  reg->GLOBAL.GRSTCTL = (reg->GLOBAL.GRSTCTL & ~GRSTCTL_TXFNUM_MASK)
      | GRSTCTL_RXFFLSH | GRSTCTL_TXFFLSH | GRSTCTL_TXFNUM(GRSTCTL_TXFNUM_ALL);

  devSetAddress(device, 0);

  /* Reset all enabled endpoints except for Control Endpoints */
  for (size_t index = 2; index < device->base.numberOfEndpoints; ++index)
    device->endpoints[index] = NULL;
}
/*----------------------------------------------------------------------------*/
static enum Result devInit(void *object, const void *configBase)
{
  const struct UsbDeviceConfig * const config = configBase;
  assert(config != NULL);

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
  device->configured = false;
  device->enabled = false;
  device->suspended = false;

  device->endpoints = malloc(device->base.numberOfEndpoints
      * sizeof(struct UsbEndpoint *));
  if (device->endpoints == NULL)
    return E_MEMORY;

  for (size_t index = 0; index < device->base.numberOfEndpoints; ++index)
    device->endpoints[index] = NULL;

  /* Initialize control message handler after endpoint initialization */
  device->control = init(UsbControl, &controlConfig);
  if (device->control == NULL)
    return E_ERROR;

  STM_USB_OTG_Type * const reg = device->base.reg;

  /* Wait for AHB idle */
  while (!(reg->GLOBAL.GRSTCTL & GRSTCTL_AHBIDL));
  /* Issue software reset */
  reg->GLOBAL.GRSTCTL |= GRSTCTL_CSRST;
  while (reg->GLOBAL.GRSTCTL & GRSTCTL_CSRST);

  // TODO
  // if (reg->GLOBAL.CID >= CID_HAS_VBDEN)
  //   reg->GLOBAL.GCCFG |= GCCFG_VBDEN | GCCFG_PWRDWN;
  // else
  //   reg->GLOBAL.GCCFG = GCCFG_VBUSBSEN | GCCFG_PWRDWN;

  reg->GLOBAL.GCCFG = GCCFG_VBUSBSEN | GCCFG_PWRDWN;

  if (!config->vbus)
  {
    /* No VBUS pin provided, assume that VBUS is always connected */
    reg->GLOBAL.GCCFG |= GCCFG_NOVBUSSENS;
  }

  /* Disable pull-up for software disconnect */
  reg->DEV.DCTL |= DCTL_SDIS;

  reg->GLOBAL.GUSBCFG |= GUSBCFG_FDMOD;
  configPeriphLatency(device);

  reg->DEV.DCFG = (reg->DEV.DCFG & ~DCFG_DSPD_MASK)
      | (device->base.hs ? DCFG_DSPD(DSPD_HS) : DCFG_DSPD(DSPD_FS));

  // TODO Is it needed?
  reg->GLOBAL.GINTSTS = GINTSTS_MMIS;

  /* Restart the PHY clock */
  reg->PCG.PCGCCTL = 0;

  device->position = MIN(device->base.memoryCapacity >> 1, 1024);
  reg->GLOBAL.GRXFSIZ = device->position >> 2;

#ifdef CONFIG_PLATFORM_USB_DMA
  if (device->base.dma)
  {
    reg->GLOBAL.GAHBCFG |= (reg->GLOBAL.GAHBCFG & ~GAHBCFG_HBSTLEN_MASK)
        | GAHBCFG_HBSTLEN(HBSTLEN_INCR4) | GAHBCFG_DMAEN;
  }
#endif

  /* Unmask interrupts for RX and TX */
  reg->GLOBAL.GAHBCFG |= GAHBCFG_GINT;
  reg->GLOBAL.GINTMSK =
      GINTMSK_USBRST | GINTMSK_ENUMDNEM
      | GINTMSK_RXFLVLM | GINTMSK_IEPINT
      | GINTMSK_USBSUSPM | GINTMSK_WUIM;
  reg->DEV.DAINTMSK = 0;
  reg->DEV.DIEPMSK = DIEPMSK_XFRCM;
  reg->DEV.DOEPMSK = DOEPMSK_XFRCM;

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
  free(device->endpoints);
  UsbBase->deinit(device);
}
#endif
/*----------------------------------------------------------------------------*/
static void *devCreateEndpoint(void *object, uint8_t address)
{
  const unsigned int index = EP_TO_INDEX(address);
  struct UsbDevice * const device = object;

  assert(index < device->base.numberOfEndpoints);

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
static uint8_t devGetInterface([[maybe_unused]] const void *object)
{
  return 0;
}
/*----------------------------------------------------------------------------*/
static void devSetAddress(void *object, uint8_t address)
{
  struct UsbDevice * const device = object;
  STM_USB_OTG_Type * const reg = device->base.reg;

  device->configured = address != 0;
  reg->DEV.DCFG = (reg->DEV.DCFG & ~DCFG_DAD_MASK) | DCFG_DAD(address);
}
/*----------------------------------------------------------------------------*/
static void devSetConnected(void *object, bool state)
{
  struct UsbDevice * const device = object;
  STM_USB_OTG_Type * const reg = device->base.reg;

  device->enabled = state;

  if (state)
    reg->DEV.DCTL &= ~DCTL_SDIS;
  else
    reg->DEV.DCTL |= DCTL_SDIS;
}
/*----------------------------------------------------------------------------*/
static enum Result devBind(void *object, void *driver)
{
  struct UsbDevice * const device = object;
  return usbControlBindDriver(device->control, driver);
}
/*----------------------------------------------------------------------------*/
static void devUnbind(void *object, [[maybe_unused]] const void *driver)
{
  struct UsbDevice * const device = object;
  usbControlUnbindDriver(device->control);
}
/*----------------------------------------------------------------------------*/
static enum UsbSpeed devGetSpeed(const void *object)
{
  const struct UsbDevice * const device = object;
  const STM_USB_OTG_Type * const reg = device->base.reg;

  return DSTS_ENUMSPD(reg->DEV.DSTS) == ENUMSPD_HS ? USB_HS : USB_FS;
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
// /*----------------------------------------------------------------------------*/
// static void disableEpIn(struct UsbEndpoint *ep)
// {
//   STM_USB_OTG_Type * const reg = ep->device->base.reg;
//   const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

//   reg->DEV_EP_IN[number].DIEPCTL |= DIEPCTL_SNAK;
// }
/*----------------------------------------------------------------------------*/
static void disableEpOut(struct UsbEndpoint *ep)
{
  STM_USB_OTG_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  reg->DEV_EP_OUT[number].DOEPCTL |= DOEPCTL_SNAK;
}
/*----------------------------------------------------------------------------*/
static void enableEpIn(struct UsbEndpoint *ep, size_t length)
{
  STM_USB_OTG_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);
  const size_t packets = length ? (length + ep->size - 1) / ep->size : 1;

  assert((number && packets <= CONFIG_PLATFORM_USB_IN_BUFFERS)
      || (!number && packets == 1));

  reg->DEV_EP_IN[number].DIEPTSIZ =
      DIEPTSIZ_PKTCNT(packets) | DIEPTSIZ_XFRSIZ(length);
  reg->DEV_EP_IN[number].DIEPCTL |= DIEPCTL_CNAK | DIEPCTL_EPENA;
}
/*----------------------------------------------------------------------------*/
static void enableEpOut(struct UsbEndpoint *ep)
{
  STM_USB_OTG_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  if (!number)
  {
    reg->DEV_EP_OUT[number].DOEPTSIZ = DOEPTSIZ0_XFRSIZ(ep->size)
        | DOEPTSIZ0_PKTCNT | DOEPTSIZ0_STUPCNT(1);
  }
  else
  {
    reg->DEV_EP_OUT[number].DOEPTSIZ = DOEPTSIZ_XFRSIZ(ep->size)
        | DOEPTSIZ_PKTCNT(1);
  }

  reg->DEV_EP_OUT[number].DOEPCTL |= DOEPCTL_CNAK | DOEPCTL_EPENA;
}
/*----------------------------------------------------------------------------*/
static uint32_t encodeEp0Size(size_t size)
{
  assert(size <= 64);

  if (size <= 8)
    return 3; /* 8 bytes */
  if (size <= 16)
    return 2; /* 16 bytes */
  if (size <= 32)
    return 1; /* 32 bytes */

  /* 64 bytes */
  return 0;
}
/*----------------------------------------------------------------------------*/
static void flushEpInFifo(struct UsbEndpoint *ep)
{
  struct UsbDevice * const device = ep->device;
  STM_USB_OTG_Type * const reg = device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  reg->DEV_EP_IN[number].DIEPCTL |= DIEPCTL_SNAK;
  /* Wait for core to apply NAK */
  while (!(reg->DEV_EP_IN[number].DIEPINT & DIEPINT_INEPNE));
  /* Wait for core to become idle */
  while (!(reg->GLOBAL.GRSTCTL & GRSTCTL_AHBIDL));

  /* Flush FIFO and reset packet configuration */
  reg->GLOBAL.GRSTCTL = GRSTCTL_TXFFLSH | GRSTCTL_TXFNUM(number);
  reg->DEV_EP_IN[number].DIEPTSIZ = 0;

  /* Wait until FIFO is cleared */
  while (reg->GLOBAL.GRSTCTL & GRSTCTL_TXFFLSH);
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_USB_DMA
static enum Result dmaEpEnqueue(struct UsbEndpoint *ep,
    struct UsbRequest *request)
{
  const IrqState state = irqSave();

  if (pointerQueueFull(&ep->requests))
  {
    irqRestore(state);
    return E_FULL;
  }

  if (pointerQueueEmpty(&ep->requests))
  {
    STM_USB_OTG_Type * const reg = ep->device->base.reg;
    const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

    if (ep->address & USB_EP_DIRECTION_IN)
    {
      reg->DEV_EP_IN[number].DIEPDMA = (uintptr_t)request->buffer;
      enableEpIn(ep, request->length);
    }
    else
    {
      reg->DEV_EP_OUT[number].DOEPDMA = (uintptr_t)request->buffer;
      enableEpOut(ep);
    }
  }

  pointerQueuePushBack(&ep->requests, request);

  irqRestore(state);
  return E_OK;
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_USB_DMA
static void dmaEpHandler(struct UsbEndpoint *ep, size_t length, bool setup)
{
  if (pointerQueueEmpty(&ep->requests))
    return;

  STM_USB_OTG_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    struct UsbRequest *req = pointerQueueFront(&ep->requests);
    pointerQueuePopFront(&ep->requests);

    req->callback(req->argument, req, USB_REQUEST_COMPLETED);

    if (!pointerQueueEmpty(&ep->requests))
    {
      req = pointerQueueFront(&ep->requests);

      reg->DEV_EP_IN[number].DIEPDMA = (uintptr_t)req->buffer;
      enableEpIn(ep, req->length);
    }
    else
    {
      // TODO
      // disableEpIn(ep);
    }
  }
  else
  {
    struct UsbRequest * const req = pointerQueueFront(&ep->requests);
    size_t read;

    if (sieEpReadData(ep, req->buffer, req->capacity, length, &read))
    {
      const enum UsbRequestStatus requestStatus = setup ?
          USB_REQUEST_SETUP : USB_REQUEST_COMPLETED;

      pointerQueuePopFront(&ep->requests);
      req->length = read;
      req->callback(req->argument, req, requestStatus);

      if (!pointerQueueEmpty(&ep->requests))
      {
        reg->DEV_EP_OUT[number].DOEPDMA = (uintptr_t)req->buffer;
        enableEpOut(ep);
      }
      else
        disableEpOut(ep);
    }
  }
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result sieEpEnqueue(struct UsbEndpoint *ep,
    struct UsbRequest *request)
{
  const IrqState state = irqSave();

  if (pointerQueueFull(&ep->requests))
  {
    irqRestore(state);
    return E_FULL;
  }

  if (pointerQueueEmpty(&ep->requests))
  {
    if (ep->address & USB_EP_DIRECTION_IN)
      sieEpWriteData(ep, request->buffer, request->length);
    else
      enableEpOut(ep);
  }

  pointerQueuePushBack(&ep->requests, request);

  irqRestore(state);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sieEpHandler(struct UsbEndpoint *ep, size_t length, bool setup)
{
  if (pointerQueueEmpty(&ep->requests))
    return;

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    struct UsbRequest *req = pointerQueueFront(&ep->requests);
    pointerQueuePopFront(&ep->requests);

    req->callback(req->argument, req, USB_REQUEST_COMPLETED);

    if (!pointerQueueEmpty(&ep->requests))
    {
      req = pointerQueueFront(&ep->requests);
      sieEpWriteData(ep, req->buffer, req->length);
    }
    else
    {
      // TODO
      // disableEpIn(ep);
    }
  }
  else
  {
    struct UsbRequest * const req = pointerQueueFront(&ep->requests);
    size_t read;

    if (sieEpReadData(ep, req->buffer, req->capacity, length, &read))
    {
      const enum UsbRequestStatus requestStatus = setup ?
          USB_REQUEST_SETUP : USB_REQUEST_COMPLETED;

      pointerQueuePopFront(&ep->requests);
      req->length = read;
      req->callback(req->argument, req, requestStatus);

      if (!pointerQueueEmpty(&ep->requests))
        enableEpOut(ep);
      else
        disableEpOut(ep);
    }
  }
}
/*----------------------------------------------------------------------------*/
static bool sieEpReadData(struct UsbEndpoint *ep, uint8_t *buffer,
    size_t capacity, size_t length, size_t *read)
{
  STM_USB_OTG_Type * const reg = ep->device->base.reg;
  const bool ok = length <= capacity;

  if (ok)
  {
    uint32_t *start = (uint32_t *)buffer;
    uint32_t * const end = start + (length >> 2);

    *read = length;
    buffer = (uint8_t *)end;
    length &= 3;

    while (start < end)
      *start++ = reg->FIFO[0].DATA[0];

    if (length)
    {
      uint32_t lastWord = reg->FIFO[0].DATA[0];

      switch (length)
      {
        case 3:
          *buffer++ = (uint8_t)lastWord;
          lastWord >>= 8;
          [[fallthrough]];
        case 2:
          *buffer++ = (uint8_t)lastWord;
          lastWord >>= 8;
          [[fallthrough]];
        case 1:
          *buffer = (uint8_t)lastWord;
          break;
      }
    }
  }

  return ok;
}
/*----------------------------------------------------------------------------*/
static void sieEpWriteData(struct UsbEndpoint *ep, const uint8_t *buffer,
    size_t length)
{
  STM_USB_OTG_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  enableEpIn(ep, length);

  if (length)
  {
    const uint32_t *start = (const uint32_t *)buffer;
    const uint32_t * const end = start + (length >> 2);

    buffer = (const uint8_t *)end;
    length &= 3;

    while (start < end)
    {
      reg->FIFO[number].DATA[0] = *start++;
    }

    if (length)
    {
      uint32_t lastWord = 0;

      switch (length)
      {
        case 3:
          lastWord = buffer[2] << 16;
          [[fallthrough]];
        case 2:
          lastWord |= buffer[1] << 8;
          [[fallthrough]];
        case 1:
          lastWord |= buffer[0];
          break;
      }

      reg->FIFO[number].DATA[0] = lastWord;
    }
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
    const struct DataEndpointClass *subclass = SieUsbEndpoint;

#ifdef CONFIG_PLATFORM_USB_DMA
    if (config->parent->base.dma)
      subclass = DmaUsbEndpoint;
#endif

    ep->subclass = subclass;
    ep->device = config->parent;
    ep->address = config->address;

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
    struct UsbRequest * const req = pointerQueueFront(&ep->requests);
    pointerQueuePopFront(&ep->requests);

    req->callback(req->argument, req, USB_REQUEST_CANCELLED);
  }
}
/*----------------------------------------------------------------------------*/
static void epDisable(void *object)
{
  struct UsbEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;
  STM_USB_OTG_Type * const reg = device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    flushEpInFifo(ep);
    reg->DEV_EP_IN[number].DIEPCTL |= DIEPCTL_EPDIS;
  }
  else
    reg->DEV_EP_OUT[number].DOEPCTL |= DOEPCTL_EPDIS;

  const unsigned int index = EP_TO_INDEX(ep->address);

  if (index >= 2 && device->endpoints[index] == ep)
    device->endpoints[index] = NULL;
}
/*----------------------------------------------------------------------------*/
static void epEnable(void *object, uint8_t type, uint16_t size)
{
  struct UsbEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;
  STM_USB_OTG_Type * const reg = device->base.reg;
  const unsigned int index = EP_TO_INDEX(ep->address);
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  if (index >= 2)
  {
    assert(device->endpoints[index] == NULL);
    device->endpoints[index] = ep;
  }

  ep->size = size;

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    /* Allocate FIFO for IN endpoint */
    const size_t fifoPackets = number ? CONFIG_PLATFORM_USB_IN_BUFFERS : 1;
    const size_t fifoSize = (size * fifoPackets) > (DIEPTXF_INEPTXFD_MIN << 2) ?
        ((size * fifoPackets + 3) & ~3) : (DIEPTXF_INEPTXFD_MIN << 2);

    const uint32_t value = DIEPTXF_INEPTXSA(device->position >> 2)
        | DIEPTXF_INEPTXFD(fifoSize >> 2);

    if (number)
      reg->GLOBAL.DIEPTXF[number - 1] = value;
    else
      reg->GLOBAL.DIEPTXF0 = value;

    device->position += fifoSize;
    assert(device->position <= device->base.memoryCapacity);
  }

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    uint32_t diepctl = DIEPCTL_EPTYP(type) | DIEPCTL_TXFNUM(number)
        | DIEPCTL_SNAK;

    if (number)
    {
      assert(size <= DIEPCTL_MPSIZ_MASK);
      diepctl |= DIEPCTL_MPSIZ(size) | DIEPCTL_USBAEP | DIEPCTL_SD0PID;
    }
    else
    {
      diepctl |= DIEPCTL0_MPSIZ(encodeEp0Size(size));
    }

    reg->DEV_EP_IN[number].DIEPTSIZ = 0;
    reg->DEV_EP_IN[number].DIEPCTL = diepctl;
    reg->DEV.DAINTMSK |= DAINTMSK_IEPM(number);
  }
  else
  {
    uint32_t doepctl = DOEPCTL_EPTYP(type) | DOEPCTL_SNAK | DOEPCTL_EPENA;
    uint32_t doeptsiz;

    if (number)
    {
      assert(size <= DOEPCTL_MPSIZ_MASK);
      doeptsiz = DOEPTSIZ_XFRSIZ(size) | DOEPTSIZ_PKTCNT(1);
      doepctl |= DOEPCTL_MPSIZ(size) | DOEPCTL_USBAEP | DOEPCTL_SD0PID;
    }
    else
    {
      doeptsiz = DOEPTSIZ0_XFRSIZ(size) | DOEPTSIZ0_PKTCNT
          | DOEPTSIZ0_STUPCNT(1);
      doepctl |= DOEPCTL0_MPSIZ(encodeEp0Size(size));
    }

    reg->DEV_EP_OUT[number].DOEPTSIZ = doeptsiz;
    reg->DEV_EP_OUT[number].DOEPCTL = doepctl;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result epEnqueue(void *object, struct UsbRequest *request)
{
  assert(request != NULL);
  assert(request->callback != NULL);

  struct UsbEndpoint * const ep = object;
  return ep->subclass->enqueue(ep, request);
}
/*----------------------------------------------------------------------------*/
static bool epIsStalled(void *object)
{
  const struct UsbEndpoint * const ep = object;
  const STM_USB_OTG_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  if (ep->address & USB_EP_DIRECTION_IN)
    return (reg->DEV_EP_IN[number].DIEPCTL & DIEPCTL_STALL) != 0;
  else
    return (reg->DEV_EP_OUT[number].DOEPCTL & DOEPCTL_STALL) != 0;
}
/*----------------------------------------------------------------------------*/
static void epSetStalled(void *object, bool stalled)
{
  struct UsbEndpoint * const ep = object;
  STM_USB_OTG_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    uint32_t value = reg->DEV_EP_IN[number].DIEPCTL & ~DIEPCTL_STALL;

    if (stalled)
      value |= DIEPCTL_STALL;
    if (!number)
      value |= DIEPCTL_SD0PID;

    reg->DEV_EP_IN[number].DIEPCTL = value;
  }
  else
  {
    uint32_t value = reg->DEV_EP_OUT[number].DOEPCTL & ~DOEPCTL_STALL;

    if (stalled)
      value |= DOEPCTL_STALL;
    if (!number)
      value |= DOEPCTL_SD0PID;

    reg->DEV_EP_OUT[number].DOEPCTL = value;
  }
}
