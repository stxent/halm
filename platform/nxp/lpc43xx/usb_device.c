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
static enum result epReadData(struct UsbEndpoint *, uint8_t *, uint32_t,
    uint32_t *);
static enum result epWriteData(struct UsbEndpoint *, const uint8_t *, uint32_t,
    uint32_t *);
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
/* dTD Transfer Description */
typedef volatile struct
{
  volatile uint32_t next_dTD;
  volatile uint32_t total_bytes ;
  volatile uint32_t buffer0;
  volatile uint32_t buffer1;
  volatile uint32_t buffer2;
  volatile uint32_t buffer3;
  volatile uint32_t buffer4;
  volatile uint32_t reserved;
}  DTD_T;

/* dQH  Queue Head */
typedef volatile struct
{
  volatile uint32_t cap;
  volatile uint32_t curr_dTD;
  volatile uint32_t next_dTD;
  volatile uint32_t total_bytes;
  volatile uint32_t buffer0;
  volatile uint32_t buffer1;
  volatile uint32_t buffer2;
  volatile uint32_t buffer3;
  volatile uint32_t buffer4;
  volatile uint32_t reserved;
  volatile uint32_t setup[2];
  volatile uint32_t gap[4];
}  DQH_T;

/* dQH field and bit defines */
/* Temp fixed on max, should be taken out of table */
#define QH_MAX_CTRL_PAYLOAD       0x03ff
#define QH_MAX_PKT_LEN_POS            16
#define QH_MAXP(n)                (((n) & 0x3FF)<<16)
#define QH_IOS                    (1<<15)
#define QH_ZLT                    (1<<29)

/* dTD field and bit defines */
#define TD_NEXT_TERMINATE         (1<<0)
#define TD_IOC                    (1<<15)

#define EP_NUM_MAX 12

DQH_T ep_QH[EP_NUM_MAX] __attribute__((aligned(2048)));
DTD_T ep_TD[EP_NUM_MAX] __attribute__((aligned(32)));

static bool pendingAddress = false;
static uint8_t adddr = 0;
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
    /* Clear the endpoint complete CTRL OUT & IN when */
    /* a Setup is received */
//    reg->ENDPTCOMPLETE = 0x00010001;
//    /* enable NAK inetrrupts */
    reg->ENDPTNAKEN |= 0x00010001;

    /* Check registered endpoints */
    const struct ListNode *current = listFirst(&device->endpoints);
    struct UsbEndpoint *endpoint;

    while (current)
    {
      listData(&device->endpoints, current, &endpoint);

      const uint32_t mask = ENDPT_BIT(endpoint->address);

      if (setupStatus & mask)
      {
        epHandler(endpoint, 1);
        reg->ENDPTCOMPLETE = mask;
      }

      current = listNext(current);
    }


