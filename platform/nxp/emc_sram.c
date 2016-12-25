/*
 * emc_sram.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/bits.h>
#include <halm/platform/nxp/emc_base.h>
#include <halm/platform/nxp/emc_defs.h>
#include <halm/platform/nxp/emc_sram.h>
#include <halm/platform/nxp/lpc43xx/system.h>
/*----------------------------------------------------------------------------*/
static enum result sramInit(void *, const void *);
static void sramDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass sramTable = {
    .size = sizeof(struct EmcSram),
    .init = sramInit,
    .deinit = sramDeinit
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const EmcSram = &sramTable;
/*----------------------------------------------------------------------------*/
extern const struct PinGroupEntry emcAddressPins[];
extern const pinNumber emcAddressPinMap[];
extern const struct PinGroupEntry emcControlPins[];
extern const struct EmcPinDescription emcControlPinMap;
extern const struct PinGroupEntry emcDataPins[];
extern const pinNumber emcDataPinMap[];
/*----------------------------------------------------------------------------*/
static enum result sramInit(void *object, const void *configBase)
{
  const struct EmcSramConfig * const config = configBase;
  struct EmcSram * const memory = object;

  // FIXME
//  assert(config->addressWidth <= ARRAY_SIZE(emcAddressPinMap));
//  assert(config->dataWidth == 8
//      || config->dataWidth == 16
//      || config->dataWidth == 32);
//  assert(config->channel < ARRAY_SIZE(emcControlPinMap.cs));

  const struct PinGroupEntry *group;
  struct Pin pin;

  memory->channel = config->channel;

  /* Output Enable pin */
  group = pinGroupFind(emcControlPins, emcControlPinMap.oe, 0);
  assert(group);
  pinInput((pin = pinInit(emcControlPinMap.oe)));
  pinSetFunction(pin, group->value);

  /* Write Enable pin */
  group = pinGroupFind(emcControlPins, emcControlPinMap.we, 0);
  assert(group);
  pinInput((pin = pinInit(emcControlPinMap.we)));
  pinSetFunction(pin, group->value);

  /* Chip Select pin */
  group = pinGroupFind(emcControlPins, emcControlPinMap.cs[memory->channel], 0);
  assert(group);
  pinInput((pin = pinInit(emcControlPinMap.cs[config->channel])));
  pinSetFunction(pin, group->value);

  for (size_t index = 0; index < config->addressWidth; ++index)
  {
    group = pinGroupFind(emcAddressPins, emcAddressPinMap[index], 0);
    assert(group);
    pinInput((pin = pinInit(emcAddressPinMap[index])));
    pinSetFunction(pin, group->value);
  }

  for (size_t index = 0; index < config->dataWidth; ++index)
  {
    group = pinGroupFind(emcDataPins, emcDataPinMap[index], 0);
    assert(group);
    pinInput((pin = pinInit(emcDataPinMap[index])));
    pinSetFunction(pin, group->value);
  }

  /* Enable clocks to register memory and peripheral */
  sysClockEnable(CLK_M4_EMC);
  sysClockEnable(CLK_M4_EMCDIV);
  /* Reset registers to their default values */
  sysResetEnable(RST_EMC);

  uint32_t configValue = STATICCONFIG_PB | STATICCONFIG_B;

  switch (config->dataWidth)
  {
    case 8:
      configValue |= STATICCONFIG_MW(MW_8_BIT);
      break;

    case 16:
      configValue |= STATICCONFIG_MW(MW_16_BIT);
      break;

    case 32:
      configValue |= STATICCONFIG_MW(MW_32_BIT);
      break;
  }

  LPC_EMC->CONTROL = CONTROL_E;
  LPC_EMC->STATIC[memory->channel].CONFIG = configValue;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sramDeinit(void *object)
{
  struct EmcSram * const memory = object;


}
