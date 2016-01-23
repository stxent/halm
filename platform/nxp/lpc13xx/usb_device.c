/*
 * usb_device.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <delay.h>
#include <usb/usb.h>
#include <platform/nxp/usb_device.h>
#include <platform/nxp/lpc13xx/usb_defs.h>
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
static enum result epReadData(struct UsbEndpoint *, uint8_t *, uint16_t,
    uint16_t *);
static enum result epWriteData(struct UsbEndpoint *, const uint8_t *, uint16_t,
    uint16_t *);
/*----------------------------------------------------------------------------*/
static enum result epInit(void *, const void *);
static void epDeinit(void *);
static void epClear(void *);
static enum result epEnqueue(void *, struct UsbRequest *);
static bool epIsStalled(void *);
static void epSetEnabled(void *, bool, uint16_t);
static void epSetStalled(void *, bool);
/*----------------------------------------------------------------------------*/
static const struct UsbEndpointClass epTable = {
    .size = sizeof(struct UsbEndpoint),
    .init = epInit,
    .deinit = epDeinit,

    .clear = epClear,
    .enqueue = epEnqueue,
    .isStalled = epIsStalled,
    .setEnabled = epSetEnabled,
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

  reg->USBDevIntClr = intStatus;

  if (intStatus & USBDevInt_FRAME)
  {
    usbControlUpdateStatus(device->control, DEVICE_STATUS_FRAME);
  }

  /* Device status interrupt */
  if (intStatus & USBDevInt_DEV_STAT)
  {
    const uint8_t deviceStatus = usbCommandRead(device,
        USB_CMD_GET_DEVICE_STATUS);
    uint8_t status = 0;

    if (deviceStatus & DEVICE_STATUS_RST)
    {
      resetDevice(device);
      status |= DEVICE_STATUS_RESET;
    }

    if (deviceStatus & DEVICE_STATUS_CON)
      status |= DEVICE_STATUS_CONNECTED;

    if (deviceStatus & DEVICE_STATUS_SUS)
      status |= DEVICE_STATUS_SUSPENDED;
    else
      device->configuration = 0;

    usbControlUpdateStatus(device->control, status);
  }

  /* Endpoint interrupt */
  if (intStatus & USBDevInt_EP_MASK)
  {
    /* Check registered endpoints */
    const struct ListNode *current = listFirst(&device->endpoints);
    struct UsbEndpoint *endpoint;

    while (current)
    {
      listData(&device->endpoints, current, &endpoint);

      const uint8_t index = EP_TO_INDEX(endpoint->address);
      const uint32_t mask = BIT(index + 1);

      if (intStatus & mask)
      {
        const uint8_t status = usbCommandRead(device,
            USB_CMD_CLEAR_INTERRUPT | index);

        epHandler(endpoint, status);
      }
      current = listNext(current);
    }
  }
}
/*----------------------------------------------------------------------------*/
static void resetDevice(struct UsbDevice *device)
{
  LPC_USB_Type * const reg = device->base.reg;

  /* Disable and clear all interrupts */
  reg->USBDevIntEn = 0;
  reg->USBDevIntClr = 0xFFFFFFFF;

  reg->USBDevIntEn = USBDevInt_DEV_STAT;
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

  return (uint8_t)reg->USBCmdData;
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
  device->configuration = 0; /* Inactive configuration */

  /* Configure interrupts */
  resetDevice(device);
  /* By default, only ACKs generate interrupts */
  usbCommandWrite(device, USB_CMD_SET_MODE, 0);

  irqSetPriority(device->base.irq, config->priority);
  irqEnable(device->base.irq);

  /* Initialize control message handler */
  device->control = init(UsbControl, &controlConfig);
  if (!device->control)
    return E_ERROR;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void devDeinit(void *object)
{
  struct UsbDevice * const device = object;

  deinit(device->control);

  irqDisable(device->base.irq);

  assert(listEmpty(&device->endpoints));
  listDeinit(&device->endpoints);

  UsbBase->deinit(device);
}
/*----------------------------------------------------------------------------*/
static void *devCreateEndpoint(void *object, uint8_t address)
{
  /* Assume that this function will be called only from one thread */
  struct UsbDevice * const device = object;

  irqDisable(device->base.irq);

  const struct ListNode *current = listFirst(&device->endpoints);
  struct UsbEndpoint *endpoint = 0;

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

  irqEnable(device->base.irq);
  return endpoint;
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
  enum result res;

  irqDisable(device->base.irq);
  res = usbControlBindDriver(device->control, driver);
  irqEnable(device->base.irq);

  return res;
}
/*----------------------------------------------------------------------------*/
static void devUnbind(void *object, const void *driver __attribute__((unused)))
{
  struct UsbDevice * const device = object;

  irqDisable(device->base.irq);
  usbControlUnbindDriver(device->control);
  irqEnable(device->base.irq);
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
  enum usbRequestStatus requestStatus = REQUEST_ERROR;

  if (queueEmpty(&endpoint->requests))
    return;
  queuePop(&endpoint->requests, &request);

  if (endpoint->address & EP_DIRECTION_IN)
  {
    if (epWriteData(endpoint, request->buffer, request->base.length, 0) == E_OK)
    {
      requestStatus = REQUEST_COMPLETED;
    }
  }
  else
  {
    uint16_t read;

    if (epReadData(endpoint, request->buffer,
        request->base.capacity, &read) == E_OK)
    {
      request->base.length = read;
      requestStatus = status & SELECT_ENDPOINT_STP ?
          REQUEST_SETUP : REQUEST_COMPLETED;
    }
    else
    {
      /* Read failed, return request to the queue */
      queuePush(&endpoint->requests, &request);
      return;
    }
  }

  request->base.callback(request->base.callbackArgument, request,
      requestStatus);
}
/*----------------------------------------------------------------------------*/
static enum result epReadData(struct UsbEndpoint *endpoint, uint8_t *buffer,
    uint16_t length, uint16_t *read)
{
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const uint8_t index = EP_TO_INDEX(endpoint->address);

  /* Set read enable bit for specific endpoint */
  reg->USBCtrl = USBCtrl_RD_EN
      | USBCtrl_LOG_ENDPOINT(EP_LOGICAL_ADDRESS(endpoint->address));

  /*
   * User Manual says that it takes 3 clock cycle to fetch the packet length
   * from the RAM. It seems that this delay should be a little bit higher.
   */
  delayTicks(4);

  uint32_t packetLength = reg->USBRxPLen;

  /* Check packet validity */
  if (!(packetLength & USBRxPLen_DV))
    return E_INTERFACE;
  /* Extract length */
  packetLength = USBRxPLen_PKT_LNGTH_VALUE(packetLength);
  /* Check for buffer overflow */
  if (packetLength > length)
    return E_VALUE;

  if (read)
    *read = packetLength;

  /* Read data from internal buffer */
  uint32_t word = 0;

  for (uint16_t position = 0; position < packetLength; ++position)
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
    const uint8_t *buffer, uint16_t length, uint16_t *written)
{
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const uint8_t index = EP_TO_INDEX(endpoint->address);

  /* Set write enable for specific endpoint */
  reg->USBCtrl = USBCtrl_WR_EN
      | USBCtrl_LOG_ENDPOINT(EP_LOGICAL_ADDRESS(endpoint->address));
  /* Set packet length */
  reg->USBTxPLen = length;

  if (written)
    *written = length;

  /* Write data */
  uint32_t word = 0;
  uint16_t position = 0;

  while (position < length)
  {
    *((uint8_t *)&word + (position & 0x03)) = buffer[position];
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
  epSetEnabled(endpoint, false, 0);
  epClear(endpoint);

  /* Protect endpoint queue from simultaneous access */
  irqDisable(device->base.irq);

  struct ListNode * const node = listFind(&device->endpoints, &endpoint);

  if (node)
    listErase(&device->endpoints, node);

  irqEnable(device->base.irq);

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
    request->base.callback(request->base.callbackArgument, request,
        REQUEST_CANCELLED);
  }
}
/*----------------------------------------------------------------------------*/
static enum result epEnqueue(void *object, struct UsbRequest *request)
{
  struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const uint8_t index = EP_TO_INDEX(endpoint->address);
  const uint32_t mask = BIT(index + 1);
  enum result res = E_OK;

  assert(request->base.callback);

  irqDisable(endpoint->device->base.irq);

  /*
   * Additional checks should be performed for data endpoints
   * to avoid USB controller hanging issues.
   */
  if (index >= 2 && !endpoint->device->configuration)
  {
    irqEnable(endpoint->device->base.irq);
    return E_IDLE;
  }

  const uint8_t status = usbCommandRead(endpoint->device,
      USB_CMD_SELECT_ENDPOINT | index);

  if (!queueFull(&endpoint->requests))
  {
    queuePush(&endpoint->requests, &request);

    const bool schedule = ((endpoint->address & EP_DIRECTION_IN) != 0)
        ^ ((status & SELECT_ENDPOINT_FE) != 0);

    if (schedule)
    {
      /* Schedule interrupt */
      reg->USBDevIntSet = mask;
    }
  }
  else
  {
    res = E_FULL;
  }

  irqEnable(endpoint->device->base.irq);
  return res;
}
/*----------------------------------------------------------------------------*/
static bool epIsStalled(void *object)
{
  struct UsbEndpoint * const endpoint = object;
  const uint8_t index = EP_TO_INDEX(endpoint->address);
  const uint8_t status = usbCommandRead(endpoint->device,
      USB_CMD_SELECT_ENDPOINT | index);

  return (status & SELECT_ENDPOINT_ST) != 0;
}
/*----------------------------------------------------------------------------*/
static void epSetEnabled(void *object, bool state,
    uint16_t size __attribute__((unused)))
{
  struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const uint8_t index = EP_TO_INDEX(endpoint->address);
  const uint32_t mask = BIT(index + 1);

  if (state)
  {
    // TODO Check whether SIE clear is needed
    reg->USBDevIntClr = mask;
    reg->USBDevIntEn |= mask;
  }
  else
  {
    reg->USBDevIntEn &= ~mask;
  }

  usbCommandWrite(endpoint->device, USB_CMD_SET_ENDPOINT_STATUS | index,
      state ? 0 : SET_ENDPOINT_STATUS_DA);
}
/*----------------------------------------------------------------------------*/
static void epSetStalled(void *object, bool stalled)
{
  struct UsbEndpoint * const endpoint = object;
  const uint8_t index = EP_TO_INDEX(endpoint->address);

  usbCommandWrite(endpoint->device, USB_CMD_SET_ENDPOINT_STATUS | index,
      stalled ? SET_ENDPOINT_STATUS_ST : 0);
}
