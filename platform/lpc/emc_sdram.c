/*
 * emc_sdram.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/delay.h>
#include <halm/platform/lpc/emc_base.h>
#include <halm/platform/lpc/emc_defs.h>
#include <halm/platform/lpc/emc_sdram.h>
#include <xcore/accel.h>
#include <xcore/helpers.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void configClockPins(uint8_t, uint8_t, const bool *);
static void configMemoryTimings(const struct EmcSdramConfig *);
static void issueConfigSequence(struct EmcSdram *,
    const struct EmcSdramConfig *);
static inline uint32_t timeToTicks(uint32_t, uint32_t);

static enum Result sdramInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_EMC_NO_DEINIT
static void sdramDeinit(void *);
#else
#  define sdramDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const EmcSdram = &(const struct EntityClass){
    .size = sizeof(struct EmcSdram),
    .init = sdramInit,
    .deinit = sdramDeinit
};
/*----------------------------------------------------------------------------*/
extern const struct PinGroupEntry emcAddressPins[];
extern const PinNumber emcAddressPinMap[];
extern const struct PinGroupEntry emcControlPins[];
extern const struct EmcPinDescription emcControlPinMap;
extern const struct PinGroupEntry emcDataPins[];
extern const PinNumber emcDataPinMap[];
/*----------------------------------------------------------------------------*/
static void configClockPins(uint8_t device, uint8_t total, const bool *clocks)
{
  /*
   * Clock and Clock Feedback mapping:
   *   4x8:     CLK3     CLK2     CLK1  CLK0
   *  2x16:    CLK23    CLK23    CLK01 CLK01
   *  1x32: CLK23-FB CLK23-FB    CLK01 CLK01
   *   2x8:  CLK3-FB     CLK2  CLK1-FB  CLK0
   *   2x8:    CLK23    CLK23    CLK01 CLK01
   *  1x16: CLK23-FB CLK23-FB    CLK01 CLK01
   */

  /*
   * Possible configurations:
   *   CLK0: x8 (CLK1 used)
   *   CLK01: x8 (CLK1 unused), x16, x32
   *   CLK1: 4x8
   *   CLK1-FB: 2x8
   *   CLK2: x8 (CLK3 used)
   *   CLK23: x8 (CLK3 unused), 2x16
   *   CLK23-FB: x32, 1x16
   *   CLK3: 4x8
   *   CLK3-FB: 2x8
   */

  const struct PinGroupEntry *group;
  struct Pin pin;

  /* Clock 0 and Clock 01 */
  if (clocks[0])
  {
    if (device == 8 && clocks[1])
    {
      group = pinGroupFind(emcControlPins, emcControlPinMap.clk[0],
          EMC_PIN_CHANNEL_DEFAULT);
    }
    else
    {
      group = pinGroupFind(emcControlPins, emcControlPinMap.clk[0],
          EMC_PIN_CHANNEL_FEEDBACK);
    }
    pinOutput((pin = pinInit(emcControlPinMap.clk[0])), false);

    assert(group);
    pinSetFunction(pin, group->value);
  }

  /* Clock 1 */
  if (clocks[1])
  {
    if (total == 32)
    {
      group = pinGroupFind(emcControlPins, emcControlPinMap.clk[1],
          EMC_PIN_CHANNEL_DEFAULT);
      pinOutput((pin = pinInit(emcControlPinMap.clk[1])), false);
    }
    else
    {
      group = pinGroupFind(emcControlPins, emcControlPinMap.clk[1],
          EMC_PIN_CHANNEL_FEEDBACK);
      pinInput((pin = pinInit(emcControlPinMap.clk[1])));
    }

    assert(group);
    pinSetFunction(pin, group->value);
  }

  /* Clock 2 and Clock 23 */
  if (clocks[2])
  {
    if (device == 8 && clocks[3])
    {
      group = pinGroupFind(emcControlPins, emcControlPinMap.clk[2],
          EMC_PIN_CHANNEL_DEFAULT);
      pinOutput((pin = pinInit(emcControlPinMap.clk[2])), false);
    }
    else
    {
      group = pinGroupFind(emcControlPins, emcControlPinMap.clk[2],
          EMC_PIN_CHANNEL_FEEDBACK);

      if (total / device == 2)
        pinOutput((pin = pinInit(emcControlPinMap.clk[2])), false);
      else
        pinInput((pin = pinInit(emcControlPinMap.clk[2])));
    }

    assert(group);
    pinSetFunction(pin, group->value);
  }

  /* Clock 3 */
  if (clocks[3])
  {
    if (total == 32)
    {
      group = pinGroupFind(emcControlPins, emcControlPinMap.clk[3],
          EMC_PIN_CHANNEL_DEFAULT);
      pinOutput((pin = pinInit(emcControlPinMap.clk[3])), false);
    }
    else
    {
      group = pinGroupFind(emcControlPins, emcControlPinMap.clk[3],
          EMC_PIN_CHANNEL_FEEDBACK);
      pinInput((pin = pinInit(emcControlPinMap.clk[3])));
    }

    assert(group);
    pinSetFunction(pin, group->value);
  }
}
/*----------------------------------------------------------------------------*/
static void configMemoryTimings(const struct EmcSdramConfig *config)
{
  const uint32_t frequency = emcGetClock() / 10;
  const uint32_t cycle = 1000000000UL / frequency;
  uint32_t ticks;

  /* Configure timings, results are in EMC_CCLK ticks */

  ticks = timeToTicks(MAX(config->timings.rp, 1), cycle) - 1;
  assert(ticks <= DYNAMICRP_MAX);
  LPC_EMC->DYNAMICRP = ticks;

  ticks = timeToTicks(MAX(config->timings.ras, 1), cycle) - 1;
  assert(ticks <= DYNAMICRAS_MAX);
  LPC_EMC->DYNAMICRAS = ticks;

  ticks = timeToTicks(MAX(config->timings.xsr, 1), cycle) - 1;
  assert(ticks <= DYNAMICSREX_MAX && ticks <= DYNAMICXSR_MAX);
  LPC_EMC->DYNAMICSREX = ticks;
  LPC_EMC->DYNAMICXSR = ticks;

  ticks = timeToTicks(MAX(config->timings.apr, 1), cycle) - 1;
  assert(ticks <= DYNAMICAPR_MAX);
  LPC_EMC->DYNAMICAPR = ticks;

  ticks = timeToTicks(config->timings.wr + config->timings.rp, cycle);
  assert(ticks <= DYNAMICDAL_MAX);
  LPC_EMC->DYNAMICDAL = ticks;

  ticks = timeToTicks(MAX(config->timings.wr, 1), cycle) - 1;
  assert(ticks <= DYNAMICWR_MAX);
  LPC_EMC->DYNAMICWR = ticks;

  ticks = timeToTicks(MAX(config->timings.rc, 1), cycle) - 1;
  assert(ticks <= DYNAMICRC_MAX && ticks <= DYNAMICRFC_MAX);
  LPC_EMC->DYNAMICRC = ticks;
  LPC_EMC->DYNAMICRFC = ticks;

  ticks = timeToTicks(MAX(config->timings.rrd, 1), cycle) - 1;
  assert(ticks <= DYNAMICRRD_MAX);
  LPC_EMC->DYNAMICRRD = ticks;

  ticks = timeToTicks(MAX(config->timings.mrd, 1), cycle) - 1;
  assert(ticks <= DYNAMICMRD_MAX);
  LPC_EMC->DYNAMICMRD = ticks;
}
/*----------------------------------------------------------------------------*/
static void issueConfigSequence(struct EmcSdram *memory,
    const struct EmcSdramConfig *config)
{
  const uint32_t frequency = emcGetClock() / 10;
  const uint32_t cycle = 1000000000UL / frequency;
  const uint32_t offset = config->columns + (config->banks >> 1)
      + (config->width.bus >> 4);

  uint32_t mode = SDRAM_MODE_LATENCY(config->latency)
      | SDRAM_MODE_OPERATION(MODE_OPERATION_STANDARD)
      | SDRAM_MODE_TYPE_SEQUENTIAL | SDRAM_MODE_WRITE_DEFAULT;
  uint32_t ticks;

  if (config->width.device == 32)
    mode |= SDRAM_MODE_BURST_LENGTH(MODE_BURST_4);
  else
    mode |= SDRAM_MODE_BURST_LENGTH(MODE_BURST_8);

  /* Issue NOP command */
  LPC_EMC->DYNAMICCONTROL = DYNAMICCONTROL_CE | DYNAMICCONTROL_CS
      | DYNAMICCONTROL_I(SDRAM_INITIALIZATION_NOP);
  udelay(200);

  /* Issue Precharge All command */
  LPC_EMC->DYNAMICCONTROL = DYNAMICCONTROL_CE | DYNAMICCONTROL_CS
      | DYNAMICCONTROL_I(SDRAM_INITIALIZATION_PALL);
  udelay((config->timings.rp + 999) / 1000);

  /* Issue NOP command */
  LPC_EMC->DYNAMICCONTROL = DYNAMICCONTROL_CE | DYNAMICCONTROL_CS
      | DYNAMICCONTROL_I(SDRAM_INITIALIZATION_NOP);
  udelay(200);

  /* Configure refresh period */
  ticks = (timeToTicks(config->timings.refresh, cycle) + 15) / 16;
  assert(ticks <= DYNAMICREFRESH_MAX);
  LPC_EMC->DYNAMICREFRESH = ticks;

  /* Issue MODE command */
  LPC_EMC->DYNAMICCONTROL = DYNAMICCONTROL_CE | DYNAMICCONTROL_CS
      | DYNAMICCONTROL_I(SDRAM_INITIALIZATION_MODE);

  /* Write MODE register */
  (void)*((volatile uint32_t *)((uintptr_t)memory->address | (mode << offset)));
  udelay((config->timings.mrd + 999) / 1000);

  /* Issue Normal Operation command */
  LPC_EMC->DYNAMICCONTROL = DYNAMICCONTROL_CE
      | DYNAMICCONTROL_I(SDRAM_INITIALIZATION_NORMAL);
}
/*----------------------------------------------------------------------------*/
static inline uint32_t timeToTicks(uint32_t time, uint32_t cycle)
{
  /* Calculations are in 100 ps resolution */
  return (time * 10 + (cycle - 1)) / cycle;
}
/*----------------------------------------------------------------------------*/
static enum Result sdramInit(void *object, const void *configBase)
{
  const struct EmcSdramConfig * const config = configBase;
  assert(config != NULL);
  assert(config->latency >= 1 && config->latency <= 3);
  assert(config->banks == 2 || config->banks == 4);
  assert(config->columns >= 8 && config->columns <= 11);
  assert(config->rows >= 11 && config->rows <= 13);
  assert(config->width.bus == 16 || config->width.bus == 32);
  assert(config->width.device == 8 || config->width.device == 16
      || config->width.device == 32);

