/*
 * usb_device.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <string.h>
#include <halm/generic/pointer_queue.h>
#include <halm/platform/stm/stm32f1xx/usb_base.h>
#include <halm/platform/stm/stm32f1xx/usb_defs.h>
#include <halm/platform/stm/stm32f1xx/usb_helpers.h>
#include <halm/platform/stm/usb_device.h>
#include <halm/usb/usb_control.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
/*----------------------------------------------------------------------------*/
struct UsbEndpoint;

/* Endpoint subclass descriptor */
struct SieEndpointClass
{
  void (*enable)(struct UsbEndpoint *, uint8_t, uint16_t);
  void (*enqueue)(struct UsbEndpoint *, struct UsbRequest *);
  void (*isr)(struct UsbEndpoint *, bool);
};

struct UsbEndpoint
{
  struct UsbEndpointBase base;

  /* Endpoint subclass */
  const struct SieEndpointClass *subclass;
  /* Parent device */
  struct UsbDevice *device;
  /* Queued requests */
  PointerQueue requests;
  /* Logical address */
  uint8_t address;
  /* Pending transfers */
  uint8_t pending;
  /**
   * Transfer state. There are 4 states for double-buffered IN transfers
   * and 3 states for double-buffered OUT transfers.
   * @n IN transfer states:
   *   - 3: initialization before sending the first packet.
   *   - 2: transmission of the first packet is started.
   *   - 1: transmission of the dummy packet is started.
   *   - 0: work state.
   * @n OUT transfer states:
   *   - 2: reception of the first packet.
   *   - 1: reception of the second packet.
   *   - 0: work state.
   */
  uint8_t state;
};

struct UsbDevice
{
  struct UsbBase base;