//    if (g_drv.USB_P_EP[0]){
////                            LPC_UART1->THR = 's';
////                              LPC_UART1->THR = '\n';
//        g_drv.USB_P_EP[0](USB_EVT_SETUP);
  }

  /* handle completion interrupts */
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
          ep_TD[EP_TO_INDEX(endpoint->address)].total_bytes &= 0xC0;

        epHandler(endpoint, 0);

        reg->ENDPTCOMPLETE = mask;
      }

      current = listNext(current);
    }
  }

  if (intStatus & USBSTS_D_NAKI)
  {
    uint32_t val = reg->ENDPTNAK;
    val &= reg->ENDPTNAKEN;
    /* handle NAK interrupts */



    const struct ListNode *current = listFirst(&device->endpoints);
    struct UsbEndpoint *endpoint;

    while (current)
    {
      listData(&device->endpoints, current, &endpoint);

      const uint32_t mask = ENDPT_BIT(endpoint->address);

      if (val & mask)
      {
        epHandler(endpoint, 0);

        reg->ENDPTNAK = val;
      }

      current = listNext(current);
    }
  }
}
/*----------------------------------------------------------------------------*/
static void resetDevice(struct UsbDevice *device)
{
  LPC_USB_Type * const reg = device->base.reg;

//  DevStatusFS2HS = false;
  /* disable all EPs */
  reg->ENDPTCTRL0 &= ~(ENDPTCTRL_RXE | ENDPTCTRL_TXE);
  reg->ENDPTCTRL1 &= ~(ENDPTCTRL_RXE | ENDPTCTRL_TXE);
  reg->ENDPTCTRL2 &= ~(ENDPTCTRL_RXE | ENDPTCTRL_TXE);
  reg->ENDPTCTRL3 &= ~(ENDPTCTRL_RXE | ENDPTCTRL_TXE);
  reg->ENDPTCTRL4 &= ~(ENDPTCTRL_RXE | ENDPTCTRL_TXE);
  reg->ENDPTCTRL5 &= ~(ENDPTCTRL_RXE | ENDPTCTRL_TXE);

  /* Clear all pending interrupts */
  reg->ENDPTNAK   = 0xFFFFFFFF;
  reg->ENDPTNAKEN = 0;
  reg->USBSTS_D     = 0xFFFFFFFF;
  reg->ENDPTSETUPSTAT = reg->ENDPTSETUPSTAT;
  reg->ENDPTCOMPLETE  = reg->ENDPTCOMPLETE;
  while (reg->ENDPTPRIME)                  /* Wait until all bits are 0 */
  {
  }
  reg->ENDPTFLUSH = 0xFFFFFFFF;
  while (reg->ENDPTFLUSH); /* Wait until all bits are 0 */


  /* Set the interrupt Threshold control interval to 0 */
  reg->USBCMD_D &= ~0x00FF0000;

  /* Zero out the Endpoint queue heads */
  memset((void*)ep_QH, 0, EP_NUM_MAX * sizeof(DQH_T));
  /* Zero out the device transfer descriptors */
  memset((void*)ep_TD, 0, EP_NUM_MAX * sizeof(DTD_T));
//  memset((void*)ep_read_len, 0, sizeof(ep_read_len));
  /* Configure the Endpoint List Address */
  /* make sure it in on 64 byte boundary !!! */
  /* init list address */
  reg->ENDPOINTLISTADDR = (uint32_t)ep_QH;
  /* Initialize device queue heads for non ISO endpoint only */
  for (unsigned i = 0; i < EP_NUM_MAX; i++)
  {
    ep_QH[i].next_dTD = (uint32_t)&ep_TD[i];
  }
  /* Enable interrupts */
  reg->USBINTR_D = USBSTS_D_UI | USBSTS_D_UEI | USBSTS_D_PCI
      | USBSTS_D_URI | USBSTS_D_SLI | USBSTS_D_NAKI;
//  reg->usbintr |= (0x1<<7);   /* Test SOF */

  /* enable ep0 IN and ep0 OUT */
  ep_QH[0].cap = QH_MAXP(64) | QH_IOS | QH_ZLT;
  ep_QH[1].cap = QH_MAXP(64) | QH_IOS | QH_ZLT;
  /* enable EP0 */
  reg->ENDPTCTRL0 = ENDPTCTRL_RXE | ENDPTCTRL_RXR | ENDPTCTRL_TXE | ENDPTCTRL_TXR;
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

  if (address) {
    pendingAddress = true;
    adddr = address;
  } else {
    reg->DEVICEADDR = DEVICEADDR_USBADR(address);
    reg->DEVICEADDR |= DEVICEADDR_USBADRA;
  }
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

  //TODO
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
  uint32_t num = EP_TO_INDEX(EPNum);

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

void USB_ProgDTD(uint32_t Edpt, uint32_t ptrBuff, uint32_t TsfSize)
{
  DTD_T*  pDTD;

  pDTD = (DTD_T*)&ep_TD[ Edpt ];

  /* Zero out the device transfer descriptors */
  memset((void*)pDTD, 0, sizeof(DTD_T));
  /* The next DTD pointer is INVALID */
  pDTD->next_dTD = 0x01 ;

  /* Length */
  pDTD->total_bytes = ((TsfSize & 0x7fff) << 16);
  pDTD->total_bytes |= TD_IOC ;
  pDTD->total_bytes |= 0x80 ;

  pDTD->buffer0 = ptrBuff;
  pDTD->buffer1 = (ptrBuff + 0x1000) & 0xfffff000;
  pDTD->buffer2 = (ptrBuff + 0x2000) & 0xfffff000;
  pDTD->buffer3 = (ptrBuff + 0x3000) & 0xfffff000;
  pDTD->buffer4 = (ptrBuff + 0x4000) & 0xfffff000;

  ep_QH[Edpt].next_dTD = (uint32_t)(&ep_TD[ Edpt ]);
  ep_QH[Edpt].total_bytes &= (~0xC0) ;
}

uint32_t USB_ReadReqEP(uint32_t EPNum, uint8_t *pData, uint32_t len)
{
  LPC_USB_Type *reg = LPC_USB0;
  uint32_t num = EP_TO_INDEX(EPNum);
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
  DTD_T*  pDTD ;

  n = ENDPT_BIT(EPNum);
  pDTD = (DTD_T*)&ep_TD[n];

  /* return the total bytes read */
  return (pDTD->total_bytes >> 16) & 0x7FFF;
//  cnt = ep_read_len[EPNum & 0x0F] - cnt;
//  return (cnt);
}

uint32_t USB_WriteEP(uint32_t EPNum, uint8_t *pData, uint32_t cnt)
{
  LPC_USB_Type *reg = LPC_USB0;
  uint32_t mask = ENDPT_BIT(EPNum);

  USB_ProgDTD(EP_TO_INDEX(EPNum), (uint32_t)pData, cnt);
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
    return; /* FIXME Something went wrong */
  queuePop(&endpoint->requests, &request);

  if (endpoint->address & EP_DIRECTION_IN)
  {
    requestStatus = REQUEST_COMPLETED;

    if (pendingAddress) {
      pendingAddress = false;
      LPC_USB_Type * const reg = endpoint->device->base.reg;
      reg->DEVICEADDR = DEVICEADDR_USBADR(adddr);
      reg->DEVICEADDR |= DEVICEADDR_USBADRA;
    }

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

      if (xxx)
      {
        request->base.length = xxx;
        requestStatus = REQUEST_COMPLETED;
      }
      else
      {
        /* Read failed, return request to the queue */
        queuePush(&endpoint->requests, &request);
      }
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
  const unsigned int index = EP_TO_INDEX(endpoint->address);
//  const uint32_t mask = BIT(index);
  enum result res = E_OK;

  assert(request->base.callback);

  irqDisable(endpoint->device->base.irq);

//  /*
//   * Additional checks should be performed for data endpoints
//   * to avoid USB controller hanging issues.
//   */
//  if (index >= 2 && !endpoint->device->configuration)
//  {
//    irqEnable(endpoint->device->base.irq);
//    return E_IDLE;
//  }

  if (!queueFull(&endpoint->requests))
  {
    queuePush(&endpoint->requests, &request);

    uint32_t mmm = ENDPT_BIT(endpoint->address);

    if (!(reg->ENDPTSTAT & mmm))
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
//  const unsigned int index = EP_TO_INDEX(endpoint->address);

  uint32_t lep = endpoint->address & 0x0F;
  if (endpoint->address & 0x80)
  {
    return (((uint32_t*)&(reg->ENDPTCTRL0))[lep] & ENDPTCTRL_TXS) != 0;
  }
  else
  {
    return (((uint32_t*)&(reg->ENDPTCTRL0))[lep] & ENDPTCTRL_RXS) != 0;
  }
}
/*----------------------------------------------------------------------------*/
static void epSetEnabled(void *object, bool state, uint8_t type, uint16_t size)
{
  struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;
  const unsigned int index = EP_TO_INDEX(endpoint->address);
//  const uint32_t mask = BIT(index);

  if (index <= 1) //FIXME!!!11
    return;

  uint32_t ep_cfg;

  const unsigned int lep = endpoint->address & 0x7F;

  ep_cfg = ((uint32_t*)&(reg->ENDPTCTRL0))[lep];

  /* set EP type */
  if (type != ENDPOINT_TYPE_ISOCHRONOUS)
  {
    /* init EP capabilities */
    ep_QH[index].cap  = QH_MAXP(size) | QH_IOS | QH_ZLT ;
    /* The next DTD pointer is INVALID */
    ep_TD[index].next_dTD = 0x01 ;
  }
  else
  {
    /* init EP capabilities */
    ep_QH[index].cap  = QH_MAXP(0x400) | QH_ZLT;
  }
  /* setup EP control register */
  if (endpoint->address & 0x80)
  {
    ep_cfg &= ~0xFFFF0000;
    ep_cfg |= ENDPTCTRL_TXT(type) | ENDPTCTRL_TXR;
  }
  else
  {
    ep_cfg &= ~0xFFFF;
    ep_cfg |= ENDPTCTRL_RXT(type) | ENDPTCTRL_RXR;
  }
  ((uint32_t*)&(reg->ENDPTCTRL0))[lep] = ep_cfg;


  if (state)
  {
    if (endpoint->address & 0x80)
    {
      ((uint32_t*)&(reg->ENDPTCTRL0))[lep] |= ENDPTCTRL_TXE;
    }
    else
    {
      ((uint32_t*)&(reg->ENDPTCTRL0))[lep] |= ENDPTCTRL_RXE;
//      /* enable NAK interrupt */
//      uint32_t bitpos = ENDPT_BIT(endpoint->address);
//      reg->ENDPTNAKEN |= bitpos;
    }
  }
  else
  {
    if (endpoint->address & 0x80)
    {
      ((uint32_t*)&(reg->ENDPTCTRL0))[lep] &= ~ENDPTCTRL_TXE;
    }
    else
    {
      ((uint32_t*)&(reg->ENDPTCTRL0))[lep] &= ~ENDPTCTRL_RXE;
      /* disable NAK interrupt */
      uint32_t bitpos = ENDPT_BIT(endpoint->address);
      reg->ENDPTNAKEN &= ~bitpos;
    }
  }


  //RESET EP
  if (state)
  {
    uint32_t mask = ENDPT_BIT(endpoint->address);

    /* flush EP buffers */
    reg->ENDPTFLUSH = mask;
    while (reg->ENDPTFLUSH & mask);
    /* reset data toggles */
    if (endpoint->address & 0x80)
    {
      ((uint32_t*)&(reg->ENDPTCTRL0))[lep] |= ENDPTCTRL_TXR;
    }
    else
    {
      ((uint32_t*)&(reg->ENDPTCTRL0))[lep] |= ENDPTCTRL_RXR;
    }
  }
}
/*----------------------------------------------------------------------------*/
static void epSetStalled(void *object, bool stalled)
{
  struct UsbEndpoint * const endpoint = object;
  LPC_USB_Type * const reg = endpoint->device->base.reg;
//  const unsigned int index = EP_TO_INDEX(endpoint->address);

  uint32_t lep = endpoint->address & 0x0F;

  if (stalled)
  {
    if (endpoint->address & 0x80)
    {
      ((uint32_t*)&(reg->ENDPTCTRL0))[lep] |= ENDPTCTRL_TXS;
    }
    else
    {
      ((uint32_t*)&(reg->ENDPTCTRL0))[lep] |= ENDPTCTRL_RXS;
    }
  }
  else
  {
    if (endpoint->address & 0x80)
    {
      ((uint32_t*)&(reg->ENDPTCTRL0))[lep] &= ENDPTCTRL_TXS;
      /* reset data toggle */
      ((uint32_t*)&(reg->ENDPTCTRL0))[lep] |= ENDPTCTRL_TXR;
    }
    else
    {
      ((uint32_t*)&(reg->ENDPTCTRL0))[lep] &= ENDPTCTRL_RXS;
      /* reset data toggle */
      ((uint32_t*)&(reg->ENDPTCTRL0))[lep] |= ENDPTCTRL_RXR;
    }
  }
}
