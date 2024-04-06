/*
 * pit.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/imxrt/pit.h>
#include <halm/platform/imxrt/pit_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static void setTimerFrequency(struct Pit *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);
static enum Result tmrInit64(void *, const void *);
static void tmrEnable(void *);
static void tmrDisable(void *);
static void tmrSetCallback(void *, void (*)(void *), void *);
static uint32_t tmrGetFrequency(const void *);
static void tmrSetFrequency(void *, uint32_t);
static uint32_t tmrGetOverflow(const void *);
static void tmrSetOverflow(void *, uint32_t);
static uint32_t tmrGetValue(const void *);
static uint64_t tmrGetValue64(const void *);
static void tmrSetValue(void *, uint32_t);
static void tmrSetValue64(void *, uint64_t);

#ifndef CONFIG_PLATFORM_IMXRT_PIT_NO_DEINIT
static void tmrDeinit(void *);
#else
#  define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct TimerClass * const Pit = &(const struct TimerClass){
    .size = sizeof(struct Pit),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .enable = tmrEnable,
    .disable = tmrDisable,
    .setAutostop = NULL,
    .setCallback = tmrSetCallback,
    .getFrequency = tmrGetFrequency,
    .setFrequency = tmrSetFrequency,
    .getOverflow = tmrGetOverflow,
    .setOverflow = tmrSetOverflow,
    .getValue = tmrGetValue,
    .setValue = tmrSetValue
};

const struct Timer64Class * const Pit64 = &(const struct Timer64Class){
    .base = {
        .size = sizeof(struct Pit),
        .init = tmrInit64,
        .deinit = tmrDeinit,

        .enable = tmrEnable,
        .disable = tmrDisable,
        .setAutostop = NULL,
        .setCallback = NULL,
        .getFrequency = tmrGetFrequency,
        .setFrequency = NULL,
        .getOverflow = NULL,
        .setOverflow = NULL,
        .getValue = tmrGetValue,
        .setValue = tmrSetValue
    },
    .getValue64 = tmrGetValue64,
    .setValue64 = tmrSetValue64
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Pit * const timer = object;
  IMX_PIT_Type * const reg = timer->base.reg;

  if (reg->CHANNEL[timer->base.counter].TFLG & TFLG_TIF)
  {
    /* Clear pending interrupt flag */
    reg->CHANNEL[timer->base.counter].TFLG = TFLG_TIF;

    if (timer->callback != NULL)
      timer->callback(timer->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
static void setTimerFrequency(struct Pit *timer, uint32_t frequency)
{
  assert(frequency && timer->base.chain);

  IMX_PIT_Type * const reg = timer->base.reg;

  if (frequency)
  {
    const uint32_t clock = pitGetClock(&timer->base);
    assert(frequency <= clock);

    const uint32_t divisor = clock / frequency;
    assert(divisor > 1);

    reg->CHANNEL[timer->base.channel].LDVAL = divisor - 1;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct PitConfig * const config = configBase;
  assert(config != NULL);

  const struct PitBaseConfig baseConfig = {
      .channel = config->channel,
      .chain = config->chain
  };
  struct Pit * const timer = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = PitBase->init(timer, &baseConfig)) != E_OK)
    return res;

  timer->base.handler = interruptHandler;
  timer->callback = NULL;

  IMX_PIT_Type * const reg = timer->base.reg;

  reg->CHANNEL[timer->base.channel].LDVAL = UINT32_MAX;
  reg->CHANNEL[timer->base.channel].TCTRL = 0;

  if (timer->base.chain)
  {
    if (config->frequency)
      setTimerFrequency(timer, config->frequency);

    /* Configure chained timer */
    reg->CHANNEL[timer->base.channel + 1].LDVAL = UINT32_MAX;
    reg->CHANNEL[timer->base.channel + 1].TCTRL = TCTRL_CHN;
  }

  irqSetPriority(timer->base.irq, config->priority);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit64(void *object, const void *)
{
  const struct PitConfig config = {
      .frequency = 0,
      .priority = 0,
      .channel = 0,
      .chain = true
  };

  return tmrInit(object, &config);
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_IMXRT_PIT_NO_DEINIT
static void tmrDeinit(void *object)
{
  struct Pit * const timer = object;
  IMX_PIT_Type * const reg = timer->base.reg;

  reg->CHANNEL[timer->base.channel].TCTRL = 0;
  if (timer->base.chain)
    reg->CHANNEL[timer->base.channel + 1].TCTRL = 0;

  PitBase->deinit(timer);
}
#endif
/*----------------------------------------------------------------------------*/
static void tmrEnable(void *object)
{
  struct Pit * const timer = object;
  IMX_PIT_Type * const reg = timer->base.reg;

  if (timer->base.chain)
    reg->CHANNEL[timer->base.channel + 1].TCTRL |= TCTRL_TEN;
  reg->CHANNEL[timer->base.channel].TCTRL |= TCTRL_TEN;
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object)
{
  struct Pit * const timer = object;
  IMX_PIT_Type * const reg = timer->base.reg;

  reg->CHANNEL[timer->base.channel].TCTRL &= ~TCTRL_TEN;
  if (timer->base.chain)
    reg->CHANNEL[timer->base.channel + 1].TCTRL &= ~TCTRL_TEN;
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Pit * const timer = object;
  IMX_PIT_Type * const reg = timer->base.reg;

  timer->callback = callback;
  timer->callbackArgument = argument;

  if (timer->callback != NULL)
  {
    reg->CHANNEL[timer->base.counter].TFLG = TFLG_TIF;
    reg->CHANNEL[timer->base.counter].TCTRL |= TCTRL_TIE;
  }
  else
    reg->CHANNEL[timer->base.counter].TCTRL &= ~TCTRL_TIE;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object)
{
  const struct Pit * const timer = object;
  const IMX_PIT_Type * const reg = timer->base.reg;
  const uint32_t clock = pitGetClock(&timer->base);
  const uint32_t divisor = reg->CHANNEL[timer->base.channel].LDVAL + 1;

  return (timer->base.chain && divisor) ? clock / divisor : clock;
}
/*----------------------------------------------------------------------------*/
static void tmrSetFrequency(void *object, uint32_t frequency)
{
  setTimerFrequency(object, frequency);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow(const void *object)
{
  const struct Pit * const timer = object;
  const IMX_PIT_Type * const reg = timer->base.reg;

  return reg->CHANNEL[timer->base.counter].LDVAL + 1;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *object, uint32_t overflow)
{
  struct Pit * const timer = object;
  IMX_PIT_Type * const reg = timer->base.reg;

  reg->CHANNEL[timer->base.counter].LDVAL = overflow - 1;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object)
{
  const struct Pit * const timer = object;
  const IMX_PIT_Type * const reg = timer->base.reg;

  return reg->CHANNEL[timer->base.counter].LDVAL
      - reg->CHANNEL[timer->base.counter].CVAL;
}
/*----------------------------------------------------------------------------*/
static uint64_t tmrGetValue64(const void *object)
{
  const struct Pit * const timer = object;
  const IMX_PIT_Type * const reg = timer->base.reg;
  uint32_t high, low;

  /* Errata workaround */
  do
  {
    high = reg->LTMR64H;
    low = reg->LTMR64L;
  }
  while (low == reg->CHANNEL[timer->base.channel].CVAL);

  return UINT64_MAX - (((uint64_t)high << 32) | low);
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object, uint32_t value)
{
  assert(value == 0);

  struct Pit * const timer = object;
  IMX_PIT_Type * const reg = timer->base.reg;
  const uint32_t enabled = reg->CHANNEL[timer->base.channel].TCTRL & TCTRL_TIE;

  /* Clear both timers by disabling and then enabling them back */

  if (timer->base.chain)
    reg->CHANNEL[timer->base.channel + 1].TCTRL &= ~TCTRL_TIE;
  reg->CHANNEL[timer->base.channel].TCTRL &= ~TCTRL_TIE;

  if (timer->base.chain)
    reg->CHANNEL[timer->base.channel + 1].TCTRL |= enabled;
  reg->CHANNEL[timer->base.channel].TCTRL |= enabled;
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue64(void *object, uint64_t value)
{
  assert(value == 0);
  tmrSetValue(object, value);
}
