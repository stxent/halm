/*
 * usb_base.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <xcore/memory.h>
#include <halm/platform/nxp/lpc43xx/system.h>
#include <halm/platform/nxp/lpc43xx/system_defs.h>
#include <halm/platform/nxp/lpc43xx/usb_base.h>
#include <halm/platform/nxp/lpc43xx/usb_defs.h>
/*----------------------------------------------------------------------------*/
#define ENDPOINT_REQUESTS     CONFIG_USB_DEVICE_ENDPOINT_REQUESTS
#define USB0_ENDPOINT_NUMBER  12
#define USB1_ENDPOINT_NUMBER  8
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
const struct PinEntry usbPins[] = {
    {
        .key = PIN(PORT_1, 5), /* USB0_PWR_FAULT */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_1, 7), /* USB0_PPWR */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_2, 0), /* USB0_PPWR */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_2, 1), /* USB0_PWR_FAULT */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_2, 3), /* USB0_PPWR */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_2, 4), /* USB0_PWR_FAULT */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_6, 3), /* USB0_PPWR */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(PORT_6, 6), /* USB0_PWR_FAULT */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_8, 0), /* USB0_PWR_FAULT */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(PORT_USB, PIN_USB0_DM), /* USB0_DM */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_USB, PIN_USB0_DP), /* USB0_DP */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_USB, PIN_USB0_ID), /* USB0_ID */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_USB, PIN_USB0_VBUS), /* USB0_VBUS */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_2, 5), /* USB1_VBUS1 */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(PORT_9, 5), /* USB1_PPWR */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(PORT_9, 6), /* USB1_PWR_FAULT */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(PORT_USB, PIN_USB1_DM), /* USB1_DM */
        .channel = 1,
        .value = 0
    }, {
        .key = PIN(PORT_USB, PIN_USB1_DP), /* USB1_DP */
        .channel = 1,
        .value = 0
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const UsbBase = &devTable;
static struct UsbBase *descriptors[2] = {0};

#ifdef CONFIG_PLATFORM_USB_0
static struct QueueHead usb0QueueHeads[USB0_ENDPOINT_NUMBER]
    __attribute__((aligned(2048)));
static struct TransferDescriptor usb0TransferDescriptors[ENDPOINT_REQUESTS]
    __attribute__((aligned(32)));
#endif

#ifdef CONFIG_PLATFORM_USB_1
static struct QueueHead usb1QueueHeads[USB1_ENDPOINT_NUMBER]
    __attribute__((aligned(2048)));
static struct TransferDescriptor usb1TransferDescriptors[ENDPOINT_REQUESTS]
    __attribute__((aligned(32)));
#endif
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
  device->queueHeads = 0;

  /* Try to set peripheral descriptor */
  if ((res = setDescriptor(device->channel, 0, device)) != E_OK)
    return res;

  res = queueInit(&device->descriptorPool,
      sizeof(struct TransferDescriptor *), ENDPOINT_REQUESTS);
  if (res != E_OK)
    return res;

  configPins(device, configBase);

  struct TransferDescriptor *poolMemory = 0;

  if (!device->channel)
  {
    sysClockEnable(CLK_M4_USB0);
    sysClockEnable(CLK_USB0);
    sysResetEnable(RST_USB0);

    device->irq = USB0_IRQ;
    device->reg = LPC_USB0;
    device->numberOfEndpoints = USB0_ENDPOINT_NUMBER;

    LPC_CREG->CREG0 &= ~CREG0_USB0PHY;
    LPC_USB0->OTGSC = OTGSC_VD | OTGSC_OT;

#ifdef CONFIG_PLATFORM_USB_0
    device->queueHeads = usb0QueueHeads;
    poolMemory = usb0TransferDescriptors;
#endif
  }
  else
  {
    sysClockEnable(CLK_M4_USB1);
    sysClockEnable(CLK_USB1);
    sysResetEnable(RST_USB1);

    device->irq = USB1_IRQ;
    device->reg = LPC_USB1;
    device->numberOfEndpoints = USB1_ENDPOINT_NUMBER;

    LPC_SCU->SFSUSB = SFSUSB_ESEA | SFSUSB_EPWR | SFSUSB_VBUS;

#ifdef CONFIG_PLATFORM_USB_1
    device->queueHeads = usb1QueueHeads;
    poolMemory = usb1TransferDescriptors;
#endif
  }

  assert(poolMemory);

  for (unsigned int index = 0; index < ENDPOINT_REQUESTS; ++index)
  {
    struct TransferDescriptor * const entry = poolMemory + index;
    queuePush(&device->descriptorPool, &entry);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void devDeinit(void *object)
{
  struct UsbBase * const device = object;

  if (!device->channel)
  {
    LPC_CREG->CREG0 |= CREG0_USB0PHY;

    sysClockDisable(CLK_USB0);
    sysClockDisable(CLK_M4_USB0);
  }
  else
  {
    LPC_SCU->SFSUSB &= ~(SFSUSB_EPWR | SFSUSB_VBUS);

    sysClockDisable(CLK_USB1);
    sysClockDisable(CLK_M4_USB1);
  }

  setDescriptor(device->channel, device, 0);
}