  /* Array of registered endpoints */
  struct UsbEndpoint *endpoints[16];
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
  /* Device is suspended */
  bool suspended;
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
static void devSetPower(void *, uint16_t);
static enum UsbSpeed devGetSpeed(const void *);
static UsbStringNumber devStringAppend(void *, struct UsbString);
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
static void epReadPacketMemory(void *, const volatile void *, size_t);
static uint8_t epTypeToPlatformType(uint8_t);
static void epWritePacketMemory(volatile void *, const void *, size_t);
static uint32_t sizeToNumBlocks(size_t);

static void dbEpEnable(struct UsbEndpoint *, uint8_t, uint16_t);
static void dbEpEnqueue(struct UsbEndpoint *, struct UsbRequest *);
static void dbEpHandler(struct UsbEndpoint *, bool);
static bool dbEpReadData(struct UsbEndpoint *, uint8_t *, size_t, size_t *);
static void dbEpSetBufferSize(struct UsbEndpoint *, size_t);
static void dbEpWriteFirstPacket(struct UsbEndpoint *, const uint8_t *, size_t);
static void dbEpWritePacket(struct UsbEndpoint *, const uint8_t *, size_t);

static void sbEpEnable(struct UsbEndpoint *, uint8_t, uint16_t);
static void sbEpEnqueue(struct UsbEndpoint *, struct UsbRequest *);
static void sbEpHandler(struct UsbEndpoint *, bool);
static bool sbEpReadData(struct UsbEndpoint *, uint8_t *, size_t, size_t *);
static void sbEpSetBufferSize(struct UsbEndpoint *, size_t);
static void sbEpWriteData(struct UsbEndpoint *, const uint8_t *, size_t);
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
static const struct SieEndpointClass * const DbUsbEndpoint =
    &(const struct SieEndpointClass){
    .enable = dbEpEnable,
    .enqueue = dbEpEnqueue,
    .isr = dbEpHandler
};

static const struct SieEndpointClass * const SbUsbEndpoint =
    &(const struct SieEndpointClass){
    .enable = sbEpEnable,
    .enqueue = sbEpEnqueue,
    .isr = sbEpHandler
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
static void interruptHandler(void *object)
{
  struct UsbDevice * const device = object;
  STM_USB_Type * const reg = device->base.reg;
  const uint16_t intStatus = reg->ISTR;

  if (intStatus & ISTR_RESET)
  {
    reg->ISTR = ISTR_MASK & ~ISTR_RESET;
    resetDevice(device);
    usbControlNotify(device->control, USB_DEVICE_EVENT_RESET);
  }

  if (intStatus & ISTR_SUSP)
  {
    reg->ISTR = ISTR_MASK & ~ISTR_SUSP;
    if (!device->suspended)
      usbControlNotify(device->control, USB_DEVICE_EVENT_SUSPEND);
    device->suspended = true;
  }

  if (intStatus & ISTR_WKUP)
  {
    reg->ISTR = ISTR_MASK & ~ISTR_WKUP;
    if (device->suspended)
      usbControlNotify(device->control, USB_DEVICE_EVENT_RESUME);
    device->suspended = false;
  }

  if (intStatus & ISTR_CTR)
  {
    unsigned int id = ISTR_EP_ID_VALUE(intStatus);
    volatile uint32_t * const eprReg = &reg->EPR[id];
    const uint32_t epr = *eprReg;
    const bool setup = (epr & EPR_SETUP) != 0;

    if (!(intStatus & ISTR_DIR))
      id |= USB_EP_DIRECTION_IN;

    struct UsbEndpoint * const ep = device->endpoints[EP_TO_INDEX(id)];

    if (intStatus & ISTR_DIR)
      *eprReg = (epr & EPR_TOGGLE_MASK) | EPR_CTR_TX;
    else
      *eprReg = (epr & EPR_TOGGLE_MASK) | EPR_CTR_RX;

    ep->subclass->isr(ep, setup);
  }
}
/*----------------------------------------------------------------------------*/
static void resetDevice(struct UsbDevice *device)
{
  STM_USB_Type * const reg = device->base.reg;

  /* Set inactive configuration */
  device->configured = false;
  device->suspended = false;
  device->scheduledAddress = 0;
  device->position = DESCRIPTOR_TABLE_SIZE;

  for (size_t i = 0; i < 8; ++i)
  {
    const uint32_t epr = reg->EPR[i];

    reg->EPR[i] = eprMakeRxStat(epr, EPR_STAT_DISABLED)
        | eprMakeTxStat(epr, EPR_STAT_DISABLED);
  }

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
  const enum Result res = UsbBase->init(object, &baseConfig);
  if (res != E_OK)
    return res;

  device->base.handler = interruptHandler;
  device->position = DESCRIPTOR_TABLE_SIZE;
  device->scheduledAddress = 0;
  device->configured = false;
  device->enabled = false;
  device->suspended = false;
  memset(device->endpoints, 0, sizeof(device->endpoints));

  /* Initialize control message handler after endpoint initialization */
  device->control = init(UsbControl, &controlConfig);
  if (!device->control)
    return E_ERROR;

  STM_USB_Type * const reg = device->base.reg;

  reg->CNTR = 0; // FIXME Look for correct sequence
  reg->BTABLE = 0;
  reg->ISTR = 0;
  reg->CNTR = CNTR_RESETM | CNTR_CTRM | CNTR_SUSPM | CNTR_WKUPM;

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

    device->endpoints[index] = init(UsbEndpoint, &config);
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
  STM_USB_Type * const reg = device->base.reg;

  device->configured = address != 0;

  if (address == 0)
    reg->DADDR = DADDR_ADD(address) | DADDR_EF;
  else
    device->scheduledAddress = address;
}
/*----------------------------------------------------------------------------*/
static void devSetConnected(void *object, bool state)
{
  struct UsbDevice * const device = object;

  device->enabled = state;
  usbSoftConnectionControl(object, state);
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
static UsbStringNumber devStringAppend(void *object, struct UsbString string)
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
static void epReadPacketMemory(void *dst, const volatile void *src,
    size_t length)
{
  /* Global address space, 32-bit words where the upper half is unused */
  const volatile uint32_t *buffer = src;

  /* Local USB address space, 16-bit words */
  uint16_t *start = dst;
  uint16_t * const end = start + (length >> 1);

  while (start < end)
    *start++ = *buffer++;

  if (length & 1)
    *(uint8_t *)start = (uint8_t)*buffer;
}
/*----------------------------------------------------------------------------*/
static uint8_t epTypeToPlatformType(uint8_t type)
{
  static const uint8_t epTypeTable[] = {
      [ENDPOINT_TYPE_CONTROL] = EPR_TYPE_CONTROL,
      [ENDPOINT_TYPE_ISOCHRONOUS] = EPR_TYPE_ISO,
      [ENDPOINT_TYPE_BULK] = EPR_TYPE_BULK,
      [ENDPOINT_TYPE_INTERRUPT] = EPR_TYPE_INTERRUPT
  };

  return epTypeTable[type];
}
/*----------------------------------------------------------------------------*/
static void epWritePacketMemory(volatile void *dst, const void *src,
    size_t length)
{
  /* Local USB address space */
  const uint16_t *buffer = src;

  /* Global address space */
  volatile uint32_t *start = dst;
  volatile uint32_t * const end = start + (length >> 1);

  while (start < end)
    *start++ = *buffer++;

  if (length & 1)
    *start = *(const uint8_t *)buffer;
}
/*----------------------------------------------------------------------------*/
static uint32_t sizeToNumBlocks(size_t size)
{
  if (size <= 62)
    return COUNT_RX_NUM_BLOCK((size + 1) >> 1);
  else
    return COUNT_RX_NUM_BLOCK((size + 31) >> 5) | COUNT_RX_BLSIZE;
}
/*----------------------------------------------------------------------------*/
static void dbEpEnable(struct UsbEndpoint *ep, uint8_t type, uint16_t size)
{
  struct UsbDevice * const device = ep->device;
  STM_USB_Type * const reg = device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  volatile uint32_t * const eprReg = &reg->EPR[number];
  uint32_t value = (*eprReg & ~EPR_TOGGLE_MASK) | EPR_EP_KIND;
  value |= EPR_EA(number) | EPR_EP_TYPE(epTypeToPlatformType(type));

  ep->pending = 0;

  const unsigned int entry = calcDbEpEntry(number);

  *calcEpAddr(reg, entry) = device->position;
  *calcEpAddr(reg, entry + 1) = device->position + size;
  device->position += size * 2;

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    ep->state = 3;

    /* DTOG_TX = 0, SW_BUF = 0 */
    *eprReg = eprMakeDtog(value, 0, EPR_DTOG_RX | EPR_DTOG_TX, 0);
    *eprReg = eprMakeTxStat(value, EPR_STAT_NAK) & ~EPR_CTR_MASK;
  }
  else
  {
    ep->state = 2;

    dbEpSetBufferSize(ep, size);
    /* DTOG_RX = 0, SW_BUF = 0 */
    *eprReg = eprMakeDtog(value, 0, EPR_DTOG_RX | EPR_DTOG_TX, 0);
    *eprReg = eprMakeRxStat(value, EPR_STAT_NAK) & ~EPR_CTR_MASK;
  }
}
/*----------------------------------------------------------------------------*/
static void dbEpEnqueue(struct UsbEndpoint *ep, struct UsbRequest *request)
{
  if (ep->address & USB_EP_DIRECTION_IN)
  {
    if (ep->pending < 2 && ep->pending == pointerQueueSize(&ep->requests))
    {
      if (ep->state == 3)
      {
        ep->state = 2;
        dbEpWriteFirstPacket(ep, request->buffer, request->length);
        ++ep->pending;
      }
      else if (ep->state == 0)
      {
        dbEpWritePacket(ep, request->buffer, request->length);
        ++ep->pending;
      }
    }
  }
  else
  {
    if (pointerQueueEmpty(&ep->requests))
    {
      STM_USB_Type * const reg = ep->device->base.reg;
      const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

      switch (ep->state)
      {
        case 2:
          /* Start new double buffered sequence */
          reg->EPR[number] = eprMakeRxStat(reg->EPR[number], EPR_STAT_VALID);
          ep->state = 1;
          break;

        case 1:
          ep->state = 0;
          break;

        case 0:
          /* Toggle software buffer pointer */
          reg->EPR[number] = eprMakeDtog(reg->EPR[number], 0, 0, EPR_DTOG_TX);
          break;
      }
    }
  }

  pointerQueuePushBack(&ep->requests, request);
}
/*----------------------------------------------------------------------------*/
static void dbEpHandler(struct UsbEndpoint *ep,
    bool setup __attribute__((unused)))
{
  STM_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    struct UsbRequest *req;

    if (ep->pending > 1)
    {
      /* Send previously written packet, the driver is in the work state */
      reg->EPR[number] = eprMakeDtog(reg->EPR[number], 0, 0, EPR_DTOG_RX);
    }

    if (ep->state == 1)
    {
      /* The mock packet has been sent, switch to a work state */
      ep->state = 0;
    }
    else
    {
      /* The first packet has been sent or the driver is in the work state */
      assert(!pointerQueueEmpty(&ep->requests));

      --ep->pending;
      req = pointerQueueFront(&ep->requests);
      pointerQueuePopFront(&ep->requests);
      req->callback(req->callbackArgument, req, USB_REQUEST_COMPLETED);

      if (ep->state == 2)
      {
        /* Wait for the transmission of the dummy packet */
        ep->state = 1;
      }
    }

    if (!ep->state)
    {
      /*
       * Write up to two packets into the packet memory,
       * state should be already changed.
       */
      while (ep->pending < 2 && ep->pending < pointerQueueSize(&ep->requests))
      {
        req = *pointerQueueAt(&ep->requests, ep->pending);
        dbEpWritePacket(ep, req->buffer, req->length);
        ++ep->pending;
      }
    }
  }
  else
  {
    assert(!pointerQueueEmpty(&ep->requests));

    struct UsbRequest * const req = pointerQueueFront(&ep->requests);
    size_t read;

    if (dbEpReadData(ep, req->buffer, req->capacity, &read))
    {
      pointerQueuePopFront(&ep->requests);
      req->length = read;
      req->callback(req->callbackArgument, req, USB_REQUEST_COMPLETED);
    }
  }
}
/*----------------------------------------------------------------------------*/
static bool dbEpReadData(struct UsbEndpoint *ep, uint8_t *buffer,
    size_t length, size_t *read)
{
  STM_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  const uint32_t epr = reg->EPR[number];
  const bool dtog = (epr & EPR_DTOG_RX) != 0;
  const unsigned int entry = calcDbEpEntry(number) + !dtog;
  const size_t available = COUNT_RX_VALUE(*calcEpCount(reg, entry));
  const bool ok = available <= length;

  if (pointerQueueSize(&ep->requests) > 1 || !ok)
  {
    if (!ep->state)
    {
      /* Switch buffers */
      reg->EPR[number] = eprMakeDtog(epr, 0, 0, EPR_DTOG_TX);
    }
    else
      ep->state = 0;
  }

  if (ok)
  {
    epReadPacketMemory(buffer, calcEpBuffer(reg, entry), available);
    *read = available;
  }

  return ok;
}
/*----------------------------------------------------------------------------*/
static void dbEpSetBufferSize(struct UsbEndpoint *ep, size_t size)
{
  assert(size > 0 && size <= 512);

  const unsigned int entry = calcDbEpEntry(USB_EP_LOGICAL_ADDRESS(ep->address));
  STM_USB_Type * const reg = ep->device->base.reg;
  const uint32_t numBlocks = sizeToNumBlocks(size);

  *calcEpCount(reg, entry) = numBlocks;
  *calcEpCount(reg, entry + 1) = numBlocks;
}
/*----------------------------------------------------------------------------*/
static void dbEpWriteFirstPacket(struct UsbEndpoint *ep,
    const uint8_t *buffer, size_t length)
{
  STM_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);
  const unsigned int entry = calcDbEpEntry(number);