  struct EmcSdram * const memory = object;
  uint8_t channel = config->channel;

  /* Try to register module */
  if (!emcSetDynamicMemoryDescriptor(channel, NULL, object))
    return E_BUSY;

  memory->channel = channel;
  memory->address = emcGetDynamicMemoryAddress(channel);

  const struct PinGroupEntry *group;
  struct Pin pin;

  /* Clock and Clock Feedback pins */
  configClockPins(config->width.device, config->width.bus, config->clocks);

  /* Chip Select pin */
  group = pinGroupFind(emcControlPins, emcControlPinMap.dycs[channel],
      EMC_PIN_CHANNEL_DEFAULT);
  assert(group != NULL);
  pinOutput((pin = pinInit(emcControlPinMap.dycs[channel])), true);
  pinSetFunction(pin, group->value);

  /* Address bus */
  for (size_t index = 0; index < 12; ++index)
  {
    group = pinGroupFind(emcAddressPins, emcAddressPinMap[index],
        EMC_PIN_CHANNEL_DEFAULT);
    assert(group != NULL);
    pinOutput((pin = pinInit(emcAddressPinMap[index])), false);
    pinSetFunction(pin, group->value);
  }

  /* Bank Select pins */
  if (config->banks >= 2)
  {
    group = pinGroupFind(emcAddressPins, emcAddressPinMap[13],
        EMC_PIN_CHANNEL_DEFAULT);
    assert(group);
    pinOutput((pin = pinInit(emcAddressPinMap[13])), false);
    pinSetFunction(pin, group->value);
  }
  if (config->banks == 4)
  {
    group = pinGroupFind(emcAddressPins, emcAddressPinMap[14],
        EMC_PIN_CHANNEL_DEFAULT);
    assert(group);
    pinOutput((pin = pinInit(emcAddressPinMap[14])), false);
    pinSetFunction(pin, group->value);
  }

