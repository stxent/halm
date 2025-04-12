/*
 * usb_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/gen_1/usb_defs.h>
#include <halm/platform/stm32/system.h>
#include <halm/platform/stm32/usb_base.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void configPins(struct UsbBase *, const struct UsbBaseConfig *);
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
        .key = PIN(PORT_A, 11), /* USB_DM */
        .channel = 0,
        .value = PIN_ANALOG
    }, {
        .key = PIN(PORT_A, 12), /* USB_DP */
        .channel = 0,
        .value = PIN_ANALOG
    }, {
        .key = PIN(PORT_A, 13), /* USB_NOE */
        .channel = 0,
        .value = 2
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct UsbBase *instance = NULL;
/*----------------------------------------------------------------------------*/
static void configPins(struct UsbBase *device,
    const struct UsbBaseConfig *config)
{
  const struct PinEntry *pinEntry;
  struct Pin pin;

  /* DM */
  pinEntry = pinFind(usbPins, config->dm, config->channel);
  assert(pinEntry != NULL);

  pin = pinInit(config->dm);
  pinInput(pin);
  pinSetFunction(pin, pinEntry->value);

  /* DP */
  pinEntry = pinFind(usbPins, config->dp, config->channel);
  assert(pinEntry != NULL);

  pin = pinInit(config->dp);
  pinInput(pin);
  pinSetFunction(pin, pinEntry->value);

  /* Soft Connect */
  device->connect = pinInit(config->connect);
  if (pinValid(device->connect))
    pinOutput(device->connect, true);
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
void usbSoftConnectionControl(struct UsbBase *device, bool state)
{
  STM_USB_Type * const reg = device->reg;

  if (state)
    reg->BCDR |= BCDR_DPPU;
  else
    reg->BCDR &= ~BCDR_DPPU;
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

  configPins(device, config);

  /* Enable clocks to the register interface and peripheral */
  sysClockEnable(CLK_USB);
  /* Reset registers to default values */
  sysResetPulse(RST_USB);

  device->channel = 0;
  device->handler = NULL;
  device->irq = USB_IRQ;
  device->reg = STM_USB;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *)
{
  sysClockDisable(CLK_USB);
  instance = NULL;
}
#endif
