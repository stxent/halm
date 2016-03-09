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
//static enum result epReadData(struct UsbEndpoint *, uint8_t *, uint32_t,
//    uint32_t *);
//static enum result epWriteData(struct UsbEndpoint *, const uint8_t *, size_t,
//    size_t *);
/*----------------------------------------------------------------------------*/
static enum result epInit(void *, const void *);
static void epDeinit(void *);
static void epClear(void *);
static enum result epEnqueue(void *, struct UsbRequest *);
static bool epIsStalled(void *);
static void epSetEnabled(void *, bool, uint8_t, uint16_t);
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
struct EndpointQueueHead ep_QH[ENDPT_NUMBER] __attribute__((aligned(2048)));
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
    reg->ENDPTNAK = epStatus;

    const struct ListNode *current = listFirst(&device->endpoints);
    struct UsbEndpoint *endpoint;

    while (current)
    {
      listData(&device->endpoints, current, &endpoint);

      const uint32_t mask = ENDPT_BIT(endpoint->address);

      if (epStatus & mask)
      {
        if (endpoint->address & EP_DIRECTION_IN)
        {
          // Is this needed?
//          ep_TD[EP_TO_DESCRIPTOR_NUMBER(endpoint->address)].token &=
//              TD_TOKEN_STATUS(TOKEN_STATUS_ACTIVE | TOKEN_STATUS_HALTED);
        }

        reg->ENDPTCOMPLETE = mask;

        epHandler(endpoint, 0);
      }

      current = listNext(current);
    }
  }

