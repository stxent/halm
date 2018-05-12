/*
 * usb_device.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <string.h>
#include <xcore/containers/queue.h>
#include <halm/platform/stm/stm32f1xx/usb_base.h>
#include <halm/platform/stm/stm32f1xx/usb_defs.h>
#include <halm/platform/stm/stm32f1xx/usb_helpers.h>
#include <halm/platform/stm/usb_device.h>
#include <halm/usb/usb_control.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
/*----------------------------------------------------------------------------*/
struct UsbSieEndpoint;

/* Endpoint subclass descriptor */
struct SieEndpointClass
{
  void (*enable)(struct UsbSieEndpoint *, uint8_t, uint16_t);
  void (*enqueue)(struct UsbSieEndpoint *, struct UsbRequest *);
  void (*isr)(struct UsbSieEndpoint *, bool);
};

struct UsbSieEndpoint
{
  struct UsbEndpoint base;

  /* Endpoint subclass */
  const struct SieEndpointClass *subclass;
  /* Parent device */
  struct UsbDevice *device;
  /* Queued requests */
  struct Queue requests;
  /* Logical address */
  uint8_t address;
  /* Pending transfers */
  uint8_t pending;
  /* First transfer indicator */
  bool first;
};

struct UsbDevice
{
  struct UsbBase base;

