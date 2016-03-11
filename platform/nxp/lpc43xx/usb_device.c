/*
 * usb_device.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <platform/nxp/lpc43xx/usb_defs.h>
#include <platform/nxp/usb_device.h>
#include <usb/usb.h>
#include <usb/usb_defs.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static void resetDevice(struct UsbDevice *);
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
static enum result epEnqueueRx(struct UsbEndpoint *, struct UsbRequest *);
static enum result epEnqueueTx(struct UsbEndpoint *, struct UsbRequest *);
static enum result epExtractDataLength(const struct UsbEndpoint *, size_t *);
static enum result epPrepareTransferDescriptor(struct UsbEndpoint *, uint8_t *,
    size_t);
static enum result epReadSetupPacket(struct UsbEndpoint *, uint8_t *, size_t,
    size_t *);
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
struct EndpointTransferDescriptor ep_TD[ENDPT_NUMBER] __attribute__((aligned(32)));
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct UsbDevice * const device = object;
  LPC_USB_Type * const reg = device->base.reg;

  const uint32_t intStatus = reg->USBSTS_D;
  reg->USBSTS_D = intStatus;

  if (intStatus & USBSTS_D_URI)
  {
    uint8_t status = 0;
    resetDevice(device);
    status |= DEVICE_STATUS_RESET;
    usbControlUpdateStatus(device->control, status);
  }

//  if (intStatus & USBSTS_D_SLI)
//  {
//    uint8_t status = 0;
//    status |= DEVICE_STATUS_SUSPENDED;
//    usbControlUpdateStatus(device->control, status);
//  }
//
//  if (intStatus & USBSTS_D_PCI)
//  {
//    uint8_t status = 0;
//    usbControlUpdateStatus(device->control, status);
//  }

  const uint32_t setupStatus = reg->ENDPTSETUPSTAT;

  if (setupStatus)
  {
    /* Iterate over registered endpoints */
    const struct ListNode *current = listFirst(&device->endpoints);
    struct UsbEndpoint *endpoint;

    /* TODO Separate container for control endpoints */
    while (current)
    {
      listData(&device->endpoints, current, &endpoint);

      const uint32_t mask = ENDPT_BIT(endpoint->address);

      if (setupStatus & mask)
      {
        reg->ENDPTCOMPLETE = mask;

        epHandler(endpoint, 1);
      }

      current = listNext(current);
    }
  }

  /* Handle completion interrupts */
  const uint32_t epStatus = reg->ENDPTCOMPLETE;

  if (epStatus)
  {
    const struct ListNode *current = listFirst(&device->endpoints);
    struct UsbEndpoint *endpoint;

    while (current)
    {
      listData(&device->endpoints, current, &endpoint);

      const uint32_t mask = ENDPT_BIT(endpoint->address);

      if (epStatus & mask)
      {
        reg->ENDPTCOMPLETE = mask;

        epHandler(endpoint, 0);
      }

      current = listNext(current);
    }
  }
}
/*----------------------------------------------------------------------------*/
static void resetDevice(struct UsbDevice *device)
{
  LPC_USB_Type * const reg = device->base.reg;

  /* Disable all endpoints */
  for (unsigned int index = 0; index < ENDPT_NUMBER; ++index)
    reg->ENDPTCTRL[index] &= ~(ENDPTCTRL_RXE | ENDPTCTRL_TXE);

  /* Clear all pending interrupts */
  reg->ENDPTNAK = 0xFFFFFFFF;
  reg->ENDPTNAKEN = 0;
  reg->USBSTS_D = 0xFFFFFFFF;
  reg->ENDPTSETUPSTAT = reg->ENDPTSETUPSTAT;
  reg->ENDPTCOMPLETE = reg->ENDPTCOMPLETE;

  while (reg->ENDPTPRIME);
  reg->ENDPTFLUSH = 0xFFFFFFFF;
  while (reg->ENDPTFLUSH);

  /* Set the Interrupt Threshold control interval to 0 */
  reg->USBCMD_D &= ~0x00FF0000;

  /* Zero out the Endpoint queue heads */
  memset(device->base.queueHeads, 0,
      ENDPT_NUMBER * sizeof(struct EndpointQueueHead));

  /* Zero out the device transfer descriptors */
  memset(ep_TD, 0, ENDPT_NUMBER * sizeof(struct EndpointTransferDescriptor));

  /* Configure the Endpoint List Address */
  reg->ENDPOINTLISTADDR = (uint32_t)device->base.queueHeads;

  /* Initialize device queue heads for non ISO endpoint only */ //FIXME
  for (unsigned int index = 0; index < ENDPT_NUMBER; ++index)
    device->base.queueHeads[index].next = (uint32_t)&ep_TD[index];

  /* Enable interrupts */
  reg->USBINTR_D = USBSTS_D_UI | USBSTS_D_UEI | USBSTS_D_PCI
      | USBSTS_D_URI | USBSTS_D_SLI;
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

  LPC_USB_Type * const reg = device->base.reg;

  /* Reset the controller */
  reg->USBCMD_D = USBCMD_D_RST;
  while (reg->USBCMD_D & USBCMD_D_RST);

  /* Program the controller to be the USB device controller */
  reg->USBMODE_D = USBMODE_D_CM(CM_DEVICE_CONTROLLER)
      | USBMODE_D_SDIS | USBMODE_D_SLOM;

  /* Configure NVIC interrupts */
  irqSetPriority(device->base.irq, config->priority);
  irqEnable(device->base.irq);

  /* Configure interrupts */
  resetDevice(device);

  devSetAddress(device, 0);

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
  struct UsbDevice * const device = object;
  LPC_USB_Type * const reg = device->base.reg;

  if (address)
    reg->DEVICEADDR = DEVICEADDR_USBADRA | DEVICEADDR_USBADR(address);
  else
    reg->DEVICEADDR = DEVICEADDR_USBADR(address);
}
/*----------------------------------------------------------------------------*/
static void devSetConnected(void *object, bool state)
{
  struct UsbDevice * const device = object;
  LPC_USB_Type * const reg = device->base.reg;

  if (state)
    reg->USBCMD_D |= USBCMD_D_RS;
  else
    reg->USBCMD_D &= ~USBCMD_D_RS;
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
  queuePeek(&endpoint->requests, &request);

  if (endpoint->address & EP_DIRECTION_IN)
  {
    queuePop(&endpoint->requests, 0);
    requestStatus = REQUEST_COMPLETED;

    if (!queueEmpty(&endpoint->requests))
    {
      struct UsbRequest *nextRequest;
      queuePeek(&endpoint->requests, &nextRequest);

      epEnqueueTx(endpoint, nextRequest);
    }
  }
  else
  {
    if (status) //FIXME Add enum
    {
      size_t read;

      if (epReadSetupPacket(endpoint, request->buffer, request->base.capacity,
          &read) == E_OK)
      {
        queuePop(&endpoint->requests, 0);
        request->base.length = read;
        requestStatus = REQUEST_SETUP;
      }
    }
    else
    {
      size_t read;

      if (epExtractDataLength(endpoint, &read) == E_OK)
      {
        queuePop(&endpoint->requests, 0);
        request->base.length = request->base.capacity - read;
        requestStatus = REQUEST_COMPLETED;
      }
    }

    if (!queueEmpty(&endpoint->requests))
    {
      struct UsbRequest *firstRequest;
      queuePeek(&endpoint->requests, &firstRequest);

      epEnqueueRx(endpoint, firstRequest);
    }

    if (requestStatus == REQUEST_ERROR)
      return;
  }

  request->base.callback(request->base.callbackArgument, request,
      requestStatus);
}
/*----------------------------------------------------------------------------*/
static enum result epEnqueueRx(struct UsbEndpoint *endpoint,
    struct UsbRequest *request)
{
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const uint32_t mask = ENDPT_BIT(endpoint->address);

