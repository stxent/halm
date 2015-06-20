/*
 * usb_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <usb/usb.h>
#include <platform/nxp/lpc17xx/system.h>
#include <platform/nxp/lpc17xx/usb_base.h>
#include <platform/nxp/lpc17xx/usb_defs.h>
/*----------------------------------------------------------------------------*/
#define CONFIG_USB_REQUESTS 4
/*----------------------------------------------------------------------------*/
static enum result configPins(struct UsbDevice *,
    const struct UsbDeviceConfig *);
static enum result setDescriptor(uint8_t, const struct UsbDevice *,
    struct UsbDevice *);
static void usbCommand(struct UsbDevice *, uint8_t);
static void usbCommandWrite(struct UsbDevice *, uint8_t, uint16_t);
static void waitForInt(struct UsbDevice *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum result usbInit(void *, const void *);
static void usbDeinit(void *);
static void *usbAllocate(void *, uint16_t, uint8_t);
/*----------------------------------------------------------------------------*/
static const struct UsbDeviceClass usbTable = {
    .size = sizeof(struct UsbDevice),
    .init = usbInit,
    .deinit = usbDeinit,

    .allocate = usbAllocate
};
/*----------------------------------------------------------------------------*/
// TODO Other USB pins
const struct PinEntry usbPins[] = {
    {
        .key = PIN(0, 29), /* USB_D+ */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 30), /* USB_D- */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(2, 9), /* USB_CONNECT */
        .channel = 0,
        .value = 1
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct UsbDeviceClass * const UsbDevice = &usbTable;
static struct UsbDevice *descriptors[1] = {0};
/*----------------------------------------------------------------------------*/
static void epHandler(struct UsbEndpoint *, uint8_t);
static enum result epSetup(struct UsbDevice *, uint16_t, uint8_t);
enum result epReadData(struct UsbEndpoint *, uint8_t *, uint16_t, uint16_t *);
enum result epWriteData(struct UsbEndpoint *, const uint8_t *, uint16_t,
    uint16_t *);
/*----------------------------------------------------------------------------*/
static enum result epInit(void *, const void *);
static void epDeinit(void *);
static void *epAllocate(const void *);
static enum result epEnqueue(void *, struct UsbRequest *);
static void epErase(void *, const struct UsbRequest *);
static void epStall(void *, bool);
/*----------------------------------------------------------------------------*/
static const struct UsbEndpointClass epTable = {
    .size = sizeof(struct UsbEndpoint),
    .init = epInit,
    .deinit = epDeinit,

    .allocate = epAllocate,
    .enqueue = epEnqueue,
    .erase = epErase,
    .stall = epStall
};
/*----------------------------------------------------------------------------*/
const struct UsbEndpointClass * const UsbEndpoint = &epTable;
/*----------------------------------------------------------------------------*/
static enum result configPins(struct UsbDevice *device,
    const struct UsbDeviceConfig *config)
{
  const pin_t pinArray[] = {
      config->dm, config->dp, config->usbConnect
  };
  const struct PinEntry *pinEntry;
  struct Pin pin;

  for (uint8_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      pinEntry = pinFind(usbPins, pinArray[index], device->channel);
      if (!pinEntry)
        return E_VALUE;
      pinInput((pin = pinInit(pinArray[index])));
      pinSetFunction(pin, pinEntry->value);
    }
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel,
    const struct UsbDevice *state, struct UsbDevice *device)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state,
      device) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
static void usbCommand(struct UsbDevice *device, uint8_t command)
{
  LPC_USB_Type * const reg = device->reg;

  /* Clear CDFULL and CCEMPTY */
  reg->USBDevIntClr = USBDevIntSt_CDFULL | USBDevIntSt_CCEMPTY;

  /* Write command code */
  reg->USBCmdCode = USBCmdCode_CMD_PHASE(USB_CMD_PHASE_COMMAND)
      | USBCmdCode_CMD_CODE(command);

  waitForInt(device, USBDevIntSt_CCEMPTY);
}
/*----------------------------------------------------------------------------*/
static void usbCommandWrite(struct UsbDevice *device, uint8_t command,
    uint16_t data)
{
  LPC_USB_Type * const reg = device->reg;

  /* Write command code */
  usbCommand(device, command);

  /* Write command data */
  reg->USBCmdCode = USBCmdCode_CMD_PHASE(USB_CMD_PHASE_WRITE)
      | USBCmdCode_CMD_WDATA(data);

  waitForInt(device, USBDevIntSt_CCEMPTY);
}
/*----------------------------------------------------------------------------*/
static void waitForInt(struct UsbDevice *device, uint32_t mask)
{
  LPC_USB_Type * const reg = device->reg;

  /* Wait for specific interrupt */
  while ((reg->USBDevIntSt & mask) != mask);
  /* Clear pending interrupt flags */
  reg->USBDevIntClr = mask;
}
/*----------------------------------------------------------------------------*/
//FIXME
bool uhan(void *object)
{
  struct UsbDevice * const device = object;
  LPC_USB_Type * const reg = device->reg;
  const uint32_t intStatus = reg->USBDevIntSt;

  // Endpoint interrupt
  if (intStatus & USBDevIntSt_EP_SLOW)
  {
    // Clear EP_SLOW
    reg->USBDevIntClr = USBDevIntSt_EP_SLOW;

    // Check registered endpoints
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
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void USB_ISR(void)
{
  descriptors[0]->handler(descriptors[0]);
  uhan(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
static enum result usbInit(void *object, const void *configBase)
{
  const struct UsbDeviceConfig * const config = configBase;
  struct UsbDevice * const device = object;
  enum result res;

  /* Try to set peripheral descriptor */
  device->channel = config->channel;

  res = setDescriptor(device->channel, 0, device);
  if (res != E_OK)
    return res;

  res = configPins(device, configBase);
  if (res != E_OK)
    return res;

  res = listInit(&device->endpoints, sizeof(struct UsbEndpoint *));
  if (res != E_OK)
    return res;

  sysPowerEnable(PWR_USB);

  device->handler = 0;
  device->irq = USB_IRQ;
  device->reg = LPC_USB;

  LPC_USB_Type * const reg = device->reg;

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

  irqEnable(device->irq); //FIXME

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void usbDeinit(void *object)
{
  const struct UsbDevice * const device = object;

  sysPowerDisable(PWR_USB);
  setDescriptor(device->channel, device, 0);
}
/*----------------------------------------------------------------------------*/
static void *usbAllocate(void *device, uint16_t size, uint8_t address)
{
  const struct UsbEndpointConfig config = {
    .parent = device,
    .size = size,
    .address = address
  };

  return init(UsbEndpoint, &config);
}
/*----------------------------------------------------------------------------*/
void epHandler(struct UsbEndpoint *endpoint, uint8_t status)
{
  enum result res;

  if (endpoint->address & 0x80)
  {
    /* IN */
    if (!queueEmpty(&endpoint->requests))
    {
      struct UsbRequest *request;

      queuePop(&endpoint->requests, &request);
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

    if (queueEmpty(&endpoint->requests))
    {
      endpoint->busy = false;
//      USBHwNakIntEnablex(0);
    }
  }
  else
  {
    /* OUT */
    if (!queueEmpty(&endpoint->requests))
    {
      struct UsbRequest *request;
      uint16_t read;

      queuePop(&endpoint->requests, &request);
      res = epReadData(endpoint, request->buffer, request->capacity, &read);

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

      if (request->callback)
        request->callback(request, request->callbackArgument);
    }
    else
    {
      // TODO Clear buffer
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum result epSetup(struct UsbDevice *device, uint16_t size,
    uint8_t address)
{
  LPC_USB_Type * const reg = device->reg;

  const uint8_t index = EP_TO_INDEX(address);

  /* Only control endpoints will be enabled immediately */
  if (index <= 1)
  {
    /* Realize endpoint */
    reg->USBReEp |= 1 << index;
    reg->USBEpInd = index;
    reg->USBMaxPSize = size;
    waitForInt(device, USBDevIntSt_EP_RLZED);

    /* Enable endpoint */
    usbCommandWrite(device, USB_CMD_SET_ENDPOINT_STATUS | index, 0);
  }

  /* Enable interrupt */
  reg->USBEpIntEn |= 1 << index;
  reg->USBDevIntEn |= USBDevIntSt_EP_SLOW;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
enum result epReadData(struct UsbEndpoint *endpoint, uint8_t *buffer,
    uint16_t length, uint16_t *read)
{
  LPC_USB_Type * const reg = endpoint->device->reg;

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

  // select endpoint and clear buffer
  usbCommand(endpoint->device, USB_CMD_SELECT_ENDPOINT | index);
  usbCommand(endpoint->device, USB_CMD_CLEAR_BUFFER);

  if (read)
    *read = packetLength;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
enum result epWriteData(struct UsbEndpoint *endpoint, const uint8_t *buffer,
    uint16_t length, uint16_t *written)
{
  LPC_USB_Type * const reg = endpoint->device->reg;

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
  struct UsbDevice * const device = config->parent;
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

  res = epSetup(device, endpoint->size, endpoint->address);
  if (res != E_OK)
    return res;

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
static void *epAllocate(const void *object)
{
  const struct UsbEndpoint * const endpoint = object;
  const struct UsbRequestConfig config = {
    .size = endpoint->size
  };

  return init(UsbRequest, &config);
}
/*----------------------------------------------------------------------------*/
static enum result epEnqueue(void *object, struct UsbRequest *request)
{
  struct UsbEndpoint * const endpoint = object;
  //  const bool empty = queueEmpty(&endpoint->requests);

  if ((endpoint->address & 0x80) && !endpoint->busy) //FIXME Magic numbers
  {
    endpoint->busy = true; //FIXME Spinlock

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
      request->status = 0;
    }

    if (request->callback)
      request->callback(request, request->callbackArgument);
  }
  else
  {
    queuePush(&endpoint->requests, &request);
  }

  //  if (empty)
  //  {
  //    USBHwNakIntEnablex(INACK_CI | INACK_II | INACK_BI);
  //  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void epErase(void *object, const struct UsbRequest *request)
{

}
/*----------------------------------------------------------------------------*/
static void epStall(void *object, bool state)
{

}
/*----------------------------------------------------------------------------*/
void USBHwNakIntEnablex(struct UsbDevice *device, uint8_t bIntBits)
{
  usbCommandWrite(device, USB_CMD_SET_MODE, bIntBits << 1);
}
