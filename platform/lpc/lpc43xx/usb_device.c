/*
 * usb_device.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/lpc43xx/usb_base.h>
#include <halm/platform/lpc/lpc43xx/usb_defs.h>
#include <halm/platform/lpc/usb_device.h>
#include <halm/usb/usb_control.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
#include <xcore/accel.h>
#include <assert.h>
#include <stdlib.h>
/*----------------------------------------------------------------------------*/
#define CONTROL_IN  1
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
struct UsbEndpoint
{
  struct UsbEndpointBase base;

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
static bool initEndpoints(struct UsbDevice *);
static void initPeripheral(struct UsbDevice *);
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
static enum UsbSpeed devGetSpeed(const void *);
static void devSetPower(void *, uint16_t);
static UsbStringIndex devStringAppend(void *, struct UsbString);
static void devStringErase(void *, struct UsbString);

#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *);
#else
#  define devDeinit deletedDestructorTrap
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

    .getSpeed = devGetSpeed,
    .setPower = devSetPower,

    .stringAppend = devStringAppend,
    .stringErase = devStringErase
};
/*----------------------------------------------------------------------------*/
static void epCommonHandler(struct UsbEndpoint *);
static struct TransferDescriptor *epAllocDescriptor(struct UsbEndpoint *,
    struct UsbRequest *, uint8_t *, size_t);
static void epAppendDescriptor(struct UsbEndpoint *,
    struct TransferDescriptor *);
static enum Result epEnqueueRx(struct UsbEndpoint *, struct UsbRequest *);
static enum Result epEnqueueTx(struct UsbEndpoint *, struct UsbRequest *);
static void epExtractDataLength(const struct UsbEndpoint *,
    struct UsbRequest *);
static void epFlush(struct UsbEndpoint *);
static inline struct UsbRequest *epGetHeadRequest(const struct QueueHead *);
static enum EndpointStatus epGetStatus(const struct UsbEndpoint *);
static void epPopDescriptor(struct UsbEndpoint *);
static void epPrime(struct UsbEndpoint *);
static enum Result epReadSetupPacket(struct UsbEndpoint *,
    struct UsbRequest *);
