/*
 * gppwm_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/memory.h>
#include <halm/pin.h>
#include <halm/platform/nxp/gppwm_base.h>
#include <halm/platform/nxp/lpc17xx/clocking.h>
#include <halm/platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/* Pack match channel and pin function in one value */
#define PACK_VALUE(function, channel) (((channel) << 4) | (function))
/*----------------------------------------------------------------------------*/
static bool setDescriptor(uint8_t, const struct GpPwmUnitBase *,
    struct GpPwmUnitBase *);
/*----------------------------------------------------------------------------*/
static enum result unitInit(void *, const void *);
static void unitDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass unitTable = {
    .size = 0, /* Abstract class */
    .init = unitInit,
    .deinit = unitDeinit
};
/*----------------------------------------------------------------------------*/
/* PWM1 block mapped to 0 channel */
const struct PinEntry gpPwmPins[] = {
    {
        .key = PIN(1, 18), /* PWM1 output 1 */
        .channel = 0,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(1, 20), /* PWM1 output 2 */
        .channel = 0,
        .value = PACK_VALUE(2, 2)
    }, {
        /* Unavailable on LPC175x series */
        .key = PIN(1, 21), /* PWM1 output 3 */
        .channel = 0,
        .value = PACK_VALUE(2, 3)
    }, {
        .key = PIN(1, 23), /* PWM1 output 4 */
        .channel = 0,
        .value = PACK_VALUE(2, 4)
    }, {
        .key = PIN(1, 24), /* PWM1 output 5 */
        .channel = 0,
        .value = PACK_VALUE(2, 5)
    }, {
        .key = PIN(1, 26), /* PWM1 output 6 */
        .channel = 0,
        .value = PACK_VALUE(2, 6)
    }, {
        .key = PIN(2, 0), /* PWM1 output 1 */
        .channel = 0,
        .value = PACK_VALUE(1, 1)
    }, {
        .key = PIN(2, 1), /* PWM1 output 2 */
        .channel = 0,
        .value = PACK_VALUE(1, 2)
    }, {
        .key = PIN(2, 2), /* PWM1 output 3 */
        .channel = 0,
        .value = PACK_VALUE(1, 3)
    }, {
        .key = PIN(2, 3), /* PWM1 output 4 */
        .channel = 0,
        .value = PACK_VALUE(1, 4)
    }, {
        .key = PIN(2, 4), /* PWM1 output 5 */
        .channel = 0,
        .value = PACK_VALUE(1, 5)
    }, {
        .key = PIN(2, 5), /* PWM1 output 6 */
        .channel = 0,
        .value = PACK_VALUE(1, 6)
    }, {
        /* Unavailable on LPC175x series */
        .key = PIN(3, 25), /* PWM1 output 2 */
        .channel = 0,
        .value = PACK_VALUE(2, 2)
    }, {
        /* Unavailable on LPC175x series */
        .key = PIN(3, 26), /* PWM1 output 3 */
        .channel = 0,
        .value = PACK_VALUE(3, 3)
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const GpPwmUnitBase = &unitTable;
static struct GpPwmUnitBase *descriptors[1] = {0};
/*----------------------------------------------------------------------------*/
static bool setDescriptor(uint8_t channel, const struct GpPwmUnitBase *state,
    struct GpPwmUnitBase *unit)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state, unit);
}
/*----------------------------------------------------------------------------*/
uint32_t gpPwmGetClock(const struct GpPwmUnitBase *unit __attribute__((unused)))
{
  return clockFrequency(MainClock) / sysClockDivToValue(DEFAULT_DIV);
}
/*----------------------------------------------------------------------------*/
static enum result unitInit(void *object, const void *configBase)
{
  const struct GpPwmUnitBaseConfig * const config = configBase;
  struct GpPwmUnitBase * const unit = object;

  unit->channel = config->channel;

  /* Try to set peripheral descriptor */
  if (!setDescriptor(unit->channel, 0, unit))
    return E_BUSY;

  sysPowerEnable(PWR_PWM1);
  sysClockControl(CLK_PWM1, DEFAULT_DIV);
  unit->reg = LPC_PWM1;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void unitDeinit(void *object)
{
  const struct GpPwmUnitBase * const unit = object;

  sysPowerDisable(PWR_PWM1);
  setDescriptor(unit->channel, unit, 0);
}
