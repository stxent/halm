/*
 * emc_sram.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
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
#  define sramDeinit deletedDestructorTrap
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
  assert(config != NULL);
  assert(config->timings.rc >= config->timings.oe);
  assert(config->timings.wc >= config->timings.we);
  assert(config->width.data == 8 || config->width.data == 16
      || config->width.data == 32);

  struct EmcSram * const memory = object;
  uint8_t channel = config->channel;

  /* Try to register module */
  if (!emcSetStaticMemoryDescriptor(channel, NULL, object))
    return E_BUSY;

  memory->channel = channel;
  memory->address = emcGetStaticMemoryAddress(channel);

  const size_t byteLanes = config->width.data >> 3;
  const struct PinGroupEntry *group;
  struct Pin pin;

  /* Address bus */
  for (size_t index = 0; index < config->width.address; ++index)
  {
    group = pinGroupFind(emcAddressPins, emcAddressPinMap[index],
        EMC_PIN_CHANNEL_DEFAULT);
    assert(group != NULL);
    pinOutput((pin = pinInit(emcAddressPinMap[index])), false);
    pinSetFunction(pin, group->value);
  }

  /* Data bus */
  for (size_t index = 0; index < config->width.data; ++index)
  {
    group = pinGroupFind(emcDataPins, emcDataPinMap[index],
        EMC_PIN_CHANNEL_DEFAULT);
    assert(group != NULL);
    pinInput((pin = pinInit(emcDataPinMap[index])));
    pinSetFunction(pin, group->value);
  }

  /* Output Enable pin */
  group = pinGroupFind(emcControlPins, emcControlPinMap.oe,
      EMC_PIN_CHANNEL_DEFAULT);
  assert(group != NULL);
  pinOutput((pin = pinInit(emcControlPinMap.oe)), true);
  pinSetFunction(pin, group->value);

  /* Byte Lane Select pins */
  for (size_t index = 0; index < byteLanes; ++index)
  {
    group = pinGroupFind(emcControlPins, emcControlPinMap.bls[index],
        EMC_PIN_CHANNEL_DEFAULT);
    assert(group);
    pinOutput((pin = pinInit(emcControlPinMap.bls[index])), true);
    pinSetFunction(pin, group->value);
  }

  /* Write Enable pin */
  if (config->partitioned)
  {
    group = pinGroupFind(emcControlPins, emcControlPinMap.we,
        EMC_PIN_CHANNEL_DEFAULT);
    assert(group != NULL);
    pinOutput((pin = pinInit(emcControlPinMap.we)), true);
    pinSetFunction(pin, group->value);
  }

  /* Chip Select pin */
  group = pinGroupFind(emcControlPins, emcControlPinMap.cs[channel],
      EMC_PIN_CHANNEL_DEFAULT);
  assert(group != NULL);
  pinOutput((pin = pinInit(emcControlPinMap.cs[config->channel])), true);
  pinSetFunction(pin, group->value);

  uint32_t staticMemoryConfig = STATICCONFIG_B;

  if (config->partitioned)
    staticMemoryConfig |= STATICCONFIG_PB;

  switch (byteLanes)
  {
    case 1:
      staticMemoryConfig |= STATICCONFIG_MW(MW_8_BIT);
      break;

    case 2:
      staticMemoryConfig |= STATICCONFIG_MW(MW_16_BIT);
      break;

    case 4:
      staticMemoryConfig |= STATICCONFIG_MW(MW_32_BIT);
      break;
  }

  const uint32_t frequency = emcGetClock() / 10;
  const uint32_t cycle = 1000000000UL / frequency;

  /* Calculations are in 100 ps resolution */
  const uint32_t oe = 10 * config->timings.oe;
  const uint32_t rc = 10 * config->timings.rc;
  const uint32_t wc = 10 * config->timings.wc;
  const uint32_t we = 10 * config->timings.we;

  const uint32_t aaTime = MAX(rc, cycle);
  const uint32_t weTime = MAX(we, cycle);
  const uint32_t weDelay = MAX(wc, 3 * cycle) - weTime - cycle;

  /* Results are in HCLK ticks */
  const uint32_t oeTicks = (oe + (cycle - 1)) / cycle;
  assert(oeTicks <= STATICWAITOEN_MAX);
  const uint32_t rdTicks = (aaTime + (cycle - 1)) / cycle - 1;
  assert(rdTicks <= STATICWAITRD_MAX);
  const uint32_t weTicks = (weDelay + (cycle - 1)) / cycle - 1;
  assert(weTicks <= STATICWAITWEN_MAX);
  const uint32_t wrTicks = (weDelay + weTime + (cycle - 1)) / cycle - 2;
  assert(wrTicks <= STATICWAITWR_MAX);

  LPC_EMC->STATIC[channel].CONFIG = staticMemoryConfig;
  LPC_EMC->STATIC[channel].WAITOEN = oeTicks;
  LPC_EMC->STATIC[channel].WAITRD = rdTicks;
  LPC_EMC->STATIC[channel].WAITWEN = weTicks;
  LPC_EMC->STATIC[channel].WAITWR = wrTicks;
  LPC_EMC->STATIC[channel].WAITTURN = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_EMC_NO_DEINIT
static void sramDeinit(void *object)
{
  struct EmcSram * const memory = object;
  emcSetStaticMemoryDescriptor(memory->channel, object, NULL);
}
#endif
