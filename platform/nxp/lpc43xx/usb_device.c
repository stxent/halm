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
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static void resetDevice(struct UsbDevice *);
static void resetQueueHeads(struct UsbDevice *);
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
static void epCommonHandler(struct UsbEndpoint *);
static void epControlHandler(struct UsbEndpoint *);
static struct TransferDescriptor *epAllocateDescriptor(struct UsbEndpoint *,
    struct UsbRequest *, uint8_t *, size_t);
static void epAppendDescriptor(struct UsbEndpoint *,
    struct TransferDescriptor *);
static enum result epEnqueueRx(struct UsbEndpoint *, struct UsbRequest *);
static enum result epEnqueueTx(struct UsbEndpoint *, struct UsbRequest *);
static void epExtractDataLength(const struct UsbEndpoint *,
    struct UsbRequest *);
static void epFlush(struct UsbEndpoint *);
static inline struct UsbRequest *epGetHeadRequest(const struct QueueHead *);
static enum endpointStatus epGetStatus(const struct UsbEndpoint *);
static void epPopDescriptor(struct UsbEndpoint *);
static void epPrime(struct UsbEndpoint *);
static enum result epReadSetupPacket(struct UsbEndpoint *, struct UsbRequest *);
static void epReprime(struct UsbEndpoint *);
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
    resetDevice(device);
    usbControlEvent(device->control, USB_DEVICE_EVENT_RESET);
  }

  /* Port change detect */
  if (intStatus & USBSTS_D_PCI)
  {
    usbControlEvent(device->control, USB_DEVICE_EVENT_PORT_CHANGE);
  }

  const bool suspended = (intStatus & USBSTS_D_SLI) != 0;

  if (suspended != device->suspended)
  {
    usbControlEvent(device->control, suspended ?
        USB_DEVICE_EVENT_SUSPEND : USB_DEVICE_EVENT_RESUME);
    device->suspended = suspended;
  }

  /* Handle setup packets */
  const uint32_t setupStatus = reg->ENDPTSETUPSTAT;

  if ((setupStatus & ENDPT_BIT(0)) && device->ep0out)
  {
    epControlHandler(device->ep0out);
  }

  /* Handle completion interrupts */
  const uint32_t epStatus = reg->ENDPTCOMPLETE;

  if (epStatus)
  {
    if ((epStatus & ENDPT_BIT(0)) && device->ep0out)
    {
      reg->ENDPTCOMPLETE = ENDPT_BIT(0);
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

  /* Handle Start of Frame interrupt */
  if (intStatus & USBSTS_D_SRI)
  {
    usbControlEvent(device->control, USB_DEVICE_EVENT_FRAME);
  }
}
/*----------------------------------------------------------------------------*/
static void resetDevice(struct UsbDevice *device)
{
  LPC_USB_Type * const reg = device->base.reg;

  device->configuration = 0; /* Inactive configuration */
  device->suspended = false;

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
  reg->USBCMD_D &= ~USBCMD_D_ITC_MASK;

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

  /* Initialize control message handler */
  device->control = init(UsbControl, &controlConfig);
  if (!device->control)
    return E_ERROR;

  LPC_USB_Type * const reg = device->base.reg;

  /* Reset the controller */
  reg->USBCMD_D = USBCMD_D_RST;
  while (reg->USBCMD_D & USBCMD_D_RST);

  /* Program the controller to be the USB device controller */
  reg->USBMODE_D = USBMODE_D_CM(CM_DEVICE_CONTROLLER)
      | USBMODE_D_SDIS | USBMODE_D_SLOM;

  /* Recommended by NXP technical support */
  reg->SBUSCFG = SBUSCFG_AHB_BRST(AHB_BRST_INCR16_UNSPECIFIED);

#ifndef CONFIG_PLATFORM_USB_HS
  /* Force Full Speed mode */
  reg->PORTSC1_D |= PORTSC1_D_PFSC;
#endif

  resetQueueHeads(device);
  resetDevice(device);

  devSetAddress(device, 0);

  /* Configure NVIC interrupts */
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
  const bool ep0out = !USB_EP_ADDRESS(address)
      && !(address & USB_EP_DIRECTION_IN);

  if (ep0out && device->ep0out)
    return device->ep0out;

  const irqState state = irqSave();

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

    if (ep0out)
      device->ep0out = endpoint;
    else
      listPush(&device->endpoints, &endpoint);
  }

  irqRestore(state);
  return endpoint;
}
/*----------------------------------------------------------------------------*/
static enum usbSpeed devGetSpeed(const void *object)
{
  const struct UsbDevice * const device = object;
  const LPC_USB_Type * const reg = device->base.reg;

