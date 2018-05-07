/*
 * usb_device.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <string.h>
#include <xcore/containers/queue.h>
#include <xcore/memory.h>
#include <halm/delay.h>
#include <halm/platform/stm/stm32f1xx/usb_base.h>
#include <halm/platform/stm/stm32f1xx/usb_defs.h>
#include <halm/platform/stm/stm32f1xx/usb_helpers.h>
#include <halm/platform/stm/usb_device.h>
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
static void epHandler(struct UsbSieEndpoint *, bool);
static enum Result epReadData(struct UsbSieEndpoint *, uint8_t *,
    size_t, size_t *);
static void epReadPacketMemory(void *, const void *, size_t);
static void epWriteData(struct UsbSieEndpoint *, const uint8_t *, size_t);
static void epWritePacketMemory(void *, const void *, size_t);
static void setRxEpBufferSize(struct UsbSieEndpoint *, unsigned int, size_t);
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
    unsigned int ep = ISTR_EP_ID_VALUE(intStatus);
    const unsigned int number = ep;

    if (!(intStatus & ISTR_DIR))
      ep |= USB_EP_DIRECTION_IN;

    const bool setup = (reg->EPR[number] & EPR_SETUP) != 0;

    if (!(intStatus & ISTR_DIR))
    {
      reg->EPR[number] = (reg->EPR[number] & (EPR_TOGGLE_MASK & ~EPR_CTR_TX))
          | EPR_CTR_RX;
    }
    else
    {
      reg->EPR[number] = (reg->EPR[number] & (EPR_TOGGLE_MASK & ~EPR_CTR_RX))
          | EPR_CTR_TX;
    }

    epHandler((struct UsbSieEndpoint *)device->endpoints[EP_TO_INDEX(ep)],
        setup);
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
    changeTxStat(&reg->EPR[i], EPR_STAT_DISABLED);
    changeRxStat(&reg->EPR[i], EPR_STAT_DISABLED);
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
static void epHandler(struct UsbSieEndpoint *ep, bool setup)
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
      epWriteData(ep, request->buffer, request->length);
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

      if (!queueEmpty(&ep->requests))
        changeRxStat(&reg->EPR[number], EPR_STAT_VALID);

      const enum UsbRequestStatus requestStatus = setup ?
          USB_REQUEST_SETUP : USB_REQUEST_COMPLETED;

      request->length = read;
      request->callback(request->callbackArgument, request, requestStatus);
    }
    else
    {
      /* The queue contains at least one request */
      changeRxStat(&reg->EPR[number], EPR_STAT_VALID);
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum Result epReadData(struct UsbSieEndpoint *ep, uint8_t *buffer,
    size_t length, size_t *read)
{
  STM_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);
  const size_t available = COUNT_RX_VALUE(*calcRxEpCount(reg, number));

  if (available <= length)
  {
    epReadPacketMemory(buffer, calcRxEpBuffer(reg, number), available);
    *read = available;
    return E_OK;
  }
  else
    return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static void epReadPacketMemory(void *dst, const void *src, size_t length)
{
  /* Local USB address space, 16-bit words */
  uint16_t *dstBuffer = dst;
  /* Global address space, 32-bit words where the upper half is unused */
  const uint32_t *srcBuffer = src;

  for (; length >= 2; length -= 2)
    *dstBuffer++ = (uint32_t)(*srcBuffer++);

  if (length)
    *(uint8_t *)dstBuffer = (uint8_t)*srcBuffer;
}
/*----------------------------------------------------------------------------*/
static void epWriteData(struct UsbSieEndpoint *ep, const uint8_t *buffer,
    size_t length)
{
  STM_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  epWritePacketMemory(calcTxEpBuffer(reg, number), buffer, length);
  *calcTxEpCount(reg, number) = length;
  changeTxStat(&reg->EPR[number], EPR_STAT_VALID);
}
/*----------------------------------------------------------------------------*/
static void epWritePacketMemory(void *dst, const void *src, size_t length)
{
  /* Global address space */
  uint32_t *dstBuffer = dst;
  /* Local USB address space */
  const uint16_t *srcBuffer = src;

  for (; length >= 2; length -= 2)
    *dstBuffer++ = *srcBuffer++;

  if (length)
    *dstBuffer = *(const uint8_t *)srcBuffer;
}
/*----------------------------------------------------------------------------*/
static void setRxEpBufferSize(struct UsbSieEndpoint *ep, unsigned int number,
    size_t size)
{
  assert(size > 0 && size <= 512);

  STM_USB_Type * const reg = ep->device->base.reg;
  uint32_t value;

  if (size <= 62)
    value = COUNT_RX_NUM_BLOCK((size + 1) >> 1);
  else
    value = COUNT_RX_NUM_BLOCK((size + 31) >> 5) | COUNT_RX_BLSIZE;

  *calcRxEpCount(reg, number) = value;
}
/*----------------------------------------------------------------------------*/
static enum Result epInit(void *object, const void *configBase)
{
  const struct UsbEndpointConfig * const config = configBase;
  struct UsbSieEndpoint * const ep = object;