  /* All buffers empty, use first buffer */
  epWritePacketMemory(calcEpBuffer(reg, entry), buffer, length);
  *calcEpCount(reg, entry) = length;

  /* Workaround for the unconditional sending of the second packet */
  *calcEpCount(reg, entry + 1) = 0;
  /* Initiate first transaction in a double buffered sequence */
  reg->EPR[number] = eprMakeTxStat(reg->EPR[number], EPR_STAT_VALID);
}
/*----------------------------------------------------------------------------*/
static void dbEpWritePacket(struct UsbEndpoint *ep,
    const uint8_t *buffer, size_t length)
{
  STM_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);
  const unsigned int current = (reg->EPR[number] & EPR_DTOG_RX) != 0;
  const unsigned int entry = calcDbEpEntry(number) + current;

  if (!ep->pending)
  {
    /* All buffers empty, use first buffer */
    epWritePacketMemory(calcEpBuffer(reg, entry), buffer, length);
    *calcEpCount(reg, entry) = length;

    /* Toggle software buffer pointer */
    reg->EPR[number] = eprMakeDtog(reg->EPR[number], 0, 0, EPR_DTOG_RX);
  }
  else
  {
    /* One of the buffers is already filled */
    epWritePacketMemory(calcEpBuffer(reg, entry), buffer, length);
    *calcEpCount(reg, entry) = length;
  }
}
/*----------------------------------------------------------------------------*/
static void sbEpEnable(struct UsbEndpoint *ep, uint8_t type, uint16_t size)
{
  struct UsbDevice * const device = ep->device;
  STM_USB_Type * const reg = device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  volatile uint32_t * const eprReg = &reg->EPR[number];
  uint32_t value = *eprReg & ~EPR_TOGGLE_MASK;
  value |= EPR_EA(number) | EPR_EP_TYPE(epTypeToPlatformType(type));

  /* Transfer state is not used in single buffer mode */
  ep->pending = 0;

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    *calcEpAddr(reg, calcTxEpEntry(number)) = device->position;

    *eprReg = eprMakeDtog(value, 0, EPR_DTOG_TX, 0);
    *eprReg = eprMakeTxStat(value, EPR_STAT_NAK) & ~EPR_CTR_TX;
  }
  else
  {
    *calcEpAddr(reg, calcRxEpEntry(number)) = device->position;
    sbEpSetBufferSize(ep, size);

    *eprReg = eprMakeDtog(value, 0, EPR_DTOG_RX, 0);
    *eprReg = eprMakeRxStat(value, EPR_STAT_NAK) & ~EPR_CTR_RX;
  }

  device->position += size;
}
/*----------------------------------------------------------------------------*/
static void sbEpEnqueue(struct UsbEndpoint *ep, struct UsbRequest *request)
{
  if (pointerQueueEmpty(&ep->requests))
  {
    if (!(ep->address & USB_EP_DIRECTION_IN))
    {
      STM_USB_Type * const reg = ep->device->base.reg;
      const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

      reg->EPR[number] = eprMakeRxStat(reg->EPR[number], EPR_STAT_VALID);
    }
    else if (!ep->pending)
    {
      sbEpWriteData(ep, request->buffer, request->length);
      ++ep->pending;
    }
  }

  pointerQueuePushBack(&ep->requests, request);
}
/*----------------------------------------------------------------------------*/
static void sbEpHandler(struct UsbEndpoint *ep, bool setup)
{
  if (pointerQueueEmpty(&ep->requests))
    return;

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    if (USB_EP_LOGICAL_ADDRESS(ep->address) == 0)
    {
      struct UsbDevice * const device = ep->device;

      if (device->scheduledAddress != 0)
      {
        STM_USB_Type * const reg = ep->device->base.reg;

        /*
         * Set a previously saved device address after the status stage
         * of the control transaction.
         */
        reg->DADDR = DADDR_ADD(device->scheduledAddress) | DADDR_EF;
        device->scheduledAddress = 0;
      }
    }

    struct UsbRequest *req = pointerQueueFront(&ep->requests);
    pointerQueuePopFront(&ep->requests);

    req->callback(req->callbackArgument, req, USB_REQUEST_COMPLETED);
    --ep->pending;

    if (!pointerQueueEmpty(&ep->requests))
    {
      req = pointerQueueFront(&ep->requests);
      sbEpWriteData(ep, req->buffer, req->length);
      ++ep->pending;
    }
  }
  else
  {
    struct UsbRequest * const req = pointerQueueFront(&ep->requests);
    size_t read;

    if (sbEpReadData(ep, req->buffer, req->capacity, &read))
    {
      const enum UsbRequestStatus requestStatus = setup ?
          USB_REQUEST_SETUP : USB_REQUEST_COMPLETED;

      pointerQueuePopFront(&ep->requests);
      req->length = read;
      req->callback(req->callbackArgument, req, requestStatus);
    }
  }
}
/*----------------------------------------------------------------------------*/
static bool sbEpReadData(struct UsbEndpoint *ep, uint8_t *buffer,
    size_t length, size_t *read)
{
  STM_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);
  const unsigned int entry = calcRxEpEntry(number);
  const size_t available = COUNT_RX_VALUE(*calcEpCount(reg, entry));
  const bool ok = available <= length;

  if (ok)
  {
    epReadPacketMemory(buffer, calcEpBuffer(reg, entry), available);
    *read = available;
  }

  if (pointerQueueSize(&ep->requests) > 1 || !ok)
    reg->EPR[number] = eprMakeRxStat(reg->EPR[number], EPR_STAT_VALID);

  return ok;
}
/*----------------------------------------------------------------------------*/
static void sbEpSetBufferSize(struct UsbEndpoint *ep, size_t size)
{
  assert(size > 0 && size <= 512);

  const unsigned int entry = calcRxEpEntry(USB_EP_LOGICAL_ADDRESS(ep->address));
  *calcEpCount(ep->device->base.reg, entry) = sizeToNumBlocks(size);
}
/*----------------------------------------------------------------------------*/
static void sbEpWriteData(struct UsbEndpoint *ep, const uint8_t *buffer,
    size_t length)
{
  STM_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);
  const unsigned int entry = calcTxEpEntry(number);

  epWritePacketMemory(calcEpBuffer(reg, entry), buffer, length);
  *calcEpCount(reg, entry) = length;
  reg->EPR[number] = eprMakeTxStat(reg->EPR[number], EPR_STAT_VALID);
}
/*----------------------------------------------------------------------------*/
static enum Result epInit(void *object, const void *configBase)
{
  const struct UsbEndpointConfig * const config = configBase;
  struct UsbEndpoint * const ep = object;

