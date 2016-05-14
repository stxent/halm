/*
 * usb_device.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <platform/nxp/lpc17xx/usb_defs.h>
#include <platform/nxp/usb_device.h>
#include <usb/usb.h>
/*----------------------------------------------------------------------------*/
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
static enum usbSpeed devGetSpeed(const void *);
static void devSetAddress(void *, uint8_t);
static void devSetConnected(void *, bool);
static enum result devBind(void *, void *);
static void devUnbind(void *, const void *);
static uint8_t devGetConfiguration(const void *);
static void devSetConfiguration(void *, uint8_t);
static enum result devAppendDescriptor(void *, const void *);
static void devEraseDescriptor(void *, const void *);
/*----------------------------------------------------------------------------*/
static const struct UsbDeviceClass devTable = {
    .size = sizeof(struct UsbDevice),
    .init = devInit,
    .deinit = devDeinit,

    .createEndpoint = devCreateEndpoint,
    .getSpeed = devGetSpeed,
    .setAddress = devSetAddress,
    .setConnected = devSetConnected,

    .bind = devBind,
    .unbind = devUnbind,

    .getConfiguration = devGetConfiguration,
    .setConfiguration = devSetConfiguration,

    .appendDescriptor = devAppendDescriptor,
    .eraseDescriptor = devEraseDescriptor
};
/*----------------------------------------------------------------------------*/
const struct UsbDeviceClass * const UsbDevice = &devTable;
/*----------------------------------------------------------------------------*/
static void epHandler(struct UsbEndpoint *, uint8_t);
static enum result epReadData(struct UsbEndpoint *, uint8_t *, size_t,
    size_t *);
