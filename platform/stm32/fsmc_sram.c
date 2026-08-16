/*
 * fsmc_sram.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/fsmc_base.h>
#include <halm/platform/stm32/fsmc_defs.h>
#include <halm/platform/stm32/fsmc_sram.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static inline uint32_t timeToTicks(uint32_t, uint32_t);

static enum Result sramInit(void *, const void *);

#ifndef CONFIG_PLATFORM_STM32_FSMC_NO_DEINIT
static void sramDeinit(void *);
#else
#  define sramDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const FsmcSram = &(const struct EntityClass){
    .size = sizeof(struct FsmcSram),
    .init = sramInit,
    .deinit = sramDeinit
};
/*----------------------------------------------------------------------------*/
extern const struct PinGroupEntry fsmcAddressPins[];
extern const PinNumber fsmcAddressPinMap[];
extern const struct PinGroupEntry fsmcControlPins[];
extern const struct FsmcPinDescription fsmcControlPinMap;
extern const struct PinGroupEntry fsmcDataPins[];
extern const PinNumber fsmcDataPinMap[];
/*----------------------------------------------------------------------------*/
static inline uint32_t timeToTicks(uint32_t time, uint32_t cycle)
{
  /* Calculations are in 100 ps resolution */
  return (time * 10 + (cycle - 1)) / cycle;
}
/*----------------------------------------------------------------------------*/
static enum Result sramInit(void *object, const void *configBase)
{
  const struct FsmcSramConfig * const config = configBase;
  assert(config != NULL);
  assert(config->width.data == 8 || config->width.data == 16);

  struct FsmcSram * const memory = object;
  const uint8_t bank = 0; /* Bank 1 is used for NOR/PSRAM memory */

  /* Try to register module */
  if (!fsmcSetMemoryDescriptor(bank, NULL, object))
    return E_BUSY;

  memory->subbank = config->subbank;
  memory->address = fsmcGetMemoryAddress(bank, memory->subbank);

  const unsigned int byteLanes = config->width.data / 8;
  const struct PinGroupEntry *group;
  struct Pin pin;

  /* Address bus */
  for (size_t index = 0; index < config->width.address; ++index)
  {
    group = pinGroupFind(fsmcAddressPins, fsmcAddressPinMap[index], 0);
    assert(group != NULL);
    pinOutput((pin = pinInit(fsmcAddressPinMap[index])), false);
    pinSetFunction(pin, group->value);
    pinSetSlewRate(pin, config->speed);
  }

  /* Data bus */
  for (size_t index = 0; index < config->width.data; ++index)
  {
    group = pinGroupFind(fsmcDataPins, fsmcDataPinMap[index], 0);
    assert(group != NULL);
    pinInput((pin = pinInit(fsmcDataPinMap[index])));
    pinSetFunction(pin, group->value);
    pinSetSlewRate(pin, config->speed);
  }

  /* Output Enable pin */
  group = pinGroupFind(fsmcControlPins, fsmcControlPinMap.noe, 0);
  assert(group != NULL);
  pinOutput((pin = pinInit(fsmcControlPinMap.noe)), true);
  pinSetFunction(pin, group->value);
  pinSetSlewRate(pin, config->speed);

  /* Byte Lane Select pins */
  for (unsigned int lane = 0; lane < byteLanes; ++lane)
  {
    group = pinGroupFind(fsmcControlPins, fsmcControlPinMap.nbl[lane], 0);
    assert(group);
    pinOutput((pin = pinInit(fsmcControlPinMap.nbl[lane])), true);
    pinSetFunction(pin, group->value);
    pinSetSlewRate(pin, config->speed);
  }

  /* Write Enable pin */
  if (config->useWriteEnable)
  {
    group = pinGroupFind(fsmcControlPins, fsmcControlPinMap.nwe, 0);
    assert(group != NULL);
    pinOutput((pin = pinInit(fsmcControlPinMap.nwe)), true);
    pinSetFunction(pin, group->value);
    pinSetSlewRate(pin, config->speed);
  }

  /* Chip Select pin */
  group = pinGroupFind(fsmcControlPins,
      fsmcControlPinMap.ne[memory->subbank], 0);
  assert(group != NULL);
  pinOutput((pin = pinInit(fsmcControlPinMap.ne[memory->subbank])), true);
  pinSetFunction(pin, group->value);
  pinSetSlewRate(pin, config->speed);

  /* Disable memory bank */
  STM_FSMC->BANK[memory->subbank].BCR = 0;

  /* Results are in FSMC_CLK ticks */
  const uint32_t frequency = fsmcGetClock() / 10;
  const uint32_t cycle = 1000000000UL / frequency;

  const uint32_t oeTicks = timeToTicks(MAX(config->timings.oe, 1), cycle);
  assert(oeTicks <= BTR_ADDSET_MAX);

  const uint32_t weTicks = timeToTicks(MAX(config->timings.we, 1), cycle);
  assert(weTicks <= BTR_ADDSET_MAX);

  const uint32_t taTicks = timeToTicks(config->timings.turnaround, cycle);
  const uint32_t taReadTicks = MAX(taTicks, 1) - 1;
  assert(taReadTicks <= BTR_BUSTURN_MAX);
  const uint32_t taWriteTicks = MAX(taTicks, 2) - 2;
  assert(taWriteTicks <= BWTR_BUSTURN_MAX);

  uint32_t rdTicks = timeToTicks(config->timings.rd, cycle);
  rdTicks = MAX(rdTicks, oeTicks + 1);
  assert(rdTicks - oeTicks <= BTR_DATAST_MAX);

  uint32_t wrTicks = timeToTicks(config->timings.wr, cycle);
  wrTicks = MAX(wrTicks, weTicks + 2) - 1;
  assert(wrTicks - weTicks <= BTR_DATAST_MAX);

  const uint32_t btr = BTR_ACCMOD(BTR_ACCMOD_A)
      | BTR_ADDSET(oeTicks)
      | BTR_DATAST(rdTicks - oeTicks)
      | BTR_BUSTURN(taReadTicks);
  const uint32_t bwtr = BWTR_ACCMOD(BWTR_ACCMOD_A)
      | BWTR_ADDSET(weTicks)
      | BWTR_DATAST(wrTicks - weTicks)
      | BWTR_BUSTURN(taWriteTicks);

  /* Configure memory read and write timing */
  STM_FSMC->BANK[memory->subbank].BTR = btr;
  STM_FSMC->BANK_W[memory->subbank].BWTR = bwtr;

  uint32_t bcr = BCR_MBKEN | BCR_MTYP(BCR_MTYP_SRAM) | BCR_WREN | BCR_EXTMOD;
  bcr |= byteLanes == 2 ? BCR_MWID(BCR_MWID_16) : BCR_MWID(BCR_MWID_8);

  /* Configure and enable memory bank */
  STM_FSMC->BANK[memory->subbank].BCR = bcr;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_FSMC_NO_DEINIT
static void sramDeinit(void *object)
{
  struct FsmcSram * const memory = object;
  fsmcSetMemoryDescriptor(memory->subbank, object, NULL);
}
#endif
