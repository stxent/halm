/*
 * sdmmc_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/lpc43xx/clocking.h>
#include <halm/platform/lpc/lpc43xx/system.h>
#include <halm/platform/lpc/sdmmc_base.h>
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#define SDDELAY_SAMPLE(value) BIT_FIELD((value), 0)
#define SDDELAY_DRV(value)    BIT_FIELD((value), 8)
/*----------------------------------------------------------------------------*/
static void configPins(struct SdmmcBase *, const struct SdmmcBaseConfig *);
static bool setInstance(struct SdmmcBase *);
/*----------------------------------------------------------------------------*/
static enum Result sdioInit(void *, const void *);
static void sdioDeinit(void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const SdmmcBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = sdioInit,
    .deinit = sdioDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinEntry sdmmcPins[] = {
    {
        .key = PIN(PORT_1, 3), /* SD_RST */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_1, 4), /* SD_VOLT1 */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_1, 5), /* SD_POW */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_1, 6), /* SD_CMD */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_1, 8), /* SD_VOLT0 */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_1, 9), /* SD_DAT0 */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_1, 10), /* SD_DAT1 */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_1, 11), /* SD_DAT2 */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_1, 12), /* SD_DAT3 */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_1, 13), /* SD_CD */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_C, 0), /* SD_CLK */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_C, 1), /* SD_VOLT0 */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_C, 2), /* SD_RST */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_C, 3), /* SD_VOLT1 */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_C, 4), /* SD_DAT0 */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_C, 5), /* SD_DAT1 */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_C, 6), /* SD_DAT2 */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_C, 7), /* SD_DAT3 */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_C, 8), /* SD_CD */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_C, 9), /* SD_POW */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_C, 10), /* SD_CMD */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_C, 11), /* SD_DAT4 */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_C, 12), /* SD_DAT5 */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_C, 13), /* SD_DAT6 */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_C, 14), /* SD_DAT7 */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_D, 1), /* SD_POW */
        .channel = 0,
        .value = 5
    }, {
        .key = PIN(PORT_D, 15), /* SD_WP */
        .channel = 0,
        .value = 5
    }, {
        .key = PIN(PORT_D, 16), /* SD_VOLT2 */
        .channel = 0,
        .value = 5
    }, {
        .key = PIN(PORT_F, 10), /* SD_WP */
        .channel = 0,
        .value = 6
    }, {
        .key = PIN(PORT_F, 11), /* SD_VOLT2 */
        .channel = 0,
        .value = 6
    }, {
        .key = PIN(PORT_CLK, 0), /* SD_CLK */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_CLK, 2), /* SD_CLK */
        .channel = 0,
        .value = 4
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct SdmmcBase *instance = 0;
/*----------------------------------------------------------------------------*/
static void configPins(struct SdmmcBase *interface,
    const struct SdmmcBaseConfig *config)
{
  const PinNumber pinArray[] = {
      config->clk,
      config->cmd,
      config->dat0,
      config->dat1,
      config->dat2,
      config->dat3
  };
  bool wide = true;

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (!pinArray[index])
    {
      /* First three pins are mandatory */
      assert(index >= 3);

      wide = false;
      continue;
    }

    const struct PinEntry * const pinEntry = pinFind(sdmmcPins,
        pinArray[index], 0);
    assert(pinEntry);

    const struct Pin pin = pinInit(pinArray[index]);

    pinInput(pin);
    pinSetFunction(pin, pinEntry->value);
  }

  interface->wide = wide;
}
/*----------------------------------------------------------------------------*/
static bool setInstance(struct SdmmcBase *object)
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
void SDIO_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
uint32_t sdmmcGetClock(const struct SdmmcBase *interface
    __attribute__((unused)))
{
  /* Clock frequency should not exceed 52 MHz */
  return clockFrequency(SdioClock);
}
/*----------------------------------------------------------------------------*/
static enum Result sdioInit(void *object, const void *configBase)
{
  const struct SdmmcBaseConfig * const config = configBase;
  struct SdmmcBase * const interface = object;

  if (!setInstance(interface))
    return E_BUSY;

  configPins(interface, config);

  sysClockEnable(CLK_M4_SDIO);
  sysClockEnable(CLK_SDIO);
  sysResetEnable(RST_SDIO);

  /* Initialize SD/MMC delay register */
  LPC_SCU->SDDELAY = SDDELAY_SAMPLE(0x08) | SDDELAY_DRV(0x0F);

  interface->handler = 0;
  interface->irq = SDIO_IRQ;
  interface->reg = LPC_SDMMC;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sdioDeinit(void *object __attribute__((unused)))
{
  sysClockDisable(CLK_SDIO);
  sysClockDisable(CLK_M4_SDIO);

  instance = 0;
}