  /* Data bus */
  for (size_t index = 0; index < config->width.bus; ++index)
  {
    group = pinGroupFind(emcDataPins, emcDataPinMap[index],
        EMC_PIN_CHANNEL_DEFAULT);
    assert(group != NULL);
    pinInput((pin = pinInit(emcDataPinMap[index])));
    pinSetFunction(pin, group->value);
  }

  /* Data Mask Output pins */
  for (size_t index = 0; index < (config->width.bus >> 3); ++index)
  {
    group = pinGroupFind(emcControlPins, emcControlPinMap.dqmout[index],
        EMC_PIN_CHANNEL_DEFAULT);
    assert(group);
    pinInput((pin = pinInit(emcControlPinMap.dqmout[index])));
    pinSetFunction(pin, group->value);
  }

  /* Clock Enable pin */
  group = pinGroupFind(emcControlPins, emcControlPinMap.ckeout[channel],
      EMC_PIN_CHANNEL_DEFAULT);
  assert(group != NULL);
  pinOutput((pin = pinInit(emcControlPinMap.ckeout[channel])), false);
  pinSetFunction(pin, group->value);

  /* CAS pin */
  group = pinGroupFind(emcControlPins, emcControlPinMap.cas,
      EMC_PIN_CHANNEL_DEFAULT);
  assert(group != NULL);
  pinOutput((pin = pinInit(emcControlPinMap.cas)), true);
  pinSetFunction(pin, group->value);