  return PORTSC1_D_PSPD_VALUE(reg->PORTSC1_D) == PSPD_HIGH_SPEED ?
      USB_HS : USB_FS;
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
    enum usbRequestStatus requestStatus = USB_REQUEST_ERROR;

    if (ep->address & USB_EP_DIRECTION_IN)
    {
      if (packetStatus == STATUS_DATA_PACKET)
        requestStatus = USB_REQUEST_COMPLETED;
    }
    else
    {
      if (packetStatus == STATUS_SETUP_PACKET)
      {
        if (epReadSetupPacket(ep, request) == E_OK)
          requestStatus = USB_REQUEST_SETUP;
      }
      else if (packetStatus == STATUS_DATA_PACKET)
      {
        epExtractDataLength(ep, request);
        requestStatus = USB_REQUEST_COMPLETED;
      }
    }

    epPopDescriptor(ep);

    request->callback(request->callbackArgument, request, requestStatus);
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
  enum usbRequestStatus requestStatus = USB_REQUEST_ERROR;

  if (packetStatus == STATUS_SETUP_PACKET)
  {
    if (epReadSetupPacket(ep, request) == E_OK)
      requestStatus = USB_REQUEST_SETUP;
  }
  else if (packetStatus == STATUS_DATA_PACKET)
  {
    epExtractDataLength(ep, request);
    requestStatus = USB_REQUEST_COMPLETED;
  }

  epPopDescriptor(ep);

  /* Prime next packet manually in case of the reception of setup packet */
  if (packetStatus == STATUS_SETUP_PACKET
      && head->listHead != TD_NEXT_TERMINATE)
  {
    epReprime(ep);
  }

  request->callback(request->callbackArgument, request, requestStatus);
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
static void epAppendDescriptor(struct UsbEndpoint *ep,
    struct TransferDescriptor *descriptor)
{
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  struct QueueHead * const head = &ep->device->base.queueHeads[index];
  const uint32_t mask = ENDPT_BIT(ep->address);

  if (head->listHead != TD_NEXT_TERMINATE)
  {
    /* Linked list is not empty */

    struct TransferDescriptor * const tail =
        (struct TransferDescriptor *)head->listTail;

    tail->next = (uint32_t)descriptor;
    head->listTail = (uint32_t)descriptor;

    if (reg->ENDPTPRIME & mask)
      return;

    uint32_t endpointStatus;

    do
    {
      reg->USBCMD_D |= USBCMD_D_ATDTW;
      endpointStatus = reg->ENDPTSTAT;
    }
    while (!(reg->USBCMD_D & USBCMD_D_ATDTW));

    reg->USBCMD_D &= ~USBCMD_D_ATDTW;

    if (endpointStatus & mask)
      return;
  }
  else
  {
    /* Store the first element of the list in unused fields of the Queue Head */
    head->listHead = (uint32_t)descriptor;
    head->listTail = (uint32_t)descriptor;
  }

  /* Linked list is empty */
  head->next = (uint32_t)descriptor;
  /* Clear Active and Halt bits as mentioned in User Manual */
  head->token &= ~TD_TOKEN_STATUS(TOKEN_STATUS_ACTIVE | TOKEN_STATUS_HALTED);

  epPrime(ep);
}
/*----------------------------------------------------------------------------*/
static enum result epEnqueueRx(struct UsbEndpoint *ep,
    struct UsbRequest *request)
{
  struct TransferDescriptor * const descriptor = epAllocateDescriptor(ep,
      request, request->buffer, request->capacity);

  if (!descriptor)
    return E_EMPTY;

  epAppendDescriptor(ep, descriptor);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result epEnqueueTx(struct UsbEndpoint *ep,
    struct UsbRequest *request)
{
  struct TransferDescriptor * const descriptor = epAllocateDescriptor(ep,
      request, request->buffer, request->length);

  if (!descriptor)
    return E_EMPTY;

  epAppendDescriptor(ep, descriptor);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void epExtractDataLength(const struct UsbEndpoint *ep,
    struct UsbRequest *request)
{
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  const struct QueueHead * const head = &ep->device->base.queueHeads[index];
  const struct TransferDescriptor * const descriptor =
      (const struct TransferDescriptor *)head->listHead;

