/*
 * usb_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/memory.h>
#include <halm/platform/nxp/lpc17xx/system.h>
#include <halm/platform/nxp/lpc17xx/usb_base.h>
#include <halm/platform/nxp/lpc17xx/usb_defs.h>
/*----------------------------------------------------------------------------*/
static void configPins(struct UsbBase *, const struct UsbBaseConfig *);
static bool setDescriptor(uint8_t, const struct UsbBase *, struct UsbBase *);
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
        .key = PIN(0, 29), /* USB_D+ */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 30), /* USB_D- */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(1, 19), /* USB_PPWR */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(1, 22), /* USB_PWRD */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(1, 27), /* USB_OVRCR */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(1, 30), /* VBUS */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(2, 9), /* USB_CONNECT */
        .channel = 0,
        .value = 1
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const UsbBase = &devTable;
static struct UsbBase *descriptors[1] = {0};
/*----------------------------------------------------------------------------*/
static void configPins(struct UsbBase *device,
    const struct UsbBaseConfig *config)
{
  const pinNumber pinArray[] = {
      config->dm, config->dp, config->connect, config->vbus
  };

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
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
static bool setDescriptor(uint8_t channel, const struct UsbBase *state,
    struct UsbBase *device)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state,
      device);
}
/*----------------------------------------------------------------------------*/
void USB_ISR(void)
{
  descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
static enum result devInit(void *object, const void *configBase)
{
  const struct UsbBaseConfig * const config = configBase;
  struct UsbBase * const device = object;

  device->channel = config->channel;
  device->handler = 0;

  /* Try to set peripheral descriptor */
  if (!setDescriptor(device->channel, 0, device))
    return E_BUSY;

  configPins(device, configBase);

  sysPowerEnable(PWR_USB);

  device->irq = USB_IRQ;
  device->reg = LPC_USB;

  /* Perform platform-specific initialization */
  LPC_USB_Type * const reg = device->reg;

  /* Enable clocks */
  reg->USBClkCtrl = USBClkCtrl_DEV_CLK_ON | USBClkCtrl_AHB_CLK_ON;

  const uint32_t clockStateMask = USBClkSt_DEV_CLK_ON | USBClkSt_AHB_CLK_ON;
  while ((reg->USBClkSt & clockStateMask) != clockStateMask);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void devDeinit(void *object)
{
  struct UsbBase * const device = object;
  LPC_USB_Type * const reg = device->reg;

  /* Disable clock */
  reg->USBClkCtrl = 0;

  sysPowerDisable(PWR_USB);
  setDescriptor(device->channel, device, 0);
}
