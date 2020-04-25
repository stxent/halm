/*
 * usb_device.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <malloc.h>
#include <xcore/accel.h>
#include <halm/delay.h>
#include <halm/generic/pointer_array.h>
#include <halm/generic/pointer_queue.h>
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
  /* Pointer to an aligned array of Queue Head descriptors */
  volatile struct DmaDescriptor *heads[32];
  /* Transfer descriptor pool */
  PointerArray descriptors;
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
  PointerQueue requests;
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
  /* Device is enabled */
  bool enabled;
};
/*----------------------------------------------------------------------------*/
#if defined(CONFIG_PLATFORM_USB_DMA) && !defined(CONFIG_PLATFORM_USB_NO_DEINIT)
static void deinitDescriptorPool(struct UsbDevice *);
#endif

#ifdef CONFIG_PLATFORM_USB_DMA
static bool initDescriptorPool(struct UsbDevice *);
#endif

static void interruptHandler(void *);
static void resetDevice(struct UsbDevice *);
static void usbCommand(struct UsbDevice *, uint8_t);
static uint8_t usbCommandRead(struct UsbDevice *, uint8_t);
static void usbCommandWrite(struct UsbDevice *, uint8_t, uint16_t);
static void usbRunCommand(LPC_USB_Type *, enum UsbCommandPhase, uint32_t,
    uint32_t);
static uint8_t usbStatusRead(struct UsbDevice *, unsigned int);
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
static enum Result epReadData(struct UsbSieEndpoint *, uint8_t *, size_t,
    size_t *);