  request->length = request->capacity
      - TD_TOKEN_TOTAL_BYTES_VALUE(descriptor->token);
}
/*----------------------------------------------------------------------------*/
static void epFlush(struct UsbEndpoint *ep)
{
  LPC_USB_Type * const reg = ep->device->base.reg;
  const uint32_t mask = ENDPT_BIT(ep->address);

  /* Remove current descriptor from the queue */
  do
  {
    reg->ENDPTFLUSH = mask;
    while (reg->ENDPTFLUSH & mask);
  }
  while (reg->ENDPTSTAT & mask);
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
  if (head->listHead == TD_NEXT_TERMINATE)
    head->listTail = TD_NEXT_TERMINATE;
  queuePush(&ep->device->base.descriptorPool, &descriptor);
}
/*----------------------------------------------------------------------------*/
static void epPrime(struct UsbEndpoint *ep)
{
  LPC_USB_Type * const reg = ep->device->base.reg;
  const uint32_t mask = ENDPT_BIT(ep->address);

  /* Prime the endpoint for read */
  reg->ENDPTPRIME |= mask;
  /* Wait until the bit is set */
  while (reg->ENDPTPRIME & mask);
}
/*----------------------------------------------------------------------------*/
static enum result epReadSetupPacket(struct UsbEndpoint *ep,
    struct UsbRequest *request)
{
  if (request->capacity < 8)
    return E_VALUE;

  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  const struct QueueHead * const head = &ep->device->base.queueHeads[index];

  epFlush(ep);

  uint32_t buffer[2];
  uint32_t setupStatus;

  /* Clear the setup interrupt */
  setupStatus = reg->ENDPTSETUPSTAT;
  reg->ENDPTSETUPSTAT = setupStatus;

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

  /* Wait until setup interrupt is cleared */
  while ((setupStatus = reg->ENDPTSETUPSTAT))
    reg->ENDPTSETUPSTAT = setupStatus;

  /* Copy content of the setup packet into the request buffer */
  memcpy(request->buffer, buffer, sizeof(buffer));
  request->length = sizeof(buffer);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void epReprime(struct UsbEndpoint *ep)
{
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  struct QueueHead * const head = &ep->device->base.queueHeads[index];
  struct TransferDescriptor * const descriptor =
      (struct TransferDescriptor *)head->listHead;

  head->next = (uint32_t)descriptor;
  /* Clear Active and Halt bits as mentioned in User Manual */
  head->token &= ~TD_TOKEN_STATUS(TOKEN_STATUS_ACTIVE | TOKEN_STATUS_HALTED);

  epPrime(ep);
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

  /* Protect endpoint list from simultaneous access */
  const irqState state = irqSave();

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

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static void epClear(void *object)
{
  struct UsbEndpoint * const endpoint = object;

  epFlush(endpoint);

  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(endpoint->address);
  struct QueueHead * const head = &endpoint->device->base.queueHeads[index];

  while (head->listHead != TD_NEXT_TERMINATE)
  {
    struct UsbRequest * const request = epGetHeadRequest(head);

    request->callback(request->callbackArgument, request,
        USB_REQUEST_CANCELLED);

    epPopDescriptor(endpoint);
  }
}
/*----------------------------------------------------------------------------*/
static void epDisable(void *object)
{
  struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(endpoint->address);

  reg->ENDPTCTRL[number] &= endpoint->address & 0x80 ?
      ~ENDPTCTRL_TXE : ~ENDPTCTRL_RXE;
}
/*----------------------------------------------------------------------------*/
static void epEnable(void *object, uint8_t type, uint16_t size)
{
  struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(endpoint->address);
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(endpoint->address);
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

  /* Flush endpoint descriptors */
  epFlush(endpoint);

  /* Reset data toggles */
  reg->ENDPTCTRL[number] |= tx ? ENDPTCTRL_TXR : ENDPTCTRL_RXR;
}
/*----------------------------------------------------------------------------*/
static enum result epEnqueue(void *object, struct UsbRequest *request)
{
  assert(request->callback);

  struct UsbEndpoint * const endpoint = object;
  enum result res = E_OK;

  const irqState state = irqSave();

  if (endpoint->address & USB_EP_DIRECTION_IN)
    res = epEnqueueTx(endpoint, request);
  else
    res = epEnqueueRx(endpoint, request);

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
static bool epIsStalled(void *object)
{
  const struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(endpoint->address);

  const uint32_t stallMask = endpoint->address & 0x80 ?
      ENDPTCTRL_TXS : ENDPTCTRL_RXS;

  return (reg->ENDPTCTRL[number] & stallMask) != 0;
}
/*----------------------------------------------------------------------------*/
static void epSetStalled(void *object, bool stalled)
{
  struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(endpoint->address);

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