//  if (intStatus & USBSTS_D_NAKI)
//  {
//    uint32_t val = reg->ENDPTNAK;
//    val &= reg->ENDPTNAKEN;
//    /* handle NAK interrupts */
//
//
//
//    const struct ListNode *current = listFirst(&device->endpoints);
//    struct UsbEndpoint *endpoint;
//
//    while (current)
//    {
//      listData(&device->endpoints, current, &endpoint);
//
//      const uint32_t mask = ENDPT_BIT(endpoint->address);
//
//      if (val & mask)
//      {
//        epHandler(endpoint, 0);
//
//        reg->ENDPTNAK = val;
//      }
//
//      current = listNext(current);
//    }
//  }
}
/*----------------------------------------------------------------------------*/
static void resetDevice(struct UsbDevice *device)
{
  LPC_USB_Type * const reg = device->base.reg;

  /* Disable all endpoints */
  reg->ENDPTCTRL0 &= ~(ENDPTCTRL_RXE | ENDPTCTRL_TXE);
  reg->ENDPTCTRL1 &= ~(ENDPTCTRL_RXE | ENDPTCTRL_TXE);
  reg->ENDPTCTRL2 &= ~(ENDPTCTRL_RXE | ENDPTCTRL_TXE);
  reg->ENDPTCTRL3 &= ~(ENDPTCTRL_RXE | ENDPTCTRL_TXE);
  reg->ENDPTCTRL4 &= ~(ENDPTCTRL_RXE | ENDPTCTRL_TXE);
  reg->ENDPTCTRL5 &= ~(ENDPTCTRL_RXE | ENDPTCTRL_TXE);

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
  memset(ep_QH, 0, ENDPT_NUMBER * sizeof(struct EndpointQueueHead));
  /* Zero out the device transfer descriptors */
  memset(ep_TD, 0, ENDPT_NUMBER * sizeof(struct EndpointTransferDescriptor));

  /* Configure the Endpoint List Address */
  reg->ENDPOINTLISTADDR = (uint32_t)ep_QH;

  /* Initialize device queue heads for non ISO endpoint only */
  for (unsigned int i = 0; i < ENDPT_NUMBER; i++)
  {
    ep_QH[i].next = (uint32_t)&ep_TD[i];
  }

  /* Enable interrupts */
  reg->USBINTR_D = USBSTS_D_UI | USBSTS_D_UEI | USBSTS_D_PCI
      | USBSTS_D_URI | USBSTS_D_SLI | USBSTS_D_NAKI;
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
uint32_t USB_ReadSetupPkt(uint32_t EPNum, uint32_t *pData)
{
  LPC_USB_Type *reg = LPC_USB0;
  uint32_t setup_int, cnt = 0;
  uint32_t num = EP_TO_DESCRIPTOR_NUMBER(EPNum);

  setup_int = reg->ENDPTSETUPSTAT ;
  /* Clear the setup interrupt */
  reg->ENDPTSETUPSTAT = setup_int;

  /* ********************************** */
  /*  Check if we have received a setup */
  /* ********************************** */
  if (setup_int & (1<<0))                    /* Check only for bit 0 */
    /* No setup are admitted on other endpoints than 0 */
  {
    do
    {
      /* Setup in a setup - must considere only the second setup */
      /*- Set the tripwire */
      reg->USBCMD_D |= USBCMD_D_SUTW ;

      /* Transfer Set-up data to the gtmudsCore_Request buffer */
      pData[0] = ep_QH[num].setup[0];
      pData[1] = ep_QH[num].setup[1];
      cnt = 8;

    }
    while (!(reg->USBCMD_D & USBCMD_D_SUTW)) ;

    /* setup in a setup - Clear the tripwire */
    reg->USBCMD_D &= (~USBCMD_D_SUTW);
  }
  while ((setup_int = reg->ENDPTSETUPSTAT) != 0)
  {
    /* Clear the setup interrupt */
    reg->ENDPTSETUPSTAT = setup_int;
  }
  return cnt;
}

void USB_ProgDTD(uint32_t Edpt, uint32_t ptrBuff, size_t transferSize)
{
  struct EndpointTransferDescriptor *pDTD;

  pDTD = (struct EndpointTransferDescriptor *)&ep_TD[ Edpt ];

  /* Zero out the device transfer descriptors */
  memset((void*)pDTD, 0, sizeof(struct EndpointTransferDescriptor));
  /* The next DTD pointer is INVALID */
  pDTD->next = TD_NEXT_TERMINATE;

  /* Length */
  pDTD->token = TD_TOKEN_STATUS(TOKEN_STATUS_ACTIVE) | TD_TOKEN_IOC
      | TD_TOKEN_TOTAL_BYTES(transferSize);

  pDTD->buffer0 = ptrBuff;
  pDTD->buffer1 = (ptrBuff + 0x1000) & 0xFFFFF000;
  pDTD->buffer2 = (ptrBuff + 0x2000) & 0xFFFFF000;
  pDTD->buffer3 = (ptrBuff + 0x3000) & 0xFFFFF000;
  pDTD->buffer4 = (ptrBuff + 0x4000) & 0xFFFFF000;

  ep_QH[Edpt].next = (uint32_t)(&ep_TD[ Edpt ]);
  ep_QH[Edpt].token &= ~TD_TOKEN_STATUS(TOKEN_STATUS_ACTIVE
      | TOKEN_STATUS_HALTED);
}

uint32_t USB_ReadReqEP(uint32_t EPNum, uint8_t *pData, uint32_t len)
{
  LPC_USB_Type *reg = LPC_USB0;
  uint32_t num = EP_TO_DESCRIPTOR_NUMBER(EPNum);
  uint32_t mask = ENDPT_BIT(EPNum);

  USB_ProgDTD(num, (uint32_t)pData, len);
//  ep_read_len[EPNum & 0x0F] = len;
  /* prime the endpoint for read */
  reg->ENDPTPRIME |= mask;
  return len;
}

uint32_t USB_ReadEP(uint32_t EPNum, uint8_t *pData)
{
  uint32_t cnt, n;
  struct EndpointTransferDescriptor *pDTD ;

  n = EP_TO_DESCRIPTOR_NUMBER(EPNum);
  pDTD = (struct EndpointTransferDescriptor *)&ep_TD[n];

  return TD_TOKEN_TOTAL_BYTES_VALUE(pDTD->token);
}

uint32_t USB_WriteEP(uint32_t EPNum, uint8_t *pData, uint32_t cnt)
{
  LPC_USB_Type *reg = LPC_USB0;
  uint32_t mask = ENDPT_BIT(EPNum);

  USB_ProgDTD(EP_TO_DESCRIPTOR_NUMBER(EPNum), (uint32_t)pData, cnt);
  /* prime the endpoint for transmit */
  reg->ENDPTPRIME |= mask;

  /* check if priming succeeded */
  while (reg->ENDPTPRIME & mask);
  return (cnt);
}





static void epHandler(struct UsbEndpoint *endpoint, uint8_t status)
{
  struct UsbRequest *request = 0;
  enum usbRequestStatus requestStatus = REQUEST_ERROR;

  if (queueEmpty(&endpoint->requests))
    return;
  queuePop(&endpoint->requests, &request);

  if (endpoint->address & EP_DIRECTION_IN)
  {
    requestStatus = REQUEST_COMPLETED;

    if (!queueEmpty(&endpoint->requests))
    {
      struct UsbRequest *nextRequest;
      queuePeek(&endpoint->requests, &nextRequest);

      /*if (*/USB_WriteEP(endpoint->address, nextRequest->buffer, nextRequest->base.length);/* == nextRequest->base.length)
      {
        requestStatus = REQUEST_COMPLETED;
      }*/

    }
  }
  else
  {
    if (status)
    {
      //SETUP
      uint32_t xxx = USB_ReadSetupPkt(endpoint->address, (uint32_t*)request->buffer);

      if (xxx)
      {
        request->base.length = xxx;
        requestStatus = REQUEST_SETUP;
      }
      else
      {
        /* Read failed, return request to the queue */
        queuePush(&endpoint->requests, &request);
      }
    }
    else
    {
      uint32_t xxx = USB_ReadEP(endpoint->address, request->buffer);

//      if (xxx)
//      {
        request->base.length = request->base.capacity - xxx;
        requestStatus = REQUEST_COMPLETED;
//      }
//      else
//      {
//        /* Read failed, return request to the queue */
//        queuePush(&endpoint->requests, &request);
//      }
    }

    if (!queueEmpty(&endpoint->requests))
    {
      struct UsbRequest *firstRequest;
      queuePeek(&endpoint->requests, &firstRequest);

      USB_ReadReqEP(endpoint->address, firstRequest->buffer, firstRequest->base.capacity);
    }
  }

  request->base.callback(request->base.callbackArgument, request,
      requestStatus);
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
  epSetEnabled(endpoint, false, 0, 0);
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
  const uint32_t mask = ENDPT_BIT(endpoint->address);
  enum result res = E_OK;

  assert(request->base.callback);

  irqDisable(endpoint->device->base.irq);

  if (!queueFull(&endpoint->requests))
  {
    queuePush(&endpoint->requests, &request);

    if (!(ep_TD[EP_TO_DESCRIPTOR_NUMBER(endpoint->address)].token & TD_TOKEN_STATUS(TOKEN_STATUS_ACTIVE)) && !(reg->ENDPTCOMPLETE & mask))


//    if (!(reg->ENDPTSTAT & mask))
    {
      if (endpoint->address & EP_DIRECTION_IN)
      {
        if (USB_WriteEP(endpoint->address, request->buffer, request->base.length) != request->base.length)
        {
          irqEnable(endpoint->device->base.irq);
          return E_ERROR;
        }
      }
      else
      {
        struct UsbRequest *firstRequest;
        queuePeek(&endpoint->requests, &firstRequest);

        USB_ReadReqEP(endpoint->address, firstRequest->buffer, firstRequest->base.capacity);
      }
    }

    //TODO
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
static void epSetEnabled(void *object, bool state, uint8_t type, uint16_t size)
{
  struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const unsigned int index = EP_TO_DESCRIPTOR_NUMBER(endpoint->address);
  const unsigned int number = EP_TO_LOGICAL_NUMBER(endpoint->address);
  const uint32_t mask = ENDPT_BIT(endpoint->address);
  const bool tx = (endpoint->address & 0x80) != 0;

  uint32_t controlValue = reg->ENDPTCTRL[number];

  /* Set endpoint type */
  if (type != ENDPOINT_TYPE_ISOCHRONOUS)
  {
    ep_QH[index].capabilities = QH_MAX_PACKET_LENGTH(size) | QH_IOS | QH_ZLT ;
    ep_TD[index].next = TD_NEXT_TERMINATE;
  }
  else
  {
    ep_QH[index].capabilities = QH_MAX_PACKET_LENGTH(0x400) | QH_ZLT;
  }

  /* Setup endpoint control register */
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

  if (state)
  {
    if (tx)
    {
      reg->ENDPTCTRL[number] |= ENDPTCTRL_TXE;
    }
    else
    {
      reg->ENDPTCTRL[number] |= ENDPTCTRL_RXE;
//      /* Enable NAK interrupt */
//      reg->ENDPTNAKEN |= mask;
    }

    /* Flush endpoint buffers */
    reg->ENDPTFLUSH = mask;
    while (reg->ENDPTFLUSH & mask);

    /* Reset data toggles */
    reg->ENDPTCTRL[number] |= tx ? ENDPTCTRL_TXR : ENDPTCTRL_RXR;
  }
  else
  {
    if (tx)
    {
      reg->ENDPTCTRL[number] &= ~ENDPTCTRL_TXE;
    }
    else
    {
      reg->ENDPTCTRL[number] &= ~ENDPTCTRL_RXE;
      /* Disable NAK interrupt */
      reg->ENDPTNAKEN &= ~mask;
    }
  }
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
