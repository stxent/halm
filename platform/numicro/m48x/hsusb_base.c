/*
 * hsusb_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/delay.h>
#include <halm/platform/numicro/hsusb_base.h>
#include <halm/platform/numicro/m48x/system_defs.h>
#include <halm/platform/numicro/system.h>
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
const struct EntityClass * const HsUsbBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = devInit,
    .deinit = devDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinEntry hsUsbPins[] = {
    {
        .key = PIN(PORT_B, 10), /* HSUSB_VBUS_EN */
        .channel = 0,
        .value = 14
    }, {
        .key = PIN(PORT_B, 11), /* HSUSB_VBUS_ST */
        .channel = 0,
        .value = 14
    }, {
        .key = PIN(PORT_B, 15), /* HSUSB_VBUS_EN */
        .channel = 0,
        .value = 15
    }, {
        .key = PIN(PORT_C, 14), /* HSUSB_VBUS_ST */
        .channel = 0,
        .value = 15
    }, {
        /* Mock High-Speed USB D- pin descriptor */
        .key = PIN(PORT_HSUSB, PIN_HSUSB_DM),
        .channel = 0,
        .value = 0
    }, {
        /* Mock High-Speed USB D+ pin descriptor */
        .key = PIN(PORT_HSUSB, PIN_HSUSB_DP),
        .channel = 0,
        .value = 0
    }, {
        /* Mock High-Speed USB ID pin descriptor */
        .key = PIN(PORT_HSUSB, PIN_HSUSB_ID),
        .channel = 0,
        .value = 0
    }, {
        /* Mock High-Speed USB VBUS pin descriptor */
        .key = PIN(PORT_HSUSB, PIN_HSUSB_VBUS),
        .channel = 0,
        .value = 0
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct UsbBase *instance = NULL;
/*----------------------------------------------------------------------------*/
static void configPins(const struct UsbBaseConfig *config)
{
  const PinNumber pinArray[] = {
      config->dm, config->dp, config->vbus
  };

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      const struct PinEntry * const pinEntry = pinFind(hsUsbPins,
          pinArray[index], config->channel);
      assert(pinEntry != NULL);

      const struct Pin pin = pinInit(pinArray[index]);

      pinInput(pin);
      pinSetFunction(pin, pinEntry->value);
    }
  }
}
/*----------------------------------------------------------------------------*/
static bool setInstance(struct UsbBase *object)
{
  if (instance == NULL)
  {
    instance = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void HSUSBD_ISR(void)
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

  device->reg = NM_HSUSBD;
  device->irq = HSUSBD_IRQ;
  device->handler = NULL;
  device->channel = 0;

  configPins(config);

  /* Enable clock to peripheral */
  sysClockEnable(CLK_HSUSBD);
  /* Reset registers to default values */
  sysResetBlock(RST_HSUSBD);

  sysUnlockReg();
  NM_SYS->USBPHY = (NM_SYS->USBPHY & ~(USBPHY_HSUSBROLE_MASK | USBPHY_HSUSBACT))
      | (USBPHY_HSUSBROLE(USBROLE_DEVICE) | USBPHY_HSUSBEN);
  sysLockReg();

  /* Wait for 10 us or longer */
  udelay(10);

  sysUnlockReg();
  NM_SYS->USBPHY |= USBPHY_HSUSBACT;
  sysLockReg();

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *)
{
  sysClockDisable(CLK_HSUSBD);
  instance = NULL;
}
#endif