  /* Array of registered endpoints */
  struct UsbSieEndpoint *endpoints[16];
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
static enum Result devStringAppend(void *, struct UsbString);
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
static void epReadPacketMemory(void *, const void *, size_t);
static uint8_t epTypeToPlatformType(uint8_t);
static void epWritePacketMemory(void *, const void *, size_t);
static uint32_t sizeToNumBlocks(size_t);

static void dbEpEnable(struct UsbSieEndpoint *, uint8_t, uint16_t);
static void dbEpEnqueue(struct UsbSieEndpoint *, struct UsbRequest *);
static void dbEpHandler(struct UsbSieEndpoint *, bool);
static bool dbEpReadData(struct UsbSieEndpoint *, uint8_t *, size_t, size_t *);
static void dbEpSetBufferSize(struct UsbSieEndpoint *, size_t);
static void dbEpWriteData(struct UsbSieEndpoint *, const uint8_t *, size_t);

static void sbEpEnable(struct UsbSieEndpoint *, uint8_t, uint16_t);
static void sbEpEnqueue(struct UsbSieEndpoint *, struct UsbRequest *);
static void sbEpHandler(struct UsbSieEndpoint *, bool);
static bool sbEpReadData(struct UsbSieEndpoint *, uint8_t *, size_t, size_t *);
static void sbEpSetBufferSize(struct UsbSieEndpoint *, size_t);
static void sbEpWriteData(struct UsbSieEndpoint *, const uint8_t *, size_t);
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
static const struct SieEndpointClass * const DoubleBufferedEndpoint =
    &(const struct SieEndpointClass){
    .enable = dbEpEnable,
    .enqueue = dbEpEnqueue,
    .isr = dbEpHandler
};

static const struct SieEndpointClass * const SingleBufferedEndpoint =
    &(const struct SieEndpointClass){
    .enable = sbEpEnable,
    .enqueue = sbEpEnqueue,
    .isr = sbEpHandler
};

static const struct UsbEndpointClass * const UsbSieEndpoint =
    &(const struct UsbEndpointClass){
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
static void interruptHandler(void *object)
{
  struct UsbDevice * const device = object;
  STM_USB_Type * const reg = device->base.reg;
  const uint16_t intStatus = reg->ISTR;

  if (intStatus & ISTR_RESET)
  {
    reg->ISTR = ISTR_MASK & ~ISTR_RESET;
    resetDevice(device);
    usbControlEvent(device->control, USB_DEVICE_EVENT_RESET);
  }

  if (intStatus & ISTR_SUSP)
  {
    reg->ISTR = ISTR_MASK & ~ISTR_SUSP;
    if (!device->suspended)
      usbControlEvent(device->control, USB_DEVICE_EVENT_SUSPEND);
    device->suspended = true;
  }

  if (intStatus & ISTR_WKUP)
  {
    reg->ISTR = ISTR_MASK & ~ISTR_WKUP;
    if (device->suspended)
      usbControlEvent(device->control, USB_DEVICE_EVENT_RESUME);
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

    struct UsbSieEndpoint * const ep = device->endpoints[EP_TO_INDEX(id)];

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
  enum Result res;

  /* Call base class constructor */
  res = UsbBase->init(object, &baseConfig);
  if (res != E_OK)
    return res;

  device->base.handler = interruptHandler;
  device->position = DESCRIPTOR_TABLE_SIZE;
  device->scheduledAddress = 0;
  device->configured = false;
  device->enabled = false;
  device->suspended = false;
  memset(device->endpoints, 0, sizeof(device->endpoints));

  /* Initialize control message handler */
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

  struct UsbSieEndpoint *ep = 0;
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
  STM_USB_Type * const reg = device->base.reg;

  device->configured = address != 0;

  if (!address)
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
static void epReadPacketMemory(void *dst, const void *src, size_t length)
{
  /* Global address space, 32-bit words where the upper half is unused */
  const uint32_t *buffer = src;

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
static void epWritePacketMemory(void *dst, const void *src, size_t length)
{
  /* Local USB address space */
  const uint16_t *buffer = src;

  /* Global address space */
  uint32_t *start = dst;
  uint32_t * const end = start + (length >> 1);

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
static void dbEpEnable(struct UsbSieEndpoint *ep, uint8_t type, uint16_t size)
{
  STM_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);
  const unsigned int entry = calcDbEpEntry(number);

  volatile uint32_t * const eprReg = &reg->EPR[number];
  uint32_t value = (*eprReg & ~EPR_TOGGLE_MASK) | EPR_EP_KIND;
  value |= EPR_EA(number) | EPR_EP_TYPE(epTypeToPlatformType(type));

  ep->pending = 0;
  ep->first = true;

  *calcEpAddr(reg, entry) = ep->device->position;
  *calcEpAddr(reg, entry + 1) = ep->device->position + size;
  ep->device->position += size * 2;

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    /* DTOG_TX = 0, SW_BUF = 0 */
    *eprReg = eprMakeDtog(value, 0, EPR_DTOG_RX | EPR_DTOG_TX, 0);
    *eprReg = eprMakeTxStat(value, EPR_STAT_NAK) & ~EPR_CTR_MASK;
  }
  else
  {
    dbEpSetBufferSize(ep, size);
    /* DTOG_RX = 0, SW_BUF = 0 */
    *eprReg = eprMakeDtog(value, 0, EPR_DTOG_RX | EPR_DTOG_TX, 0);
    *eprReg = eprMakeRxStat(value, EPR_STAT_NAK) & ~EPR_CTR_MASK;
  }
}
/*----------------------------------------------------------------------------*/
static void dbEpEnqueue(struct UsbSieEndpoint *ep, struct UsbRequest *request)
{
  if (ep->address & USB_EP_DIRECTION_IN)
  {
    if (ep->pending < 2 && ep->pending == queueSize(&ep->requests))
    {
      dbEpWriteData(ep, request->buffer, request->length);
      ++ep->pending;
    }
  }
  else
  {
    if (queueEmpty(&ep->requests))
    {
      STM_USB_Type * const reg = ep->device->base.reg;
      const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

      if (ep->first)
      {
        ep->first = false;

        /* Start new double buffered sequence */
        reg->EPR[number] = eprMakeRxStat(reg->EPR[number], EPR_STAT_VALID);
      }
      else
      {
        /* Toggle software buffer selector */
        reg->EPR[number] = eprMakeDtog(reg->EPR[number], 0, 0, EPR_DTOG_TX);
      }
    }
  }

  queuePush(&ep->requests, &request);
}
/*----------------------------------------------------------------------------*/
static void dbEpHandler(struct UsbSieEndpoint *ep,
    bool setup __attribute__((unused)))
{
  if (queueEmpty(&ep->requests))
    return;

  STM_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    if (ep->pending > 1)
    {
      const uint32_t epr = reg->EPR[number];
      const uint32_t dtog = epr & (EPR_DTOG_RX | EPR_DTOG_TX);

      if (!dtog || !(dtog ^ (EPR_DTOG_RX | EPR_DTOG_TX)))
      {
        /* Start deferred transfer */
        reg->EPR[number] = eprMakeDtog(reg->EPR[number], 0, 0, EPR_DTOG_RX);
      }
    }

    struct UsbRequest *req;

    queuePop(&ep->requests, &req);
    req->callback(req->callbackArgument, req, USB_REQUEST_COMPLETED);
    --ep->pending;

    while (ep->pending < 2 && ep->pending < queueSize(&ep->requests))
    {
      req = *(struct UsbRequest **)queueAt(&ep->requests, ep->pending);
      dbEpWriteData(ep, req->buffer, req->length);
      ++ep->pending;
    }
  }
  else
  {
    struct UsbRequest *req;
    size_t read;

    queuePeek(&ep->requests, &req);

    if (dbEpReadData(ep, req->buffer, req->capacity, &read))
    {
      queuePop(&ep->requests, 0);
      req->length = read;
      req->callback(req->callbackArgument, req, USB_REQUEST_COMPLETED);
    }
  }
}
/*----------------------------------------------------------------------------*/
static bool dbEpReadData(struct UsbSieEndpoint *ep, uint8_t *buffer,
    size_t length, size_t *read)
{
  STM_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  const uint32_t epr = reg->EPR[number];
  const bool dtog = (epr & EPR_DTOG_RX) != 0;
  const bool swbuf = (epr & EPR_DTOG_TX) != 0;
  const unsigned int entry = calcDbEpEntry(number) + !dtog;
  const size_t available = COUNT_RX_VALUE(*calcEpCount(reg, entry));
  const bool ok = available <= length;

  if ((queueSize(&ep->requests) > 1 || !ok) && dtog == swbuf)
  {
    /* Switch buffers */
    reg->EPR[number] = eprMakeDtog(epr, 0, 0, EPR_DTOG_TX);
  }

  if (ok)
  {
    epReadPacketMemory(buffer, calcEpBuffer(reg, entry), available);
    *read = available;
  }

  return ok;
}
/*----------------------------------------------------------------------------*/
static void dbEpSetBufferSize(struct UsbSieEndpoint *ep, size_t size)
{
  assert(size > 0 && size <= 512);

  const unsigned int entry = calcDbEpEntry(USB_EP_LOGICAL_ADDRESS(ep->address));
  STM_USB_Type * const reg = ep->device->base.reg;
  const uint32_t numBlocks = sizeToNumBlocks(size);

  *calcEpCount(reg, entry) = numBlocks;
  *calcEpCount(reg, entry + 1) = numBlocks;
}
/*----------------------------------------------------------------------------*/
static void dbEpWriteData(struct UsbSieEndpoint *ep, const uint8_t *buffer,
    size_t length)
{
  STM_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);
  const unsigned int entry = calcDbEpEntry(number);

  if (!ep->pending)
  {
    const unsigned int current = (reg->EPR[number] & EPR_DTOG_RX) != 0;

    /* All buffers empty, use first buffer */
    epWritePacketMemory(calcEpBuffer(reg, entry + current), buffer, length);
    *calcEpCount(reg, entry + current) = length;

    if (ep->first)
    {
      ep->first = false;

      /* Workaround for the first packet */
      *calcEpCount(reg, entry + !current) = 0;
      /* Initiate first transaction in a double buffered sequence */
      reg->EPR[number] = eprMakeTxStat(reg->EPR[number], EPR_STAT_VALID);
    }
    else
    {
      const uint32_t epr = reg->EPR[number];
      const uint32_t dtog = epr & (EPR_DTOG_RX | EPR_DTOG_TX);

      if (!dtog || !(dtog ^ (EPR_DTOG_RX | EPR_DTOG_TX)))
      {
        /* Toggle software buffer selector */
        reg->EPR[number] = eprMakeDtog(reg->EPR[number], 0, 0, EPR_DTOG_RX);
      }
    }
  }
  else
  {
    const uint32_t epr = reg->EPR[number];
    const uint32_t dtog = (epr & EPR_DTOG_TX) != 0;
    const uint32_t swbuf = (epr & EPR_DTOG_RX) != 0;

    /* Workaround for the first packet */
    const unsigned int current = dtog != swbuf ? swbuf : 1;

    /* One of the buffers is already filled */
    epWritePacketMemory(calcEpBuffer(reg, entry + current), buffer, length);
    *calcEpCount(reg, entry + current) = length;
  }
}
/*----------------------------------------------------------------------------*/
static void sbEpEnable(struct UsbSieEndpoint *ep, uint8_t type, uint16_t size)
{
  STM_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  volatile uint32_t * const eprReg = &reg->EPR[number];
  uint32_t value = *eprReg & ~EPR_TOGGLE_MASK;
  value |= EPR_EA(number) | EPR_EP_TYPE(epTypeToPlatformType(type));

  /* Pending transfers number and first packet indicator are unused */

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    *calcEpAddr(reg, calcTxEpEntry(number)) = ep->device->position;

    *eprReg = eprMakeDtog(value, 0, EPR_DTOG_TX, 0);
    *eprReg = eprMakeTxStat(value, EPR_STAT_NAK) & ~EPR_CTR_TX;
  }
  else
  {
    *calcEpAddr(reg, calcRxEpEntry(number)) = ep->device->position;
    sbEpSetBufferSize(ep, size);

    *eprReg = eprMakeDtog(value, 0, EPR_DTOG_RX, 0);
    *eprReg = eprMakeRxStat(value, EPR_STAT_NAK) & ~EPR_CTR_RX;
  }

  ep->device->position += size;
}
/*----------------------------------------------------------------------------*/
static void sbEpEnqueue(struct UsbSieEndpoint *ep, struct UsbRequest *request)
{
  if (queueEmpty(&ep->requests))
  {
    if (ep->address & USB_EP_DIRECTION_IN)
    {
      sbEpWriteData(ep, request->buffer, request->length);
    }
    else
    {
      STM_USB_Type * const reg = ep->device->base.reg;
      const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

      reg->EPR[number] = eprMakeRxStat(reg->EPR[number], EPR_STAT_VALID);
    }
  }
  queuePush(&ep->requests, &request);
}
/*----------------------------------------------------------------------------*/
static void sbEpHandler(struct UsbSieEndpoint *ep, bool setup)
{
  if (queueEmpty(&ep->requests))
    return;

  STM_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    if (!number)
    {
      struct UsbDevice * const device = ep->device;

      if (device->scheduledAddress)
      {
        /*
         * Set a previously saved device address after the status stage
         * of the control transaction.
         */
        reg->DADDR = DADDR_ADD(device->scheduledAddress) | DADDR_EF;
        device->scheduledAddress = 0;
      }
    }

    struct UsbRequest *request;

    queuePop(&ep->requests, &request);
    request->callback(request->callbackArgument, request,
        USB_REQUEST_COMPLETED);

    if (!queueEmpty(&ep->requests))
    {
      queuePeek(&ep->requests, &request);
      sbEpWriteData(ep, request->buffer, request->length);
    }
  }
  else
  {
    struct UsbRequest *request;
    size_t read;

    queuePeek(&ep->requests, &request);

    if (sbEpReadData(ep, request->buffer, request->capacity, &read))
    {
      const enum UsbRequestStatus requestStatus = setup ?
          USB_REQUEST_SETUP : USB_REQUEST_COMPLETED;

      queuePop(&ep->requests, 0);
      request->length = read;
      request->callback(request->callbackArgument, request, requestStatus);
    }
  }
}
/*----------------------------------------------------------------------------*/
static bool sbEpReadData(struct UsbSieEndpoint *ep, uint8_t *buffer,
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

  if (!ok || !queueEmpty(&ep->requests))
    reg->EPR[number] = eprMakeRxStat(reg->EPR[number], EPR_STAT_VALID);

  return ok;
}
/*----------------------------------------------------------------------------*/
static void sbEpSetBufferSize(struct UsbSieEndpoint *ep, size_t size)
{
  assert(size > 0 && size <= 512);

  const unsigned int entry = calcRxEpEntry(USB_EP_LOGICAL_ADDRESS(ep->address));
  *calcEpCount(ep->device->base.reg, entry) = sizeToNumBlocks(size);
}
/*----------------------------------------------------------------------------*/
static void sbEpWriteData(struct UsbSieEndpoint *ep, const uint8_t *buffer,
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
  struct UsbSieEndpoint * const ep = object;

  const enum Result res = queueInit(&ep->requests, sizeof(struct UsbRequest *),
      CONFIG_PLATFORM_USB_DEVICE_EP_REQUESTS);

  if (res == E_OK)
  {
    ep->subclass = 0;
    ep->device = config->parent;
    ep->address = config->address;
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

  const IrqState state = irqSave();
  device->endpoints[EP_TO_INDEX(ep->address)] = 0;
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
  struct UsbSieEndpoint * const ep = object;

  switch (type)
  {
    case ENDPOINT_TYPE_BULK:
#ifdef CONFIG_PLATFORM_USB_DOUBLE_BUFFERING
      ep->subclass = DoubleBufferedEndpoint;
      break;
#endif

    default:
      ep->subclass = SingleBufferedEndpoint;
      break;
  }

  ep->subclass->enable(ep, type, size);
}
/*----------------------------------------------------------------------------*/
static enum Result epEnqueue(void *object, struct UsbRequest *request)
{
  assert(request->callback);

  struct UsbSieEndpoint * const ep = object;

  irqDisable(ep->device->base.irq);
  assert(!queueFull(&ep->requests));
  ep->subclass->enqueue(ep, request);
  irqEnable(ep->device->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static bool epIsStalled(void *object)
{
  const struct UsbSieEndpoint * const ep = object;
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
  struct UsbSieEndpoint * const ep = object;
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
