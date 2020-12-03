/*
 * usb_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/lpc17xx/system.h>
#include <halm/platform/lpc/lpc17xx/usb_base.h>
#include <halm/platform/lpc/lpc17xx/usb_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void configPins(struct UsbBase *, const struct UsbBaseConfig *);
static bool setInstance(struct UsbBase *);
/*----------------------------------------------------------------------------*/
static enum Result devInit(void *, const void *);

#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *);
#else
#define devDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const UsbBase = &(const struct EntityClass){
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
static struct UsbBase *instance = 0;
/*----------------------------------------------------------------------------*/
static void configPins(struct UsbBase *device,
    const struct UsbBaseConfig *config)
{
  const PinNumber pinArray[] = {
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
static bool setInstance(struct UsbBase *object)
{
  if (!instance)
  {
    instance = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void USB_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
static enum Result devInit(void *object, const void *configBase)
{
  const struct UsbBaseConfig * const config = configBase;
  struct UsbBase * const device = object;

  assert(config->channel == 0);

  if (!setInstance(device))
    return E_BUSY;

  device->reg = LPC_USB;
  device->irq = USB_IRQ;
  device->handler = 0;
  device->channel = config->channel;

  configPins(device, configBase);
  sysPowerEnable(PWR_USB);

  /* Perform platform-specific initialization */
  LPC_USB_Type * const reg = device->reg;

  /* Enable clocks */
  reg->USBClkCtrl = USBClkCtrl_DEV_CLK_ON | USBClkCtrl_AHB_CLK_ON;

  const uint32_t clockStateMask = USBClkSt_DEV_CLK_ON | USBClkSt_AHB_CLK_ON;
  while ((reg->USBClkSt & clockStateMask) != clockStateMask);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *object)
{
  struct UsbBase * const device = object;
  LPC_USB_Type * const reg = device->reg;

  /* Disable clock */
  reg->USBClkCtrl = 0;
  /* Disable power */
  sysPowerDisable(PWR_USB);

  instance = 0;
}
#endif