static enum result epWriteData(struct UsbEndpoint *, const uint8_t *, size_t);
/*----------------------------------------------------------------------------*/
static enum result epInit(void *, const void *);
static void epDeinit(void *);
static void epClear(void *);
static void epDisable(void *);
static void epEnable(void *, uint8_t, uint16_t);
static enum result epEnqueue(void *, struct UsbRequest *);
static bool epIsStalled(void *);
static void epSetStalled(void *, bool);
/*----------------------------------------------------------------------------*/
static const struct UsbEndpointClass epTable = {
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
const struct UsbEndpointClass * const UsbEndpoint = &epTable;
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

    /* Check registered endpoints */
    const uint32_t epIntStatus = reg->USBEpIntSt;
    const struct ListNode *current = listFirst(&device->endpoints);
    struct UsbEndpoint *endpoint;

    while (current)
    {
      listData(&device->endpoints, current, &endpoint);

      const unsigned int index = EP_TO_INDEX(endpoint->address);
      const uint32_t mask = BIT(index);

      if (epIntStatus & mask)
      {
        const uint8_t status = usbCommandRead(device,
            USB_CMD_CLEAR_INTERRUPT | index);

        reg->USBEpIntClr = mask;

        epHandler(endpoint, status);
      }
      current = listNext(current);
    }
  }

  /* Start of Frame interrupt */
  if (intStatus & USBDevInt_FRAME)
  {
    usbControlEvent(device->control, USB_DEVICE_EVENT_FRAME);
  }
}
/*----------------------------------------------------------------------------*/
static void resetDevice(struct UsbDevice *device)
{
  LPC_USB_Type * const reg = device->base.reg;

  /* Set inactive configuration */
  device->configuration = 0;

  /* Reset device interrupts */
  reg->USBDevIntEn = 0;
  reg->USBDevIntClr = 0xFFFFFFFF;
  reg->USBDevIntPri = 0;
  /* Reset endpoint interrupts */
  reg->USBEpIntEn = 0;
  reg->USBEpIntClr = 0xFFFFFFFF;
  reg->USBEpIntPri = 0;

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
      .parent = object
  };
  struct UsbDevice * const device = object;
  enum result res;

  /* Call base class constructor */
  res = UsbBase->init(object, &baseConfig);
  if (res != E_OK)
    return res;

  res = listInit(&device->endpoints, sizeof(struct UsbEndpoint *));
  if (res != E_OK)
    return res;

  device->base.handler = interruptHandler;

  /* Initialize control message handler */
  device->control = init(UsbControl, &controlConfig);
  if (!device->control)
    return E_ERROR;

  /* Configure interrupts and reset system variables */
  resetDevice(device);
  /* By default, only ACKs generate interrupts */
  usbCommandWrite(device, USB_CMD_SET_MODE, 0);

  irqSetPriority(device->base.irq, config->priority);
  irqEnable(device->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void devDeinit(void *object)
{
  struct UsbDevice * const device = object;

  irqDisable(device->base.irq);

  deinit(device->control);

  assert(listEmpty(&device->endpoints));
  listDeinit(&device->endpoints);

  UsbBase->deinit(device);
}
/*----------------------------------------------------------------------------*/
static void *devCreateEndpoint(void *object, uint8_t address)
{
  /* Assume that this function will be called only from one thread */
  struct UsbDevice * const device = object;
  struct UsbEndpoint *endpoint = 0;

  const irqState state = irqSave();
  const struct ListNode *current = listFirst(&device->endpoints);

  while (current)
  {
    listData(&device->endpoints, current, &endpoint);
    if (endpoint->address == address)
      break;
    current = listNext(current);
  }

  if (!current)
  {
    const struct UsbEndpointConfig config = {
      .parent = device,
      .address = address
    };

    endpoint = init(UsbEndpoint, &config);
  }

  irqRestore(state);
  return endpoint;
}
/*----------------------------------------------------------------------------*/
static enum usbSpeed devGetSpeed(const void *object __attribute__((unused)))
{
  return USB_FS;
}
/*----------------------------------------------------------------------------*/
static void devSetAddress(void *object, uint8_t address)
{
  usbCommandWrite(object, USB_CMD_SET_ADDRESS,
      SET_ADDRESS_DEV_EN | SET_ADDRESS_DEV_ADDR(address));
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
static uint8_t devGetConfiguration(const void *object)
{
  const struct UsbDevice * const device = object;

  return device->configuration;
}
/*----------------------------------------------------------------------------*/
static void devSetConfiguration(void *object, uint8_t configuration)
{
  struct UsbDevice * const device = object;

  usbCommandWrite(object, USB_CMD_CONFIGURE_DEVICE,
      configuration > 0 ? CONFIGURE_DEVICE_CONF_DEVICE : 0);
  device->configuration = configuration;
}
/*----------------------------------------------------------------------------*/
static enum result devAppendDescriptor(void *object, const void *descriptor)
{
  struct UsbDevice * const device = object;

  return usbControlAppendDescriptor(device->control, descriptor);
}
/*----------------------------------------------------------------------------*/
static void devEraseDescriptor(void *object, const void *descriptor)
{
  struct UsbDevice * const device = object;

  usbControlEraseDescriptor(device->control, descriptor);
}
/*----------------------------------------------------------------------------*/
static void epHandler(struct UsbEndpoint *endpoint, uint8_t status)
{
  struct UsbRequest *request = 0;

  if (queueEmpty(&endpoint->requests))
    return;
  queuePop(&endpoint->requests, &request);

  if (endpoint->address & USB_EP_DIRECTION_IN)
  {
    struct UsbRequest *next = 0;

    if (!queueEmpty(&endpoint->requests))
      queuePeek(&endpoint->requests, &next);

    request->callback(request->callbackArgument, request,
        USB_REQUEST_COMPLETED);

    /* Send next packet */
    if (next)
    {
      if (epWriteData(endpoint, next->buffer, next->length) != E_OK)
      {
        queuePop(&endpoint->requests, 0);
        next->callback(next->callbackArgument, next, USB_REQUEST_ERROR);
      }
    }
  }
  else
  {
    size_t read;

    if (epReadData(endpoint, request->buffer, request->capacity, &read) == E_OK)
    {
      const enum usbRequestStatus requestStatus = status & SELECT_ENDPOINT_STP ?
          USB_REQUEST_SETUP : USB_REQUEST_COMPLETED;

      request->length = read;
      request->callback(request->callbackArgument, request, requestStatus);
    }
    else
    {
      /* Read failed, return request to the queue */
      queuePush(&endpoint->requests, &request);
      return;
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum result epReadData(struct UsbEndpoint *endpoint, uint8_t *buffer,
    size_t length, size_t *read)
{
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const unsigned int index = EP_TO_INDEX(endpoint->address);

  /* Set read enable bit for specific endpoint */
  reg->USBCtrl = USBCtrl_RD_EN
      | USBCtrl_LOG_ENDPOINT(USB_EP_LOGICAL_ADDRESS(endpoint->address));

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
  usbCommand(endpoint->device, USB_CMD_SELECT_ENDPOINT | index);
  usbCommand(endpoint->device, USB_CMD_CLEAR_BUFFER);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result epWriteData(struct UsbEndpoint *endpoint,
    const uint8_t *buffer, size_t length)
{
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const unsigned int index = EP_TO_INDEX(endpoint->address);

  /* Set write enable for specific endpoint */
  reg->USBCtrl = USBCtrl_WR_EN
      | USBCtrl_LOG_ENDPOINT(USB_EP_LOGICAL_ADDRESS(endpoint->address));
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
  usbCommand(endpoint->device, USB_CMD_SELECT_ENDPOINT | index);
  usbCommand(endpoint->device, USB_CMD_VALIDATE_BUFFER);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result epInit(void *object, const void *configBase)
{
  const struct UsbEndpointConfig * const config = configBase;
  struct UsbDevice * const device = config->parent;
  struct UsbEndpoint * const endpoint = object;
  enum result res;

  res = queueInit(&endpoint->requests, sizeof(struct UsbRequest *),
      CONFIG_USB_DEVICE_ENDPOINT_REQUESTS);
  if (res != E_OK)
    return res;

  endpoint->address = config->address;
  endpoint->device = device;

  listPush(&device->endpoints, &endpoint);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void epDeinit(void *object)
{
  struct UsbEndpoint * const endpoint = object;
  struct UsbDevice * const device = endpoint->device;

  /* Disable interrupts and remove pending requests */
  epDisable(endpoint);
  epClear(endpoint);

  const irqState state = irqSave();
  struct ListNode * const node = listFind(&device->endpoints, &endpoint);

  if (node)
    listErase(&device->endpoints, node);

  irqRestore(state);

  assert(queueEmpty(&endpoint->requests));
  queueDeinit(&endpoint->requests);
}
/*----------------------------------------------------------------------------*/
static void epClear(void *object)
{
  struct UsbEndpoint * const endpoint = object;
  struct UsbRequest *request;

  while (!queueEmpty(&endpoint->requests))
  {
    queuePop(&endpoint->requests, &request);
    request->callback(request->callbackArgument, request,
        USB_REQUEST_CANCELLED);
  }
}
/*----------------------------------------------------------------------------*/
static void epDisable(void *object)
{
  struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const unsigned int index = EP_TO_INDEX(endpoint->address);
  const uint32_t mask = BIT(index);

  reg->USBEpIntEn &= ~mask;

  usbCommandWrite(endpoint->device, USB_CMD_SET_ENDPOINT_STATUS | index,
      SET_ENDPOINT_STATUS_DA);
}
/*----------------------------------------------------------------------------*/
static void epEnable(void *object, uint8_t type __attribute__((unused)),
    uint16_t size)
{
  struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const unsigned int index = EP_TO_INDEX(endpoint->address);
  const uint32_t mask = BIT(index);

  /* Enable interrupt */
  reg->USBEpIntClr = mask;
  reg->USBEpIntEn |= mask;

  /* Realize endpoint */
  reg->USBReEp |= mask;
  reg->USBEpInd = index;
  reg->USBMaxPSize = size;
  waitForInt(endpoint->device, USBDevInt_EP_RLZED);

  usbCommandWrite(endpoint->device, USB_CMD_SET_ENDPOINT_STATUS | index, 0);
}
/*----------------------------------------------------------------------------*/
static enum result epEnqueue(void *object, struct UsbRequest *request)
{
  assert(request->callback);

  struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const unsigned int index = EP_TO_INDEX(endpoint->address);
  const uint32_t mask = BIT(index);

  /*
   * Additional checks should be performed for data endpoints
   * to avoid USB controller hanging.
   */
  if (index >= 2 && !endpoint->device->configuration)
    return E_IDLE;

  const irqState state = irqSave();

  const uint8_t status = usbCommandRead(endpoint->device,
      USB_CMD_SELECT_ENDPOINT | index);
  enum result res = E_OK;

  if (!queueFull(&endpoint->requests))
  {
    if (endpoint->address & USB_EP_DIRECTION_IN)
    {
      if (!(status & SELECT_ENDPOINT_FE) && queueEmpty(&endpoint->requests))
      {
        if (epWriteData(endpoint, request->buffer, request->length) != E_OK)
          res = E_ERROR;
      }
    }
    else
    {
      if (status & SELECT_ENDPOINT_FE)
      {
        /* Schedule interrupt */
        reg->USBDevIntSet = mask;
      }
    }

    if (res == E_OK)
      queuePush(&endpoint->requests, &request);
  }
  else
    res = E_FULL;

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
static bool epIsStalled(void *object)
{
  struct UsbEndpoint * const endpoint = object;
  const unsigned int index = EP_TO_INDEX(endpoint->address);
  const uint8_t status = usbCommandRead(endpoint->device,
      USB_CMD_SELECT_ENDPOINT | index);

  return (status & SELECT_ENDPOINT_ST) != 0;
}
/*----------------------------------------------------------------------------*/
static void epSetStalled(void *object, bool stalled)
{
  struct UsbEndpoint * const endpoint = object;
  const unsigned int index = EP_TO_INDEX(endpoint->address);

  usbCommandWrite(endpoint->device, USB_CMD_SET_ENDPOINT_STATUS | index,
      stalled ? SET_ENDPOINT_STATUS_ST : 0);
}