  if (pointerQueueInit(&ep->requests, CONFIG_PLATFORM_USB_DEVICE_EP_REQUESTS))
  {
    ep->subclass = 0;
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

  /* Disable interrupts and remove pending requests */
  epDisable(ep);
  epClear(ep);

  const IrqState state = irqSave();
  device->endpoints[EP_TO_INDEX(ep->address)] = 0;
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
    struct UsbRequest * const req = pointerQueueFront(&ep->requests);
    pointerQueuePopFront(&ep->requests);

    req->callback(req->callbackArgument, req, USB_REQUEST_CANCELLED);
  }
}
/*----------------------------------------------------------------------------*/
static void epDisable(void *object)
{
  struct UsbEndpoint * const ep = object;
  STM_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);
  uint32_t epr = reg->EPR[number] & (EPR_TOGGLE_MASK & ~EPR_EP_KIND);

  if (ep->address & USB_EP_DIRECTION_IN)
    epr = eprMakeTxStat(epr, EPR_STAT_DISABLED);
  else
    epr = eprMakeRxStat(epr, EPR_STAT_DISABLED);

  reg->EPR[number] = epr;
}
/*----------------------------------------------------------------------------*/
static void epEnable(void *object, uint8_t type, uint16_t size)
{
  struct UsbEndpoint * const ep = object;

  switch (type)
  {
    case ENDPOINT_TYPE_ISOCHRONOUS:
    case ENDPOINT_TYPE_BULK:
#ifdef CONFIG_PLATFORM_USB_DOUBLE_BUFFERING
      ep->subclass = DbUsbEndpoint;
      break;
#endif

    default:
      ep->subclass = SbUsbEndpoint;
      break;
  }

  ep->subclass->enable(ep, type, size);
}
/*----------------------------------------------------------------------------*/
static enum Result epEnqueue(void *object, struct UsbRequest *request)
{
  assert(request->callback);

