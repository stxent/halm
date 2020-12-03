/*
 * emc_sram.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/lpc/emc_base.h>
#include <halm/platform/lpc/emc_defs.h>
#include <halm/platform/lpc/emc_sram.h>
#include <xcore/helpers.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static enum Result sramInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_EMC_NO_DEINIT
static void sramDeinit(void *);
#else
#define sramDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const EmcSram = &(const struct EntityClass){
    .size = sizeof(struct EmcSram),
    .init = sramInit,
    .deinit = sramDeinit
};
/*----------------------------------------------------------------------------*/
extern const struct PinGroupEntry emcAddressPins[];
extern const PinNumber emcAddressPinMap[];
extern const struct PinGroupEntry emcControlPins[];
extern const struct EmcPinDescription emcControlPinMap;
extern const struct PinGroupEntry emcDataPins[];
extern const PinNumber emcDataPinMap[];
/*----------------------------------------------------------------------------*/
static enum Result sramInit(void *object, const void *configBase)
{
  const struct EmcSramConfig * const config = configBase;
  assert(config);

  struct EmcSram * const memory = object;

  /* Try to register module */
  if (!emcSetStaticMemoryDescriptor(config->channel, 0, object))
    return E_BUSY;

  const size_t byteLanes = config->dataWidth >> 3;

  assert(!(config->dataWidth & 0x7));
  assert(byteLanes == 1 || byteLanes == 2 || byteLanes == 4);
  assert(config->timings.rc >= config->timings.oe);
  assert(config->timings.wc >= config->timings.we);

  memory->channel = config->channel;
  memory->address = emcGetStaticMemoryAddress(memory->channel);

  const struct PinGroupEntry *group;
  struct Pin pin;

  /* Address bus */
  for (size_t index = 0; index < config->addressWidth; ++index)
  {
    group = pinGroupFind(emcAddressPins, emcAddressPinMap[index], 0);
    assert(group);
    pinInput((pin = pinInit(emcAddressPinMap[index])));
    pinSetFunction(pin, group->value);
  }

  /* Data bus */
  for (size_t index = 0; index < config->dataWidth; ++index)
  {
    group = pinGroupFind(emcDataPins, emcDataPinMap[index], 0);
    assert(group);
    pinInput((pin = pinInit(emcDataPinMap[index])));
    pinSetFunction(pin, group->value);
  }

  /* Output Enable pin */
  group = pinGroupFind(emcControlPins, emcControlPinMap.oe, 0);
  assert(group);
  pinInput((pin = pinInit(emcControlPinMap.oe)));
  pinSetFunction(pin, group->value);

  /* Byte Lane Select pins */
  for (size_t index = 0; index < byteLanes; ++index)
  {
    group = pinGroupFind(emcControlPins, emcControlPinMap.bls[index], 0);
    assert(group);
    pinInput((pin = pinInit(emcControlPinMap.bls[index])));
    pinSetFunction(pin, group->value);
  }

  /* Write Enable pin */
  if (config->partitioned)
  {
    group = pinGroupFind(emcControlPins, emcControlPinMap.we, 0);
    assert(group);
    pinInput((pin = pinInit(emcControlPinMap.we)));
    pinSetFunction(pin, group->value);
  }

  /* Chip Select pin */
  group = pinGroupFind(emcControlPins, emcControlPinMap.cs[memory->channel], 0);
  assert(group);
  pinInput((pin = pinInit(emcControlPinMap.cs[config->channel])));
  pinSetFunction(pin, group->value);

  uint32_t configValue = STATICCONFIG_B;

  if (config->partitioned)
    configValue |= STATICCONFIG_PB;

  switch (byteLanes)
  {
    case 1:
      configValue |= STATICCONFIG_MW(MW_8_BIT);
      break;

    case 2:
      configValue |= STATICCONFIG_MW(MW_16_BIT);
      break;

    case 4:
      configValue |= STATICCONFIG_MW(MW_32_BIT);
      break;
  }

  const uint32_t frequency = emcGetClock() / 10;
  const uint32_t cycleTime = 1000000000UL / frequency;

  /* Calculations are in 100 ps resolution */
  const uint32_t oe = 10 * config->timings.oe;
  const uint32_t rc = 10 * config->timings.rc;
  const uint32_t wc = 10 * config->timings.wc;
  const uint32_t we = 10 * config->timings.we;

  const uint32_t aaTime = MAX(rc, cycleTime);
  const uint32_t weTime = MAX(we, cycleTime);
  const uint32_t weDelay = MAX(wc, 3 * cycleTime) - weTime - cycleTime;

  /* Results are in HCLK ticks */
  const uint32_t oeTicks = (oe + (cycleTime - 1)) / cycleTime;
  assert(oeTicks < 16);
  const uint32_t rdTicks = (aaTime + (cycleTime - 1)) / cycleTime - 1;
  assert(rdTicks < 32);
  const uint32_t weTicks = (weDelay + (cycleTime - 1)) / cycleTime - 1;
  assert(weTicks < 16);
  const uint32_t wrTicks = (weDelay + weTime + (cycleTime - 1)) / cycleTime - 2;
  assert(wrTicks < 32);

  LPC_EMC->STATIC[memory->channel].CONFIG = configValue;
  LPC_EMC->STATIC[memory->channel].WAITOEN = oeTicks;
  LPC_EMC->STATIC[memory->channel].WAITRD = rdTicks;
  LPC_EMC->STATIC[memory->channel].WAITWEN = weTicks;
  LPC_EMC->STATIC[memory->channel].WAITWR = wrTicks;
  LPC_EMC->STATIC[memory->channel].WAITTURN = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_EMC_NO_DEINIT
static void sramDeinit(void *object)
{
  struct EmcSram * const memory = object;

  emcSetStaticMemoryDescriptor(memory->channel, object, 0);
}
#endif
