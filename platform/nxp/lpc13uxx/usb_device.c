/*
 * usb_device.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <string.h>
#include <xcore/accel.h>
#include <halm/delay.h>
#include <halm/generic/pointer_queue.h>
#include <halm/platform/nxp/lpc13uxx/usb_base.h>
#include <halm/platform/nxp/lpc13uxx/usb_defs.h>
#include <halm/platform/nxp/usb_device.h>
#include <halm/usb/usb_control.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
/*----------------------------------------------------------------------------*/
struct UsbSieEndpoint
{
  struct UsbEndpoint base;

  /* Parent device */
  struct UsbDevice *device;
  /* Queued requests */
  PointerQueue requests;
  /* Address inside the packet buffer memory */
  uint16_t position;
  /* Max request size */
  uint16_t size;
  /* Logical address */
  uint8_t address;
};

struct UsbDevice
{
  struct UsbBase base;

  /* Array of registered endpoints */
  struct UsbEndpoint *endpoints[USB_EP_NUMBER];
  /* Control message handler */
  struct UsbControl *control;

  /* The last allocated address inside the packet buffer memory */
  uint16_t position;
  /* Device is configured */
  bool configured;
  /* Device is enabled */
  bool enabled;


  uint8_t scheduledAddress;
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static void resetDevice(struct UsbDevice *);
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
static UsbStringNumber devStringAppend(void *, struct UsbString);
static void devStringErase(void *, struct UsbString);

#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *);
#else
#define devDeinit deletedDestructorTrap
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

    .setPower = devSetPower,
    .getSpeed = devGetSpeed,

