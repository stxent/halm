/*
 * usb_device.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <xcore/containers/array.h>
#include <xcore/containers/queue.h>
#include <xcore/memory.h>
#include <halm/platform/nxp/lpc17xx/usb_base.h>
#include <halm/platform/nxp/lpc17xx/usb_defs.h>
#include <halm/platform/nxp/usb_device.h>
#include <halm/usb/usb_control.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_USB_DEVICE_POOL_SIZE
#define CONFIG_PLATFORM_USB_DEVICE_POOL_SIZE
#endif

struct DmaDescriptorPool
{
  /* Transfer descriptor pool */
  struct Array descriptors;
  /* Pointer to an aligned array of Queue Head descriptors */
  struct DmaDescriptor **heads;
  /* Memory allocated for Transfer descriptors */
  struct DmaDescriptor memory[CONFIG_PLATFORM_USB_DEVICE_POOL_SIZE];
};
/*----------------------------------------------------------------------------*/
struct UsbDmaEndpoint
{
  struct UsbEndpoint base;

  /* Parent device */
  struct UsbDevice *device;
  /* Head of the Transfer Descriptor list */
  struct DmaDescriptor *head;
  /* Tail of the Transfer Descriptor list */
  struct DmaDescriptor *tail;
  /* Logical address */
  uint8_t address;
  /* Endpoint type */
  uint8_t type;
};

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
  struct UsbEndpoint *endpoints[32];
  /* Control message handler */
  struct UsbControl *control;

  /* DMA descriptors */
  struct DmaDescriptorPool *pool;

  /* Device is configured */
  bool configured;
};
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_USB_DMA
static void deinitDescriptorPool(struct UsbDevice *);
static enum result initDescriptorPool(struct UsbDevice *);
#endif

