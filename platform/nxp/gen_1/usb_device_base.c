/*
 * usb_device_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <usb/usb.h>
#include <platform/nxp/gen_1/usb_defs.h>
#include <platform/nxp/gen_1/usb_device_base.h>
/*----------------------------------------------------------------------------*/
#define CONFIG_USB_REQUESTS 4
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static void usbCommand(struct UsbDeviceBase *, uint8_t);
static uint8_t usbCommandRead(struct UsbDeviceBase *, uint8_t);
static void usbCommandWrite(struct UsbDeviceBase *, uint8_t, uint16_t);
static void waitForInt(struct UsbDeviceBase *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum result devInit(void *, const void *);
static void devDeinit(void *);
static void *devAllocate(void *, uint16_t, uint8_t);
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
static void epSetup(struct UsbDeviceBase *, uint8_t);
static enum result epReadData(struct UsbEndpoint *, uint8_t *, uint16_t,
    uint16_t *);
static enum result epWriteData(struct UsbEndpoint *, const uint8_t *, uint16_t,
    uint16_t *);
/*----------------------------------------------------------------------------*/
static enum result epInit(void *, const void *);
static void epDeinit(void *);
static void epClear(void *);
static enum result epEnqueue(void *, struct UsbRequest *);
static uint8_t epGetStatus(void *);
static void epSetEnabled(void *, bool);
static void epSetStalled(void *, bool);
/*----------------------------------------------------------------------------*/
static const struct UsbEndpointClass epTable = {
    .size = sizeof(struct UsbEndpoint),
    .init = epInit,
    .deinit = epDeinit,

    .clear = epClear,
    .enqueue = epEnqueue,
    .getStatus = epGetStatus,
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

  /* Device status interrupt */
  if (intStatus & USBDevIntSt_DEV_STAT)
  {
    reg->USBDevIntClr = USBDevIntSt_DEV_STAT;

    const uint8_t devStatus = usbCommandRead(device,
        USB_CMD_GET_DEVICE_STATUS);

    if (devStatus & (DEVICE_STATUS_CON_CH
        | DEVICE_STATUS_SUS_CH | DEVICE_STATUS_RST))
    {
      uint8_t status = 0;

      if (devStatus & DEVICE_STATUS_CON)
        status |= DEV_STATUS_CONNECT;
      if (devStatus & DEVICE_STATUS_SUS)
        status |= DEV_STATUS_SUSPEND;
      if (devStatus & DEVICE_STATUS_RST)
        status |= DEV_STATUS_RESET;

      /* TODO Call handler */
    }
  }

  /* Endpoint interrupt */
  if (intStatus & USBDevIntSt_EP_SLOW)
  {
    reg->USBDevIntClr = USBDevIntSt_EP_SLOW;

    /* Check registered endpoints */
    const struct ListNode *current = listFirst(&device->endpoints);
    struct UsbEndpoint *endpoint;

    while (current)
    {
      listData(&device->endpoints, current, &endpoint);

      const uint32_t mask = 1 << EP_TO_INDEX(endpoint->address);

      if (reg->USBEpIntSt & mask)
      {
        reg->USBEpIntClr = mask;

        waitForInt(device, USBDevIntSt_CDFULL);

        const uint32_t rawEpStatus = reg->USBCmdData;
        uint8_t status = 0;

        if (rawEpStatus & SELECT_ENDPOINT_FE)
          status |= EP_STATUS_DATA;
        if (rawEpStatus & SELECT_ENDPOINT_ST)
          status |= EP_STATUS_STALLED;
        if (rawEpStatus & SELECT_ENDPOINT_STP)
          status |= EP_STATUS_SETUP;
        if (rawEpStatus & SELECT_ENDPOINT_EPN)
          status |= EP_STATUS_NACKED;
        if (rawEpStatus & SELECT_ENDPOINT_PO)
          status |= EP_STATUS_ERROR;

        epHandler(endpoint, status);
      }
      current = listNext(current);
    }
  }
}
/*----------------------------------------------------------------------------*/
static void usbCommand(struct UsbDeviceBase *device, uint8_t command)
{
  LPC_USB_Type * const reg = device->parent.reg;

  /* Clear CDFULL and CCEMPTY */
  reg->USBDevIntClr = USBDevIntSt_CDFULL | USBDevIntSt_CCEMPTY;

  /* Write command code */
  reg->USBCmdCode = USBCmdCode_CMD_PHASE(USB_CMD_PHASE_COMMAND)
      | USBCmdCode_CMD_CODE(command);

  waitForInt(device, USBDevIntSt_CCEMPTY);
}
/*----------------------------------------------------------------------------*/
static uint8_t usbCommandRead(struct UsbDeviceBase *device, uint8_t command)
{
  LPC_USB_Type * const reg = device->parent.reg;

  /* Write command code */
  usbCommand(device, command);

  /* Read data */
  reg->USBCmdCode = USBCmdCode_CMD_PHASE(USB_CMD_PHASE_READ)
      | USBCmdCode_CMD_CODE(command);

  waitForInt(device, USBDevIntSt_CDFULL);

  return (uint8_t)reg->USBCmdData;
}
/*----------------------------------------------------------------------------*/
static void usbCommandWrite(struct UsbDeviceBase *device, uint8_t command,
    uint16_t data)
{
  LPC_USB_Type * const reg = device->parent.reg;

  /* Write command code */
  usbCommand(device, command);

  /* Write data */
  reg->USBCmdCode = USBCmdCode_CMD_PHASE(USB_CMD_PHASE_WRITE)
      | USBCmdCode_CMD_WDATA(data);

  waitForInt(device, USBDevIntSt_CCEMPTY);
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
static enum result devInit(void *object, const void *configBase)
{
  const struct UsbDeviceBaseConfig * const config = configBase;
  const struct UsbBaseConfig parentConfig = {
      .dm = config->dm,
      .dp = config->dp,
      .connect = config->connect,
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

  LPC_USB_Type * const reg = device->parent.reg;

  /* Enable clocks */
  reg->USBClkCtrl = USBClkCtrl_DEV_CLK_ON | USBClkCtrl_AHB_CLK_ON;

  const uint32_t clockStateMask = USBClkSt_DEV_CLK_ON | USBClkSt_AHB_CLK_ON;
  while ((reg->USBClkSt & clockStateMask) != clockStateMask);

  /* Disable and clear all interrupts */
  reg->USBDevIntEn = 0;
  reg->USBDevIntClr = 0xFFFFFFFF;
  reg->USBDevIntPri = 0;

  reg->USBEpIntEn = 0;
  reg->USBEpIntClr = 0xFFFFFFFF;
  reg->USBEpIntPri = 0;

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
  listDeinit(&device->endpoints);
  UsbBase->deinit(device);
}
/*----------------------------------------------------------------------------*/
static void *devAllocate(void *object, uint16_t size, uint8_t address)
{
  struct UsbDeviceBase * const device = object;
  const struct ListNode *current = listFirst(&device->endpoints);
  struct UsbEndpoint *endpoint;

  //TODO Spinlock
  while (current)
  {
    listData(&device->endpoints, current, &endpoint);
    //TODO Check size
    if (endpoint->address == address)
      return endpoint;
    current = listNext(current);
  }

  const struct UsbEndpointConfig config = {
    .parent = device,
    .size = size,
    .address = address
  };

  return init(UsbEndpoint, &config);
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

  if (endpoint->address & 0x80)
  {
    /* IN */
    if (!queueEmpty(&endpoint->requests))
    {
      queuePop(&endpoint->requests, &request);

      const enum result res = epWriteData(endpoint, request->buffer,
          request->length, 0);

      if (res != E_OK)
      {
        request->length = 0;
        //TODO Select error code
        request->status = EP_STATUS_STALLED;
      }
      else
      {
        request->status = 0; //FIXME
      }
    }

    if (queueEmpty(&endpoint->requests))
    {
      endpoint->busy = false;
    }
  }
  else
  {
    /* OUT */
    if (!queueEmpty(&endpoint->requests))
    {
      uint16_t read;

      queuePop(&endpoint->requests, &request);

      const enum result res = epReadData(endpoint, request->buffer,
          request->capacity, &read);

      if (res != E_OK)
      {
        request->length = 0;
        //TODO Select error code
        request->status = EP_STATUS_STALLED;
      }
      else
      {
        request->length = read;
        request->status = status;
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
static void epSetup(struct UsbDeviceBase *device, uint8_t address)
{
  LPC_USB_Type * const reg = device->parent.reg;
  const uint8_t index = EP_TO_INDEX(address);

  /* Enable interrupt */
  reg->USBEpIntEn |= 1 << index;
  reg->USBDevIntEn |= USBDevIntSt_EP_SLOW;
}
/*----------------------------------------------------------------------------*/
static enum result epReadData(struct UsbEndpoint *endpoint, uint8_t *buffer,
    uint16_t length, uint16_t *read)
{
  LPC_USB_Type * const reg = endpoint->device->parent.reg;

  const uint8_t index = EP_TO_INDEX(endpoint->address);

  /* Set read enable bit for specific endpoint */
  reg->USBCtrl = USBCtrl_RD_EN | ((endpoint->address & 0x0F) << 2);

  uint32_t packetLength;

  do
  {
    packetLength = reg->USBRxPLen;
  }
  while (!(packetLength & USBRxPLen_PKT_RDY));

  /* Check packet validity */
  if (!(packetLength & USBRxPLen_DV))
    return E_ERROR;

  /* Extract length */
  packetLength = USBRxPLen_PKT_LNGTH_VALUE(packetLength);

  /* Extract data */
  uint32_t data;

  for (uint16_t position = 0; position < packetLength; ++position)
  {
    if (!(position & 0x03))
      data = reg->USBRxData;

    if (buffer && position < length)
      buffer[position] = data & 0xFF;
    data >>= 8; // TODO Optimize
  }

  /* Clear read enable bit */
  reg->USBCtrl = 0; //FIXME

  /* Select endpoint and clear buffer */
  usbCommand(endpoint->device, USB_CMD_SELECT_ENDPOINT | index);
  usbCommand(endpoint->device, USB_CMD_CLEAR_BUFFER);

  if (read)
    *read = packetLength;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result epWriteData(struct UsbEndpoint *endpoint,
    const uint8_t *buffer, uint16_t length, uint16_t *written)
{
  LPC_USB_Type * const reg = endpoint->device->parent.reg;

  const uint8_t index = EP_TO_INDEX(endpoint->address);

  /* Set write enable for specific endpoint */
  reg->USBCtrl = USBCtrl_WR_EN | ((endpoint->address & 0x0F) << 2);
  /* Set packet length */
  reg->USBTxPLen = length;

  /* Write data */
  uint16_t position = 0;

  while (reg->USBCtrl & USBCtrl_WR_EN)
  {
    uint32_t word = 0;
    uint8_t chunk = length - position;

    if (chunk > 4)
      chunk = 4;

    memcpy(&word, buffer + position, chunk);
    /* The data is in little-endian format */
    reg->USBTxData = word;
    position += chunk;
  }

  /* Select endpoint and validate buffer */
  usbCommand(endpoint->device, USB_CMD_SELECT_ENDPOINT | index);
  usbCommand(endpoint->device, USB_CMD_VALIDATE_BUFFER);

  if (written)
    *written = length;

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
  endpoint->busy = false;
  endpoint->device = device;
  endpoint->size = config->size;

  epSetup(device, endpoint->address);

  if (!(endpoint->address & 0x7F)) //TODO Remove magic
  {
    /* Enable control endpoint, call function directly */
    epSetEnabled(endpoint, true);
  }

  listPush(&device->endpoints, &endpoint);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void epDeinit(void *object)
{
  const struct UsbEndpoint * const endpoint = object;

  //TODO
}
/*----------------------------------------------------------------------------*/
static void epClear(void *object)
{
  struct UsbEndpoint * const endpoint = object;
  struct UsbRequest *request = 0;

  while (!queueEmpty(&endpoint->requests))
  {
    queuePop(&endpoint->requests, &request);

    request->status = EP_STATUS_ERROR; //TODO
    if (request->callback)
      request->callback(request, request->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
static enum result epEnqueue(void *object, struct UsbRequest *request)
{
  struct UsbEndpoint * const endpoint = object;
  enum result res = E_OK;

  irqDisable(endpoint->device->parent.irq);

  if ((endpoint->address & 0x80) && !endpoint->busy) //FIXME Magic numbers
  {
    endpoint->busy = true;

    res = epWriteData(endpoint, request->buffer, request->length, 0);
    if (res != E_OK)
    {
      request->length = 0;
      //TODO Select error code
      request->status = EP_STATUS_STALLED;
    }
    else
    {
      request->status = 0;
    }

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
static uint8_t epGetStatus(void *object)
{
  struct UsbEndpoint * const endpoint = object;
  const uint8_t index = EP_TO_INDEX(endpoint->address);

  return usbCommandRead(endpoint->device, USB_CMD_SELECT_ENDPOINT | index);
}
/*----------------------------------------------------------------------------*/
static void epSetEnabled(void *object, bool state)
{
  struct UsbEndpoint * const endpoint = object;
  const uint8_t index = EP_TO_INDEX(endpoint->address);

  if (state)
  {
    LPC_USB_Type * const reg = endpoint->device->parent.reg;

    /* Realize endpoint */
    reg->USBReEp |= 1 << index;
    reg->USBEpInd = index;
    reg->USBMaxPSize = endpoint->size;
    waitForInt(endpoint->device, USBDevIntSt_EP_RLZED);
  }

  /* Enable or disable endpoint */
  usbCommandWrite(endpoint->device, USB_CMD_SET_ENDPOINT_STATUS | index,
      state ? 0 : SET_ENDPOINT_STATUS_DA);
}
/*----------------------------------------------------------------------------*/
static void epSetStalled(void *object, bool state)
{
  struct UsbEndpoint * const endpoint = object;
  const uint8_t index = EP_TO_INDEX(endpoint->address);

  usbCommandWrite(endpoint->device, USB_CMD_SET_ENDPOINT_STATUS | index,
      state ? SELECT_ENDPOINT_ST : 0);
}
