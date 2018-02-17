/*
 * usb_base.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/memory.h>
#include <halm/platform/stm/stm32f1xx/system.h>
#include <halm/platform/stm/stm32f1xx/usb_base.h>
#include <halm/platform/stm/stm32f1xx/usb_defs.h>
/*----------------------------------------------------------------------------*/
static void configPins(struct UsbBase *, const struct UsbBaseConfig *);
static bool setDescriptor(uint8_t, const struct UsbBase *, struct UsbBase *);
/*----------------------------------------------------------------------------*/
static enum Result devInit(void *, const void *);

#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *);
#else
#define devDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static const struct EntityClass devTable = {
    .size = 0, /* Abstract class */
    .init = devInit,
    .deinit = devDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinEntry usbPins[] = {
    {
        /* Available on STM32F105 and STM32F107 series only */
        .key = PIN(PORT_A, 9), /* OTG_FS_VBUS */
        .channel = 0,
        .value = 0
    }, {
        /* Available on STM32F105 and STM32F107 series only */
        .key = PIN(PORT_A, 10), /* OTG_FS_ID */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_A, 11), /* USBDM or OTG_FS_DM */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_A, 12), /* USBDP or OTG_FS_DP */
        .channel = 0,
        .value = 0
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
  enum
  {
    DM_INDEX,
    DP_INDEX,
    VBUS_INDEX
  };

  const PinNumber pinArray[] = {
      [DM_INDEX] = config->dm,
      [DP_INDEX] = config->dp,
      [VBUS_INDEX] = config->vbus
  };

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (index != VBUS_INDEX || pinArray[index])
    {
      const struct PinEntry * const pinEntry = pinFind(usbPins,
          pinArray[index], device->channel);
      assert(pinEntry);

      const struct Pin pin = pinInit(pinArray[index]);

      pinInput(pin);
      pinSetFunction(pin, pinEntry->value);
    }
  }

  device->connect = pinInit(config->connect);
  if (pinValid(device->connect))
    pinOutput(device->connect, true);
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
void usbSoftConnectionControl(struct UsbBase *device, bool state)
{
  if (pinValid(device->connect))
    pinWrite(device->connect, !state);
}
/*----------------------------------------------------------------------------*/
void USB_HP_ISR(void)
{
}
/*----------------------------------------------------------------------------*/
void USB_LP_ISR(void)
{
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void USB_WAKEUP_ISR(void)
{
}
/*----------------------------------------------------------------------------*/
static enum Result devInit(void *object, const void *configBase)
{
  const struct UsbBaseConfig * const config = configBase;
  struct UsbBase * const device = object;

  device->channel = config->channel;
  device->handler = 0;

  /* Try to set peripheral descriptor */
  if (!setDescriptor(device->channel, 0, device))
    return E_BUSY;

  configPins(device, configBase);

  /* Enable clocks to the register interface and peripheral */
  sysClockEnable(CLK_USB);
  /* Reset registers to default values */
  sysResetEnable(RST_USB);
  sysResetDisable(RST_USB);

  device->irq = USB_LP_CAN1_RX0_IRQ;
  device->reg = STM_USB;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *object)
{
  struct UsbBase * const device = object;

  sysClockDisable(CLK_USB);
  setDescriptor(device->channel, device, 0);
}
#endif
