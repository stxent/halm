/*
 * usb_base.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/lpc13uxx/usb_base.h>
#include <halm/platform/lpc/lpc13uxx/usb_defs.h>
#include <halm/platform/lpc/system.h>
#include <xcore/memory.h>
#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
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
        .key = PIN(0, 1), /* USB_FTOGGLE */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(0, 3), /* USB_VBUS */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 6), /* USB_CONNECT */
        .channel = 0,
        .value = 1
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
      config->dm, config->dp, config->connect, config->vbus
  };

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      const struct PinEntry * const pinEntry = pinFind(usbPins, pinArray[index],
          config->channel);
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

  configPins(config);

  sysPowerEnable(PWR_USBPAD);
  sysClockEnable(CLK_USB);
  sysClockEnable(CLK_USBSRAM);

  device->endpointList = (struct EpListEntry *)LPC_USB_SRAM;
  device->channel = 0;
  device->handler = NULL;
  device->irq = USB_IRQ;
  device->reg = LPC_USB;

  /* Perform platform-specific initialization */
  LPC_USB_Type * const reg = device->reg;

  reg->EPLISTSTART = EPLISTSTART_EP_LIST((uint32_t)device->endpointList);
  reg->DATABUFSTART = DATABUFSTART_DA_BUF((uint32_t)device->endpointList
      + epcsAlignSize(USB_EP_LIST_SIZE));

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit([[maybe_unused]] void *object)
{
  sysClockDisable(CLK_USBSRAM);
  sysClockDisable(CLK_USB);
  sysPowerDisable(PWR_USBPAD);

  instance = NULL;
}
#endif