    .stringAppend = devStringAppend,
    .stringErase = devStringErase
};
/*----------------------------------------------------------------------------*/
static void epHandler(struct UsbSieEndpoint *, uint8_t);
// static enum Result epReadData(struct UsbSieEndpoint *, uint8_t *, size_t,
//     size_t *);
// static void epReadPacketMemory(LPC_USB_Type *, uint8_t *, size_t);
// static void epWriteData(struct UsbSieEndpoint *, const uint8_t *, size_t);
// static void epWritePacketMemory(LPC_USB_Type *, const uint8_t *, size_t);
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
static const struct UsbEndpointClass * const UsbSieEndpoint =
    &(const struct UsbEndpointClass){
    .size = sizeof(struct UsbSieEndpoint),
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
static void interruptHandler(void *object)
{
  struct UsbDevice * const device = object;
  LPC_USB_Type * const reg = device->base.reg;
  uint32_t intStatus = reg->INTSTAT; // TODO const?

  reg->INTSTAT = intStatus;

  /* Device Status Interrupt (Reset, Connect change, Suspend/Resume) */
  if (intStatus & INTSTAT_DEV_INT)
  {
    uint32_t DevCmdStat = reg->DEVCMDSTAT;       		/* Device Status */

    if (DevCmdStat & DEVCMDSTAT_DRES_C)               	/* Reset */
    {
      reg->DEVCMDSTAT |= DEVCMDSTAT_DRES_C;

      resetDevice(device);
      usbControlNotify(device->control, USB_DEVICE_EVENT_RESET);
    }

    if (DevCmdStat & DEVCMDSTAT_DCON_C)                 	/* Connect change */
    {
      reg->DEVCMDSTAT |= DEVCMDSTAT_DCON_C;
    }

    if (DevCmdStat & DEVCMDSTAT_DSUS_C)
    {
      reg->DEVCMDSTAT |= DEVCMDSTAT_DSUS_C;
      usbControlNotify(device->control, (DevCmdStat & DEVCMDSTAT_DSUS) ?
          USB_DEVICE_EVENT_SUSPEND : USB_DEVICE_EVENT_RESUME);
    }
  }

  /* Endpoint's Interrupt */
  if (intStatus & 0x3FF)
  {
    /* if any of the EP0 through EP9 is set, or bit 0 through 9 on disr */
    struct UsbEndpoint ** const endpointArray = device->endpoints;
    uint32_t epIntStatus = reverseBits32(intStatus & 0x3FF);
    bool sss = (reg->DEVCMDSTAT & DEVCMDSTAT_SETUP) != 0;

    do
    {
      const unsigned int index = countLeadingZeros32(epIntStatus);
      // const uint8_t status = usbCommandRead(device,
      //     USB_CMD_CLEAR_INTERRUPT | index);

      epIntStatus -= (1UL << 31) >> index;
      epHandler((struct UsbSieEndpoint *)endpointArray[index], index == 0 && sss); // XXX

      if (index == 0 && sss)
      {
        LPC_USB->DEVCMDSTAT |= DEVCMDSTAT_SETUP;
      }
    }
    while (epIntStatus);


    // uint32_t PhyEP;
    // for (PhyEP = 0; PhyEP < USED_PHYSICAL_ENDPOINTS; PhyEP++) /* Check All Endpoints */
    // {
    //   if ( intStatus & (1 << PhyEP) )  // Is this the endpoint that caused the interrupt?
    //   {
    //     if ( IsOutEndpoint(PhyEP) ) /* OUT Endpoint */
    //     {
    //       if ( !Endpoint_IsSETUPReceived() )
    //       {
    //         if(PhyEP == 0)
    //           usb_data_buffer_size = (512 - EndPointCmdStsList[PhyEP][0].NBytes);
    //         else
    //           usb_data_buffer_OUT_size = (512 - EndPointCmdStsList[PhyEP][0].NBytes);
    //       }
    //     }
    //     else                             /* IN Endpoint */
    //     {
    //       if (Remain_length[PhyEP/2] > 0)
    //       {
    //         uint32_t i;
    //         if(PhyEP == 1) /* Control IN */
    //         {
    //           for (i = 0; i < Remain_length[PhyEP/2]; i++)
    //           {
    //             usb_data_buffer [i] = usb_data_buffer [i + EndpointMaxPacketSize[PhyEP]];
    //           }
    //           DcdDataTransfer(PhyEP,usb_data_buffer, Remain_length[PhyEP/2]);
    //         }
    //         else
    //         {
    //           for (i = 0; i < Remain_length[PhyEP/2]; i++)
    //           {
    //             usb_data_buffer_IN [i] = usb_data_buffer_IN [i + EndpointMaxPacketSize[PhyEP]];
    //           }
    //           DcdDataTransfer(PhyEP,usb_data_buffer_IN, Remain_length[PhyEP/2]);
    //         }
    //       }
    //       else
    //       {
    //         if(PhyEP == 1) /* Control IN */
    //         {
    //           if(shortpacket)
    //           {
    //             shortpacket = false;
    //             DcdDataTransfer(PhyEP, usb_data_buffer, 0);
    //           }
    //         }
    //       }
    //     }
    //   }
    // }
  }




  // const uint32_t intStatus = reg->USBDevIntSt;
  //
  // reg->USBDevIntClr = intStatus;
  //
  // /* Device status interrupt */
  // if (intStatus & USBDevInt_DEV_STAT)
  // {
  //   const uint8_t devStatus = usbCommandRead(device,
  //       USB_CMD_GET_DEVICE_STATUS);
  //
  //   if (devStatus & DEVICE_STATUS_RST)
  //   {
  //     resetDevice(device);
  //     usbControlNotify(device->control, USB_DEVICE_EVENT_RESET);
  //   }
  //
  //   if (devStatus & DEVICE_STATUS_SUS_CH)
  //   {
  //     usbControlNotify(device->control, (devStatus & DEVICE_STATUS_SUS) ?
  //         USB_DEVICE_EVENT_SUSPEND : USB_DEVICE_EVENT_RESUME);
  //   }
  // }
  //
  // /* Endpoint interrupt */
  // if (intStatus & USBDevInt_EP_MASK)
  // {
  //   struct UsbEndpoint ** const endpointArray = device->endpoints;
  //   uint32_t epIntStatus = reverseBits32((intStatus >> 1) & 0xFF);
  //
  //   do
  //   {
  //     const unsigned int index = countLeadingZeros32(epIntStatus);
  //     const uint8_t status = usbCommandRead(device,
  //         USB_CMD_CLEAR_INTERRUPT | index);
  //
  //     epIntStatus -= (1UL << 31) >> index;
  //     epHandler((struct UsbSieEndpoint *)endpointArray[index], status);
  //   }
  //   while (epIntStatus);
  // }
}
/*----------------------------------------------------------------------------*/
static void resetDevice(struct UsbDevice *device)
{
  LPC_USB_Type * const reg = device->base.reg;

//  device->position = ((uint32_t)device->base.epList & 0xFFFF) + USB_EPLIST_SIZE;
  device->position = ((uint32_t)device->base.epList & 0xFFFF) + 256;
  memset(device->base.epList, 0, USB_EPLIST_SIZE);

  reg->EPINUSE = 0;
  reg->EPSKIP = 0xFFFFFFFFUL;
  reg->EPBUFCFG = 0;

  reg->DEVCMDSTAT &= ~DEVCMDSTAT_DEV_ADDR_MASK;
  reg->DEVCMDSTAT |= DEVCMDSTAT_DEV_EN;
  /* Clear all EP interrupts, device status, and SOF interrupts. */
  reg->INTSTAT = 0xC00003FF;
  /* Enable all ten(10) EPs interrupts including EP0, note: EP won't be
  ready until it's configured/enabled when device sending SetEPStatus command
  to the command engine. */
  reg->INTEN = INTEN_DEV_INT_EN;

  /* Set inactive configuration */
  device->configured = false;

  device->scheduledAddress = 0;
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

  /* Call base class constructor */
  const enum Result res = UsbBase->init(object, &baseConfig);
  if (res != E_OK)
    return res;

  device->base.handler = interruptHandler;
  device->enabled = false;

  memset(device->endpoints, 0, sizeof(device->endpoints));
//  device->position = ((uint32_t)device->base.epList & 0xFFFF) + USB_EPLIST_SIZE;
  device->position = ((uint32_t)device->base.epList & 0xFFFF) + 256;

  /* Initialize control message handler after endpoint initialization */
  device->control = init(UsbControl, &controlConfig);
  if (!device->control)
    return E_ERROR;

  /* Configure interrupts and reset system variables */
  resetDevice(device);
  // /* By default, only ACKs generate interrupts */
  // usbCommandWrite(device, USB_CMD_SET_MODE, 0); // XXX

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
  UsbBase->deinit(device);
}
#endif
/*----------------------------------------------------------------------------*/
static void *devCreateEndpoint(void *object, uint8_t address)
{
  struct UsbDevice * const device = object;
  const unsigned int index = EP_TO_INDEX(address);

  assert(index < ARRAY_SIZE(device->endpoints));

  struct UsbEndpoint *ep = 0;
  const IrqState state = irqSave();

  if (!device->endpoints[index])
  {
    /* Initialization of endpoint is only available before the driver starts */
    assert(!device->enabled);

    const struct UsbEndpointConfig config = {
        .parent = device,
        .address = address
    };

    device->endpoints[index] = init(UsbSieEndpoint, &config);
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

  device->configured = address != 0;

  if (!address)
    reg->DEVCMDSTAT = (reg->DEVCMDSTAT & ~DEVCMDSTAT_DEV_ADDR_MASK) | DEVCMDSTAT_DEV_ADDR(0) | DEVCMDSTAT_DEV_EN; // XXX Why EN?
  else
    device->scheduledAddress = address;

//  reg->DEVCMDSTAT = (reg->DEVCMDSTAT & ~DEVCMDSTAT_DEV_ADDR_MASK) | DEVCMDSTAT_DEV_ADDR(address) | DEVCMDSTAT_DEV_EN; // XXX Why EN?
	// reg->DEVCMDSTAT |= (USB_EN | Address);
  // usbCommandWrite(device, USB_CMD_SET_ADDRESS,
  //     SET_ADDRESS_DEV_EN | SET_ADDRESS_DEV_ADDR(address));
  // usbCommandWrite(device, USB_CMD_CONFIGURE_DEVICE,
  //     device->configured ? CONFIGURE_DEVICE_CONF_DEVICE : 0);
}
/*----------------------------------------------------------------------------*/
static void devSetConnected(void *object, bool state)
{
  struct UsbDevice * const device = object;
  LPC_USB_Type * const reg = device->base.reg;

  device->enabled = state;

  if (state)
    reg->DEVCMDSTAT |= DEVCMDSTAT_DCON;
  else
    reg->DEVCMDSTAT &= ~DEVCMDSTAT_DCON;
  // usbCommandWrite(device, USB_CMD_SET_DEVICE_STATUS,
  //     state ? DEVICE_STATUS_CON : 0);
}
/*----------------------------------------------------------------------------*/
static enum Result devBind(void *object, void *driver)
{
  struct UsbDevice * const device = object;
  return usbControlBindDriver(device->control, driver);
}
/*----------------------------------------------------------------------------*/
static void devUnbind(void *object, const void *driver __attribute__((unused)))
{
  struct UsbDevice * const device = object;
  usbControlUnbindDriver(device->control);
}
/*----------------------------------------------------------------------------*/
static void devSetPower(void *object, uint16_t current)
{
  struct UsbDevice * const device = object;
  usbControlSetPower(device->control, current);
}
/*----------------------------------------------------------------------------*/
static enum UsbSpeed devGetSpeed(const void *object __attribute__((unused)))
{
  return USB_FS;
}
/*----------------------------------------------------------------------------*/
static UsbStringNumber devStringAppend(void *object, struct UsbString string)
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
static void epHandler(struct UsbSieEndpoint *ep, uint8_t status)
{
  if (pointerQueueEmpty(&ep->requests))
    return;

  if (ep->address & USB_EP_DIRECTION_IN)
  {
    struct UsbDevice * const device = ep->device;
    const unsigned int index = EP_TO_INDEX(ep->address);

    if (!USB_EP_LOGICAL_ADDRESS(ep->address))
    {
      struct UsbDevice * const device = ep->device;

      if (device->scheduledAddress)
      {
        LPC_USB_Type * const reg = ep->device->base.reg;

        /*
         * Set a previously saved device address after the status stage
         * of the control transaction.
         */
        reg->DEVCMDSTAT = (reg->DEVCMDSTAT & ~DEVCMDSTAT_DEV_ADDR_MASK) | DEVCMDSTAT_DEV_ADDR(device->scheduledAddress) | DEVCMDSTAT_DEV_EN; // XXX Why EN?
        device->scheduledAddress = 0;
      }
    }

    // XXX Is it needed
    if (device->base.epList[index * 2] & EPCS_S)
    {
      /* The endpoint is stalled */
      return;
    }

    const size_t pending = pointerQueueSize(&ep->requests);
    // size_t count = 0;

    // if (!(status & SELECT_ENDPOINT_B1FULL))
    //   ++count;
    // if ((dbAvailable & (1UL << index)) && !(status & SELECT_ENDPOINT_B2FULL))
    //   ++count;
    // count = MIN(count, pending);

    // while (count--)
    // {
      struct UsbRequest * const request = pointerQueueFront(&ep->requests);
      pointerQueuePopFront(&ep->requests);

      // epWriteData(ep, request->buffer, request->length);
//      device->base.epList[index * 2] = (device->base.epList[index * 2] & ~EPCS_NBytes_MASK) | EPCS_NBytes(request->length);
      device->base.epList[index * 2] = (device->base.epList[index * 2] & ~(EPCS_NBytes_MASK | EPCS_AddressOffset_MASK)) | EPCS_NBytes(request->length) | EPCS_AddressOffset(ep->position >> 6);
      memcpy((void *)(0x20000000 + ep->position), request->buffer, request->length);
      device->base.epList[index * 2] |= EPCS_A;

      request->callback(request->callbackArgument, request,
          USB_REQUEST_COMPLETED);
    // }
  }
  else
  {
    struct UsbDevice * const device = ep->device;
    const unsigned int index = EP_TO_INDEX(ep->address);

    struct UsbRequest * const request = pointerQueueFront(&ep->requests);

    if (!status)
    {
    const uint32_t read = ep->size - EPCS_NBytes_VALUE(device->base.epList[index * 2]);
    memcpy(request->buffer, (void *)(0x20000000 + ep->position), read);
    request->length = read;
    }
    else
    {
      memcpy(request->buffer, (void *)(0x20000000 + ep->position + ep->size), sizeof(struct UsbSetupPacket));
      request->length = sizeof(struct UsbSetupPacket);

      device->base.epList[0 * 2] &= ~EPCS_S;
      device->base.epList[1 * 2] &= ~EPCS_S;
    }
    // request->length = read;

    // if (epReadData(ep, request->buffer, request->capacity, &read) == E_OK)
    // {
      // const enum UsbRequestStatus requestStatus = status & SELECT_ENDPOINT_STP ?
      //     USB_REQUEST_SETUP : USB_REQUEST_COMPLETED;
      const enum UsbRequestStatus requestStatus = status ?
          USB_REQUEST_SETUP : USB_REQUEST_COMPLETED;

      pointerQueuePopFront(&ep->requests);
      request->callback(request->callbackArgument, request, requestStatus);
    // }

    if (!pointerQueueEmpty(&ep->requests))
    {
      device->base.epList[index * 2] = (device->base.epList[index * 2] & ~(EPCS_NBytes_MASK | EPCS_AddressOffset_MASK)) | EPCS_NBytes(ep->size) | EPCS_AddressOffset(ep->position >> 6);
      if (index == 0)
        device->base.epList[index * 2 + 1] = EPCS_AddressOffset((ep->position + ep->size) >> 6);
      device->base.epList[index * 2] |= EPCS_A;
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum Result epInit(void *object, const void *configBase)
{
  const struct UsbEndpointConfig * const config = configBase;
  struct UsbSieEndpoint * const ep = object;

  // if (pointerQueueInit(&ep->requests, CONFIG_PLATFORM_USB_DEVICE_EP_REQUESTS)) // XXX
  if (pointerQueueInit(&ep->requests, 4))
  {
    ep->address = config->address;
    ep->device = config->parent;
    ep->position = 0; // XXX
    ep->size = 0;

    return E_OK;
  }
  else
    return E_MEMORY;
}
/*----------------------------------------------------------------------------*/
static void epDeinit(void *object)
{
  struct UsbSieEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;
  const unsigned int index = EP_TO_INDEX(ep->address);

  /* Disable interrupts and remove pending requests */
  epDisable(ep);
  epClear(ep);

  const IrqState state = irqSave();
  device->endpoints[index] = 0;
  irqRestore(state);

  assert(pointerQueueEmpty(&ep->requests));
  pointerQueueDeinit(&ep->requests);
}
/*----------------------------------------------------------------------------*/
static void epClear(void *object)
{
  struct UsbSieEndpoint * const ep = object;

  while (!pointerQueueEmpty(&ep->requests))
  {
    struct UsbRequest * const request = pointerQueueFront(&ep->requests);
    pointerQueuePopFront(&ep->requests);

    request->callback(request->callbackArgument, request,
        USB_REQUEST_CANCELLED);
  }
}
/*----------------------------------------------------------------------------*/
static void epDisable(void *object)
{
  struct UsbSieEndpoint * const ep = object;
  LPC_USB_Type * const reg = ep->device->base.reg;
  const uint32_t mask = ~(1UL << EP_TO_INDEX(ep->address));

  reg->INTSTAT &= ~mask;
  reg->INTEN &= ~mask;
}
/*----------------------------------------------------------------------------*/
static void epEnable(void *object, uint8_t type __attribute__((unused)),
    uint16_t size)
{
  struct UsbSieEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;
  LPC_USB_Type * const reg = device->base.reg;
  const unsigned int index = EP_TO_INDEX(ep->address);
  const uint32_t mask = 1UL << index;

  reg->INTSTAT |= mask;
  reg->INTEN |= mask;

  // TODO Double buffering
  ep->position = device->position;
  ep->size = size;

  device->base.epList[index * 2] = EPCS_NBytes(size) | EPCS_AddressOffset(ep->position >> 6);
  if (index == 0)
    device->base.epList[index * 2 + 1] = EPCS_AddressOffset((ep->position + ep->size) >> 6);
  else
    device->base.epList[index * 2 + 1] = 0;
  if (type == ENDPOINT_TYPE_ISOCHRONOUS)
    device->base.epList[index * 2] |= EPCS_T;
//  device->position += size;
  device->position += 64; // XXX
  if (index == 0)
//    device->position += sizeof(struct UsbSetupPacket);
    device->position += 64;
}
/*----------------------------------------------------------------------------*/
static enum Result epEnqueue(void *object, struct UsbRequest *request)
{
  //XXX ep->position must not be zero
  assert(request->callback);

  struct UsbSieEndpoint * const ep = object;
  const unsigned int index = EP_TO_INDEX(ep->address);

  /*
   * Additional checks should be performed for data endpoints
   * to avoid USB controller hanging.
   */
  if (index >= 2 && !ep->device->configured)
    return E_IDLE;

  irqDisable(ep->device->base.irq);
  assert(!pointerQueueFull(&ep->requests));

  struct UsbDevice * const device = ep->device;
  // const unsigned int index = EP_TO_INDEX(ep->address);
  LPC_USB_Type * const reg = device->base.reg;

  if (ep->address & USB_EP_DIRECTION_IN)
  {
//    if (!(device->base.epList[index * 2] & EPCS_A))
//    {
//      device->base.epList[index * 2] = (device->base.epList[index * 2] & ~(EPCS_NBytes_MASK | EPCS_AddressOffset_MASK)) | EPCS_NBytes(request->length) | EPCS_AddressOffset(ep->position >> 6);
//      memcpy((void *)(0x20000000 + ep->position), request->buffer, request->length);
//      device->base.epList[index * 2] |= EPCS_A;
//
//      request->callback(request->callbackArgument, request,
//          USB_REQUEST_COMPLETED);
//    }
//    else
//    {
//      pointerQueuePushBack(&ep->requests, request);
//    }



    const bool issue = pointerQueueEmpty(&ep->requests);

    pointerQueuePushBack(&ep->requests, request);

    if (issue)
      reg->INTSETSTAT |= INTSETSTAT_EP_SET_INT(index);
  }
  else if (!(device->base.epList[index * 2] & EPCS_A))
  {
    pointerQueuePushBack(&ep->requests, request);
//    device->base.epList[index * 2] = (device->base.epList[index * 2] & ~EPCS_NBytes_MASK) | EPCS_NBytes(ep->size);
    device->base.epList[index * 2] = (device->base.epList[index * 2] & ~(EPCS_NBytes_MASK | EPCS_AddressOffset_MASK)) | EPCS_NBytes(ep->size) | EPCS_AddressOffset(ep->position >> 6);
    if (index == 0)
      device->base.epList[index * 2 + 1] = EPCS_AddressOffset((ep->position + ep->size) >> 6);
    device->base.epList[index * 2] |= EPCS_A;
  }
  else
  {
    pointerQueuePushBack(&ep->requests, request);
  }


  // const uint8_t epCode = USB_CMD_SELECT_ENDPOINT | index;
  // const uint8_t epStatus = usbCommandRead(ep->device, epCode);
  // bool invokeHandler = false;
  //
  // if (ep->address & USB_EP_DIRECTION_IN)
  // {
  //   static const uint8_t mask = SELECT_ENDPOINT_ST
  //       | SELECT_ENDPOINT_B1FULL | SELECT_ENDPOINT_B2FULL;
  //
  //   invokeHandler = !(epStatus & mask) && pointerQueueEmpty(&ep->requests);
  // }
  // else if (epStatus & SELECT_ENDPOINT_FE)
  // {
  //   invokeHandler = true;
  // }
  //
  // pointerQueuePushBack(&ep->requests, request);
  //
  // if (invokeHandler)
  // {
  //   LPC_USB_Type * const reg = ep->device->base.reg;
  //   const uint32_t mask = 1UL << (index + 1);
  //
  //   /* Schedule interrupt */
  //   reg->USBDevIntSet = mask;
  // }

  irqEnable(ep->device->base.irq);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static bool epIsStalled(void *object)
{
  struct UsbSieEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;
  LPC_USB_Type * const reg = device->base.reg;
  const unsigned int index = EP_TO_INDEX(ep->address);

  return (device->base.epList[index * 2] & EPCS_S) != 0;
}
/*----------------------------------------------------------------------------*/
static void epSetStalled(void *object, bool stalled)
{
  struct UsbSieEndpoint * const ep = object;
  struct UsbDevice * const device = ep->device;
  LPC_USB_Type * const reg = device->base.reg;
  const unsigned int index = EP_TO_INDEX(ep->address);

  // XXX Rewrite
  if (stalled)
    device->base.epList[index * 2] |= EPCS_S/* | EPCS_A*/;
  else
    device->base.epList[index * 2] = (device->base.epList[index * 2] & ~(EPCS_RF_TV | EPCS_S)) | EPCS_TR;

  // TODO
  // /* Write pending IN request to the endpoint buffer */
  // if (!stalled && (ep->address & USB_EP_DIRECTION_IN)
  //     && !pointerQueueEmpty(&ep->requests))
  // {
  //   struct UsbRequest * const request = pointerQueueFront(&ep->requests);
  //   pointerQueuePopFront(&ep->requests);
  //
  //   epWriteData(ep, request->buffer, request->length);
  //   request->callback(request->callbackArgument, request,
  //       USB_REQUEST_COMPLETED);
  // }
}
