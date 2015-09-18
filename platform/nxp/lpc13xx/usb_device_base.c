/*
 * usb_device_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <delay.h>
#include <usb/usb.h>
#include <platform/nxp/usb_device_base.h>
#include <platform/nxp/lpc13xx/usb_defs.h>
/*----------------------------------------------------------------------------*/
#define CONFIG_USB_REQUESTS 4
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static void resetDevice(struct UsbDeviceBase *);
static void usbCommand(struct UsbDeviceBase *, uint8_t);
static uint8_t usbCommandRead(struct UsbDeviceBase *, uint8_t);
static void usbCommandWrite(struct UsbDeviceBase *, uint8_t, uint16_t);
static void waitForInt(struct UsbDeviceBase *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum result devInit(void *, const void *);
static void devDeinit(void *);
static void *devAllocate(void *, uint8_t);
static void devSetAddress(void *, uint8_t);
static void devSetConfigured(void *, bool);
static void devSetConnected(void *, bool);
/*----------------------------------------------------------------------------*/
static const struct UsbDeviceClass devTable = {
    .size = sizeof(struct UsbDeviceBase),
    .init = devInit,
    .deinit = devDeinit,

    .allocate = devAllocate,
    .bind = 0,
    .setAddress = devSetAddress,
    .setConfigured = devSetConfigured,
    .setConnected = devSetConnected
};
/*----------------------------------------------------------------------------*/
const struct UsbDeviceClass * const UsbDeviceBase = &devTable;
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
  struct UsbDeviceBase * const device = object;
  LPC_USB_Type * const reg = device->parent.reg;
  const uint32_t intStatus = reg->USBDevIntSt;

  reg->USBDevIntClr = intStatus;

  if (intStatus & USBDevInt_FRAME)
  {
    if (device->eventHandler)
      device->eventHandler(device->eventHandlerArgument, DEVICE_STATUS_FRAME);
  }

  /* Device status interrupt */
  if (intStatus & USBDevInt_DEV_STAT)
  {
    const uint8_t devStatus = usbCommandRead(device,
        USB_CMD_GET_DEVICE_STATUS);
    uint8_t status = 0;

    if (devStatus & DEVICE_STATUS_RST)
    {
      resetDevice(device);
      status |= DEVICE_STATUS_RESET;
    }
    if (devStatus & DEVICE_STATUS_CON)
      status |= DEVICE_STATUS_CONNECT;
    if (devStatus & DEVICE_STATUS_SUS)
      status |= DEVICE_STATUS_SUSPEND;

    if (status && device->eventHandler)
      device->eventHandler(device->eventHandlerArgument, status);
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
      const uint32_t mask = 1 << (index + 1);

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
static void resetDevice(struct UsbDeviceBase *device)
{
  LPC_USB_Type * const reg = device->parent.reg;

  /* Disable and clear all interrupts */
  reg->USBDevIntEn = 0;
  reg->USBDevIntClr = 0xFFFFFFFF;

  reg->USBDevIntEn = USBDevInt_DEV_STAT;
}
/*----------------------------------------------------------------------------*/
static void usbCommand(struct UsbDeviceBase *device, uint8_t command)
{
  LPC_USB_Type * const reg = device->parent.reg;

  /* Write command code and wait for completion */
  reg->USBDevIntClr = USBDevInt_CCEMPTY;
  reg->USBCmdCode = USBCmdCode_CMD_PHASE(USB_CMD_PHASE_COMMAND)
      | USBCmdCode_CMD_CODE(command);
  waitForInt(device, USBDevInt_CCEMPTY);
}
/*----------------------------------------------------------------------------*/
static uint8_t usbCommandRead(struct UsbDeviceBase *device, uint8_t command)
{
  LPC_USB_Type * const reg = device->parent.reg;

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
static void usbCommandWrite(struct UsbDeviceBase *device, uint8_t command,
    uint16_t data)
{
  LPC_USB_Type * const reg = device->parent.reg;

  /* Write command code */
  usbCommand(device, command);

  /* Write data and wait for completion */
  reg->USBDevIntClr = USBDevInt_CCEMPTY;
  reg->USBCmdCode = USBCmdCode_CMD_PHASE(USB_CMD_PHASE_WRITE)
      | USBCmdCode_CMD_WDATA(data);
  waitForInt(device, USBDevInt_CCEMPTY);
}
/*----------------------------------------------------------------------------*/
static void waitForInt(struct UsbDeviceBase *device, uint32_t mask)
{
  LPC_USB_Type * const reg = device->parent.reg;

  /* Wait for specific interrupt */
  while ((reg->USBDevIntSt & mask) != mask);
  /* Clear pending interrupt flags */
  reg->USBDevIntClr = mask;
}
/*----------------------------------------------------------------------------*/
void usbDeviceBaseSetHandler(struct UsbDeviceBase *device,
    void (*handler)(void *, uint8_t), void *argument)
{
  device->eventHandlerArgument = argument;
  device->eventHandler = handler;
}
/*----------------------------------------------------------------------------*/
static enum result devInit(void *object, const void *configBase)
{
  const struct UsbDeviceBaseConfig * const config = configBase;
  const struct UsbBaseConfig parentConfig = {
      .dm = config->dm,
      .dp = config->dp,
      .connect = config->connect,
      .vbus = config->vbus,
      .channel = config->channel
  };
  struct UsbDeviceBase * const device = object;
  enum result res;

  /* Call base class constructor */
  if ((res = UsbBase->init(object, &parentConfig)) != E_OK)
    return res;

  res = listInit(&device->endpoints, sizeof(struct UsbEndpoint *));
  if (res != E_OK)
    return res;

  device->parent.handler = interruptHandler;
  device->eventHandler = 0;
  device->spinlock = SPIN_UNLOCKED;

  /* Configure interrupts */
  resetDevice(device);
  /* By default, only ACKs generate interrupts */
  usbCommandWrite(device, USB_CMD_SET_MODE, 0);

  irqEnable(device->parent.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void devDeinit(void *object)
{
  struct UsbDeviceBase * const device = object;

  irqDisable(device->parent.irq);

  assert(listEmpty(&device->endpoints));
  listDeinit(&device->endpoints);

  UsbBase->deinit(device);
}
/*----------------------------------------------------------------------------*/
static void *devAllocate(void *object, uint8_t address)
{
  struct UsbDeviceBase * const device = object;

  irqDisable(device->parent.irq);
  spinLock(&device->spinlock);

  const struct ListNode *current = listFirst(&device->endpoints);
  struct UsbEndpoint *endpoint;

  while (current)
  {
    listData(&device->endpoints, current, &endpoint);
    if (endpoint->address == address)
      return endpoint;
    current = listNext(current);
  }

  const struct UsbEndpointConfig config = {
    .parent = device,
    .address = address
  };

  endpoint = init(UsbEndpoint, &config);

  spinUnlock(&device->spinlock);
  irqEnable(device->parent.irq);

  return endpoint;
}
/*----------------------------------------------------------------------------*/
static void devSetAddress(void *object, uint8_t address)
{
  usbCommandWrite(object, USB_CMD_SET_ADDRESS,
      SET_ADDRESS_DEV_EN | SET_ADDRESS_DEV_ADDR(address));
}
/*----------------------------------------------------------------------------*/
static void devSetConfigured(void *object, bool state)
{
  usbCommandWrite(object, USB_CMD_CONFIGURE_DEVICE,
      state ? CONFIGURE_DEVICE_CONF_DEVICE : 0);
}
/*----------------------------------------------------------------------------*/
static void devSetConnected(void *object, bool state)
{
  usbCommandWrite(object, USB_CMD_SET_DEVICE_STATUS,
      state ? DEVICE_STATUS_CON : 0);
}
/*----------------------------------------------------------------------------*/
void epHandler(struct UsbEndpoint *endpoint, uint8_t status)
{
  struct UsbRequest *request = 0;

  if (!queueEmpty(&endpoint->requests))
    queuePop(&endpoint->requests, &request);

  if (endpoint->address & EP_DIRECTION_IN)
  {
    if (request)
    {
      const enum result res = epWriteData(endpoint, request->buffer,
          request->length, 0);

      request->status = res == E_OK ? REQUEST_COMPLETED : REQUEST_ERROR;
    }
  }
  else
  {
    if (request)
    {
      uint16_t read;
      const enum result res = epReadData(endpoint, request->buffer,
          request->capacity, &read);

      if (res == E_OK)
      {
        request->length = read;
        request->status = status & SELECT_ENDPOINT_STP ?
            REQUEST_SETUP : REQUEST_COMPLETED;
      }
      else
      {
        request->length = 0;
        request->status = REQUEST_ERROR;
      }
    }
    else
    {
      const uint8_t index = EP_TO_INDEX(endpoint->address);

      /* Select endpoint and clear buffer */
      usbCommand(endpoint->device, USB_CMD_SELECT_ENDPOINT | index);
      usbCommand(endpoint->device, USB_CMD_CLEAR_BUFFER);
    }
  }

  if (request && request->callback)
    request->callback(request, request->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result epReadData(struct UsbEndpoint *endpoint, uint8_t *buffer,
    uint16_t length, uint16_t *read)
{
  LPC_USB_Type * const reg = endpoint->device->parent.reg;
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
    return E_ERROR;
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
  LPC_USB_Type * const reg = endpoint->device->parent.reg;
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
  struct UsbDeviceBase * const device = config->parent;
  struct UsbEndpoint * const endpoint = object;
  enum result res;

  res = queueInit(&endpoint->requests, sizeof(struct UsbRequest *),
      CONFIG_USB_REQUESTS);
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
  struct UsbDeviceBase * const device = endpoint->device;

  /* Disable interrupts and remove pending requests */
  epSetEnabled(endpoint, false, 0);
  epClear(endpoint);

  spinLock(&device->spinlock);

  struct ListNode *current = listFirst(&device->endpoints);
  struct UsbEndpoint *currentEndpoint;

  while (current)
  {
    listData(&device->endpoints, current, &currentEndpoint);
    if (currentEndpoint == endpoint)
    {
      listErase(&device->endpoints, current);
      break;
    }
    current = listNext(current);
  }

  spinUnlock(&device->spinlock);

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

    request->status = REQUEST_CANCELLED;
    if (request->callback)
      request->callback(request, request->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
static enum result epEnqueue(void *object, struct UsbRequest *request)
{
  struct UsbEndpoint * const endpoint = object;
  const uint8_t index = EP_TO_INDEX(endpoint->address);
  enum result res = E_OK;

  irqDisable(endpoint->device->parent.irq);

  const uint8_t status = usbCommandRead(endpoint->device,
      USB_CMD_SELECT_ENDPOINT | index);

  if ((endpoint->address & EP_DIRECTION_IN) && !(status & SELECT_ENDPOINT_FE))
  {
    res = epWriteData(endpoint, request->buffer, request->length, 0);
    request->status = res == E_OK ? REQUEST_COMPLETED : REQUEST_ERROR;

    if (request->callback)
      request->callback(request, request->callbackArgument);
  }
  else if (!queueFull(&endpoint->requests))
  {
    queuePush(&endpoint->requests, &request);
  }
  else
  {
    res = E_FULL;
  }

  irqEnable(endpoint->device->parent.irq);
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
  LPC_USB_Type * const reg = endpoint->device->parent.reg;
  const uint8_t index = EP_TO_INDEX(endpoint->address);
  const uint32_t mask = 1 << (index + 1);

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
