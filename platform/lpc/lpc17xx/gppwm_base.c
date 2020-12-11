/*
 * gppwm_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/pin.h>
#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/gppwm_base.h>
#include <halm/platform/lpc/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/* Pack match channel and pin function in one value */
#define PACK_VALUE(function, channel) (((channel) << 4) | (function))
/*----------------------------------------------------------------------------*/
static bool setInstance(struct GpPwmUnitBase *);
/*----------------------------------------------------------------------------*/
static enum Result unitInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_GPPWM_NO_DEINIT
static void unitDeinit(void *);
#else
#define unitDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const GpPwmUnitBase = &(const struct EntityClass){
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
static struct GpPwmUnitBase *instance = 0;
/*----------------------------------------------------------------------------*/
static bool setInstance(struct GpPwmUnitBase *object)
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
void PWM1_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
uint32_t gpPwmGetClock(const struct GpPwmUnitBase *unit __attribute__((unused)))
{
  return clockFrequency(MainClock) / sysClockDivToValue(DEFAULT_DIV);
}
/*----------------------------------------------------------------------------*/
static enum Result unitInit(void *object, const void *configBase)
{
  const struct GpPwmUnitBaseConfig * const config = configBase;
  struct GpPwmUnitBase * const unit = object;

  if (setInstance(unit))
  {
    unit->reg = LPC_PWM1;
    unit->irq = PWM1_IRQ;
    unit->handler = 0;
    unit->channel = config->channel;

    sysPowerEnable(PWR_PWM1);
    sysClockControl(CLK_PWM1, DEFAULT_DIV);

    return E_OK;
  }
  else
    return E_BUSY;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_GPPWM_NO_DEINIT
static void unitDeinit(void *object __attribute__((unused)))
{
  sysPowerDisable(PWR_PWM1);
  instance = 0;
}
#endif
