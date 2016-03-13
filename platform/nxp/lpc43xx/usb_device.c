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
enum endpointStatus
{
  STATUS_IDLE,
  STATUS_DATA_PACKET,
  STATUS_SETUP_PACKET,
  STATUS_ERROR
};

#define CONTROL_ENDPOINT_ADDRESS 0
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static void resetDevice(struct UsbDevice *);
static void resetQueueHeads(struct UsbDevice *);
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
static void epCommonHandler(struct UsbEndpoint *);
static void epControlHandler(struct UsbEndpoint *);
static struct TransferDescriptor *epAllocateDescriptor(struct UsbEndpoint *,
    struct UsbRequest *, uint8_t *, size_t);
static enum result epAppendDescriptor(struct UsbEndpoint *,
    struct TransferDescriptor *);
static enum result epEnqueueRx(struct UsbEndpoint *, struct UsbRequest *);
static enum result epEnqueueTx(struct UsbEndpoint *, struct UsbRequest *);
static void epFillDataLength(const struct UsbEndpoint *, struct UsbRequest *);
static inline struct UsbRequest *epGetHeadRequest(const struct QueueHead *);
static enum endpointStatus epGetStatus(const struct UsbEndpoint *);
static void epPopDescriptor(struct UsbEndpoint *);
static enum result epReadSetupPacket(struct UsbEndpoint *, struct UsbRequest *);
static enum result epReprimeEndpoint(struct UsbEndpoint *);
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

  if ((setupStatus & ENDPT_BIT(CONTROL_ENDPOINT_ADDRESS)) && device->ep0out)
  {
    epControlHandler(device->ep0out);
  }

  /* Handle completion interrupts */
  const uint32_t epStatus = reg->ENDPTCOMPLETE;

  if (epStatus)
  {
    if ((epStatus & ENDPT_BIT(CONTROL_ENDPOINT_ADDRESS)) && device->ep0out)
    {
      reg->ENDPTCOMPLETE = ENDPT_BIT(CONTROL_ENDPOINT_ADDRESS);
      epControlHandler(device->ep0out);
    }
    else
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
          epCommonHandler(endpoint);
        }

        current = listNext(current);
      }
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

  /* Zero out all queue heads */
  for (unsigned int index = 0; index < ENDPT_NUMBER; ++index)
  {
    struct QueueHead * const head = &device->base.queueHeads[index];

    head->capabilities = 0;
    head->current = 0;
    head->next = 0;
    head->token = 0;
    head->buffer0 = 0;
    head->buffer1 = 0;
    head->buffer2 = 0;
    head->buffer3 = 0;
    head->buffer4 = 0;
  }

  /* Configure the Endpoint List Address */
  reg->ENDPOINTLISTADDR = (uint32_t)device->base.queueHeads;

  /* Enable interrupts */
  reg->USBINTR_D = USBSTS_D_UI | USBSTS_D_UEI | USBSTS_D_PCI
      | USBSTS_D_URI | USBSTS_D_SLI;
}
/*----------------------------------------------------------------------------*/
static void resetQueueHeads(struct UsbDevice *device)
{
  for (unsigned int index = 0; index < ENDPT_NUMBER; ++index)
  {
    struct QueueHead * const head = &device->base.queueHeads[index];

    head->listHead = TD_NEXT_TERMINATE;
    head->listTail = TD_NEXT_TERMINATE;
  }
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
  device->ep0out = 0;

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

  resetQueueHeads(device);
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
  const bool ep0in = EP_ADDRESS(address) == CONTROL_ENDPOINT_ADDRESS
      && !(address & EP_DIRECTION_IN);

  irqDisable(device->base.irq);

  if (ep0in && device->ep0out)
    return device->ep0out;

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

    if (ep0in)
      device->ep0out = endpoint;
    else
      listPush(&device->endpoints, &endpoint);
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
static void epCommonHandler(struct UsbEndpoint *ep)
{
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  struct QueueHead * const head = &ep->device->base.queueHeads[index];

  enum endpointStatus packetStatus;

  while ((packetStatus = epGetStatus(ep)) != STATUS_IDLE)
  {
    struct UsbRequest * const request = epGetHeadRequest(head);
    enum usbRequestStatus requestStatus = REQUEST_ERROR;

    if (ep->address & EP_DIRECTION_IN)
    {
      if (packetStatus == STATUS_DATA_PACKET)
        requestStatus = REQUEST_COMPLETED;
    }
    else
    {
      if (packetStatus == STATUS_SETUP_PACKET)
      {
        if (epReadSetupPacket(ep, request) == E_OK)
          requestStatus = REQUEST_SETUP;
      }
      else if (packetStatus == STATUS_DATA_PACKET)
      {
        epFillDataLength(ep, request);
        requestStatus = REQUEST_COMPLETED;
      }
    }

    epPopDescriptor(ep);

    request->base.callback(request->base.callbackArgument, request,
        requestStatus);

    uint32_t setupStatus;
    LPC_USB_Type * const reg = ep->device->base.reg;
    while ((setupStatus = reg->ENDPTSETUPSTAT))
      reg->ENDPTSETUPSTAT = setupStatus;
  }
}
/*----------------------------------------------------------------------------*/
static void epControlHandler(struct UsbEndpoint *ep)
{
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  struct QueueHead * const head = &ep->device->base.queueHeads[index];
  const enum endpointStatus packetStatus = epGetStatus(ep);

  if (packetStatus == STATUS_IDLE)
    return;

  struct UsbRequest * const request = epGetHeadRequest(head);
  enum usbRequestStatus requestStatus = REQUEST_ERROR;

  if (packetStatus == STATUS_SETUP_PACKET)
  {
    if (epReadSetupPacket(ep, request) == E_OK)
      requestStatus = REQUEST_SETUP;
  }
  else if (packetStatus == STATUS_DATA_PACKET)
  {
    epFillDataLength(ep, request);
    requestStatus = REQUEST_COMPLETED;
  }

  epPopDescriptor(ep);

  /* Prime next packet manually in case of the reception of setup packet */
  if (packetStatus == STATUS_SETUP_PACKET
      && head->listHead != TD_NEXT_TERMINATE)
  {
    epReprimeEndpoint(ep);
  }

  request->base.callback(request->base.callbackArgument, request,
      requestStatus);
}
/*----------------------------------------------------------------------------*/
static struct TransferDescriptor *epAllocateDescriptor(struct UsbEndpoint *ep,
    struct UsbRequest *node, uint8_t *buffer, size_t length)
{
  if (queueEmpty(&ep->device->base.descriptorPool))
    return 0;

  struct TransferDescriptor *descriptor;

  queuePop(&ep->device->base.descriptorPool, &descriptor);

  /* The next descriptor pointer is invalid */
  descriptor->next = TD_NEXT_TERMINATE;

  /* Setup status and size, enable interrupt on completion */
  descriptor->token = TD_TOKEN_STATUS(TOKEN_STATUS_ACTIVE) | TD_TOKEN_IOC
      | TD_TOKEN_TOTAL_BYTES(length);

  /* Store pointer to USB Request structure in reserved field */
  descriptor->listNode = (uint32_t)node;

  const uint32_t basePage = (uint32_t)buffer & 0xFFFFF000;

  descriptor->buffer0 = (uint32_t)buffer;
  descriptor->buffer1 = basePage + 0x1000;
  descriptor->buffer2 = basePage + 0x2000;
  descriptor->buffer3 = basePage + 0x3000;
  descriptor->buffer4 = basePage + 0x4000;

  return descriptor;
}
/*----------------------------------------------------------------------------*/
static enum result epAppendDescriptor(struct UsbEndpoint *ep,
    struct TransferDescriptor *descriptor)
{
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  struct QueueHead * const head = &ep->device->base.queueHeads[index];
  const uint32_t mask = ENDPT_BIT(ep->address);

  if (head->listHead != TD_NEXT_TERMINATE)
  {
    struct TransferDescriptor * const tail =
        (struct TransferDescriptor *)head->listTail;

    tail->next = (uint32_t)descriptor;
    head->listTail = (uint32_t)descriptor;

    if (reg->ENDPTPRIME & mask)
      return E_OK;

    uint32_t endpointStatus;

    do
    {
      reg->USBCMD_D |= USBCMD_D_ATDTW;
      endpointStatus = reg->ENDPTSTAT;
    }
    while (!(reg->USBCMD_D & USBCMD_D_ATDTW));

    reg->USBCMD_D &= ~USBCMD_D_ATDTW;

    if (endpointStatus & mask)
      return E_OK;
  }
  else
  {
    /* Store first element of the list in unused field of Queue Head */
    head->listHead = (uint32_t)descriptor;
    head->listTail = (uint32_t)descriptor;
  }

  head->next = (uint32_t)descriptor;
  /* Clear Active and Halt bits as mentioned in User Manual */
  head->token &= ~TD_TOKEN_STATUS(TOKEN_STATUS_ACTIVE | TOKEN_STATUS_HALTED);

  /* Prime the endpoint for read */
  reg->ENDPTPRIME |= mask;
  /* Check if priming succeeded */
  while (reg->ENDPTPRIME & mask);

  /* TODO Error handling */

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result epEnqueueRx(struct UsbEndpoint *ep,
    struct UsbRequest *request)
{
  struct TransferDescriptor * const descriptor = epAllocateDescriptor(ep,
      request, request->buffer, request->base.capacity);

  if (!descriptor)
    return E_EMPTY;

  return epAppendDescriptor(ep, descriptor);
}
/*----------------------------------------------------------------------------*/
static enum result epEnqueueTx(struct UsbEndpoint *ep,
    struct UsbRequest *request)
{
  struct TransferDescriptor * const descriptor = epAllocateDescriptor(ep,
      request, request->buffer, request->base.length);

  if (!descriptor)
    return E_EMPTY;

  return epAppendDescriptor(ep, descriptor);
}
/*----------------------------------------------------------------------------*/
static void epFillDataLength(const struct UsbEndpoint *ep,
    struct UsbRequest *request)
{
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  const struct QueueHead * const head = &ep->device->base.queueHeads[index];
  const struct TransferDescriptor * const descriptor =
      (const struct TransferDescriptor *)head->listHead;

  request->base.length = request->base.capacity
      - TD_TOKEN_TOTAL_BYTES_VALUE(descriptor->token);
}
/*----------------------------------------------------------------------------*/
static inline struct UsbRequest *epGetHeadRequest(const struct QueueHead *head)
{
  struct TransferDescriptor * const descriptor =
      (struct TransferDescriptor *)head->listHead;

  return (struct UsbRequest *)descriptor->listNode;
}
/*----------------------------------------------------------------------------*/
static enum endpointStatus epGetStatus(const struct UsbEndpoint *ep)
{
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  const struct QueueHead * const head = &ep->device->base.queueHeads[index];

  if (head->listHead == TD_NEXT_TERMINATE)
    return false;

  const LPC_USB_Type * const reg = ep->device->base.reg;
  const struct TransferDescriptor * const descriptor =
      (const struct TransferDescriptor *)head->listHead;
  const uint32_t endpointMask = ENDPT_BIT(ep->address);
  const uint32_t errorMask = TOKEN_STATUS_HALTED | TOKEN_STATUS_BUFFER_ERROR
      | TOKEN_STATUS_TRANSACTION_ERROR;
  const uint32_t status = TD_TOKEN_STATUS_VALUE(descriptor->token);

  if (reg->ENDPTSETUPSTAT & endpointMask)
    return STATUS_SETUP_PACKET;
  else if (status & errorMask)
    return STATUS_ERROR;
  else if (!(status & TOKEN_STATUS_ACTIVE))
    return STATUS_DATA_PACKET;
  else
    return STATUS_IDLE;
}
/*----------------------------------------------------------------------------*/
static void epPopDescriptor(struct UsbEndpoint *ep)
{
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  struct QueueHead * const head = &ep->device->base.queueHeads[index];
  struct TransferDescriptor * const descriptor =
      (struct TransferDescriptor *)head->listHead;

  head->listHead = (uint32_t)descriptor->next;
  queuePush(&ep->device->base.descriptorPool, &descriptor);
}
/*----------------------------------------------------------------------------*/
static enum result epReadSetupPacket(struct UsbEndpoint *ep,
    struct UsbRequest *request)
{
  if (request->base.capacity < 8)
    return E_VALUE;

  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  const struct QueueHead * const head = &ep->device->base.queueHeads[index];
  const uint32_t mask = ENDPT_BIT(ep->address);
  enum result res = E_ERROR;

  uint32_t buffer[2];
  uint32_t setupStatus;

  setupStatus = reg->ENDPTSETUPSTAT;
  reg->ENDPTSETUPSTAT = setupStatus; /* Clear the setup interrupt */ //FIXME

  if (setupStatus & mask)
  {
    do
    {
      /* Set the tripwire semaphore */
      reg->USBCMD_D |= USBCMD_D_SUTW;

      /* Transfer setup data to the buffer */
      buffer[0] = head->setup[0];
      buffer[1] = head->setup[1];
    }
    while (!(reg->USBCMD_D & USBCMD_D_SUTW));

    /* Clear the tripwire */
    reg->USBCMD_D &= ~USBCMD_D_SUTW;

    res = E_OK;
  }

  /* Wait until setup interrupt is cleared */
  while ((setupStatus = reg->ENDPTSETUPSTAT))
    reg->ENDPTSETUPSTAT = setupStatus;

  /* TODO Remove current descriptor from the queue */
  reg->ENDPTFLUSH = mask;
  //FIXME Move outside ISR
//  do
//  {
//    reg->ENDPTFLUSH = mask;
//  }
//  while ((reg->ENDPTFLUSH & mask) || (reg->ENDPTSTAT & mask));

  if (res == E_OK)
  {
    memcpy(request->buffer, buffer, sizeof(buffer));
    request->base.length = sizeof(buffer);
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static enum result epReprimeEndpoint(struct UsbEndpoint *ep)
{
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  struct QueueHead * const head = &ep->device->base.queueHeads[index];
  struct TransferDescriptor * const descriptor =
      (struct TransferDescriptor *)head->listHead;
  const uint32_t mask = ENDPT_BIT(ep->address);

  head->next = (uint32_t)descriptor;
  /* Clear Active and Halt bits as mentioned in User Manual */
  head->token &= ~TD_TOKEN_STATUS(TOKEN_STATUS_ACTIVE | TOKEN_STATUS_HALTED);

  /* Prime the endpoint for read */
  reg->ENDPTPRIME |= mask;
  /* Check if priming succeeded */
  while (reg->ENDPTPRIME & mask);

  //TODO Error handling
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result epInit(void *object, const void *configBase)
{
  const struct UsbEndpointConfig * const config = configBase;
  struct UsbDevice * const device = config->parent;
  struct UsbEndpoint * const endpoint = object;

  endpoint->address = config->address;
  endpoint->device = device;

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

  if (device->ep0out == endpoint)
  {
    device->ep0out = 0;
  }
  else
  {
    struct ListNode * const node = listFind(&device->endpoints, &endpoint);

    if (node)
      listErase(&device->endpoints, node);
  }

  irqEnable(device->base.irq);
}
/*----------------------------------------------------------------------------*/
static void epClear(void *object)
{
  struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;

  const uint32_t mask = ENDPT_BIT(endpoint->address);

  reg->ENDPTFLUSH = mask;
  while (reg->ENDPTFLUSH & mask);
  //FIXME Check STAT

  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(endpoint->address);
  struct QueueHead * const head = &endpoint->device->base.queueHeads[index];

  while (head->listHead != TD_NEXT_TERMINATE)
  {
    struct UsbRequest * const request = epGetHeadRequest(head);

    request->base.callback(request->base.callbackArgument, request,
        REQUEST_CANCELLED);

    epPopDescriptor(endpoint);
  }
}
/*----------------------------------------------------------------------------*/
static void epDisable(void *object)
{
  struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const unsigned int number = EP_LOGICAL_ADDRESS(endpoint->address);

  reg->ENDPTCTRL[number] &= endpoint->address & 0x80 ?
      ~ENDPTCTRL_TXE : ~ENDPTCTRL_RXE;
}
/*----------------------------------------------------------------------------*/
static void epEnable(void *object, uint8_t type, uint16_t size)
{
  struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(endpoint->address);
  const unsigned int number = EP_LOGICAL_ADDRESS(endpoint->address);
  struct QueueHead * const head = &endpoint->device->base.queueHeads[index];

  //TODO Different sequence for Control and Common endpoints
  /* Set endpoint type */
  if (type != ENDPOINT_TYPE_ISOCHRONOUS)
  {
    head->capabilities = QH_MAX_PACKET_LENGTH(size) | QH_IOS | QH_ZLT;
  }
  else
  {
    //TODO Configure MultO
    head->capabilities = QH_MAX_PACKET_LENGTH(0x400) | QH_ZLT;
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
  if (endpoint->address & EP_DIRECTION_IN)
    res = epEnqueueTx(endpoint, request);
  else
    res = epEnqueueRx(endpoint, request);
  irqEnable(endpoint->device->base.irq);

  return res;
}
/*----------------------------------------------------------------------------*/
static bool epIsStalled(void *object)
{
  const struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const unsigned int number = EP_LOGICAL_ADDRESS(endpoint->address);

  const uint32_t stallMask = endpoint->address & 0x80 ?
      ENDPTCTRL_TXS : ENDPTCTRL_RXS;

  return (reg->ENDPTCTRL[number] & stallMask) != 0;
}
/*----------------------------------------------------------------------------*/
static void epSetStalled(void *object, bool stalled)
{
  struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const unsigned int number = EP_LOGICAL_ADDRESS(endpoint->address);

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
