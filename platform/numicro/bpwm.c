/*
 * bpwm.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/bpwm.h>
#include <halm/platform/numicro/bpwm_defs.h>
#include <halm/pm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static bool unitAllocateChannel(struct BpwmUnit *, uint8_t);

#ifdef CONFIG_PLATFORM_NUMICRO_BPWM_PM
static void powerStateHandler(void *, enum PmState);
#endif
#ifndef CONFIG_PLATFORM_NUMICRO_BPWM_NO_DEINIT
static void unitReleaseChannel(struct BpwmUnit *, uint8_t);
#endif
/*----------------------------------------------------------------------------*/
static enum Result unitInit(void *, const void *);
static void unitEnable(void *);
static void unitDisable(void *);
static void unitSetCallback(void *, void (*)(void *), void *);
static uint32_t unitGetFrequency(const void *);
static void unitSetFrequency(void *, uint32_t);
static uint32_t unitGetOverflow(const void *);
static void unitSetOverflow(void *, uint32_t);

#ifndef CONFIG_PLATFORM_NUMICRO_BPWM_NO_DEINIT
static void unitDeinit(void *);
#else
#  define unitDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static enum Result singleEdgeInit(void *, const void *);
static void singleEdgeEnable(void *);
static void singleEdgeDisable(void *);
static void singleEdgeSetDuration(void *, uint32_t);
static void singleEdgeSetEdges(void *, uint32_t, uint32_t);

#ifndef CONFIG_PLATFORM_NUMICRO_BPWM_NO_DEINIT
static void singleEdgeDeinit(void *);
#else
#  define singleEdgeDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct TimerClass * const BpwmUnit = &(const struct TimerClass){
    .size = sizeof(struct BpwmUnit),
    .init = unitInit,
    .deinit = unitDeinit,

    .enable = unitEnable,
    .disable = unitDisable,
    .setAutostop = NULL,
    .setCallback = unitSetCallback,
    .getFrequency = unitGetFrequency,
    .setFrequency = unitSetFrequency,
    .getOverflow = unitGetOverflow,
    .setOverflow = unitSetOverflow,
    .getValue = NULL,
    .setValue = NULL
};