static void epReadPacketMemory(LPC_USB_Type *, uint8_t *, size_t);
static void epWriteData(struct UsbSieEndpoint *, const uint8_t *, size_t);
static void epWritePacketMemory(LPC_USB_Type *, const uint8_t *, size_t);
static void sieEpHandler(struct UsbSieEndpoint *, uint8_t);
/*----------------------------------------------------------------------------*/
static enum Result sieEpInit(void *, const void *);
static void sieEpDeinit(void *);
static void sieEpClear(void *);
static void sieEpDisable(void *);
static void sieEpEnable(void *, uint8_t, uint16_t);
static enum Result sieEpEnqueue(void *, struct UsbRequest *);
static bool sieEpIsStalled(void *);
static void sieEpSetStalled(void *, bool);
/*----------------------------------------------------------------------------*/
static const struct UsbEndpointClass * const UsbSieEndpoint =
    &(const struct UsbEndpointClass){
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
static void dmaEpHandler(struct UsbDmaEndpoint *);
static void dmaEpUpdateChain(struct UsbDmaEndpoint *);
static struct DmaDescriptor *epAllocDescriptor(struct UsbDmaEndpoint *,
    struct UsbRequest *);
static void epAppendDescriptor(struct UsbDmaEndpoint *, struct DmaDescriptor *);
static void epEnqueueRequest(struct UsbDmaEndpoint *, struct UsbRequest *);
/*----------------------------------------------------------------------------*/
static enum Result dmaEpInit(void *, const void *);
static void dmaEpDeinit(void *);
static void dmaEpClear(void *);
static void dmaEpDisable(void *);
static void dmaEpEnable(void *, uint8_t, uint16_t);
static enum Result dmaEpEnqueue(void *, struct UsbRequest *);
static bool dmaEpIsStalled(void *);
static void dmaEpSetStalled(void *, bool);
/*----------------------------------------------------------------------------*/
static const struct UsbEndpointClass * const UsbDmaEndpoint =
    &(const struct UsbEndpointClass){
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
#if defined(CONFIG_PLATFORM_USB_DMA) && !defined(CONFIG_PLATFORM_USB_NO_DEINIT)
static void deinitDescriptorPool(struct UsbDevice *device)
{
  struct DmaDescriptorPool * const pool = device->pool;
  pointerArrayDeinit(&pool->descriptors);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_USB_DMA
static bool initDescriptorPool(struct UsbDevice *device)
{
  struct DmaDescriptorPool *pool;

  pool = memalign(128, sizeof(struct DmaDescriptorPool));
  if (!pool)
    return false;

  /* Allocate memory for Transfer Descriptors */
  if (!pointerArrayInit(&pool->descriptors, ARRAY_SIZE(pool->memory)))
    return false;

  for (size_t index = 0; index < ARRAY_SIZE(pool->heads); ++index)
    pool->heads[index] = 0;

  for (size_t index = 0; index < ARRAY_SIZE(pool->memory); ++index)
    pointerArrayPushBack(&pool->descriptors, pool->memory + index);

  /* Configure USB Device Communication Area */
  ((LPC_USB_Type *)device->base.reg)->USBUDCAH = (uint32_t)pool->heads;

  device->pool = pool;
  return true;
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

  /* SIE endpoint interrupt */
  if (intStatus & USBDevInt_EP_SLOW)
  {
    reg->USBDevIntClr = USBDevInt_EP_SLOW;

    struct UsbEndpoint ** const endpointArray = device->endpoints;
    uint32_t epIntStatus = reverseBits32(reg->USBEpIntSt);

    while (epIntStatus)
    {
      const unsigned int index = countLeadingZeros32(epIntStatus);
      const uint8_t status = usbStatusRead(device, index);

      epIntStatus -= (1UL << 31) >> index;
      sieEpHandler((struct UsbSieEndpoint *)endpointArray[index], status);
    }
  }

  /* DMA End of Transfer interrupt */
  uint32_t dmaIntStatus = reverseBits32(reg->USBEoTIntSt);

  while (dmaIntStatus)
  {
    const unsigned int index = countLeadingZeros32(dmaIntStatus);

    dmaIntStatus -= (1UL << 31) >> index;
    reg->USBEoTIntClr = 1UL << index;

    dmaEpHandler((struct UsbDmaEndpoint *)device->endpoints[index]);
  }

  /* DMA New DD Request interrupt */
  uint32_t nddrIntStatus = reverseBits32(reg->USBNDDRIntSt);

  while (nddrIntStatus)
  {
    const unsigned int index = countLeadingZeros32(nddrIntStatus);

    nddrIntStatus -= (1UL << 31) >> index;
    reg->USBNDDRIntClr = 1UL << index;

    dmaEpHandler((struct UsbDmaEndpoint *)device->endpoints[index]);
    dmaEpUpdateChain((struct UsbDmaEndpoint *)device->endpoints[index]);
  }

  /* DMA System Error interrupt */
  if (reg->USBDMAIntSt & USBDMAIntEn_ERR)
  {
    /* Reset device in case of AHB bus error */
    resetDevice(device);
    usbControlNotify(device->control, USB_DEVICE_EVENT_RESET);
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
  reg->USBDevIntClr = 0xFFFFFFFFUL;
  reg->USBDevIntPri = 0;
  /* Reset endpoint interrupts */
  reg->USBEpIntEn = 0;
  reg->USBEpIntClr = 0xFFFFFFFFUL;
  reg->USBEpIntPri = 0;
  /* Clear DMA interrupts */
  reg->USBEpDMADis = 0xFFFFFFFFUL;
  reg->USBDMARClr = 0xFFFFFFFFUL;
  reg->USBEoTIntClr = 0xFFFFFFFFUL;

  reg->USBDMAIntEn = USBDMAIntEn_EOT | USBDMAIntEn_NDDR | USBDMAIntEn_ERR;
  reg->USBDevIntEn = USBDevInt_DEV_STAT | USBDevInt_EP_SLOW;
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
static uint8_t usbStatusRead(struct UsbDevice *device, unsigned int index)
{
  LPC_USB_Type * const reg = device->base.reg;

  reg->USBEpIntClr = 1UL << index;
  while (!(reg->USBDevIntSt & USBDevInt_CDFULL));

  return reg->USBCmdData;
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

  /* Call base class constructor */
  const enum Result res = UsbBase->init(object, &baseConfig);
  if (res != E_OK)
    return res;

  device->base.handler = interruptHandler;
  device->enabled = false;
  memset(device->endpoints, 0, sizeof(device->endpoints));

  /* Initialize control message handler after endpoint initialization */
  device->control = init(UsbControl, &controlConfig);
  if (!device->control)
    return E_ERROR;

  /* Configure interrupts and reset system variables */
  resetDevice(device);
  /* By default, only ACKs generate interrupts */
  usbCommandWrite(device, USB_CMD_SET_MODE, 0);

#ifdef CONFIG_PLATFORM_USB_DMA
  if (!initDescriptorPool(device))
    return E_MEMORY;
#else
  /* Suppress warning */
  (void)UsbDmaEndpoint;
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

#ifdef CONFIG_PLATFORM_USB_DMA
  deinitDescriptorPool(device);
#endif

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
  struct UsbDevice * const device = object;

  device->enabled = state;
  usbCommandWrite(device, USB_CMD_SET_DEVICE_STATUS,
      state ? DEVICE_STATUS_CON : 0);
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
static enum Result epReadData(struct UsbSieEndpoint *ep, uint8_t *buffer,
    size_t length, size_t *read)
{
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);
  size_t packetLength;

  /* Set read enable bit for specific endpoint */
  reg->USBCtrl = USBCtrl_RD_EN | USBCtrl_LOG_ENDPOINT(number);

  /* Wait for the length field to become valid */
  while (!((packetLength = reg->USBRxPLen) & USBRxPLen_PKT_RDY));

  /* Check packet validity */
  if (!(packetLength & USBRxPLen_DV))
    return E_INTERFACE;
  /* Extract length */
  packetLength = USBRxPLen_PKT_LNGTH_VALUE(packetLength);
  /* Check for buffer overflow */
  if (packetLength > length)
    return E_MEMORY;

  /* Read data from the internal packet memory */
  *read = packetLength;
  epReadPacketMemory(reg, buffer, packetLength);

  /* Clear read enable bit */
  reg->USBCtrl = 0;

  /* Select endpoint and clear buffer */
  usbCommand(ep->device, USB_CMD_SELECT_ENDPOINT | EP_TO_INDEX(ep->address));
  usbCommand(ep->device, USB_CMD_CLEAR_BUFFER);

  return E_OK;
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
static void epWriteData(struct UsbSieEndpoint *ep, const uint8_t *buffer,
    size_t length)
{
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  /* Set write enable for specific endpoint */
  reg->USBCtrl = USBCtrl_WR_EN | USBCtrl_LOG_ENDPOINT(number);
  /* Set packet length */
  reg->USBTxPLen = length;

  /* Write data into the internal packet memory */
  epWritePacketMemory(reg, buffer, length);

  /* Clear write enable bit */
  reg->USBCtrl = 0;

  /* Select endpoint and validate buffer */
  usbCommand(ep->device, USB_CMD_SELECT_ENDPOINT | EP_TO_INDEX(ep->address));
  usbCommand(ep->device, USB_CMD_VALIDATE_BUFFER);
}
/*----------------------------------------------------------------------------*/
static void epWritePacketMemory(LPC_USB_Type *reg, const uint8_t *buffer,
    size_t length)
{
  const uint32_t *start = (const uint32_t *)buffer;
  const uint32_t * const end = start + (length >> 2);

  buffer = (const uint8_t *)end;
  length &= 3;

  while (start < end)
  {
    reg->USBTxData = *start++;
    __dsb();
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
/*----------------------------------------------------------------------------*/
static void sieEpHandler(struct UsbSieEndpoint *ep, uint8_t status)
{
  /* Double-buffering availability mask */
  static const uint32_t dbAvailable = 0xF3CF3CF0UL;

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
      request->callback(request->callbackArgument, request,
          USB_REQUEST_COMPLETED);
    }
  }
  else
  {
    struct UsbRequest * const request = pointerQueueFront(&ep->requests);
    size_t read;

    if (epReadData(ep, request->buffer, request->capacity, &read) == E_OK)
    {
      const enum UsbRequestStatus requestStatus = status & SELECT_ENDPOINT_STP ?
          USB_REQUEST_SETUP : USB_REQUEST_COMPLETED;

      pointerQueuePopFront(&ep->requests);
      request->length = read;
      request->callback(request->callbackArgument, request, requestStatus);
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum Result sieEpInit(void *object, const void *configBase)
{
  const struct UsbEndpointConfig * const config = configBase;
  struct UsbSieEndpoint * const ep = object;

  if (pointerQueueInit(&ep->requests, CONFIG_PLATFORM_USB_DEVICE_EP_REQUESTS))
  {
    ep->address = config->address;
    ep->device = config->parent;

    return E_OK;
  }
  else
    return E_MEMORY;
}
/*----------------------------------------------------------------------------*/
static void sieEpDeinit(void *object)
{
  struct UsbSieEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;
  const unsigned int index = EP_TO_INDEX(ep->address);

  /* Disable interrupts and remove pending requests */
  sieEpDisable(ep);
  sieEpClear(ep);

  const IrqState state = irqSave();
  device->endpoints[index] = 0;
  irqRestore(state);

  assert(pointerQueueEmpty(&ep->requests));
  pointerQueueDeinit(&ep->requests);
}
/*----------------------------------------------------------------------------*/
static void sieEpClear(void *object)
{
  struct UsbSieEndpoint * const ep = object;

  while (!pointerQueueEmpty(&ep->requests))
  {
    struct UsbRequest * const request = pointerQueueFront(&ep->requests);
    pointerQueuePopFront(&ep->requests);

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
static enum Result sieEpEnqueue(void *object, struct UsbRequest *request)
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
  assert(!pointerQueueFull(&ep->requests));

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
    const uint32_t mask = 1UL << index;

    /* Schedule interrupt */
    reg->USBEpIntSet = mask;
  }

  irqEnable(ep->device->base.irq);
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
      && !pointerQueueEmpty(&ep->requests))
  {
    struct UsbRequest * const request = pointerQueueFront(&ep->requests);
    pointerQueuePopFront(&ep->requests);

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
    struct DmaDescriptor * const dd = ep->head;
    struct UsbRequest * const request = (struct UsbRequest *)dd->request;

    if (!request)
    {
      /* It is a mock descriptor, return it to pool */
      pointerArrayPushBack(&pool->descriptors, dd);
      ep->head = (struct DmaDescriptor *)dd->next;
      continue;
    }

    enum UsbRequestStatus requestStatus;

    switch (DD_STATUS_VALUE(dd->status))
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
      request->length = DD_STATUS_PRESENT_DMA_COUNT_VALUE(dd->status);

    if (dd == ep->tail)
    {
      LPC_USB_Type * const reg = ep->device->base.reg;
      const uint32_t mask = 1UL << index;

      reg->USBEpDMADis = mask;
      reg->USBDMARClr = mask;

      pool->heads[index] = 0;
      ep->tail = 0;
    }

    const bool preserve = dd == pool->heads[index];

    if (!preserve)
    {
      pointerArrayPushBack(&pool->descriptors, dd);
      ep->head = (struct DmaDescriptor *)dd->next;
    }

    request->callback(request->callbackArgument, request, requestStatus);

    if (preserve)
    {
      /*
       * Preserve current descriptor as a mock descriptor: in case of OUT
       * transfers a next descriptor will not be read until a next DMA request
       * is generated.
       */
      dd->request = 0;
      break;
    }
  }
}
/*----------------------------------------------------------------------------*/
static void dmaEpUpdateChain(struct UsbDmaEndpoint *ep)
{
  if (ep->head && !(ep->head->status & DD_STATUS_RETIRED))
  {
    LPC_USB_Type * const reg = ep->device->base.reg;
    const unsigned int index = EP_TO_INDEX(ep->address);
    const uint32_t mask = 1UL << index;

    ep->device->pool->heads[index] = ep->head;

    const uint8_t status = usbCommandRead(ep->device,
        USB_CMD_SELECT_ENDPOINT | index);

    if (!(ep->address & USB_EP_DIRECTION_IN) ^ !(status & SELECT_ENDPOINT_FE))
      reg->USBDMARSet = mask; /* Initiate a transfer */
    reg->USBEpDMAEn = mask;
  }
}
/*----------------------------------------------------------------------------*/
static struct DmaDescriptor *epAllocDescriptor(struct UsbDmaEndpoint *ep,
    struct UsbRequest *request)
{
  uint32_t control = DD_CONTROL_DMA_MODE(DMA_MODE_NORMAL)
      | DD_CONTROL_MAX_PACKET_SIZE(request->capacity);

  if (ep->type == ENDPOINT_TYPE_ISOCHRONOUS)
  {
    control |= DD_CONTROL_ISOCHRONOUS_EP | DD_CONTROL_DMA_BUFFER_LENGTH(1);
  }
  else
  {
    if (ep->address & USB_EP_DIRECTION_IN)
      control |= DD_CONTROL_DMA_BUFFER_LENGTH(request->length);
    else
      control |= DD_CONTROL_DMA_BUFFER_LENGTH(request->capacity);
  }

  struct DmaDescriptorPool * const pool = ep->device->pool;
  struct DmaDescriptor *descriptor;

  assert(!pointerArrayEmpty(&pool->descriptors));

  irqDisable(ep->device->base.irq);
  descriptor = pointerArrayBack(&pool->descriptors);
  pointerArrayPopBack(&pool->descriptors);
  irqEnable(ep->device->base.irq);

  descriptor->next = 0;
  descriptor->control = control;
  descriptor->buffer = (uint32_t)request->buffer;
  descriptor->status = 0;
  descriptor->size = 0;
  /* Store pointer to the USB Request structure in the reserved field */
  descriptor->request = (uint32_t)request;

  return descriptor;
}
/*----------------------------------------------------------------------------*/
static void epAppendDescriptor(struct UsbDmaEndpoint *ep,
    struct DmaDescriptor *descriptor)
{
  LPC_USB_Type * const reg = ep->device->base.reg;
  const uint32_t mask = 1UL << EP_TO_INDEX(ep->address);

  irqDisable(ep->device->base.irq);

  if (ep->tail)
  {
    /* The linked list is not empty */
    ep->tail->next = (uint32_t)descriptor;
    ep->tail->control |= DD_CONTROL_NEXT_DD_VALID;
    ep->tail = descriptor;
  }
  else
  {
    /* Reinitialize the linked list */
    ep->head = ep->tail = descriptor;
  }

  if (!(reg->USBEpDMASt & mask))
    reg->USBNDDRIntSet = mask;

  irqEnable(ep->device->base.irq);
}
/*----------------------------------------------------------------------------*/
static void epEnqueueRequest(struct UsbDmaEndpoint *ep,
    struct UsbRequest *request)
{
  struct DmaDescriptor * const descriptor = epAllocDescriptor(ep, request);
  epAppendDescriptor(ep, descriptor);
}
/*----------------------------------------------------------------------------*/
static enum Result dmaEpInit(void *object, const void *configBase)
{
  const struct UsbEndpointConfig * const config = configBase;
  struct UsbDmaEndpoint * const ep = object;

  ep->address = config->address;
  ep->device = config->parent;
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
  const unsigned int index = EP_TO_INDEX(ep->address);

  /* Remove pending requests */
  dmaEpDisable(ep);

  const IrqState state = irqSave();
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
  volatile struct DmaDescriptor *current = pool->heads[index];

  while (current)
  {
    struct UsbRequest * const request =
        (struct UsbRequest *)current->request;
    volatile struct DmaDescriptor * const next =
        (volatile struct DmaDescriptor *)current->next;

    irqDisable(ep->device->base.irq);
    pointerArrayPushBack(&pool->descriptors, (void *)current);
    irqEnable(ep->device->base.irq);

    request->callback(request->callbackArgument, request,
        USB_REQUEST_CANCELLED);
    current = next;
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
static enum Result dmaEpEnqueue(void *object, struct UsbRequest *request)
{
  assert(request->callback);

  epEnqueueRequest(object, request);
  return E_OK;
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
