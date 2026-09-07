/*
 * usb_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/system.h>
#include <halm/platform/numicro/usb_base.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void configPins(const struct UsbBaseConfig *);
static bool setInstance(struct UsbBase *);
/*----------------------------------------------------------------------------*/
static enum Result devInit(void *, const void *);

#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *);
#else
#  define devDeinit deletedDestructorTrap
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
        .key = PIN(PORT_USB, PIN_USB_DM), /* Mock USB_D- pin descriptor */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_USB, PIN_USB_DP), /* Mock USB_D+ pin descriptor */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_USB, PIN_USB_VBUS), /* Mock USB_VBUS pin descriptor */
        .channel = 0,
        .value = 0
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct UsbBase *instance = nullptr;
/*----------------------------------------------------------------------------*/
static void configPins(const struct UsbBaseConfig *config)
{
  /* TODO Nuvoton M03x pin configuration */
  (void)config;
}
/*----------------------------------------------------------------------------*/
static bool setInstance(struct UsbBase *object)
{
  if (instance == nullptr)
  {
    instance = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void USBD_ISR(void)
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

  device->reg = NM_USBD;
  device->irq = USBD_IRQ;
  device->handler = nullptr;
  device->channel = 0;

  configPins(config);

  /* Enable clock to peripheral */
  sysClockEnable(CLK_USBD);
  /* Reset registers to default values */
  sysResetBlock(RST_USBD);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *)
{
  sysClockDisable(CLK_USBD);
  instance = nullptr;
}
#endif
