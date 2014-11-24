/*
 * sdmmc_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/nxp/sdmmc_base.h>
#include <platform/nxp/lpc43xx/clocking.h>
#include <platform/nxp/lpc43xx/system.h>
/*----------------------------------------------------------------------------*/
static enum result configPins(struct SdmmcBase *,
    const struct SdmmcBaseConfig *);
static enum result setDescriptor(uint8_t, const struct SdmmcBase *,
    struct SdmmcBase *);
/*----------------------------------------------------------------------------*/
static enum result sdioInit(void *, const void *);
static void sdioDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass sdioTable = {
    .size = sizeof(struct SdmmcBase),
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
        .value = 6
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
        .value = 6
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
        .value = 5
    }, {
        .key = PIN(PORT_F, 11), /* SD_VOLT2 */
        .channel = 0,
        .value = 5
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
const struct EntityClass * const SdmmcBase = &sdioTable;
static struct SdmmcBase *descriptors[1] = {0};
/*----------------------------------------------------------------------------*/
static enum result configPins(struct SdmmcBase *interface,
    const struct SdmmcBaseConfig *config)
{
  const pin_t pinArray[4] = {
      config->clock,
      config->cmd,
      config->dat0,
      config->dat1,
      config->dat2,
      config->dat3
  };
  const struct PinEntry *pinEntry;
  struct Pin pin;

  for (uint8_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      pinEntry = pinFind(sdmmcPins, pinArray[index], 0);
      if (!pinEntry)
        return E_VALUE;
      pinInput((pin = pinInit(pinArray[index])));
      pinSetFunction(pin, pinEntry->value);
    }
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel,
    const struct SdmmcBase *state, struct SdmmcBase *interface)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state,
      interface) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
void SDIO_ISR(void)
{
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
uint32_t sdmmcGetClock(const struct SdmmcBase *interface
    __attribute__((unused)))
{
  return clockFrequency(SdioClock);
}
/*----------------------------------------------------------------------------*/
static enum result sdioInit(void *object, const void *configBase)
{
  const struct SdmmcBaseConfig * const config = configBase;
  struct SdmmcBase * const interface = object;
  enum result res;

  /* Try to set peripheral descriptor */
  if ((res = setDescriptor(0, 0, interface)) != E_OK)
    return res;

  if ((res = sdmmcConfigPins(interface, configBase)) != E_OK)
    return res;

  sysClockEnable(CLK_M4_SDIO);
  sysClockEnable(CLK_SDIO);
  sysResetEnable(RST_SDIO);

  interface->handler = 0;
  interface->irq = SDIO_IRQ;
  interface->reg = LPC_SDMMC;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sdioDeinit(void *object)
{
  struct SdmmcBase * const interface = object;

  sysClockEnable(CLK_SDIO);
  sysClockEnable(CLK_M4_SDIO);

  setDescriptor(0, interface, 0);
}