static void epReprime(struct UsbEndpoint *);
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
static const struct UsbEndpointClass * const UsbEndpoint =
    &(const struct UsbEndpointClass){
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
static bool initEndpoints(struct UsbDevice *device)
{
  device->endpoints =
      malloc(device->base.td.numberOfEndpoints * sizeof(struct UsbEndpoint *));

  if (device->endpoints != NULL)
  {
    for (size_t index = 0; index < device->base.td.numberOfEndpoints; ++index)
      device->endpoints[index] = NULL;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static void initPeripheral(struct UsbDevice *device)
{
  LPC_USB_Type * const reg = device->base.reg;

  usbBaseOtgTerminationControl(&device->base, false);
  usbBaseVbusDischargeControl(&device->base, true);

  /* Reset the controller */
  reg->USBCMD_D = USBCMD_D_RST;
  while (reg->USBCMD_D & USBCMD_D_RST);

  /* Program the controller to be the USB device controller */
  reg->USBMODE_D = USBMODE_D_CM(CM_DEVICE_CONTROLLER) | USBMODE_D_SLOM;

  /* Recommended by NXP technical support */
  reg->SBUSCFG = SBUSCFG_AHB_BRST(AHB_BRST_INCR16_UNSPECIFIED);

#ifndef CONFIG_PLATFORM_USB_HS
  /* Force Full Speed mode */
  reg->PORTSC1_D |= PORTSC1_D_PFSC;
#endif
}
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
    usbControlNotify(device->control, USB_DEVICE_EVENT_RESET);
  }

  /* Start of Frame */
  if (intStatus & USBSTS_D_SRI)
  {
    usbControlNotify(device->control, USB_DEVICE_EVENT_FRAME);
  }

  /* Port change detect */
  if (intStatus & USBSTS_D_PCI)
  {
    usbControlNotify(device->control, USB_DEVICE_EVENT_PORT_CHANGE);
  }

  /* Device suspend event */
  if ((intStatus & USBSTS_D_SLI) && !device->suspended)
  {
    usbControlNotify(device->control, USB_DEVICE_EVENT_SUSPEND);
    device->suspended = true;
  }

  /* Handle setup packets */
  const uint32_t setupStatus = reg->ENDPTSETUPSTAT;

  if ((setupStatus & ENDPT_BIT(CONTROL_OUT)))
  {
    epCommonHandler((struct UsbEndpoint *)device->endpoints[CONTROL_OUT]);
  }

  /* Handle completion interrupts */
  uint32_t epStatus = reg->ENDPTCOMPLETE;

  while (epStatus)
  {
    /* Convert interrupt flag position to physical endpoint number */
    const unsigned int position = 31 - countLeadingZeros32(epStatus);
    const unsigned int number = ((position & 0xF) << 1) + (position >> 4);
    const uint32_t mask = 1UL << position;

    reg->ENDPTCOMPLETE = mask;

    epCommonHandler((struct UsbEndpoint *)device->endpoints[number]);
    epStatus -= mask;
  }
}
/*----------------------------------------------------------------------------*/
static void resetDevice(struct UsbDevice *device)
{
  LPC_USB_Type * const reg = device->base.reg;

  device->suspended = false;

  /* Disable all endpoints */
  for (size_t index = 0; index < device->base.td.numberOfEndpoints; ++index)
    reg->ENDPTCTRL[index] &= ~(ENDPTCTRL_RXE | ENDPTCTRL_TXE);

  /* Clear all pending interrupts */
  reg->ENDPTNAK = 0xFFFFFFFFUL;
  reg->ENDPTNAKEN = 0;
  reg->USBSTS_D = 0xFFFFFFFFUL;
  reg->ENDPTSETUPSTAT = reg->ENDPTSETUPSTAT;
  reg->ENDPTCOMPLETE = reg->ENDPTCOMPLETE;

  while (reg->ENDPTPRIME);
  reg->ENDPTFLUSH = 0xFFFFFFFFUL;
  while (reg->ENDPTFLUSH);

  /* Set USB HS virtual frame length to 1 microframe */
  reg->BINTERVAL = 0;
  /* Set the Interrupt Threshold control interval to 0 */
  reg->USBCMD_D &= ~USBCMD_D_ITC_MASK;

  /* Zero out all queue heads */
  for (size_t index = 0; index < device->base.td.numberOfEndpoints; ++index)
  {
    struct QueueHead * const head = &device->base.td.heads[index];

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
  reg->ENDPOINTLISTADDR = (uint32_t)device->base.td.heads;

  /* Enable interrupts */
  uint32_t mask = USBINTR_D_UE | USBINTR_D_UEE
      | USBINTR_D_PCE | USBINTR_D_URE | USBINTR_D_SLE;

#ifdef CONFIG_PLATFORM_USB_SOF
  mask |= USBINTR_D_SRE;
#endif

  reg->USBINTR_D = mask;

  /* Reset device address */
  devSetAddress(device, 0);

  /* Reset all enabled endpoints except for Control Endpoints */
  for (size_t index = 2; index < device->base.td.numberOfEndpoints; ++index)
    device->endpoints[index] = NULL;
}
/*----------------------------------------------------------------------------*/
static void resetQueueHeads(struct UsbDevice *device)
{
  for (size_t index = 0; index < device->base.td.numberOfEndpoints; ++index)
  {
    struct QueueHead * const head = &device->base.td.heads[index];

    head->listHead = TD_NEXT_TERMINATE;
    head->listTail = TD_NEXT_TERMINATE;
    head->gap[0] = 0;
    head->gap[1] = 0;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result devInit(void *object, const void *configBase)
{
  const struct UsbDeviceConfig * const config = configBase;
  assert(config != NULL);

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
  const enum Result res = UsbBase->init(device, &baseConfig);
  if (res != E_OK)
    return res;

  device->base.handler = interruptHandler;

  initEndpoints(device);
  resetQueueHeads(device);

  /* Initialize control message handler after endpoint initialization */
  device->control = init(UsbControl, &controlConfig);
  if (device->control == NULL)
    return E_ERROR;

  initPeripheral(device);
  resetDevice(device);

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
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(address);
  struct UsbDevice * const device = object;

  assert(index < device->base.td.numberOfEndpoints);

  const struct UsbEndpointConfig config = {
      .parent = device,
      .address = address
  };
  struct UsbEndpoint * const ep = init(UsbEndpoint, &config);

  if (index < 2)
  {
    /* Set Control Endpoints immediately after creation */
    assert(device->endpoints[index] == NULL);
    device->endpoints[index] = ep;
  }

  return ep;
}
/*----------------------------------------------------------------------------*/
static uint8_t devGetInterface(const void *)
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

  usbBaseVbusDischargeControl(&device->base, !state);
}
/*----------------------------------------------------------------------------*/
static enum Result devBind(void *object, void *driver)
{
  struct UsbDevice * const device = object;
  return usbControlBindDriver(device->control, driver);
}
/*----------------------------------------------------------------------------*/
static void devUnbind(void *object, const void *)
{
  struct UsbDevice * const device = object;
  usbControlUnbindDriver(device->control);
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
static void devSetPower(void *object, uint16_t current)
{
  struct UsbDevice * const device = object;
  usbControlSetPower(device->control, current);
}
/*----------------------------------------------------------------------------*/
static UsbStringIndex devStringAppend(void *object, struct UsbString string)
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
static void epCommonHandler(struct UsbEndpoint *ep)
{
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  struct QueueHead * const head = &ep->device->base.td.heads[index];
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

    if (packetStatus == STATUS_SETUP_PACKET)
    {
      /*
       * All descriptors have been retired due to the reception
       * of the setup packet, terminate any pending requests.
       */
      epClear(ep->device->endpoints[CONTROL_IN]);

      if (head->listHead != TD_NEXT_TERMINATE)
        epReprime(ep);
    }

    request->callback(request->argument, request, requestStatus);
  }
}
/*----------------------------------------------------------------------------*/
static struct TransferDescriptor *epAllocDescriptor(struct UsbEndpoint *ep,
    struct UsbRequest *node, uint8_t *buffer, size_t length)
{
  struct TransferDescriptor *descriptor = NULL;
  const IrqState state = irqSave();

  if (!pointerArrayEmpty(&ep->device->base.td.descriptors))
  {
    descriptor = pointerArrayBack(&ep->device->base.td.descriptors);
    pointerArrayPopBack(&ep->device->base.td.descriptors);
  }

  irqRestore(state);

  if (descriptor != NULL)
  {
    /* Initialize allocated descriptor */

    /* The next descriptor pointer is invalid */
    descriptor->next = TD_NEXT_TERMINATE;

    /* Setup status and size, enable interrupt on completion */
    descriptor->token = TD_TOKEN_STATUS(TOKEN_STATUS_ACTIVE)
        | TD_TOKEN_IOC | TD_TOKEN_TOTAL_BYTES(length);

    /* Store pointer to the USB Request structure in reserved field */
    descriptor->listNode = (uint32_t)node;

    const uint32_t base = (uint32_t)buffer & 0xFFFFF000;

    descriptor->buffer0 = (uint32_t)buffer;
    descriptor->buffer1 = base + 0x1000;
    descriptor->buffer2 = base + 0x2000;
    descriptor->buffer3 = base + 0x3000;
    descriptor->buffer4 = base + 0x4000;
  }

  return descriptor;
}
/*----------------------------------------------------------------------------*/
static void epAppendDescriptor(struct UsbEndpoint *ep,
    struct TransferDescriptor *descriptor)
{
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  struct QueueHead * const head = &ep->device->base.td.heads[index];

  const IrqState state = irqSave();
  bool reprime = true;

  if (head->listHead != TD_NEXT_TERMINATE)
  {
    /* Linked list is not empty */

    struct TransferDescriptor * const tail =
        (struct TransferDescriptor *)head->listTail;

    tail->next = (uint32_t)descriptor;
    head->listTail = (uint32_t)descriptor;

    const uint32_t mask = ENDPT_BIT(ep->address);
    LPC_USB_Type * const reg = ep->device->base.reg;

    if (!(reg->ENDPTPRIME & mask))
    {
      uint32_t status;

      do
      {
        reg->USBCMD_D |= USBCMD_D_ATDTW;
        status = reg->ENDPTSTAT;
      }
      while (!(reg->USBCMD_D & USBCMD_D_ATDTW));
      reg->USBCMD_D &= ~USBCMD_D_ATDTW;

      reprime = (status & mask) == 0;
    }
    else
    {
      reprime = false;
    }
  }
  else
  {
    /* Linked list is empty */

    head->listHead = (uint32_t)descriptor;
    head->listTail = (uint32_t)descriptor;
  }

  if (reprime)
  {
    /* Mark a head descriptor as a first descriptor to transfer */
    head->next = head->listHead;
    /* Clear Active and Halt bits as mentioned in the User Manual */
    head->token &= ~TD_TOKEN_STATUS(TOKEN_STATUS_ACTIVE | TOKEN_STATUS_HALTED);

    epPrime(ep);
  }

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static enum Result epEnqueueRx(struct UsbEndpoint *ep,
    struct UsbRequest *request)
{
  struct TransferDescriptor * const descriptor = epAllocDescriptor(ep,
      request, request->buffer, request->capacity);

  if (descriptor != NULL)
  {
    epAppendDescriptor(ep, descriptor);
    return E_OK;
  }
  else
    return E_EMPTY;
}
/*----------------------------------------------------------------------------*/
static enum Result epEnqueueTx(struct UsbEndpoint *ep,
    struct UsbRequest *request)
{
  struct TransferDescriptor * const descriptor = epAllocDescriptor(ep,
      request, request->buffer, request->length);

  if (descriptor != NULL)
  {
    epAppendDescriptor(ep, descriptor);
    return E_OK;
  }
  else
    return E_EMPTY;
}
/*----------------------------------------------------------------------------*/
static void epExtractDataLength(const struct UsbEndpoint *ep,
    struct UsbRequest *request)
{
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  const struct QueueHead * const head = &ep->device->base.td.heads[index];
  const struct TransferDescriptor * const descriptor =
      (const struct TransferDescriptor *)head->listHead;

  request->length =
      request->capacity - TD_TOKEN_TOTAL_BYTES_VALUE(descriptor->token);
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
static enum EndpointStatus epGetStatus(const struct UsbEndpoint *ep)
{
  static const uint32_t tokenErrorMask = TOKEN_STATUS_HALTED
      | TOKEN_STATUS_BUFFER_ERROR | TOKEN_STATUS_TRANSACTION_ERROR;

  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  const struct QueueHead * const head = &ep->device->base.td.heads[index];

  if (head->listHead == TD_NEXT_TERMINATE)
    return STATUS_IDLE;

  const struct TransferDescriptor * const descriptor =
      (const struct TransferDescriptor *)head->listHead;

  const LPC_USB_Type * const reg = ep->device->base.reg;
  const uint32_t status = TD_TOKEN_STATUS_VALUE(descriptor->token);

  if (reg->ENDPTSETUPSTAT & ENDPT_BIT(ep->address))
    return STATUS_SETUP_PACKET;
  else if (status & tokenErrorMask)
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
  struct QueueHead * const head = &ep->device->base.td.heads[index];
  struct TransferDescriptor * const descriptor =
      (struct TransferDescriptor *)head->listHead;

  head->listHead = (uint32_t)descriptor->next;
  if (head->listHead == TD_NEXT_TERMINATE)
    head->listTail = TD_NEXT_TERMINATE;
  pointerArrayPushBack(&ep->device->base.td.descriptors, descriptor);
}
/*----------------------------------------------------------------------------*/
static void epPrime(struct UsbEndpoint *ep)
{
  LPC_USB_Type * const reg = ep->device->base.reg;
  const uint32_t mask = ENDPT_BIT(ep->address);

  /* Wait until all memory stores have been completed */
  __dmb();
  /* Prime the endpoint for IN/OUT */
  reg->ENDPTPRIME |= mask;
  /* Wait until the bit is set */
  while (reg->ENDPTPRIME & mask);
}
/*----------------------------------------------------------------------------*/
static enum Result epReadSetupPacket(struct UsbEndpoint *ep,
    struct UsbRequest *request)
{
  if (request->capacity < 8)
    return E_VALUE;

  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  const struct QueueHead * const head = &ep->device->base.td.heads[index];

  epFlush(ep);

  uint32_t buffer[2];
  uint32_t status;

  /* Clear the setup interrupt */
  while ((status = reg->ENDPTSETUPSTAT))
    reg->ENDPTSETUPSTAT = status;

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
  while ((status = reg->ENDPTSETUPSTAT))
    reg->ENDPTSETUPSTAT = status;

  /* Copy content of the setup packet into the request buffer */
  memcpy(request->buffer, buffer, sizeof(buffer));
  request->length = sizeof(buffer);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void epReprime(struct UsbEndpoint *ep)
{
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  struct QueueHead * const head = &ep->device->base.td.heads[index];
  struct TransferDescriptor * const descriptor =
      (struct TransferDescriptor *)head->listHead;

  head->next = (uint32_t)descriptor;
  /* Clear Active and Halt bits as mentioned in the User Manual */
  head->token &= ~TD_TOKEN_STATUS(TOKEN_STATUS_ACTIVE | TOKEN_STATUS_HALTED);

  epPrime(ep);
}
/*----------------------------------------------------------------------------*/
static enum Result epInit(void *object, const void *configBase)
{
  const struct UsbEndpointConfig * const config = configBase;
  struct UsbEndpoint * const ep = object;

  ep->address = config->address;
  ep->device = config->parent;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void epDeinit(void *object)
{
  struct UsbEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);

  /* Disable interrupts and remove pending requests */
  epDisable(ep);
  epClear(ep);

  if (index < 2)
  {
    assert(device->endpoints[index] == ep);
    device->endpoints[index] = NULL;
  }
}
/*----------------------------------------------------------------------------*/
static void epClear(void *object)
{
  struct UsbEndpoint * const ep = object;
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);
  struct QueueHead * const head = &ep->device->base.td.heads[index];

  if (head->listHead != TD_NEXT_TERMINATE)
    epFlush(ep);

  while (head->listHead != TD_NEXT_TERMINATE)
  {
    struct UsbRequest * const request = epGetHeadRequest(head);
    epPopDescriptor(ep);

    request->callback(request->argument, request, USB_REQUEST_CANCELLED);
  }
}
/*----------------------------------------------------------------------------*/
static void epDisable(void *object)
{
  struct UsbEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);
  LPC_USB_Type * const reg = device->base.reg;

  reg->ENDPTCTRL[number] &= ep->address & USB_EP_DIRECTION_IN ?
      ~ENDPTCTRL_TXE : ~ENDPTCTRL_RXE;

  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);

  if (index >= 2 && device->endpoints[index] == ep)
    device->endpoints[index] = NULL;
}
/*----------------------------------------------------------------------------*/
static void epEnable(void *object, uint8_t type, uint16_t size)
{
  struct UsbEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(ep->address);

  if (index >= 2)
  {
    assert(device->endpoints[index] == NULL);
    device->endpoints[index] = ep;
  }

  struct QueueHead * const head = &device->base.td.heads[index];

  /* Configure endpoint type */

  uint32_t capabilities = QH_MAX_PACKET_LENGTH(size);

  if (type == ENDPOINT_TYPE_CONTROL)
    capabilities |= QH_IOS;

  if (type == ENDPOINT_TYPE_ISOCHRONOUS)
    capabilities |= QH_MULT(1);
  else
    capabilities |= QH_ZLT;

  head->capabilities = capabilities;

  /* Setup endpoint control register */

  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);
  const bool tx = (ep->address & USB_EP_DIRECTION_IN) != 0;

  LPC_USB_Type * const reg = device->base.reg;
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

  /* Enable the endpoint */
  reg->ENDPTCTRL[number] |= tx ? ENDPTCTRL_TXE : ENDPTCTRL_RXE;

  /* Flush endpoint descriptors */
  epFlush(ep);

  /* Reset data toggles */
  reg->ENDPTCTRL[number] |= tx ? ENDPTCTRL_TXR : ENDPTCTRL_RXR;
}
/*----------------------------------------------------------------------------*/
static enum Result epEnqueue(void *object, struct UsbRequest *request)
{
  assert(request != NULL);
  assert(request->callback != NULL);

  struct UsbEndpoint * const ep = object;

  if (ep->address & USB_EP_DIRECTION_IN)
    return epEnqueueTx(ep, request);
  else
    return epEnqueueRx(ep, request);
}
/*----------------------------------------------------------------------------*/
static bool epIsStalled(void *object)
{
  const struct UsbEndpoint * const ep = object;
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);
  const uint32_t stallMask = ep->address & USB_EP_DIRECTION_IN ?
      ENDPTCTRL_TXS : ENDPTCTRL_RXS;

  return (reg->ENDPTCTRL[number] & stallMask) != 0;
}
/*----------------------------------------------------------------------------*/
static void epSetStalled(void *object, bool stalled)
{
  struct UsbEndpoint * const ep = object;
  LPC_USB_Type * const reg = ep->device->base.reg;
  const unsigned int number = USB_EP_LOGICAL_ADDRESS(ep->address);

  const bool tx = (ep->address & USB_EP_DIRECTION_IN) != 0;
  const uint32_t stallMask = tx ? ENDPTCTRL_TXS : ENDPTCTRL_RXS;

  if (stalled)
  {
    do
    {
      reg->ENDPTCTRL[number] |= stallMask;
    }
    while (!(reg->ENDPTCTRL[number] & stallMask));
  }
  else
  {
    const uint32_t resetToggleMask = tx ? ENDPTCTRL_TXR : ENDPTCTRL_RXR;

    reg->ENDPTCTRL[number] =
        (reg->ENDPTCTRL[number] & ~stallMask) | resetToggleMask;
  }
}
