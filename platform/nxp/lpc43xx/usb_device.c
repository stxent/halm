/*
 * usb_device.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <xcore/memory.h>
#include <halm/platform/nxp/lpc43xx/usb_base.h>
#include <halm/platform/nxp/lpc43xx/usb_defs.h>
#include <halm/platform/nxp/usb_device.h>
#include <halm/usb/usb_control.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
/*----------------------------------------------------------------------------*/
#define CONTROL_OUT 0
/*----------------------------------------------------------------------------*/
enum EndpointStatus
{
  STATUS_IDLE,
  STATUS_DATA_PACKET,
  STATUS_SETUP_PACKET,
  STATUS_ERROR
};
/*----------------------------------------------------------------------------*/
struct UsbDmaEndpoint
{
  struct UsbEndpoint base;

  /* Parent device */
  struct UsbDevice *device;
  /* Logical address */
  uint8_t address;
};

struct UsbDevice
{
  struct UsbBase base;

  /* Array of logical endpoints */
  struct UsbEndpoint **endpoints;
  /* Control message handler */
  struct UsbControl *control;

  /* Device is suspended */
  bool suspended;
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static void resetDevice(struct UsbDevice *);
static void resetQueueHeads(struct UsbDevice *);
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
static enum Result devStringAppend(void *, struct UsbString);
static void devStringErase(void *, struct UsbString);

#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *);
#else
#define devDeinit deletedDestructorTrap
#endif
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
static void epCommonHandler(struct UsbDmaEndpoint *);
static void epControlHandler(struct UsbDmaEndpoint *);
static struct TransferDescriptor *epAllocDescriptor(struct UsbDmaEndpoint *,
    struct UsbRequest *, uint8_t *, size_t);
static void epAppendDescriptor(struct UsbDmaEndpoint *,
    struct TransferDescriptor *);
static enum Result epEnqueueRx(struct UsbDmaEndpoint *, struct UsbRequest *);
static enum Result epEnqueueTx(struct UsbDmaEndpoint *, struct UsbRequest *);
static void epExtractDataLength(const struct UsbDmaEndpoint *,
    struct UsbRequest *);
static void epFlush(struct UsbDmaEndpoint *);
static inline struct UsbRequest *epGetHeadRequest(const struct QueueHead *);
static enum EndpointStatus epGetStatus(const struct UsbDmaEndpoint *);
static void epPopDescriptor(struct UsbDmaEndpoint *);
static void epPrime(struct UsbDmaEndpoint *);
static enum Result epReadSetupPacket(struct UsbDmaEndpoint *,
    struct UsbRequest *);
static void epReprime(struct UsbDmaEndpoint *);
/*----------------------------------------------------------------------------*/
static enum Result epInit(void *, const void *);
static void epDeinit(void *);
static void epClear(void *);
static void epDisable(void *);
static void epEnable(void *, uint8_t, uint16_t);
static enum Result epEnqueue(void *, struct UsbRequest *);
static bool epIsStalled(void *);
static void epSetStalled(void *, bool);
/*----------------------------------------------------------------------------*/
static const struct UsbEndpointClass epTable = {
    .size = sizeof(struct UsbDmaEndpoint),
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
const struct UsbEndpointClass * const UsbDmaEndpoint = &epTable;
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

  if ((setupStatus & ENDPT_BIT(CONTROL_OUT)) && device->endpoints[CONTROL_OUT])
  {
    epControlHandler((struct UsbDmaEndpoint *)device->endpoints[CONTROL_OUT]);
  }

  /* Handle completion interrupts */
  uint32_t epStatus = reg->ENDPTCOMPLETE;

  if (epStatus)
  {
    if ((epStatus & ENDPT_BIT(CONTROL_OUT)) && device->endpoints[CONTROL_OUT])
    {
      const uint32_t mask = ENDPT_BIT(CONTROL_OUT);

      epStatus -= mask;
      reg->ENDPTCOMPLETE = mask;
      epControlHandler((struct UsbDmaEndpoint *)device->endpoints[CONTROL_OUT]);
    }

    struct UsbEndpoint ** const endpointArray = device->endpoints;

    epStatus = reverseBits32(epStatus);

    while (epStatus)
    {
      /* Convert interrupt flag position to physical endpoint number */
      const unsigned int position = countLeadingZeros32(epStatus);
      const unsigned int number = ((position & 0xF) << 1) + (position >> 4);

      epStatus -= (1UL << 31) >> position;
      reg->ENDPTCOMPLETE = 1UL << position;

      epCommonHandler((struct UsbDmaEndpoint *)endpointArray[number]);
    }
  }
}
/*----------------------------------------------------------------------------*/
static void resetDevice(struct UsbDevice *device)
{
  LPC_USB_Type * const reg = device->base.reg;

  device->suspended = false;

  /* Disable all endpoints */
  for (unsigned int index = 0; index < device->base.numberOfEndpoints; ++index)
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
  for (unsigned int index = 0; index < device->base.numberOfEndpoints; ++index)
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
  for (unsigned int index = 0; index < device->base.numberOfEndpoints; ++index)
  {
    struct QueueHead * const head = &device->base.queueHeads[index];

    head->listHead = TD_NEXT_TERMINATE;
    head->listTail = TD_NEXT_TERMINATE;
  }
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
  enum Result res;

  /* Call base class constructor */
  res = UsbBase->init(object, &baseConfig);
  if (res != E_OK)
    return res;

  device->base.handler = interruptHandler;

  const size_t endpointBufferSize =
      device->base.numberOfEndpoints * sizeof(struct UsbEndpoint *);

  device->endpoints = malloc(endpointBufferSize);
  if (!device->endpoints)
    return E_MEMORY;
  memset(device->endpoints, 0, endpointBufferSize);

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
#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *object)
{
  struct UsbDevice * const device = object;

  irqDisable(device->base.irq);

  deinit(device->control);
  free(device->endpoints);
  UsbBase->deinit(device);
}
#endif
/*----------------------------------------------------------------------------*/
static void *devCreateEndpoint(void *object, uint8_t address)
{
  struct UsbDevice * const device = object;
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(address);

  assert(index < device->base.numberOfEndpoints);

  struct UsbEndpoint *ep = 0;
  const IrqState state = irqSave();

  if (!device->endpoints[index])
  {
    const struct UsbEndpointConfig config = {
        .parent = device,
        .address = address
    };

    device->endpoints[index] = init(UsbDmaEndpoint, &config);
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
static enum Result devBind(void *object, void *driver)
{
  struct UsbDevice * const device = object;

  const IrqState state = irqSave();
  const enum Result res = usbControlBindDriver(device->control, driver);
  irqRestore(state);

  return res;
}
/*----------------------------------------------------------------------------*/
static void devUnbind(void *object, const void *driver __attribute__((unused)))
{
  struct UsbDevice * const device = object;

  const IrqState state = irqSave();
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
static enum UsbSpeed devGetSpeed(const void *object)
{
  const struct UsbDevice * const device = object;
  const LPC_USB_Type * const reg = device->base.reg;
  const bool high = PORTSC1_D_PSPD_VALUE(reg->PORTSC1_D) == PSPD_HIGH_SPEED;

  return high ? USB_HS : USB_FS;
}
/*----------------------------------------------------------------------------*/
static enum Result devStringAppend(void *object, struct UsbString string)
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
static void epCommonHandler(struct UsbDmaEndpoint *ep)
{
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  struct QueueHead * const head = &ep->device->base.queueHeads[index];

  enum EndpointStatus packetStatus;

  while ((packetStatus = epGetStatus(ep)) != STATUS_IDLE)
  {
    struct UsbRequest * const request = epGetHeadRequest(head);
    enum UsbRequestStatus requestStatus = USB_REQUEST_ERROR;

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
static void epControlHandler(struct UsbDmaEndpoint *ep)
{
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  struct QueueHead * const head = &ep->device->base.queueHeads[index];
  const enum EndpointStatus packetStatus = epGetStatus(ep);

  if (packetStatus == STATUS_IDLE)
    return;

  struct UsbRequest * const request = epGetHeadRequest(head);
  enum UsbRequestStatus requestStatus = USB_REQUEST_ERROR;

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
static struct TransferDescriptor *epAllocDescriptor(struct UsbDmaEndpoint *ep,
    struct UsbRequest *node, uint8_t *buffer, size_t length)
{
  if (arrayEmpty(&ep->device->base.descriptorPool))
    return 0;

  struct TransferDescriptor *descriptor;

  arrayPopBack(&ep->device->base.descriptorPool, &descriptor);

  /* The next descriptor pointer is invalid */
  descriptor->next = TD_NEXT_TERMINATE;

  /* Setup status and size, enable interrupt on completion */
  descriptor->token = TD_TOKEN_STATUS(TOKEN_STATUS_ACTIVE) | TD_TOKEN_IOC
      | TD_TOKEN_TOTAL_BYTES(length);

  /* Store pointer to the USB Request structure in reserved field */
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
static void epAppendDescriptor(struct UsbDmaEndpoint *ep,
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
static enum Result epEnqueueRx(struct UsbDmaEndpoint *ep,
    struct UsbRequest *request)
{
  struct TransferDescriptor * const descriptor = epAllocDescriptor(ep,
      request, request->buffer, request->capacity);

  assert(descriptor);
  epAppendDescriptor(ep, descriptor);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result epEnqueueTx(struct UsbDmaEndpoint *ep,
    struct UsbRequest *request)
{
  struct TransferDescriptor * const descriptor = epAllocDescriptor(ep,
      request, request->buffer, request->length);

  assert(descriptor);
  epAppendDescriptor(ep, descriptor);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void epExtractDataLength(const struct UsbDmaEndpoint *ep,
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
static void epFlush(struct UsbDmaEndpoint *ep)
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
static enum EndpointStatus epGetStatus(const struct UsbDmaEndpoint *ep)
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
static void epPopDescriptor(struct UsbDmaEndpoint *ep)
{
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  struct QueueHead * const head = &ep->device->base.queueHeads[index];
  struct TransferDescriptor * const descriptor =
      (struct TransferDescriptor *)head->listHead;

  head->listHead = (uint32_t)descriptor->next;
  if (head->listHead == TD_NEXT_TERMINATE)
    head->listTail = TD_NEXT_TERMINATE;
  arrayPushBack(&ep->device->base.descriptorPool, &descriptor);
}
/*----------------------------------------------------------------------------*/
static void epPrime(struct UsbDmaEndpoint *ep)
{
  LPC_USB_Type * const reg = ep->device->base.reg;
  const uint32_t mask = ENDPT_BIT(ep->address);

  /* Prime the endpoint for read */
  reg->ENDPTPRIME |= mask;
  /* Wait until the bit is set */
  while (reg->ENDPTPRIME & mask);
}
/*----------------------------------------------------------------------------*/
static enum Result epReadSetupPacket(struct UsbDmaEndpoint *ep,
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
static void epReprime(struct UsbDmaEndpoint *ep)
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
static enum Result epInit(void *object, const void *configBase)
{
  const struct UsbEndpointConfig * const config = configBase;
  struct UsbDevice * const device = config->parent;
  struct UsbDmaEndpoint * const ep = object;

  ep->address = config->address;
  ep->device = device;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void epDeinit(void *object)
{
  struct UsbDmaEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;

  /* Disable interrupts and remove pending requests */
  epDisable(ep);
  epClear(ep);

  /* Protect endpoint array from simultaneous access */
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);

  const IrqState state = irqSave();
  device->endpoints[index] = 0;
  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static void epClear(void *object)
{
  struct UsbDmaEndpoint * const ep = object;

  epFlush(ep);

  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  struct QueueHead * const head = &ep->device->base.queueHeads[index];

  while (head->listHead != TD_NEXT_TERMINATE)
  {
    struct UsbRequest * const request = epGetHeadRequest(head);

    request->callback(request->callbackArgument, request,
        USB_REQUEST_CANCELLED);

    epPopDescriptor(ep);
  }
}
/*----------------------------------------------------------------------------*/
static void epDisable(void *object)
{
  struct UsbDmaEndpoint * const ep = object;
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  reg->ENDPTCTRL[number] &= ep->address & 0x80 ?
      ~ENDPTCTRL_TXE : ~ENDPTCTRL_RXE;
}
/*----------------------------------------------------------------------------*/
static void epEnable(void *object, uint8_t type, uint16_t size)
{
  struct UsbDmaEndpoint * const ep = object;
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);
  struct QueueHead * const head = &ep->device->base.queueHeads[index];

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
  const bool tx = (ep->address & 0x80) != 0;
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
  epFlush(ep);

  /* Reset data toggles */
  reg->ENDPTCTRL[number] |= tx ? ENDPTCTRL_TXR : ENDPTCTRL_RXR;
}
/*----------------------------------------------------------------------------*/
static enum Result epEnqueue(void *object, struct UsbRequest *request)
{
  assert(request->callback);

  struct UsbDmaEndpoint * const ep = object;
  enum Result res;

  const IrqState state = irqSave();

  if (ep->address & USB_EP_DIRECTION_IN)
    res = epEnqueueTx(ep, request);
  else
    res = epEnqueueRx(ep, request);

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
static bool epIsStalled(void *object)
{
  const struct UsbDmaEndpoint * const ep = object;
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);
  const uint32_t stallMask = ep->address & 0x80 ? ENDPTCTRL_TXS : ENDPTCTRL_RXS;

  return (reg->ENDPTCTRL[number] & stallMask) != 0;
}
/*----------------------------------------------------------------------------*/
static void epSetStalled(void *object, bool stalled)
{
  struct UsbDmaEndpoint * const ep = object;
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  const bool tx = (ep->address & 0x80) != 0;
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
