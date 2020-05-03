/*
 * usb_base.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
#include <xcore/memory.h>
#include <halm/platform/nxp/lpc13uxx/system.h>
#include <halm/platform/nxp/lpc13uxx/usb_base.h>
#include <halm/platform/nxp/lpc13uxx/usb_defs.h>
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
        .key = PIN(PORT_USB, PIN_USB_DP), /* Mock USB_D+ pin descriptor */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_USB, PIN_USB_DM), /* Mock USB_D- pin descriptor */
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

  // device->epList = memalign(256, USB_EPLIST_SIZE); // XXX Add similar to 43xx
  device->epList = 0x20004000UL; // XXX
  if (!device->epList)
    return E_MEMORY;
  // memset(device->epList, 0, sizeof(uint32_t) * USB_ENDPOINT_NUMBER * 2);

  device->reg = LPC_USB;
  device->irq = USB_IRQ;
  device->handler = 0;
  device->channel = config->channel;

  configPins(device, configBase);

  sysPowerEnable(PWR_USBPAD);
  sysClockEnable(CLK_USB);
  sysClockEnable(CLK_USBSRAM);

  /* Perform platform-specific initialization */
  LPC_USB_Type * const reg = device->reg;

  reg->EPLISTSTART = (uint32_t)device->epList;
	reg->DATABUFSTART = 0x20004000UL & 0xFFC00000UL;  //(uint32_t) usb_data_buffer) & 0xFFC00000;



  // reg->USBDevFIQSel = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *object)
{
  struct UsbBase * const device = object;

  sysClockDisable(CLK_USBSRAM);
  sysClockDisable(CLK_USB);
  sysPowerDisable(PWR_USBPAD);
  free(device->epList);

  instance = 0;
}
#endif