static void interruptHandler(void *);
static void resetDevice(struct UsbDevice *);
static void usbCommand(struct UsbDevice *, uint8_t);
static uint8_t usbCommandRead(struct UsbDevice *, uint8_t);
static void usbCommandWrite(struct UsbDevice *, uint8_t, uint16_t);
static void waitForInt(struct UsbDevice *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum result devInit(void *, const void *);
static void devDeinit(void *);
static void *devCreateEndpoint(void *, uint8_t);
static uint8_t devGetInterface(const void *);
static void devSetAddress(void *, uint8_t);
static void devSetConnected(void *, bool);
static enum result devBind(void *, void *);
static void devUnbind(void *, const void *);
static void devSetPower(void *, uint16_t);
static enum usbSpeed devGetSpeed(const void *);
static enum result devStringAppend(void *, struct UsbString);
static void devStringErase(void *, struct UsbString);
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
static enum result epReadData(struct UsbSieEndpoint *, uint8_t *,
    size_t, size_t *);
static void epWriteData(struct UsbSieEndpoint *, const uint8_t *, size_t);
static void sieEpHandler(struct UsbSieEndpoint *, uint8_t);
/*----------------------------------------------------------------------------*/
static enum result sieEpInit(void *, const void *);
static void sieEpDeinit(void *);
static void sieEpClear(void *);
static void sieEpDisable(void *);
static void sieEpEnable(void *, uint8_t, uint16_t);
static enum result sieEpEnqueue(void *, struct UsbRequest *);
static bool sieEpIsStalled(void *);
static void sieEpSetStalled(void *, bool);
/*----------------------------------------------------------------------------*/
static const struct UsbEndpointClass sieEpTable = {
    .size = sizeof(struct UsbSieEndpoint),
    .init = sieEpInit,
    .deinit = sieEpDeinit,

    .clear = sieEpClear,
    .disable = sieEpDisable,
    .enable = sieEpEnable,
    .enqueue = sieEpEnqueue,
    .isStalled = sieEpIsStalled,
    .setStalled = sieEpSetStalled
};
/*----------------------------------------------------------------------------*/
const struct UsbEndpointClass * const UsbSieEndpoint = &sieEpTable;
/*----------------------------------------------------------------------------*/
static void dmaEpHandler(struct UsbDmaEndpoint *);
/*----------------------------------------------------------------------------*/
static enum result dmaEpInit(void *, const void *);
static void dmaEpDeinit(void *);
static void dmaEpClear(void *);
static void dmaEpDisable(void *);
static void dmaEpEnable(void *, uint8_t, uint16_t);
static enum result dmaEpEnqueue(void *, struct UsbRequest *);
static bool dmaEpIsStalled(void *);
static void dmaEpSetStalled(void *, bool);
/*----------------------------------------------------------------------------*/
static const struct UsbEndpointClass dmaEpTable = {
    .size = sizeof(struct UsbDmaEndpoint),
    .init = dmaEpInit,
    .deinit = dmaEpDeinit,

    .clear = dmaEpClear,
    .disable = dmaEpDisable,
    .enable = dmaEpEnable,
    .enqueue = dmaEpEnqueue,
    .isStalled = dmaEpIsStalled,
    .setStalled = dmaEpSetStalled
};
/*----------------------------------------------------------------------------*/
const struct UsbEndpointClass * const UsbDmaEndpoint = &dmaEpTable;
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_USB_DMA
static void deinitDescriptorPool(struct UsbDevice *device)
{
  struct DmaDescriptorPool * const pool = device->pool;

  free(pool->heads);
  arrayDeinit(&pool->descriptors);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_USB_DMA
static enum result initDescriptorPool(struct UsbDevice *device)
{
  struct DmaDescriptorPool * const pool =
      malloc(sizeof(struct DmaDescriptorPool));
  if (!pool)
    return E_MEMORY;

  enum result res;

  /* Allocate memory for Transfer Descriptors */
  res = arrayInit(&pool->descriptors, sizeof(struct DmaDescriptor *),
      ARRAY_SIZE(pool->memory));
  if (res != E_OK)
    return res;

  //TODO Remove magic numbers
  pool->heads = memalign(128, 32 * sizeof(struct DmaDescriptor *));
  if (!pool->heads)
    return E_MEMORY;
  memset(pool->heads, 0, 32 * sizeof(struct DmaDescriptor *));

  for (size_t index = 0; index < ARRAY_SIZE(pool->memory); ++index)
  {
    struct DmaDescriptor * const entry = pool->memory + index;
    arrayPushBack(&pool->descriptors, &entry);
  }

  /* Configure USB Device Communication Area */
  ((LPC_USB_Type *)device->base.reg)->USBUDCAH = (uint32_t)pool->heads;

  device->pool = pool;
  return E_OK;
}
#endif
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct UsbDevice * const device = object;
  LPC_USB_Type * const reg = device->base.reg;
  const uint32_t intStatus = reg->USBDevIntSt;

  /* Device status interrupt */
  if (intStatus & USBDevInt_DEV_STAT)
  {
    reg->USBDevIntClr = USBDevInt_DEV_STAT;

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
  if (intStatus & USBDevInt_EP_SLOW)
  {
    reg->USBDevIntClr = USBDevInt_EP_SLOW;

    struct UsbEndpoint ** const endpointArray = device->endpoints;
    uint32_t epIntStatus = reverseBits32(reg->USBEpIntSt);

    while (epIntStatus)
    {
      const unsigned int index = countLeadingZeros32(epIntStatus);
      const uint8_t status = usbCommandRead(device,
          USB_CMD_CLEAR_INTERRUPT | index);

      epIntStatus -= (1UL << 31) >> index;
      reg->USBEpIntClr = 1UL << index;

      sieEpHandler((struct UsbSieEndpoint *)endpointArray[index], status);
    }
  }

  /* DMA transfer completion interrupt */
  uint32_t dmaIntStatus = reverseBits32(reg->USBEoTIntSt);

  if (dmaIntStatus)
  {
    struct UsbEndpoint ** const endpointArray = device->endpoints;

    while (dmaIntStatus)
    {
      const unsigned int index = countLeadingZeros32(dmaIntStatus);

      dmaIntStatus -= (1UL << 31) >> index;
      reg->USBEoTIntClr = 1UL << index;

      dmaEpHandler((struct UsbDmaEndpoint *)endpointArray[index]);
    }
  }
}
/*----------------------------------------------------------------------------*/
static void resetDevice(struct UsbDevice *device)
{
  LPC_USB_Type * const reg = device->base.reg;

  /* Set inactive configuration */
  device->configured = false;

  /* Reset device interrupts */
  reg->USBDevIntEn = 0;
  reg->USBDevIntClr = 0xFFFFFFFF;
  reg->USBDevIntPri = 0;
  /* Reset endpoint interrupts */
  reg->USBEpIntEn = 0;
  reg->USBEpIntClr = 0xFFFFFFFF;
  reg->USBEpIntPri = 0;
  /* Clear DMA interrupts */
  reg->USBEpDMADis = 0xFFFFFFFF;
  reg->USBDMARClr = 0xFFFFFFFF;
  reg->USBEoTIntClr = 0xFFFFFFFF;

  reg->USBDMAIntEn = USBDMAIntEn_EOT;
  reg->USBDevIntEn = USBDevInt_DEV_STAT | USBDevInt_EP_SLOW;
}
/*----------------------------------------------------------------------------*/
static void usbCommand(struct UsbDevice *device, uint8_t command)
{
  LPC_USB_Type * const reg = device->base.reg;

  /* Write command code and wait for completion */
  reg->USBDevIntClr = USBDevInt_CCEMPTY;
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
  reg->USBDevIntClr = USBDevInt_CDFULL;
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
  reg->USBDevIntClr = USBDevInt_CCEMPTY;
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
static enum result devInit(void *object, const void *configBase)
{
  const struct UsbDeviceConfig * const config = configBase;
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
  enum result res;

  /* Call base class constructor */
  res = UsbBase->init(object, &baseConfig);
  if (res != E_OK)
    return res;

  device->base.handler = interruptHandler;
  memset(device->endpoints, 0, sizeof(device->endpoints));

  /* Initialize control message handler */
  device->control = init(UsbControl, &controlConfig);
  if (!device->control)
    return E_ERROR;

  /* Configure interrupts and reset system variables */
  resetDevice(device);
  /* By default, only ACKs generate interrupts */
  usbCommandWrite(device, USB_CMD_SET_MODE, 0);

#ifdef CONFIG_PLATFORM_USB_DMA
  if ((res = initDescriptorPool(device)) != E_OK)
    return res;
#endif

  irqSetPriority(device->base.irq, config->priority);
  irqEnable(device->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void devDeinit(void *object)
{
  struct UsbDevice * const device = object;

  irqDisable(device->base.irq);

#ifdef CONFIG_PLATFORM_USB_DMA
  deinitDescriptorPool(device);
#endif

  deinit(device->control);
  UsbBase->deinit(device);
}
/*----------------------------------------------------------------------------*/
static void *devCreateEndpoint(void *object, uint8_t address)
{
  struct UsbDevice * const device = object;
  const unsigned int index = EP_TO_INDEX(address);

  assert(index < ARRAY_SIZE(device->endpoints));

  struct UsbEndpoint *ep = 0;
  const irqState state = irqSave();

  if (!device->endpoints[index])
  {
    const struct UsbEndpointConfig config = {
        .parent = device,
        .address = address
    };
    const struct UsbEndpointClass *type;

#ifdef CONFIG_PLATFORM_USB_DMA
    type = index >= 2 ? UsbDmaEndpoint : UsbSieEndpoint;
#else
    type = UsbSieEndpoint;
#endif

    device->endpoints[index] = init(type, &config);
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
  usbCommandWrite(object, USB_CMD_SET_DEVICE_STATUS,
      state ? DEVICE_STATUS_CON : 0);
}
/*----------------------------------------------------------------------------*/
static enum result devBind(void *object, void *driver)
{
  struct UsbDevice * const device = object;

  const irqState state = irqSave();
  const enum result res = usbControlBindDriver(device->control, driver);
  irqRestore(state);

  return res;
}
/*----------------------------------------------------------------------------*/
static void devUnbind(void *object, const void *driver __attribute__((unused)))
{
  struct UsbDevice * const device = object;

  const irqState state = irqSave();
  usbControlUnbindDriver(device->control);
  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static void devSetPower(void *object, uint16_t current)
{
  struct UsbDevice * const device = object;

  usbControlSetPower(device->control, current);
}
/*----------------------------------------------------------------------------*/
static enum usbSpeed devGetSpeed(const void *object __attribute__((unused)))
{
  return USB_FS;
}
/*----------------------------------------------------------------------------*/
static enum result devStringAppend(void *object, struct UsbString string)
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
static enum result epReadData(struct UsbSieEndpoint *ep, uint8_t *buffer,
    size_t length, size_t *read)
{
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int index = EP_TO_INDEX(ep->address);

  /* Set read enable bit for specific endpoint */
  reg->USBCtrl = USBCtrl_RD_EN
      | USBCtrl_LOG_ENDPOINT(USB_EP_LOGICAL_ADDRESS(ep->address));

  size_t packetLength;

  /* Wait for length field to become valid */
  while (!((packetLength = reg->USBRxPLen) & USBRxPLen_PKT_RDY));

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
  reg->USBCtrl = USBCtrl_WR_EN
      | USBCtrl_LOG_ENDPOINT(USB_EP_LOGICAL_ADDRESS(ep->address));
  /* Set packet length */
  reg->USBTxPLen = length;

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

  /* Clear write enable bit */
  reg->USBCtrl = 0;

  /* Select endpoint and validate buffer */
  usbCommand(ep->device, USB_CMD_SELECT_ENDPOINT | index);
  usbCommand(ep->device, USB_CMD_VALIDATE_BUFFER);
}
/*----------------------------------------------------------------------------*/
static void sieEpHandler(struct UsbSieEndpoint *ep, uint8_t status)
{
  if (queueEmpty(&ep->requests))
    return;

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    const unsigned int index = EP_TO_INDEX(ep->address);

    while (!queueEmpty(&ep->requests))
    {
      const uint8_t epStatus = usbCommandRead(ep->device,
          USB_CMD_SELECT_ENDPOINT | index);

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

      const enum usbRequestStatus requestStatus = status & SELECT_ENDPOINT_STP ?
          USB_REQUEST_SETUP : USB_REQUEST_COMPLETED;

      request->length = read;
      request->callback(request->callbackArgument, request, requestStatus);
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum result sieEpInit(void *object, const void *configBase)
{
  const struct UsbEndpointConfig * const config = configBase;
  struct UsbDevice * const device = config->parent;
  struct UsbSieEndpoint * const ep = object;

  const enum result res = queueInit(&ep->requests,
      sizeof(struct UsbRequest *), CONFIG_PLATFORM_USB_DEVICE_EP_REQUESTS);

  if (res == E_OK)
  {
    ep->address = config->address;
    ep->device = device;
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static void sieEpDeinit(void *object)
{
  struct UsbSieEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;

  /* Disable interrupts and remove pending requests */
  sieEpDisable(ep);
  sieEpClear(ep);

  const unsigned int index = EP_TO_INDEX(ep->address);

  const irqState state = irqSave();
  device->endpoints[index] = 0;
  irqRestore(state);

  assert(queueEmpty(&ep->requests));
  queueDeinit(&ep->requests);
}
/*----------------------------------------------------------------------------*/
static void sieEpClear(void *object)
{
  struct UsbSieEndpoint * const ep = object;
  struct UsbRequest *request;

  while (!queueEmpty(&ep->requests))
  {
    queuePop(&ep->requests, &request);
    request->callback(request->callbackArgument, request,
        USB_REQUEST_CANCELLED);
  }
}
/*----------------------------------------------------------------------------*/
static void sieEpDisable(void *object)
{
  struct UsbSieEndpoint * const ep = object;
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int index = EP_TO_INDEX(ep->address);
  const uint32_t mask = 1UL << index;

  reg->USBEpIntEn &= ~mask;

  usbCommandWrite(ep->device, USB_CMD_SET_ENDPOINT_STATUS | index,
      SET_ENDPOINT_STATUS_DA);
}
/*----------------------------------------------------------------------------*/
static void sieEpEnable(void *object, uint8_t type __attribute__((unused)),
    uint16_t size)
{
  struct UsbSieEndpoint * const ep = object;
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int index = EP_TO_INDEX(ep->address);
  const uint32_t mask = 1UL << index;

  /* Enable interrupt */
  reg->USBEpIntClr = mask;
  reg->USBEpIntEn |= mask;

  /* Realize endpoint */
  reg->USBReEp |= mask;
  reg->USBEpInd = index;
  reg->USBMaxPSize = size;
  waitForInt(ep->device, USBDevInt_EP_RLZED);

  usbCommandWrite(ep->device, USB_CMD_SET_ENDPOINT_STATUS | index, 0);
}
/*----------------------------------------------------------------------------*/
static enum result sieEpEnqueue(void *object, struct UsbRequest *request)
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

  const irqState state = irqSave();

  const uint8_t status = usbCommandRead(ep->device,
      USB_CMD_SELECT_ENDPOINT | index);
  bool enqueue;

  assert(!queueFull(&ep->requests));

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    enqueue = (status & (SELECT_ENDPOINT_FE | SELECT_ENDPOINT_ST))
        || queueSize(&ep->requests);

    if (!enqueue)
    {
      epWriteData(ep, request->buffer, request->length);
      request->callback(request->callbackArgument, request,
          USB_REQUEST_COMPLETED);
    }
  }
  else
  {
    enqueue = true;

    if (status & SELECT_ENDPOINT_FE)
    {
      LPC_USB_Type * const reg = ep->device->base.reg;
      const uint32_t mask = 1UL << index;

      /* Schedule interrupt */
      reg->USBEpIntSet = mask;
    }
  }

  if (enqueue)
    queuePush(&ep->requests, &request);

  irqRestore(state);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static bool sieEpIsStalled(void *object)
{
  struct UsbSieEndpoint * const ep = object;
  const unsigned int index = EP_TO_INDEX(ep->address);
  const uint8_t status = usbCommandRead(ep->device,
      USB_CMD_SELECT_ENDPOINT | index);

  return (status & SELECT_ENDPOINT_ST) != 0;
}
/*----------------------------------------------------------------------------*/
static void sieEpSetStalled(void *object, bool stalled)
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
/*----------------------------------------------------------------------------*/
static void dmaEpHandler(struct UsbDmaEndpoint *ep)
{
  struct DmaDescriptorPool * const pool = ep->device->pool;
  const unsigned int index = EP_TO_INDEX(ep->address);

  while (ep->head && (ep->head->status & DD_STATUS_RETIRED))
  {
    struct UsbRequest * const request =
        (struct UsbRequest *)ep->head->request;

    if (!request)
    {
      /* It is a mock descriptor, return it to pool */
      arrayPushBack(&pool->descriptors, &ep->head);
      ep->head = (struct DmaDescriptor *)ep->head->next;
      continue;
    }

    enum usbRequestStatus requestStatus;

    switch (DD_STATUS_VALUE(ep->head->status))
    {
      case DD_NORMAL_COMPLETION:
      case DD_DATA_UNDERRUN:
        requestStatus = USB_REQUEST_COMPLETED;
        break;

      default:
        requestStatus = USB_REQUEST_ERROR;
        break;
    }

    if (!(ep->address & USB_EP_DIRECTION_IN))
    {
      request->length = DD_STATUS_PRESENT_DMA_COUNT_VALUE(ep->head->status);
    }

    if (!ep->head->next || ep->head != pool->heads[index])
    {
      arrayPushBack(&pool->descriptors, &ep->head);
      ep->head = (struct DmaDescriptor *)ep->head->next;

      if (!ep->head)
      {
        LPC_USB_Type * const reg = ep->device->base.reg;
        const uint32_t mask = 1UL << index;

        pool->heads[index] = 0;
        ep->tail = 0;

        reg->USBEpDMADis = mask;
        reg->USBDMARClr = mask;
      }
    }
    else
    {
      /* Preserve current descriptor as a mock descriptor */
      ep->head->request = 0;
    }

    request->callback(request->callbackArgument, request, requestStatus);

    if (!ep->head->request)
      break;
  }
}
/*----------------------------------------------------------------------------*/
static struct DmaDescriptor *epAllocDescriptor(struct UsbDmaEndpoint *ep,
    struct UsbRequest *node, uint8_t *buffer, size_t length)
{
  struct DmaDescriptorPool * const pool = ep->device->pool;
  struct DmaDescriptor *descriptor;

  if (arrayEmpty(&pool->descriptors))
    return 0;
  arrayPopBack(&pool->descriptors, &descriptor);

  descriptor->next = 0;
  descriptor->control = DD_CONTROL_DMA_MODE(DMA_MODE_NORMAL)
      | DD_CONTROL_MAX_PACKET_SIZE(node->capacity);

  if (ep->type == ENDPOINT_TYPE_ISOCHRONOUS)
  {
    descriptor->control |= DD_CONTROL_ISOCHRONOUS_EP
        | DD_CONTROL_DMA_BUFFER_LENGTH(1);
  }
  else
  {
    descriptor->control |= DD_CONTROL_DMA_BUFFER_LENGTH(length);
  }

  descriptor->buffer = (uint32_t)buffer;
  descriptor->status = 0;
  descriptor->size = 0;

  /* Store pointer to the USB Request structure in reserved field */
  descriptor->request = (uint32_t)node;

  return descriptor;
}
/*----------------------------------------------------------------------------*/
static void epAppendDescriptor(struct UsbDmaEndpoint *ep,
    struct DmaDescriptor *descriptor)
{
  LPC_USB_Type * const reg = ep->device->base.reg;
  struct DmaDescriptorPool * const pool = ep->device->pool;
  const unsigned int index = EP_TO_INDEX(ep->address);
  const uint32_t mask = 1UL << index;

  const irqState state = irqSave();

  if (pool->heads[index])
  {
    /* Linked list is not empty */
    ep->tail->next = (uint32_t)descriptor;
    ep->tail->control |= DD_CONTROL_NEXT_DD_VALID;
    ep->tail = descriptor;
  }
  else
  {
    pool->heads[index] = descriptor;
    ep->head = ep->tail = descriptor;
  }

  const uint8_t status = usbCommandRead(ep->device,
      USB_CMD_SELECT_ENDPOINT | index);

  if (!(status & SELECT_ENDPOINT_ST) && !(reg->USBEpDMASt & mask))
  {
    if (!(ep->address & USB_EP_DIRECTION_IN) ^ !(status & SELECT_ENDPOINT_FE))
      reg->USBDMARSet = mask; /* Initiate a transfer */

    reg->USBEpDMAEn = mask;
  }

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static enum result epEnqueueRx(struct UsbDmaEndpoint *ep,
    struct UsbRequest *request)
{
  struct DmaDescriptor * const descriptor = epAllocDescriptor(ep, request,
      request->buffer, request->capacity);

  assert(descriptor);
  epAppendDescriptor(ep, descriptor);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result epEnqueueTx(struct UsbDmaEndpoint *ep,
    struct UsbRequest *request)
{
  struct DmaDescriptor * const descriptor = epAllocDescriptor(ep, request,
      request->buffer, request->length);

  assert(descriptor);
  epAppendDescriptor(ep, descriptor);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result dmaEpInit(void *object, const void *configBase)
{
  const struct UsbEndpointConfig * const config = configBase;
  struct UsbDevice * const device = config->parent;
  struct UsbDmaEndpoint * const ep = object;

  ep->address = config->address;
  ep->device = device;
  ep->head = 0;
  ep->tail = 0;
  ep->type = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void dmaEpDeinit(void *object)
{
  struct UsbDmaEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;

  /* Remove pending requests */
  dmaEpDisable(ep);

  const unsigned int index = EP_TO_INDEX(ep->address);

  const irqState state = irqSave();
  device->endpoints[index] = 0;
  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static void dmaEpClear(void *object)
{
  struct UsbDmaEndpoint * const ep = object;
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int index = EP_TO_INDEX(ep->address);
  const uint32_t mask = 1UL << index;

  reg->USBEpDMADis = mask;
  reg->USBDMARClr = mask;

  struct DmaDescriptorPool * const pool = ep->device->pool;
  struct DmaDescriptor *current = pool->heads[index];

  while (current)
  {
    struct UsbRequest * const request = (struct UsbRequest *)current->request;

    request->callback(request->callbackArgument, request,
        USB_REQUEST_CANCELLED);
    current = (struct DmaDescriptor *)current->next;
  }

  pool->heads[index] = 0;
  ep->head = ep->tail = 0;
}
/*----------------------------------------------------------------------------*/
static void dmaEpDisable(void *object)
{
  struct UsbDmaEndpoint * const ep = object;
  const unsigned int index = EP_TO_INDEX(ep->address);

  dmaEpClear(ep);

  usbCommandWrite(ep->device, USB_CMD_SET_ENDPOINT_STATUS | index,
      SET_ENDPOINT_STATUS_DA);
}
/*----------------------------------------------------------------------------*/
static void dmaEpEnable(void *object, uint8_t type, uint16_t size)
{
  struct UsbDmaEndpoint * const ep = object;
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int index = EP_TO_INDEX(ep->address);
  const uint32_t mask = 1UL << index;

  ep->type = type;

  /* Disable slave mode */
  reg->USBEpIntEn &= ~mask;
  /* Clear pending interrupts */
  reg->USBEoTIntClr = mask;
  reg->USBEpIntClr = mask;

  /* Realize endpoint */
  reg->USBReEp |= mask;
  reg->USBEpInd = index;
  reg->USBMaxPSize = size;
  waitForInt(ep->device, USBDevInt_EP_RLZED);

  usbCommandWrite(ep->device, USB_CMD_SET_ENDPOINT_STATUS | index, 0);
}
/*----------------------------------------------------------------------------*/
static enum result dmaEpEnqueue(void *object, struct UsbRequest *request)
{
  assert(request->callback);

  struct UsbDmaEndpoint * const ep = object;
  enum result res;

  const irqState state = irqSave();

  if (ep->address & USB_EP_DIRECTION_IN)
    res = epEnqueueTx(ep, request);
  else
    res = epEnqueueRx(ep, request);

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
static bool dmaEpIsStalled(void *object)
{
  struct UsbDmaEndpoint * const ep = object;
  const unsigned int index = EP_TO_INDEX(ep->address);
  const uint8_t status = usbCommandRead(ep->device,
      USB_CMD_SELECT_ENDPOINT | index);

  return (status & SELECT_ENDPOINT_ST) != 0;
}
/*----------------------------------------------------------------------------*/
static void dmaEpSetStalled(void *object, bool stalled)
{
  struct UsbDmaEndpoint * const ep = object;
  struct DmaDescriptorPool * const pool = ep->device->pool;
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int index = EP_TO_INDEX(ep->address);
  const uint32_t mask = 1UL << index;

  usbCommandWrite(ep->device, USB_CMD_SET_ENDPOINT_STATUS | index,
      stalled ? SET_ENDPOINT_STATUS_ST : 0);

  if (stalled)
  {
    reg->USBEpDMADis = mask;
    reg->USBDMARClr = mask;
  }
  else if (pool->heads[index])
  {
    if (ep->address & USB_EP_DIRECTION_IN)
      reg->USBDMARSet = mask;
    reg->USBEpDMAEn = mask;
  }
}