  /* RAS pin */
  group = pinGroupFind(emcControlPins, emcControlPinMap.ras,
      EMC_PIN_CHANNEL_DEFAULT);
  assert(group != NULL);
  pinOutput((pin = pinInit(emcControlPinMap.ras)), true);
  pinSetFunction(pin, group->value);

  /* Write Enable pin */
  group = pinGroupFind(emcControlPins, emcControlPinMap.we,
      EMC_PIN_CHANNEL_DEFAULT);
  assert(group != NULL);
  pinOutput((pin = pinInit(emcControlPinMap.we)), true);
  pinSetFunction(pin, group->value);

  /* Configure address mapping */

  const uint32_t width = config->width.device >> 3;
  const uint32_t size = (1UL << (config->rows + config->columns))
      * config->banks * width;
  uint32_t sizeMapping = 9 - countLeadingZeros32(size);
  uint32_t widthMapping = width >> 1;

  if (sizeMapping > 7)
    sizeMapping = 0;
  if (config->width.device == 32 && config->columns > 8)
  {
    /* Handle special cases: 8Mx32, 16Mx32, 32Mx32 */
    --sizeMapping;
    --widthMapping;
  }

  uint32_t dynamicMemoryConfig = DYNAMICCONFIG_AM0(widthMapping)
      | DYNAMICCONFIG_AM0(sizeMapping << 2);

  if (config->width.bus == 32)
    dynamicMemoryConfig |= DYNAMICCONFIG_AM1;

  LPC_EMC->DYNAMICREADCONFIG = DYNAMICREADCONFIG_RD(DATA_STRATEGY_0_5_CLOCKS);
  LPC_EMC->DYNAMIC[channel].CONFIG = dynamicMemoryConfig;
  LPC_EMC->DYNAMIC[channel].RASCAS = DYNAMICRASCAS_RAS(config->latency)
      | DYNAMICRASCAS_CAS(config->latency);

  /* Configure memory timings */
  emcSetClockDelay(config->timings.delay);
  configMemoryTimings(config);

  /* Send initialization commands */
  issueConfigSequence(memory, config);

  /* Enable internal data buffers */
  LPC_EMC->DYNAMIC[channel].CONFIG |= DYNAMICCONFIG_B;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_EMC_NO_DEINIT
static void sdramDeinit(void *object)
{
  struct EmcSdram * const memory = object;
  emcSetDynamicMemoryDescriptor(memory->channel, object, NULL);
}
#endif
