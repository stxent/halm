/*
 * usb_base.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/gen_2/usb_defs.h>
#include <halm/platform/stm32/system.h>
#include <halm/platform/stm32/usb_base.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void configPins(const struct UsbBaseConfig *);
static bool setInstance(uint8_t, struct UsbBase *);
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
#ifdef CONFIG_PLATFORM_STM32_USB_FS
    {
        .key = PIN(PORT_A, 8), /* OTG_FS_SOF */
        .channel = 0,
        .value = 10
    }, {
        .key = PIN(PORT_A, 9), /* OTG_FS_VBUS */
        .channel = 0,
        .value = PIN_DEFAULT
    }, {
        .key = PIN(PORT_A, 10), /* OTG_FS_ID */
        .channel = 0,
        .value = 10
    }, {
        .key = PIN(PORT_A, 11), /* OTG_FS_DM */
        .channel = 0,
        .value = 10
    }, {
        .key = PIN(PORT_A, 12), /* OTG_FS_DP */
        .channel = 0,
        .value = 10
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_USB_HS
    {
        .key = PIN(PORT_A, 3), /* OTG_HS_ULPI_D0 */
        .channel = 1,
        .value = 10
    }, {
        .key = PIN(PORT_A, 4), /* OTG_HS_SOF */
        .channel = 1,
        .value = 12
    }, {
        .key = PIN(PORT_A, 5), /* OTG_HS_ULPI_CK */
        .channel = 1,
        .value = 10
    }, {
        .key = PIN(PORT_B, 0), /* OTG_HS_ULPI_D1 */
        .channel = 1,
        .value = 10
    }, {
        .key = PIN(PORT_B, 1), /* OTG_HS_ULPI_D2 */
        .channel = 1,
        .value = 10
    }, {
        .key = PIN(PORT_B, 5), /* OTG_HS_ULPI_D7 */
        .channel = 1,
        .value = 10
    }, {
        .key = PIN(PORT_B, 10), /* OTG_HS_ULPI_D3 */
        .channel = 1,
        .value = 10
    }, {
        .key = PIN(PORT_B, 11), /* OTG_HS_ULPI_D4 */
        .channel = 1,
        .value = 10
    }, {
    //     .key = PIN(PORT_B, 12), /* OTG_HS_ULPI_D5 */ // TODO
    //     .channel = 1,
    //     .value = 10
    // }, {
        .key = PIN(PORT_B, 12), /* OTG_HS_ID */
        .channel = 1,
        .value = 12
    }, {
    //     .key = PIN(PORT_B, 13), /* OTG_HS_ULPI_D6 */ // TODO
    //     .channel = 1,
    //     .value = 10
    // }, {
        .key = PIN(PORT_B, 13), /* OTG_HS_VBUS */
        .channel = 1,
        .value = PIN_DEFAULT
    }, {
        .key = PIN(PORT_B, 14), /* OTG_HS_DM */
        .channel = 1,
        .value = 12
    }, {
        .key = PIN(PORT_B, 15), /* OTG_HS_DP */
        .channel = 1,
        .value = 12
    }, {
        .key = PIN(PORT_C, 0), /* OTG_HS_ULPI_STP */
        .channel = 1,
        .value = 10
    }, {
        .key = PIN(PORT_C, 2), /* OTG_HS_ULPI_DIR */
        .channel = 1,
        .value = 10
    }, {
        .key = PIN(PORT_C, 3), /* OTG_HS_ULPI_NXT */
        .channel = 1,
        .value = 10
    }, {
        .key = PIN(PORT_H, 4), /* OTG_HS_ULPI_NXT */
        .channel = 1,
        .value = 10
    }, {
        .key = PIN(PORT_I, 11), /* OTG_HS_ULPI_DIR */
        .channel = 1,
        .value = 10
    },
#endif
    {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct UsbBase *instances[2] = {NULL};
/*----------------------------------------------------------------------------*/
static void configPins(const struct UsbBaseConfig *config)
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
    if (pinArray[index])
    {
      const struct PinEntry * const pinEntry = pinFind(usbPins,
          pinArray[index], config->channel);
      assert(pinEntry != NULL);

      const struct Pin pin = pinInit(pinArray[index]);

      pinInput(pin);

      if (index != VBUS_INDEX)
        pinSetFunction(pin, pinEntry->value);
    }
  }
}
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct UsbBase *object)
{
  if (instances[channel] == NULL)
  {
    instances[channel] = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_USB_FS
void OTG_FS_WKUP_ISR(void)
{
  instances[0]->handler(instances[0]);
}

void OTG_FS_ISR(void)
{
  instances[0]->handler(instances[0]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_USB_HS
void OTG_HS_EP1_OUT_ISR(void)
{
  instances[1]->handler(instances[1]);
}

void OTG_HS_EP1_IN_ISR(void)
{
  instances[1]->handler(instances[1]);
}

void OTG_HS_WKUP_ISR(void)
{
  instances[1]->handler(instances[1]);
}

void OTG_HS_ISR(void)
{
  instances[1]->handler(instances[1]);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result devInit(void *object, const void *configBase)
{
  const struct UsbBaseConfig * const config = configBase;
  struct UsbBase * const device = object;

  assert(config->channel < 2);
#ifndef CONFIG_PLATFORM_STM32_USB_FS
  assert(config->channel != 0);
#endif
#ifndef CONFIG_PLATFORM_STM32_USB_HS
  assert(config->channel != 1);
#endif

  if (!setInstance(config->channel, device))
    return E_BUSY;

  configPins(config);

  if (config->channel == 0)
  {
    /* Enable clocks to the register interface and peripheral */
    sysClockEnable(CLK_OTGFS);
    /* Reset registers to default values */
    sysResetPulse(RST_OTGFS);

    device->reg = STM_USB_OTG_FS;
    device->handler = NULL;
    device->irq = OTG_FS_IRQ;
    device->memoryCapacity = 1280;
    device->channel = 0;
    device->numberOfEndpoints = 8;
    device->dma = false;
    device->hs = false;

    STM_USB_OTG_FS->GLOBAL.GUSBCFG |= GUSBCFG_PHYSEL;
  }
  else
  {
    /* Enable clocks to the register interface and peripheral */
    sysClockEnable(CLK_OTGHS);
    /* Reset registers to default values */
    sysResetPulse(RST_OTGHS);

    device->reg = STM_USB_OTG_HS;
    device->handler = NULL;
    device->irq = OTG_HS_IRQ;
    device->memoryCapacity = 4096;
    device->channel = 1;
    device->numberOfEndpoints = 12;

#ifdef CONFIG_PLATFORM_USB_DMA
    device->dma = true;
#else
    device->dma = false;
#endif

#ifdef CONFIG_PLATFORM_USB_HS
    // TODO Add ULPI HS mode to upper-level driver
    device->hs = true;
#else
    device->hs = false;
#endif

    if (!device->hs)
    {
      /* ULPI clock should be disabled when internal FS PHY is used */
      sysClockDisable(CLK_OTGHSULPI);
    }
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *object)
{
  struct UsbBase * const device = object;

  if (device->channel == 0)
    sysClockDisable(CLK_OTGFS);
  else
    sysClockDisable(CLK_OTGHS);

  instances[device->channel] = NULL;
}
#endif
