/*
 * usb_base.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <platform/nxp/lpc43xx/system.h>
#include <platform/nxp/lpc43xx/system_defs.h>
#include <platform/nxp/lpc43xx/usb_base.h>
#include <platform/nxp/lpc43xx/usb_defs.h>
/*----------------------------------------------------------------------------*/
static void configPins(struct UsbBase *, const struct UsbBaseConfig *);
static enum result setDescriptor(uint8_t, const struct UsbBase *,
    struct UsbBase *);
/*----------------------------------------------------------------------------*/
static enum result devInit(void *, const void *);
static void devDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass devTable = {
    .size = 0, /* Abstract class */
    .init = devInit,
    .deinit = devDeinit
};
/*----------------------------------------------------------------------------*/
//TODO
const struct PinEntry usbPins[] = {
    {
        .key = PIN(PORT_USB, PIN_USB0_DP), /* USB0_D+ */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_USB, PIN_USB0_DM), /* USB0_D- */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_USB, PIN_USB0_VBUS), /* VBUS0 */
        .channel = 0,
        .value = 0
    }, {
//        .key = PIN(2, 9), /* USB_CONNECT */
//        .channel = 0,
//        .value = 1
//    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const UsbBase = &devTable;
static struct UsbBase *descriptors[2] = {0};

//TODO Add kconfig options
static struct EndpointQueueHead usb0QueueHeads[ENDPT_NUMBER]
    __attribute__((aligned(2048)));
static struct EndpointQueueHead usb1QueueHeads[ENDPT_NUMBER]
    __attribute__((aligned(2048)));

//static struct EndpointTransferDescriptor usb0TransferDescriptors[ENDPT_NUMBER]
//    __attribute__((aligned(32)));
/*----------------------------------------------------------------------------*/
static void configPins(struct UsbBase *device,
    const struct UsbBaseConfig *config)
{
  const pinNumber pinArray[] = {
      config->dm, config->dp, config->connect, config->vbus
  };

  for (unsigned int index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      const struct PinEntry * const pinEntry = pinFind(usbPins, pinArray[index],
          device->channel);
      assert(pinEntry);

      const struct Pin pin = pinInit(pinArray[index]);

      pinInput(pin);
      pinSetFunction(pin, pinEntry->value);
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel, const struct UsbBase *state,
    struct UsbBase *device)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state,
      device) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
void USB0_ISR(void)
{
  descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void USB1_ISR(void)
{
  descriptors[1]->handler(descriptors[1]);
}
/*----------------------------------------------------------------------------*/
static enum result devInit(void *object, const void *configBase)
{
  const struct UsbBaseConfig * const config = configBase;
  struct UsbBase * const device = object;
  enum result res;

  device->channel = config->channel;
  device->handler = 0;

  /* Try to set peripheral descriptor */
  if ((res = setDescriptor(device->channel, 0, device)) != E_OK)
    return res;

  configPins(device, configBase);

  switch (device->channel)
  {
    case 0:
      sysClockEnable(CLK_M4_USB0);
      sysClockEnable(CLK_USB0);
      sysResetEnable(RST_USB0);

      device->irq = USB0_IRQ;
      device->reg = LPC_USB0;
      device->queueHeads = usb0QueueHeads;

      LPC_CREG->CREG0 &= ~CREG0_USB0PHY;

      //TODO Add speed LS/FS/HS config in structure
      LPC_USB0->OTGSC = OTGSC_VD | OTGSC_OT;
      break;

    case 1:
      sysClockEnable(CLK_M4_USB1);
      sysClockEnable(CLK_USB1);
      sysResetEnable(RST_USB1);

      device->irq = USB1_IRQ;
      device->reg = LPC_USB1;
      device->queueHeads = usb1QueueHeads;

      LPC_USB1->PORTSC1_D |= PORTSC1_D_PFSC;
      break;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void devDeinit(void *object)
{
  struct UsbBase * const device = object;

  switch (device->channel)
  {
    case 0:
      LPC_CREG->CREG0 |= CREG0_USB0PHY;

      sysClockDisable(CLK_USB0);
      sysClockDisable(CLK_M4_USB0);
      break;

    case 1:
      //TODO
      sysClockDisable(CLK_USB1);
      sysClockDisable(CLK_M4_USB1);
      break;
  }

  setDescriptor(device->channel, device, 0);
}
