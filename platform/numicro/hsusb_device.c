/*
 * hsusb_device.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/delay.h>
#include <halm/generic/pointer_queue.h>
#include <halm/platform/numicro/hsusb_base.h>
#include <halm/platform/numicro/hsusb_defs.h>
#include <halm/platform/numicro/hsusb_device.h>
#include <halm/usb/usb_control.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
#include <assert.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#define EP_COUNT 12
/*----------------------------------------------------------------------------*/
struct ControlUsbEndpointConfig
{
  /** Mandatory: hardware device. */
  struct UsbDevice *parent;
  /** Mandatory: logical address of the endpoint. */
  uint8_t address;
};

struct ControlUsbEndpoint
{
  struct UsbEndpointBase base;

  /* Parent device */
  struct UsbDevice *device;
  /* Queued requests */
  PointerQueue requests;
  /* Logical address */
  uint8_t address;
};

struct UsbEndpointConfig
{
  /** Mandatory: hardware device. */
  struct UsbDevice *parent;
  /** Mandatory: logical address of the endpoint. */
  uint8_t address;
};

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
  /* Enable manual size validation for all packets */
  bool manual;
};

struct UsbDevice
{
  struct UsbBase base;

  /* Control IN endpoint */
  struct ControlUsbEndpoint controlEpIn;
  /* Control OUT endpoint */
  struct ControlUsbEndpoint controlEpOut;
  /* Array of registered endpoints */
  struct UsbEndpoint *endpoints[EP_COUNT];
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
#  define devDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct UsbDeviceClass * const HsUsbDevice =
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
static inline NM_HSUSBD_EP_Type *epGetChannel(struct UsbEndpoint *);
static void epHandlePacket(struct UsbEndpoint *);
static void epHandleControlPacket(struct ControlUsbEndpoint *);
static void epHandleSetupPacket(struct ControlUsbEndpoint *);
static bool epReadData(struct UsbEndpoint *, uint8_t *, size_t, size_t *);
static bool epReadControlData(struct ControlUsbEndpoint *, uint8_t *, size_t,
    size_t *);
static void epReadPacketData(void *, volatile const void *, size_t);
static void epWriteData(struct UsbEndpoint *, const uint8_t *, size_t);
static void epWriteControlData(struct ControlUsbEndpoint *, const uint8_t *,
    size_t);
static void epWritePacketData(volatile void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
static enum Result controlEpInit(void *, const void *);
static void controlEpDeinit(void *);
static void controlEpClear(void *);
static void controlEpDisable(void *);
static void controlEpEnable(void *, uint8_t, uint16_t);
static enum Result controlEpEnqueue(void *, struct UsbRequest *);
static bool controlEpIsStalled(void *);
static void controlEpSetStalled(void *, bool);

static enum Result dataEpInit(void *, const void *);
static void dataEpDeinit(void *);
static void dataEpClear(void *);
static void dataEpDisable(void *);
static void dataEpEnable(void *, uint8_t, uint16_t);
static enum Result dataEpEnqueue(void *, struct UsbRequest *);
static bool dataEpIsStalled(void *);
static void dataEpSetStalled(void *, bool);
/*----------------------------------------------------------------------------*/
static const struct UsbEndpointClass * const ControlUsbEndpoint =
    &(const struct UsbEndpointClass){
    .size = sizeof(struct UsbEndpoint),
    .init = controlEpInit,
    .deinit = controlEpDeinit,

    .clear = controlEpClear,
    .disable = controlEpDisable,
    .enable = controlEpEnable,
    .enqueue = controlEpEnqueue,
    .isStalled = controlEpIsStalled,
    .setStalled = controlEpSetStalled
};

static const struct UsbEndpointClass * const DataUsbEndpoint =
    &(const struct UsbEndpointClass){
    .size = sizeof(struct UsbEndpoint),
    .init = dataEpInit,
    .deinit = dataEpDeinit,

    .clear = dataEpClear,
    .disable = dataEpDisable,
    .enable = dataEpEnable,
    .enqueue = dataEpEnqueue,
    .isStalled = dataEpIsStalled,
    .setStalled = dataEpSetStalled
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct UsbDevice * const device = object;
  NM_HSUSBD_Type * const reg = device->base.reg;
  const uint32_t intStatus = reg->BUSINTSTS;
  const uint32_t epStatus = reg->GINTSTS;

  reg->BUSINTSTS = intStatus;

  /* Device status interrupts */
  if (intStatus & BUSINTSTS_RSTIF)
  {
    resetDevice(device);
    usbControlNotify(device->control, USB_DEVICE_EVENT_RESET);
  }
  if (intStatus & BUSINTSTS_RESUMEIF)
  {
    usbControlNotify(device->control, USB_DEVICE_EVENT_RESUME);
  }
  if (intStatus & BUSINTSTS_SUSPENDIF)
  {
    usbControlNotify(device->control, USB_DEVICE_EVENT_SUSPEND);
  }
  if (intStatus & BUSINTSTS_HISPDIF)
  {
    usbControlNotify(device->control, USB_DEVICE_EVENT_PORT_CHANGE);
  }

  /* Control Endpoint interrupt */
  if (epStatus & GINTSTS_CEPIF)
  {
    const uint32_t controlIntEnable = reg->CEPINTEN;
    const uint32_t controlIntStatus = reg->CEPINTSTS;

    reg->CEPINTSTS = controlIntStatus;

    if ((controlIntEnable & CEPINTEN_SETUPPKIEN)
        && (controlIntStatus & CEPINTSTS_SETUPPKIF))
    {
      epHandleSetupPacket(&device->controlEpOut);
    }

    if ((controlIntEnable & CEPINTEN_RXPKIEN)
        && (controlIntStatus & CEPINTSTS_RXPKIF))
    {
      epHandleControlPacket(&device->controlEpOut);
    }

    if (controlIntStatus & CEPINTSTS_TXPKIF)
    {
      epHandleControlPacket(&device->controlEpIn);
    }

    if ((controlIntEnable & CEPINTEN_STSDONEIEN)
        && (controlIntStatus & CEPINTSTS_STSDONEIF))
    {
      reg->CEPINTEN &= ~CEPINTEN_STSDONEIEN;

      if (device->scheduledAddress != 0)
      {
        /*
         * Set a previously saved device address after the status stage
         * of the control transfer.
         */
        reg->FADDR = device->scheduledAddress;
        device->scheduledAddress = 0;
      }

      epHandleControlPacket(&device->controlEpIn);
    }
  }

  if (epStatus & GINTSTS_EPIF_MASK)
  {
    uint32_t status = reverseBits32(GINTEN_EPIEN_VALUE(epStatus));

    while (status)
    {
      const unsigned int index = countLeadingZeros32(status);

      epHandlePacket(device->endpoints[index]);
      status -= (1UL << 31) >> index;
    }
  }

  if (intStatus & BUSINTSTS_SOFIF)
  {
    // TODO SOF handling
    usbControlNotify(device->control, USB_DEVICE_EVENT_FRAME);
  }
}
/*----------------------------------------------------------------------------*/
static void resetDevice(struct UsbDevice *device)
{
  NM_HSUSBD_Type * const reg = device->base.reg;

  for (size_t index = 0; index < EP_COUNT; ++index)
  {
    reg->EP[index].EPCFG = 0;
    reg->EP[index].EPINTEN = 0;
    reg->EP[index].EPRSPCTL = EPRSPCTL_FLUSH;
  }

  /* Reset DMA */
  reg->DMACNT = 0;
  reg->DMACTL = DMACTL_DMARST;
  reg->DMACTL = 0;

  /* Reset buffer allocation */
  device->position = 0;

  devSetAddress(device, 0);

  /* Reset all enabled endpoints except for Control Endpoints */
  for (size_t index = 0; index < EP_COUNT; ++index)
  {
    struct UsbEndpoint **ep = &device->endpoints[index];

    if (*ep != NULL)
    {
      (*ep)->index = 0;
      *ep = NULL;
    }
  }
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
  const struct ControlUsbEndpointConfig controlEpConfig[] = {
      {
          .parent = object,
          .address = USB_EP_DIRECTION_IN
      }, {
          .parent = object,
          .address = 0
      }
  };
  const struct UsbControlConfig controlConfig = {
      .parent = object,
      .vid = config->vid,
      .pid = config->pid
  };
  struct UsbDevice * const device = object;
  enum Result res;

  /* Call base class constructor */
  res = HsUsbBase->init(device, &baseConfig);
  if (res != E_OK)
    return res;

  /* Initialize control endpoints, class descriptors should be set manually */
  ((struct Entity *)&device->controlEpIn)->descriptor = ControlUsbEndpoint;
  res = ControlUsbEndpoint->init(&device->controlEpIn, &controlEpConfig[0]);
  if (res != E_OK)
    return res;
  ((struct Entity *)&device->controlEpOut)->descriptor = ControlUsbEndpoint;
  res = ControlUsbEndpoint->init(&device->controlEpOut, &controlEpConfig[1]);
  if (res != E_OK)
    return res;

  device->base.handler = interruptHandler;
  device->scheduledAddress = 0;
  device->configured = false;
  device->enabled = false;

  for (size_t index = 0; index < EP_COUNT; ++index)
    device->endpoints[index] = NULL;

  /* Initialize control message handler after endpoint initialization */
  device->control = init(UsbControl, &controlConfig);
  if (device->control == NULL)
    return E_ERROR;

  /* Reset system variables and configure interrupts */
  NM_HSUSBD_Type * const reg = device->base.reg;

  /* Disable interrupts and clear pending interrupt flags */
  reg->GINTEN = 0;
  reg->BUSINTEN = 0;
  reg->BUSINTSTS = BUSINTSTS_MASK;

  reg->OPER = 0;
  reg->FADDR = 0;
  reg->PHYCTL = PHYCTL_PHYEN | PHYCTL_WKEN;

  for (size_t index = 0; index < EP_COUNT; ++index)
  {
    reg->EP[index].EPCFG = 0;
    reg->EP[index].EPINTEN = 0;
    reg->EP[index].EPRSPCTL = EPRSPCTL_FLUSH;
  }

#ifndef CONFIG_PLATFORM_USB_DEVICE_FORCE_FS
  reg->OPER |= OPER_HISPDEN;
#endif

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
  ControlUsbEndpoint->deinit(&device->controlEpOut);
  ControlUsbEndpoint->deinit(&device->controlEpIn);
  HsUsbBase->deinit(device);
}
#endif
/*----------------------------------------------------------------------------*/
static void *devCreateEndpoint(void *object, uint8_t address)
{
  struct UsbDevice * const device = object;
  struct UsbEndpointBase *ep;

  if (USB_EP_LOGICAL_ADDRESS(address) == 0)
  {
    if (address & USB_EP_DIRECTION_IN)
      ep = (struct UsbEndpointBase *)&device->controlEpIn;
    else
      ep = (struct UsbEndpointBase *)&device->controlEpOut;
  }
  else
  {
    const struct UsbEndpointConfig config = {
        .parent = device,
        .address = address
    };

    ep = init(DataUsbEndpoint, &config);
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
  device->scheduledAddress = address;

  if (address == 0)
  {
    NM_HSUSBD_Type * const reg = device->base.reg;
    reg->FADDR = address;
  }
}
/*----------------------------------------------------------------------------*/
static void devSetConnected(void *object, bool state)
{
  struct UsbDevice * const device = object;
  NM_HSUSBD_Type * const reg = device->base.reg;

  device->enabled = state;

  if (state)
  {
    reg->BUSINTSTS = BUSINTSTS_MASK;
    reg->BUSINTEN = BUSINTEN_RSTIEN | BUSINTEN_HISPDIEN
        | BUSINTEN_RESUMEIEN | BUSINTEN_SUSPENDIEN;

    reg->GINTEN = GINTEN_USBIEN | GINTEN_CEPIEN | GINTEN_EPIEN_MASK;
    reg->PHYCTL |= PHYCTL_DPPUEN | PHYCTL_PHYEN;
  }
  else
  {
    reg->BUSINTEN = 0;
    reg->GINTEN = 0;

    reg->PHYCTL &= ~(PHYCTL_DPPUEN | PHYCTL_PHYEN);
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
static enum UsbSpeed devGetSpeed(const void *object)
{
  const struct UsbDevice * const device = object;
  const NM_HSUSBD_Type * const reg = device->base.reg;

  return reg->OPER & OPER_CURSPD ? USB_HS : USB_FS;
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
static inline NM_HSUSBD_EP_Type *epGetChannel(struct UsbEndpoint *ep)
{
  return ((NM_HSUSBD_Type *)ep->device->base.reg)->EP + ep->index;
}
/*----------------------------------------------------------------------------*/
static void epHandlePacket(struct UsbEndpoint *ep)
{
  NM_HSUSBD_EP_Type * const channel = epGetChannel(ep);
  const bool error = (channel->EPINTSTS & EPINTSTS_ERRIF) != 0;

  channel->EPINTSTS = EPINTSTS_MASK;

  if (!pointerQueueEmpty(&ep->requests))
  {
    struct UsbRequest *req = pointerQueueFront(&ep->requests);

    if (ep->address & USB_EP_DIRECTION_IN)
    {
      pointerQueuePopFront(&ep->requests);
      req->callback(req->argument, req, error ?
          USB_REQUEST_ERROR : USB_REQUEST_COMPLETED);

      if (!pointerQueueEmpty(&ep->requests))
      {
        req = pointerQueueFront(&ep->requests);
        epWriteData(ep, req->buffer, req->length);
      }
    }
    else
    {
      enum UsbRequestStatus status = USB_REQUEST_ERROR;
      size_t read;

      if (!error && epReadData(ep, req->buffer, req->capacity, &read))
      {
        req->length = read;
        status = USB_REQUEST_COMPLETED;
      }

      pointerQueuePopFront(&ep->requests);
      req->callback(req->argument, req, status);

      if (pointerQueueEmpty(&ep->requests))
        channel->EPINTEN &= ~(EPINTEN_RXPKIEN | EPINTEN_SHORTRXIEN);
    }
  }
}
/*----------------------------------------------------------------------------*/
static void epHandleControlPacket(struct ControlUsbEndpoint *ep)
{
  if (pointerQueueEmpty(&ep->requests))
    return;

  struct UsbDevice * const device = ep->device;
  NM_HSUSBD_Type * const reg = device->base.reg;

  if (ep->address == USB_EP_DIRECTION_IN)
  {
    struct UsbRequest *req = pointerQueueFront(&ep->requests);
    pointerQueuePopFront(&ep->requests);

    req->callback(req->argument, req, USB_REQUEST_COMPLETED);

    if (!pointerQueueEmpty(&ep->requests))
    {
      req = pointerQueueFront(&ep->requests);
      epWriteControlData(ep, req->buffer, req->length);
    }
  }
  else
  {
    struct UsbRequest *req = pointerQueueFront(&ep->requests);
    size_t read;

    if (epReadControlData(ep, req->buffer, req->capacity, &read))
    {
      pointerQueuePopFront(&ep->requests);
      req->length = read;
      req->callback(req->argument, req, USB_REQUEST_COMPLETED);
    }

    /* Trigger reception of a next packet */
    if (pointerQueueEmpty(&ep->requests))
      reg->CEPINTEN &= ~(CEPINTEN_SETUPPKIEN | CEPINTEN_RXPKIEN);
  }

  reg->CEPCTL &= ~CEPCTL_NAKCLR;
}
/*----------------------------------------------------------------------------*/
static void epHandleSetupPacket(struct ControlUsbEndpoint *ep)
{
  NM_HSUSBD_Type * const reg = ep->device->base.reg;

  if (!pointerQueueEmpty(&ep->requests))
  {
    struct UsbRequest * const req = pointerQueueFront(&ep->requests);
    pointerQueuePopFront(&ep->requests);

    uint16_t * const buffer = req->buffer;

    /* Only 16 LSB are used from each SETUP register */
    for (size_t index = 0; index < ARRAY_SIZE(reg->SETUP); ++index)
      buffer[index] = reg->SETUP[index];

    req->length = sizeof(uint16_t) * ARRAY_SIZE(reg->SETUP);
    req->callback(req->argument, req, USB_REQUEST_SETUP);
  }

  reg->CEPCTL &= ~CEPCTL_NAKCLR;
}
/*----------------------------------------------------------------------------*/
static bool epReadData(struct UsbEndpoint *ep, uint8_t *buffer,
    size_t length, size_t *read)
{
  NM_HSUSBD_EP_Type * const channel = epGetChannel(ep);
  const uint32_t available = EPDATCNT_DATCNT_VALUE(channel->EPDATCNT);

  if (available <= length)
  {
    epReadPacketData(buffer, &channel->EPDAT, available);
    *read = available;

    return true;
  }
  else
  {
    /* Flush erroneous FIFO state */
    channel->EPRSPCTL |= EPRSPCTL_FLUSH;
    return false;
  }
}
/*----------------------------------------------------------------------------*/
static bool epReadControlData(struct ControlUsbEndpoint *ep, uint8_t *buffer,
    size_t length, size_t *read)
{
  const NM_HSUSBD_Type * const reg = ep->device->base.reg;
  const uint32_t available = reg->CEPRXCNT;

  if (available <= length)
  {
    epReadPacketData(buffer, &reg->CEPDAT, available);
    *read = available;

    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static void epReadPacketData(void *memory, volatile const void *peripheral,
    size_t length)
{
  uint32_t *destination = memory;
  const uint32_t * const end = destination + (length >> 2);

  while (destination != end)
    *destination++ = *(volatile const uint32_t *)peripheral;
  length &= 3;

  if (length)
  {
    uint8_t *bytes = (uint8_t *)destination;

    while (length--)
      *bytes++ = *(volatile const uint8_t *)peripheral;
  }
}
/*----------------------------------------------------------------------------*/
static void epWriteData(struct UsbEndpoint *ep, const uint8_t *buffer,
    size_t length)
{
  NM_HSUSBD_EP_Type * const channel = epGetChannel(ep);

  epWritePacketData(&channel->EPDAT, (const uint32_t *)buffer, length);

  if (ep->manual)
  {
    if (length)
      channel->EPTXCNT = length;
  }
  else
  {
    if (length % channel->EPMPS != 0)
      channel->EPRSPCTL |= EPRSPCTL_SHORTTXEN;
  }

  if (!length)
    channel->EPRSPCTL |= EPRSPCTL_ZEROLEN;
}
/*----------------------------------------------------------------------------*/
static void epWriteControlData(struct ControlUsbEndpoint *ep,
    const uint8_t *buffer, size_t length)
{
  struct UsbDevice * const device = ep->device;
  NM_HSUSBD_Type * const reg = device->base.reg;

  if (length)
  {
    epWritePacketData(&reg->CEPDAT, (const uint32_t *)buffer, length);
    reg->CEPTXCNT = length;
  }
  else
  {
    reg->CEPINTSTS = CEPINTSTS_STSDONEIF;
    reg->CEPINTEN |= CEPINTEN_STSDONEIEN;

    reg->CEPCTL |= CEPCTL_ZEROLEN;
  }
}
/*----------------------------------------------------------------------------*/
static void epWritePacketData(volatile void *peripheral, const void *memory,
    size_t length)
{
  /*
   * Errata for M480 series: word write mode is unsupported for
   * endpoints J, K, L. These endpoints should be used for OUT transfers only.
   */
  const uint32_t *source = memory;
  const uint32_t * const end = source + (length >> 2);

  while (source != end)
    *(volatile uint32_t *)peripheral = *source++;
  length &= 3;

  if (length)
  {
    const uint8_t *bytes = (uint8_t *)source;

    while (length--)
      *(volatile uint8_t *)peripheral = *bytes++;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result controlEpInit(void *object, const void *configBase)
{
  const struct ControlUsbEndpointConfig * const config = configBase;
  struct ControlUsbEndpoint * const ep = object;

  if (pointerQueueInit(&ep->requests, CONFIG_USB_DEVICE_CONTROL_REQUESTS))
  {
    ep->address = config->address;
    ep->device = config->parent;

    return E_OK;
  }
  else
    return E_MEMORY;
}
/*----------------------------------------------------------------------------*/
static void controlEpDeinit(void *object)
{
  struct ControlUsbEndpoint * const ep = object;

  /* Disable interrupts and remove pending requests */
  controlEpDisable(ep);
  controlEpClear(ep);

  assert(pointerQueueEmpty(&ep->requests));
  pointerQueueDeinit(&ep->requests);
}
/*----------------------------------------------------------------------------*/
static void controlEpClear(void *object)
{
  struct ControlUsbEndpoint * const ep = object;

  while (!pointerQueueEmpty(&ep->requests))
  {
    struct UsbRequest * const request = pointerQueueFront(&ep->requests);
    pointerQueuePopFront(&ep->requests);

    request->callback(request->argument, request, USB_REQUEST_CANCELLED);
  }
}
/*----------------------------------------------------------------------------*/
static void controlEpDisable(void *object)
{
  struct ControlUsbEndpoint * const ep = object;
  NM_HSUSBD_Type * const reg = ep->device->base.reg;

  reg->CEPINTEN = 0;
}
/*----------------------------------------------------------------------------*/
static void controlEpEnable(void *object, uint8_t type __attribute__((unused)),
    uint16_t size)
{
  struct ControlUsbEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;
  NM_HSUSBD_Type * const reg = device->base.reg;

  if (ep->address == 0)
  {
    /* Allocate buffer for Control OUT endpoint only, buffer is shared */
    reg->CEPBUFST = device->position;
    reg->CEPBUFEND = device->position + size - 1;

    // TODO Fix addresses 76 - 587 allocated instead of 64 - 575
    device->position += (size + (EPBUF_ADDR_ALIGNMENT - 1))
        & ~(EPBUF_ADDR_ALIGNMENT - 1);
  }

  reg->CEPCTL = CEPCTL_FLUSH;

  reg->CEPINTSTS = EPINTSTS_MASK;
  reg->CEPINTEN = CEPINTEN_TXPKIEN;
}
/*----------------------------------------------------------------------------*/
static enum Result controlEpEnqueue(void *object, struct UsbRequest *request)
{
  assert(request != NULL);
  assert(request->callback != NULL);

  struct ControlUsbEndpoint * const ep = object;
  const IrqState state = irqSave();
  enum Result res = E_FULL;

  if (!pointerQueueFull(&ep->requests))
  {
    if (ep->address & USB_EP_DIRECTION_IN)
    {
      /* Send packet when the queue is empty */
      if (pointerQueueEmpty(&ep->requests))
        epWriteControlData(ep, request->buffer, request->length);
    }
    else
    {
      /* Trigger reception when the queue is empty */
      if (pointerQueueEmpty(&ep->requests))
      {
        NM_HSUSBD_Type * const reg = ep->device->base.reg;
        reg->CEPINTEN |= CEPINTEN_SETUPPKIEN | CEPINTEN_RXPKIEN;
      }
    }

    pointerQueuePushBack(&ep->requests, request);
    res = E_OK;
  }

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
static bool controlEpIsStalled(void *object)
{
  const struct ControlUsbEndpoint * const ep = object;
  const NM_HSUSBD_Type * const reg = ep->device->base.reg;

  return (reg->CEPCTL & CEPCTL_STALLEN) != 0;
}
/*----------------------------------------------------------------------------*/
static void controlEpSetStalled(void *object, bool stalled)
{
  struct ControlUsbEndpoint * const ep = object;
  NM_HSUSBD_Type * const reg = ep->device->base.reg;

  if (stalled)
    reg->CEPCTL = (reg->CEPCTL & ~CEPCTL_NAKCLR) | CEPCTL_STALLEN;
  else
    reg->CEPCTL = reg->CEPCTL & ~(CEPCTL_NAKCLR | CEPCTL_STALLEN);

  // TODO
  // /* Write pending IN request to the endpoint buffer */
  // if (!stalled && (ep->address & USB_EP_DIRECTION_IN)
  //     && !pointerQueueEmpty(&ep->requests))
  // {
  //   struct UsbRequest * const request = pointerQueueFront(&ep->requests);
  //   epWriteData(ep, request->buffer, request->length);
  // }
}
/*----------------------------------------------------------------------------*/
static enum Result dataEpInit(void *object, const void *configBase)
{
  const struct UsbEndpointConfig * const config = configBase;
  struct UsbEndpoint * const ep = object;

  if (pointerQueueInit(&ep->requests, CONFIG_PLATFORM_USB_DEVICE_EP_REQUESTS))
  {
    ep->address = config->address;
    ep->device = config->parent;
    ep->index = 0;
    ep->manual = true;

    return E_OK;
  }
  else
    return E_MEMORY;
}
/*----------------------------------------------------------------------------*/
static void dataEpDeinit(void *object)
{
  struct UsbEndpoint * const ep = object;

  /* Disable interrupts and remove pending requests */
  dataEpDisable(ep);
  dataEpClear(ep);

  assert(pointerQueueEmpty(&ep->requests));
  pointerQueueDeinit(&ep->requests);
}
/*----------------------------------------------------------------------------*/
static void dataEpClear(void *object)
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
static void dataEpDisable(void *object)
{
  struct UsbEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;
  NM_HSUSBD_EP_Type * const channel = epGetChannel(ep);

  channel->EPCFG &= ~EPCFG_EPEN;
  channel->EPINTEN = 0;

  assert(device->endpoints[ep->index] == ep);
  device->endpoints[ep->index] = NULL;

  ep->index = 0;
}
/*----------------------------------------------------------------------------*/
static void dataEpEnable(void *object, uint8_t type, uint16_t size)
{
  struct UsbEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;
  const IrqState state = irqSave();

  /* Allocate free endpoint */
  assert(!ep->index);

  while (ep->index < EP_COUNT)
  {
    struct UsbEndpoint * const current = device->endpoints[ep->index];

    if (current == NULL)
      break;

    ++ep->index;
  }

  assert(ep->index != EP_COUNT);
  device->endpoints[ep->index] = ep;
  irqRestore(state);

  NM_HSUSBD_EP_Type * const channel = epGetChannel(ep);

  channel->EPMPS = size;
  channel->EPBUFST = device->position;
  channel->EPBUFEND = device->position + size - 1;

  device->position += (size + (EPBUF_ADDR_ALIGNMENT - 1))
      & ~(EPBUF_ADDR_ALIGNMENT - 1);

  uint32_t epcfg = EPCFG_EPEN | EPCFG_EPNUM(EP_TO_NUMBER(ep->address));
  uint8_t eptype;
  uint8_t validation;

  switch (type)
  {
    case ENDPOINT_TYPE_INTERRUPT:
      eptype = EPTYPE_INTERRUPT;
      validation = MODE_MANUAL_VALIDATE;
      break;

    case ENDPOINT_TYPE_ISOCHRONOUS:
      eptype = EPTYPE_ISOCHRONOUS;
      validation = MODE_AUTO_VALIDATE;
      break;

    default:
      eptype = EPTYPE_BULK;
//      validation = MODE_AUTO_VALIDATE; // TODO Auto-validation mode
      validation = MODE_MANUAL_VALIDATE;
      break;
  }
  ep->manual = validation != MODE_AUTO_VALIDATE;

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    epcfg |= EPCFG_EPDIR;

    channel->EPINTSTS = EPINTSTS_MASK;
    channel->EPINTEN = EPINTEN_SHORTTXIEN | EPINTEN_TXPKIEN | EPINTEN_ERRIEN;
  }
  else
  {
    channel->EPINTSTS = EPINTSTS_MASK;
    channel->EPINTEN = EPINTEN_ERRIEN;
  }

  channel->EPRSPCTL = EPRSPCTL_FLUSH | EPRSPCTL_TOGGLE
      | EPRSPCTL_MODE(validation);
  channel->EPCFG = epcfg | EPCFG_EPTYPE(eptype);
}
/*----------------------------------------------------------------------------*/
static enum Result dataEpEnqueue(void *object, struct UsbRequest *request)
{
  assert(request != NULL);
  assert(request->callback != NULL);

  struct UsbEndpoint * const ep = object;
  NM_HSUSBD_EP_Type * const channel = epGetChannel(ep);
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
        channel->EPINTEN = EPINTEN_RXPKIEN | EPINTEN_SHORTRXIEN;
    }

    pointerQueuePushBack(&ep->requests, request);
    res = E_OK;
  }

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
static bool dataEpIsStalled(void *object)
{
  struct UsbEndpoint * const ep = object;
  const NM_HSUSBD_EP_Type * const channel = epGetChannel(ep);

  return (channel->EPRSPCTL & EPRSPCTL_HALT) != 0;
}
/*----------------------------------------------------------------------------*/
static void dataEpSetStalled(void *object, bool stalled)
{
  struct UsbEndpoint * const ep = object;
  NM_HSUSBD_EP_Type * const channel = epGetChannel(ep);
  // const bool in = (ep->address & USB_EP_DIRECTION_IN) != 0;

  if (stalled)
    channel->EPRSPCTL |= EPRSPCTL_HALT;
  else
    channel->EPRSPCTL &= ~EPRSPCTL_HALT;

  // TODO Is flushing needed during unstall?
  // if (!stalled)
  // {
  //   if (in)
  //   {
  //     /* Flush TX FIFO */
  //     channel->EPRSPCTL |= EPRSPCTL_FLUSH;
  //     while (channel->EPRSPCTL & EPRSPCTL_FLUSH);
  //   }

  //   channel->EPRSPCTL &= ~EPRSPCTL_HALT;

  //   /* Write pending IN request to the endpoint buffer */
  //   if (in && !pointerQueueEmpty(&ep->requests))
  //   {
  //     struct UsbRequest * const request = pointerQueueFront(&ep->requests);
  //     epWriteData(ep, request->buffer, request->length);
  //   }
  // }
  // else
  //   channel->EPRSPCTL |= EPRSPCTL_HALT;
}