  const enum Result res = queueInit(&ep->requests,
      sizeof(struct UsbRequest *), CONFIG_PLATFORM_USB_DEVICE_EP_REQUESTS);

  if (res == E_OK)
  {
    ep->address = config->address;
    ep->device = config->parent;
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

  if (ep->address & USB_EP_DIRECTION_IN)
    changeTxStat(&reg->EPR[number], EPR_STAT_DISABLED);
  else
    changeRxStat(&reg->EPR[number], EPR_STAT_DISABLED);
}
/*----------------------------------------------------------------------------*/
static void epEnable(void *object, uint8_t type, uint16_t size)
{
  static const uint8_t epTypeTable[] = {
      [ENDPOINT_TYPE_CONTROL] = EPR_TYPE_CONTROL,
      [ENDPOINT_TYPE_ISOCHRONOUS] = EPR_TYPE_ISO,
      [ENDPOINT_TYPE_BULK] = EPR_TYPE_BULK,
      [ENDPOINT_TYPE_INTERRUPT] = EPR_TYPE_INTERRUPT
  };

  struct UsbSieEndpoint * const ep = object;
  STM_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);
  uint32_t value = 0;

  /* Translate USB endpoint type to platform-specific code */
  value |= EPR_EA(number);
  value |= EPR_EP_TYPE(epTypeTable[type]);

  reg->EPR[number] = value;

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    *calcTxEpAddr(reg, number) = ep->device->position;
    reg->EPR[number] = (reg->EPR[number] & (EPR_TOGGLE_MASK | EPR_DTOG_TX))
        | (EPR_CTR_RX | EPR_CTR_TX);
    changeTxStat(&reg->EPR[number], EPR_STAT_NAK);
    ep->device->position += size;
  }
  else
  {
    *calcRxEpAddr(reg, number) = ep->device->position;
    setRxEpBufferSize(ep, number, size);
    reg->EPR[number] = (reg->EPR[number] & (EPR_TOGGLE_MASK | EPR_DTOG_RX))
        | (EPR_CTR_RX | EPR_CTR_TX);
    changeRxStat(&reg->EPR[number], EPR_STAT_NAK);
    ep->device->position += size;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result epEnqueue(void *object, struct UsbRequest *request)
{
  assert(request->callback);

  struct UsbSieEndpoint * const ep = object;
  STM_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  irqDisable(ep->device->base.irq);
  assert(!queueFull(&ep->requests));

  if (queueEmpty(&ep->requests))
  {
    if (ep->address & USB_EP_DIRECTION_IN)
      epWriteData(ep, request->buffer, request->length);
    else
      changeRxStat(&reg->EPR[number], EPR_STAT_VALID);
  }
  queuePush(&ep->requests, &request);

  irqEnable(ep->device->base.irq);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static bool epIsStalled(void *object)
{
  struct UsbSieEndpoint * const ep = object;
  STM_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  if (ep->address & USB_EP_DIRECTION_IN)
    return EPR_STAT_TX_VALUE(reg->EPR[number]) == EPR_STAT_STALL;
  else
    return EPR_STAT_RX_VALUE(reg->EPR[number]) == EPR_STAT_STALL;
}
/*----------------------------------------------------------------------------*/
static void epSetStalled(void *object, bool stalled)
{
  struct UsbSieEndpoint * const ep = object;
  STM_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    if (!stalled)
      reg->EPR[number] = reg->EPR[number] & (EPR_TOGGLE_MASK | EPR_DTOG_TX);

    changeTxStat(&reg->EPR[number], stalled ? EPR_STAT_STALL : EPR_STAT_NAK);
  }
  else
  {
    if (!stalled)
      reg->EPR[number] = reg->EPR[number] & (EPR_TOGGLE_MASK | EPR_DTOG_RX);

    changeRxStat(&reg->EPR[number], stalled ? EPR_STAT_STALL : EPR_STAT_NAK);
  }
}