  const enum result res = epPrepareTransferDescriptor(endpoint, request->buffer,
      request->base.capacity);
  if (res != E_OK)
    return res;

  /* Prime the endpoint for read */
  reg->ENDPTPRIME |= mask;

  /* Check if priming succeeded */
  while (reg->ENDPTPRIME & mask);

  /* TODO Error handling */
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result epEnqueueTx(struct UsbEndpoint *endpoint,
    struct UsbRequest *request)
{
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const uint32_t mask = ENDPT_BIT(endpoint->address);

  const enum result res = epPrepareTransferDescriptor(endpoint, request->buffer,
      request->base.length);
  if (res != E_OK)
    return res;

  /* Prime the endpoint for transmit */
  reg->ENDPTPRIME |= mask;

  /* Check if priming succeeded */
  while (reg->ENDPTPRIME & mask);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result epExtractDataLength(const struct UsbEndpoint *endpoint,
    size_t *read)
{
  assert(read);

  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(endpoint->address);
  const struct EndpointTransferDescriptor * const descriptor =
      &ep_TD[index];

  if (TD_TOKEN_STATUS_VALUE(descriptor->token) & TOKEN_STATUS_BUFFER_ERROR)
    return E_ERROR;

  /* TODO Status handling */
  *read = TD_TOKEN_TOTAL_BYTES_VALUE(descriptor->token);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result epPrepareTransferDescriptor(struct UsbEndpoint *endpoint,
    uint8_t *buffer, size_t length)
{
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(endpoint->address);
  struct EndpointTransferDescriptor * const descriptor = &ep_TD[index];

  /* Zero out the device transfer descriptors */
  memset(descriptor, 0, sizeof(struct EndpointTransferDescriptor));
  /* The next descriptor pointer is invalid */
  descriptor->next = TD_NEXT_TERMINATE;

  /* Setup status and size, enable interrupt on completion */
  descriptor->token = TD_TOKEN_STATUS(TOKEN_STATUS_ACTIVE) | TD_TOKEN_IOC
      | TD_TOKEN_TOTAL_BYTES(length);

  const uint32_t basePage = (uint32_t)buffer & 0xFFFFF000;

  descriptor->buffer0 = (uint32_t)buffer;
  descriptor->buffer1 = basePage + 0x1000;
  descriptor->buffer2 = basePage + 0x2000;
  descriptor->buffer3 = basePage + 0x3000;
  descriptor->buffer4 = basePage + 0x4000;

  struct EndpointQueueHead * const queueHead =
      &endpoint->device->base.queueHeads[index];

  queueHead->next = (uint32_t)descriptor;
  /* Clear Active and Halt bits as mentioned in User Manual */
  queueHead->token &= ~TD_TOKEN_STATUS(TOKEN_STATUS_ACTIVE
      | TOKEN_STATUS_HALTED);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result epReadSetupPacket(struct UsbEndpoint *endpoint,
    uint8_t *buffer, size_t length, size_t *read)
{
  if (length < 8)
    return E_VALUE;

  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(endpoint->address);
  const struct EndpointQueueHead * const queueHead =
      &endpoint->device->base.queueHeads[index];
  const uint32_t mask = ENDPT_BIT(endpoint->address);
  enum result res = E_ERROR;

  uint32_t setupStatus = reg->ENDPTSETUPSTAT;
  reg->ENDPTSETUPSTAT = setupStatus; /* Clear the setup interrupt */

  if (setupStatus & mask)
  {
    uint32_t * const bufferOverlay = (uint32_t *)buffer;

    do
    {
      /* Set the tripwire semaphore */
      reg->USBCMD_D |= USBCMD_D_SUTW;

      /* Transfer setup data to the buffer */
      bufferOverlay[0] = queueHead->setup[0];
      bufferOverlay[1] = queueHead->setup[1];
    }
    while (!(reg->USBCMD_D & USBCMD_D_SUTW));

    /* Clear the tripwire */
    reg->USBCMD_D &= ~USBCMD_D_SUTW;

    *read = 8;
    res = E_OK;
  }

  /* Wait until setup interrupt is cleared */
  while ((setupStatus = reg->ENDPTSETUPSTAT) != 0)
    reg->ENDPTSETUPSTAT = setupStatus;

  /* TODO Remove current descriptor from the queue */
  return res;
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
static void epDisable(void *object)
{
  struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const unsigned int number = EP_TO_LOGICAL_NUMBER(endpoint->address);

  reg->ENDPTCTRL[number] &= endpoint->address & 0x80 ?
      ~ENDPTCTRL_TXE : ~ENDPTCTRL_RXE;
}
/*----------------------------------------------------------------------------*/
static void epEnable(void *object, uint8_t type, uint16_t size)
{
  struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(endpoint->address);
  const unsigned int number = EP_TO_LOGICAL_NUMBER(endpoint->address);
  struct EndpointQueueHead * const queueHead =
      &endpoint->device->base.queueHeads[index];

  //TODO Different sequence for Control and Common endpoints
  /* Set endpoint type */
  if (type != ENDPOINT_TYPE_ISOCHRONOUS)
  {
    queueHead->capabilities = QH_MAX_PACKET_LENGTH(size) | QH_IOS | QH_ZLT;
    ep_TD[index].next = TD_NEXT_TERMINATE;
  }
  else
  {
    //TODO Configure MultO
    queueHead->capabilities = QH_MAX_PACKET_LENGTH(0x400) | QH_ZLT;
  }

  /* Setup endpoint control register */
  const bool tx = (endpoint->address & 0x80) != 0;
  uint32_t controlValue = reg->ENDPTCTRL[number];

  if (tx)
  {
    controlValue &= ~0xFFFF0000;
    controlValue |= ENDPTCTRL_TXT(type);
  }
  else
  {
    controlValue &= ~0x0000FFFF;
    controlValue |= ENDPTCTRL_RXT(type);
  }
  reg->ENDPTCTRL[number] = controlValue;

  /* Enable endpoint */
  reg->ENDPTCTRL[number] |= tx ? ENDPTCTRL_TXE : ENDPTCTRL_RXE;

  /* Flush endpoint buffers */
  const uint32_t mask = ENDPT_BIT(endpoint->address);

  reg->ENDPTFLUSH = mask;
  while (reg->ENDPTFLUSH & mask);

  /* Reset data toggles */
  reg->ENDPTCTRL[number] |= tx ? ENDPTCTRL_TXR : ENDPTCTRL_RXR;
}
/*----------------------------------------------------------------------------*/
static enum result epEnqueue(void *object, struct UsbRequest *request)
{
  assert(request->base.callback);

  struct UsbEndpoint * const endpoint = object;
  enum result res = E_OK;

  irqDisable(endpoint->device->base.irq);

  if (!queueFull(&endpoint->requests))
  {
    const bool empty = queueEmpty(&endpoint->requests);

    queuePush(&endpoint->requests, &request);

    if (empty)
    {
      if (endpoint->address & EP_DIRECTION_IN)
      {
        res = epEnqueueTx(endpoint, request);
      }
      else
      {
        res = epEnqueueRx(endpoint, request);
      }
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
  const struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const unsigned int number = EP_TO_LOGICAL_NUMBER(endpoint->address);

  const uint32_t stallMask = endpoint->address & 0x80 ?
      ENDPTCTRL_TXS : ENDPTCTRL_RXS;

  return (reg->ENDPTCTRL[number] & stallMask) != 0;
}
/*----------------------------------------------------------------------------*/
static void epSetStalled(void *object, bool stalled)
{
  struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const unsigned int number = EP_TO_LOGICAL_NUMBER(endpoint->address);

  const bool tx = (endpoint->address & 0x80) != 0;
  const uint32_t stallMask = tx ? ENDPTCTRL_TXS : ENDPTCTRL_RXS;

  if (stalled)
  {
    reg->ENDPTCTRL[number] |= stallMask;
  }
  else
  {
    const uint32_t resetToggleMask = tx ? ENDPTCTRL_TXR : ENDPTCTRL_RXR;

    reg->ENDPTCTRL[number] &= ~stallMask;
    reg->ENDPTCTRL[number] |= resetToggleMask;
  }
}