  struct UsbEndpoint * const ep = object;

  irqDisable(ep->device->base.irq);
  assert(!pointerQueueFull(&ep->requests));
  ep->subclass->enqueue(ep, request);
  irqEnable(ep->device->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static bool epIsStalled(void *object)
{
  const struct UsbEndpoint * const ep = object;
  const STM_USB_Type * const reg = ep->device->base.reg;
  const uint32_t epr = reg->EPR[USB_EP_LOGICAL_ADDRESS(ep->address)];

  if (ep->address & USB_EP_DIRECTION_IN)
    return EPR_STAT_TX_VALUE(epr) == EPR_STAT_STALL;
  else
    return EPR_STAT_RX_VALUE(epr) == EPR_STAT_STALL;
}
/*----------------------------------------------------------------------------*/
static void epSetStalled(void *object, bool stalled)
{
  struct UsbEndpoint * const ep = object;
  STM_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);
  volatile uint32_t * const eprReg = &reg->EPR[number];
  const uint32_t epr = *eprReg;

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    if (!stalled)
      *eprReg = eprMakeDtog(epr, 0, EPR_DTOG_TX, 0);

    *eprReg = eprMakeTxStat(epr, stalled ? EPR_STAT_STALL : EPR_STAT_NAK);
  }
  else
  {
    if (!stalled)
      *eprReg = eprMakeDtog(epr, 0, EPR_DTOG_RX, 0);

    *eprReg = eprMakeRxStat(epr, stalled ? EPR_STAT_STALL : EPR_STAT_NAK);
  }
}