const struct PwmClass * const Bpwm = &(const struct PwmClass){
    .size = sizeof(struct Bpwm),
    .init = singleEdgeInit,
    .deinit = singleEdgeDeinit,

    .enable = singleEdgeEnable,
    .disable = singleEdgeDisable,
    .setDuration = singleEdgeSetDuration,
    .setEdges = singleEdgeSetEdges
};
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_BPWM_PM
static void powerStateHandler(void *object, enum PmState state)
{
  if (state == PM_ACTIVE)
  {
    struct BpwmUnit * const unit = object;
    bpwmSetFrequency(&unit->base, unit->frequency);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct BpwmUnit * const timer = object;
  NM_BPWM_Type * const reg = timer->base.reg;

  /* Clear Period Interrupt flag */
  reg->INTSTS = INTSTS_PIF0;

  timer->callback(timer->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static bool unitAllocateChannel(struct BpwmUnit *unit, uint8_t channel)
{
  const uint8_t mask = 1 << channel;

  if (!(unit->used & mask))
  {
    unit->used |= mask;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_BPWM_NO_DEINIT
static void unitReleaseChannel(struct BpwmUnit *unit, uint8_t channel)
{
  unit->used &= ~(1 << channel);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result unitInit(void *object, const void *configBase)
{
  const struct BpwmUnitConfig * const config = configBase;
  assert(config != NULL);
  assert(config->resolution >= 2);

  const struct BpwmUnitBaseConfig baseConfig = {
      .channel = config->channel
  };
  struct BpwmUnit * const unit = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = BpwmUnitBase->init(unit, &baseConfig)) != E_OK)
    return res;

  unit->base.handler = interruptHandler;
  unit->callback = NULL;
  unit->centered = config->centered;
  unit->frequency = config->frequency;
  unit->resolution = config->resolution;
  unit->used = 0;

  const uint32_t period = unit->resolution - (unit->centered ? 0 : 1);
  NM_BPWM_Type * const reg = unit->base.reg;

  reg->CNTEN = 0;
  reg->CNTCLR = CNTCLR_CNTCLR0;
  while (reg->CNTCLR & CNTCLR_CNTCLR0);

  /* Disable interruts and clear all pending interrupt flags */
  reg->INTEN = 0;
  reg->INTSTS = INTSTS_ZIF0 | INTSTS_PIF0
      | INTSTS_CMPUIF_MASK | INTSTS_CMPDIF_MASK;

  if (unit->centered)
  {
    reg->CTL0 = CTL0_CTRLD_MASK;
    reg->CTL1 = CTL1_CNTTYPE0(CNTTYPE0_UP_DOWN);
  }
  else
  {
    reg->CTL0 = 0;
    reg->CTL1 = CTL1_CNTTYPE0(CNTTYPE0_UP);
  }

  reg->CTL0 |= CTL0_IMMLDEN_MASK;
  reg->PERIOD = period;
  reg->CTL0 &= ~CTL0_IMMLDEN_MASK;

  /* Configure prescaler */
  bpwmSetFrequency(&unit->base, unit->frequency);

#ifdef CONFIG_PLATFORM_NUMICRO_BPWM_PM
  if ((res = pmRegister(powerStateHandler, unit)) != E_OK)
    return res;
#endif

  irqSetPriority(unit->base.irq, config->priority);
  irqEnable(unit->base.irq);

  /* Timer is left in a disabled state */
  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_BPWM_NO_DEINIT
static void unitDeinit(void *object)
{
  struct BpwmUnit * const unit = object;
  NM_BPWM_Type * const reg = unit->base.reg;

  reg->CNTEN = 0;
  irqDisable(unit->base.irq);

#ifdef CONFIG_PLATFORM_NUMICRO_BPWM_PM
  pmUnregister(unit);
#endif

  BpwmUnitBase->deinit(unit);
}
#endif
/*----------------------------------------------------------------------------*/
static void unitEnable(void *object)
{
  struct BpwmUnit * const unit = object;
  NM_BPWM_Type * const reg = unit->base.reg;

  /* Clear Period Interrupt flag */
  reg->INTSTS = INTSTS_PIF0;
  reg->CNTEN = CNTEN_CNTEN0;
}
/*----------------------------------------------------------------------------*/
static void unitDisable(void *object)
{
  struct BpwmUnit * const unit = object;
  NM_BPWM_Type * const reg = unit->base.reg;

  /* Stop the timer */
  reg->CNTEN = 0;
}
/*----------------------------------------------------------------------------*/
static void unitSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct BpwmUnit * const unit = object;
  NM_BPWM_Type * const reg = unit->base.reg;

  unit->callbackArgument = argument;
  unit->callback = callback;

  if (unit->callback != NULL)
  {
    /* Clear Period Interrupt flag and enable Period Interrupt */
    reg->INTSTS = INTSTS_PIF0;
    reg->INTEN |= INTEN_PIEN0;
  }
  else
    reg->INTEN &= ~INTEN_PIEN0;
}
/*----------------------------------------------------------------------------*/
static uint32_t unitGetFrequency(const void *object)
{
  const struct BpwmUnit * const unit = object;
  return unit->frequency;
}
/*----------------------------------------------------------------------------*/
static void unitSetFrequency(void *object, uint32_t frequency)
{
  struct BpwmUnit * const unit = object;

  unit->frequency = frequency;
  bpwmSetFrequency(&unit->base, unit->frequency);
}
/*----------------------------------------------------------------------------*/
static uint32_t unitGetOverflow(const void *object)
{
  const struct BpwmUnit * const unit = object;
  return unit->resolution;
}
/*----------------------------------------------------------------------------*/
static void unitSetOverflow(void *object, uint32_t overflow)
{
  struct BpwmUnit * const unit = object;
  NM_BPWM_Type * const reg = unit->base.reg;

  unit->resolution = overflow;
  reg->PERIOD = unit->centered ? overflow : overflow - 1;
}
/*----------------------------------------------------------------------------*/
static enum Result singleEdgeInit(void *object, const void *configBase)
{
  const struct BpwmConfig * const config = configBase;
  assert(config != NULL);

  struct Bpwm * const pwm = object;
  struct BpwmUnit * const unit = config->parent;

  /* Initialize output pin */
  pwm->channel = bpwmConfigPin(unit->base.channel, config->pin);

  /* Allocate channel */
  if (unitAllocateChannel(unit, pwm->channel))
  {
    NM_BPWM_Type * const reg = unit->base.reg;
    uint32_t wgctl0 = reg->WGCTL0;
    uint32_t wgctl1 = reg->WGCTL1;

    wgctl0 &= ~(WGCTL0_ZPCTL_MASK(pwm->channel)
        | WGCTL0_PRDPCTL_MASK(pwm->channel));
    wgctl1 &= ~(WGCTL1_CMPUCTL_MASK(pwm->channel)
        | WGCTL1_CMPDCTL_MASK(pwm->channel));

    if (unit->centered)
    {
      wgctl1 |= WGCTL1_CMPUCTL(pwm->channel, CMPUCTL_HIGH);
      wgctl1 |= WGCTL1_CMPDCTL(pwm->channel, CMPUCTL_LOW);
    }
    else
    {
      wgctl0 |= WGCTL0_ZPCTL(pwm->channel, ZPCTL_HIGH);
      wgctl1 |= WGCTL1_CMPUCTL(pwm->channel, CMPUCTL_LOW);
    }

    if (config->inversion)
      reg->POLCTL |= POLCTL_PINV(pwm->channel);
    else
      reg->POLCTL &= ~POLCTL_PINV(pwm->channel);

    reg->CMPDAT[pwm->channel] = 0;
    reg->WGCTL0 = wgctl0;
    reg->WGCTL1 = wgctl1;

    pwm->unit = unit;
    return E_OK;
  }
  else
    return E_BUSY;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_BPWM_NO_DEINIT
static void singleEdgeDeinit(void *object)
{
  struct Bpwm * const pwm = object;
  NM_BPWM_Type * const reg = pwm->unit->base.reg;

  reg->POEN &= ~POEN_POEN(pwm->channel);
  unitReleaseChannel(pwm->unit, pwm->channel);
}
#endif
/*----------------------------------------------------------------------------*/
static void singleEdgeEnable(void *object)
{
  struct Bpwm * const pwm = object;
  NM_BPWM_Type * const reg = pwm->unit->base.reg;

  reg->POEN |= POEN_POEN(pwm->channel);
}
/*----------------------------------------------------------------------------*/
static void singleEdgeDisable(void *object)
{
  struct Bpwm * const pwm = object;
  NM_BPWM_Type * const reg = pwm->unit->base.reg;

  reg->POEN &= ~POEN_POEN(pwm->channel);
}
/*----------------------------------------------------------------------------*/
static void singleEdgeSetDuration(void *object, uint32_t duration)
{
  struct Bpwm * const pwm = object;
  NM_BPWM_Type * const reg = pwm->unit->base.reg;

  reg->CMPDAT[pwm->channel] = duration;
}
/*----------------------------------------------------------------------------*/
static void singleEdgeSetEdges(void *object,
    uint32_t leading __attribute__((unused)), uint32_t trailing)
{
  assert(leading == 0); /* Leading edge time must be zero */
  singleEdgeSetDuration(object, trailing);
}
/*----------------------------------------------------------------------------*/
/**
 * Create single edge PWM channel.
 * @param unit Pointer to a BpwmUnit object.
 * @param pin Pin used as a signal output.
 * @param inversion Enable output inversion.
 * @return Pointer to a new Pwm object on success or zero on error.
 */
void *bpwmCreate(void *unit, PinNumber pin, bool inversion)
{
  const struct BpwmConfig channelConfig = {
      .parent = unit,
      .pin = pin,
      .inversion = inversion
  };

  return init(Bpwm, &channelConfig);
}
