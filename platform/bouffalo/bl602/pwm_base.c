/*
 * pwm_base.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/bouffalo/clocking.h>
#include <halm/platform/bouffalo/pwm_base.h>
#include <halm/platform/bouffalo/pwm_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define PWM_CHANNEL_COUNT ARRAY_SIZE(BL_PWM->CHANNEL)
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t, struct PwmBase *);
/*----------------------------------------------------------------------------*/
static enum Result pwmInit(void *, const void *);

#ifndef CONFIG_PLATFORM_BOUFFALO_PWM_NO_DEINIT
static void pwmDeinit(void *);
#else
#  define pwmDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const PwmBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = pwmInit,
    .deinit = pwmDeinit
};
/*----------------------------------------------------------------------------*/
static struct PwmBase *instances[PWM_CHANNEL_COUNT] = {nullptr};
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct PwmBase *object)
{
  if (instances[channel] == nullptr)
  {
    instances[channel] = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
[[gnu::interrupt]] void PWM_ISR(void)
{
  const uint32_t status = BL_PWM->INT_CONFIG & INT_CONFIG_INTSTS_MASK;
  uint32_t cleared = 0;

  for (size_t channel = 0; channel < PWM_CHANNEL_COUNT; ++channel)
  {
    if (status & BIT(channel))
    {
      if (instances[channel] != nullptr)
        instances[channel]->handler(instances[channel]);

      cleared |= BIT(channel);
    }
  }

  BL_PWM->INT_CONFIG |= INT_CONFIG_INTCLR(cleared);
}
/*----------------------------------------------------------------------------*/
uint8_t pwmChannelFromPin(PinNumber key)
{
  return PIN_TO_OFFSET(key) % PWM_CHANNEL_COUNT;
}
/*----------------------------------------------------------------------------*/
void pwmConfigPins(PinNumber key, bool inversion)
{
  const struct Pin pin = pinInit(key);

  pinOutput(pin, inversion);
  pinSetFunction(pin, PIN_PWM_FUNCTION);
}
/*----------------------------------------------------------------------------*/
uint32_t pwmGetClock(const struct PwmBase *base)
{
  const uint32_t source =
      CONFIG_CLKSEL_VALUE(BL_PWM->CHANNEL[base->channel].CONFIG);

  switch (source)
  {
    case CLKSEL_XCLK:
      return clockFrequency(ExternalOsc);

    case CLKSEL_BCLK:
      return clockFrequency(SocClock);

    default:
      /* TODO BL602 System RTC */
      return 0;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result pwmInit(void *object, const void *configBase)
{
  const struct PwmBaseConfig * const config = configBase;
  struct PwmBase * const interface = object;

  assert(config->channel < PWM_CHANNEL_COUNT);
  if (!setInstance(config->channel, interface))
    return E_BUSY;

  interface->channel = config->channel;
  interface->handler = nullptr;
  interface->irq = PWM_IRQ;
  interface->reg = BL_PWM;

  /* Reset channel registers to a known state */
  BL_PWM->CHANNEL[interface->channel].CONFIG = 0;
  BL_PWM->CHANNEL[interface->channel].INTERRUPT = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_BOUFFALO_PWM_NO_DEINIT
static void pwmDeinit(void *object)
{
  const struct PwmBase * const interface = object;
  instances[interface->channel] = nullptr;
}
#endif
