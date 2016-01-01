/*
 * usb_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <platform/nxp/lpc17xx/system.h>
#include <platform/nxp/lpc17xx/usb_base.h>
#include <platform/nxp/lpc17xx/usb_defs.h>
/*----------------------------------------------------------------------------*/
static enum result configPins(struct UsbBase *, const struct UsbBaseConfig *);
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
        .key = PIN(0, 29), /* USB_D+ */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 30), /* USB_D- */
        .channel = 0,
        .value = 1
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
static enum result configPins(struct UsbBase *device,
    const struct UsbBaseConfig *config)
{
  const pinNumber pinArray[] = {
      config->dm, config->dp, config->connect, config->vbus
  };
  const struct PinEntry *pinEntry;
  struct Pin pin;

  for (uint8_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      pinEntry = pinFind(usbPins, pinArray[index], device->channel);
      if (!pinEntry)
        return E_VALUE;
      pinInput((pin = pinInit(pinArray[index])));
      pinSetFunction(pin, pinEntry->value);
    }
  }

  return E_OK;
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
void USB_ISR(void)
{
  descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
static enum result devInit(void *object, const void *configBase)
{
  const struct UsbBaseConfig * const config = configBase;
  struct UsbBase * const device = object;
  enum result res;

  /* Try to set peripheral descriptor */
  device->channel = config->channel;

  res = setDescriptor(device->channel, 0, device);
  if (res != E_OK)
    return res;

  res = configPins(device, configBase);
  if (res != E_OK)
    return res;

  sysPowerEnable(PWR_USB);

  device->handler = 0;
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
