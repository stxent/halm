/*
 * sct_timer.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/sct_defs.h>
#include <halm/platform/nxp/sct_timer.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static enum Result genericTimerInit(void *, const struct SctTimerConfig *);

#ifdef CONFIG_PLATFORM_NXP_SCT_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);
static enum Result tmrInitUnified(void *, const void *);
static void tmrEnable(void *);
static void tmrDisable(void *);
static void tmrSetCallback(void *, void (*)(void *), void *);
static uint32_t tmrGetFrequency(const void *);
static void tmrSetFrequency(void *, uint32_t);
static uint32_t tmrGetOverflow(const void *);
static uint32_t tmrGetOverflowUnified(const void *);
static void tmrSetOverflow(void *, uint32_t);
static void tmrSetOverflowUnified(void *, uint32_t);
static uint32_t tmrGetValue(const void *);
static uint32_t tmrGetValueUnified(const void *);
static void tmrSetValue(void *, uint32_t);
static void tmrSetValueUnified(void *, uint32_t);

#ifndef CONFIG_PLATFORM_NXP_SCT_NO_DEINIT
static void tmrDeinit(void *);
#else
#define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static const struct TimerClass tmrTable = {
    .size = sizeof(struct SctTimer),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .enable = tmrEnable,
    .disable = tmrDisable,
    .setCallback = tmrSetCallback,
    .getFrequency = tmrGetFrequency,
    .setFrequency = tmrSetFrequency,
    .getOverflow = tmrGetOverflow,
    .setOverflow = tmrSetOverflow,
    .getValue = tmrGetValue,
    .setValue = tmrSetValue
};

static const struct TimerClass tmrUnifiedTable = {
    .size = sizeof(struct SctTimer),
    .init = tmrInitUnified,
    .deinit = tmrDeinit,

    .enable = tmrEnable,
    .disable = tmrDisable,
    .setCallback = tmrSetCallback,
    .getFrequency = tmrGetFrequency,
    .setFrequency = tmrSetFrequency,
    .getOverflow = tmrGetOverflowUnified,
    .setOverflow = tmrSetOverflowUnified,
    .getValue = tmrGetValueUnified,
    .setValue = tmrSetValueUnified
};
/*----------------------------------------------------------------------------*/
const struct TimerClass * const SctTimer = &tmrTable;
const struct TimerClass * const SctUnifiedTimer = &tmrUnifiedTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct SctTimer * const timer = object;
  timer->callback(timer->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result genericTimerInit(void *object,
    const struct SctTimerConfig *config)
{
  const struct SctBaseConfig baseConfig = {
      .channel = config->channel,
      .edge = PIN_RISING,
      .input = SCT_INPUT_NONE,
      .part = config->part
  };
  struct SctTimer * const timer = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = SctBase->init(timer, &baseConfig)) != E_OK)
    return res;

  LPC_SCT_Type * const reg = timer->base.reg;
  const unsigned int part = timer->base.part == SCT_HIGH;

  if (!sctAllocateEvent(&timer->base, &timer->event))
    return E_BUSY;

  timer->base.handler = interruptHandler;
  timer->base.mask = 1 << timer->event;
  timer->callback = 0;

  /* Disable the timer before any configuration is done */
  reg->CTRL_PART[part] = CTRL_HALT;

  /* Set desired timer frequency */
  timer->frequency = config->frequency;
  sctSetFrequency(&timer->base, timer->frequency);

  /* Disable match value reload and set current match register value */
  reg->CONFIG |= CONFIG_NORELOAD(part);

  /* Match value must be configured after event initialization */
  if (timer->base.part != SCT_UNIFIED)
    reg->MATCH_PART[timer->event][part] = 0xFFFF;
  else
    reg->MATCH[timer->event] = 0xFFFFFFFFUL;

  /* Configure event */
  reg->EV[timer->event].CTRL = EVCTRL_MATCHSEL(timer->event)
      | EVCTRL_COMBMODE(COMBMODE_MATCH) | EVCTRL_MATCHMEM
      | EVCTRL_DIRECTION(DIRECTION_INDEPENDENT);

  if (timer->base.part == SCT_HIGH)
    reg->EV[timer->event].CTRL |= EVCTRL_HEVENT;

  /* Reset current state and enable allocated event in state 0 */
  reg->STATE_PART[part] = 0;
  reg->EV[timer->event].STATE = 0x00000001;
  /* Enable timer clearing on allocated event */
  reg->LIMIT_PART[part] = timer->base.mask;
  /* By default the timer is disabled */

#ifdef CONFIG_PLATFORM_NXP_SCT_PM
  if ((res = pmRegister(powerStateHandler, timer)) != E_OK)
    return res;
#endif

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NXP_SCT_PM
static void powerStateHandler(void *object, enum PmState state)
{
  if (state == PM_ACTIVE)
  {
    struct SctTimer * const timer = object;
    sctSetFrequency(&timer->base, timer->frequency);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct SctTimerConfig * const config = configBase;
  assert(config);
  assert(config->part != SCT_UNIFIED);

  return genericTimerInit(object, config);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInitUnified(void *object, const void *configBase)
{
  const struct SctTimerConfig * const config = configBase;
  assert(config);
  assert(config->part == SCT_UNIFIED);

  return genericTimerInit(object, config);
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_SCT_NO_DEINIT
static void tmrDeinit(void *object)
{
  struct SctTimer * const timer = object;
  LPC_SCT_Type * const reg = timer->base.reg;
  const unsigned int part = timer->base.part == SCT_HIGH;

  /* Halt the timer */
  reg->CTRL_PART[part] = CTRL_HALT;
  reg->EVEN &= ~timer->base.mask;
  reg->LIMIT_PART[part] = 0;

  /* Disable allocated event */
  reg->EV[timer->event].CTRL = 0;
  reg->EV[timer->event].STATE = 0;
  sctReleaseEvent(&timer->base, timer->event);

  /* Reset to default state */
  reg->CONFIG &= ~CONFIG_NORELOAD(part);

#ifdef CONFIG_PLATFORM_NXP_SCT_PM
  pmUnregister(timer);
#endif

  SctBase->deinit(timer);
}
#endif
/*----------------------------------------------------------------------------*/
static void tmrEnable(void *object)
{
  struct SctTimer * const timer = object;
  LPC_SCT_Type * const reg = timer->base.reg;
  const unsigned int part = timer->base.part == SCT_HIGH;

  reg->EVFLAG = timer->base.mask;
  reg->CTRL_PART[part] &= ~CTRL_HALT;
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object)
{
  struct SctTimer * const timer = object;
  LPC_SCT_Type * const reg = timer->base.reg;
  const unsigned int part = timer->base.part == SCT_HIGH;

  reg->CTRL_PART[part] |= CTRL_HALT;
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SctTimer * const timer = object;
  LPC_SCT_Type * const reg = timer->base.reg;
  const uint16_t eventMask = timer->base.mask;

  timer->callbackArgument = argument;
  timer->callback = callback;

  if (callback)
  {
    /* Clear pending requests */
    reg->EVFLAG = eventMask;
    /* Enable interrupt requests */
    reg->EVEN |= eventMask;
  }
  else
    reg->EVEN &= ~eventMask;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object)
{
  const struct SctTimer * const timer = object;
  const LPC_SCT_Type * const reg = timer->base.reg;
  const unsigned int part = timer->base.part == SCT_HIGH;
  const uint32_t baseClock = sctGetClock(&timer->base);
  const uint32_t prescaler = CTRL_PRE_VALUE(reg->CTRL_PART[part]) + 1;

  return baseClock / prescaler;
}
/*----------------------------------------------------------------------------*/
static void tmrSetFrequency(void *object, uint32_t frequency)
{
  struct SctTimer * const timer = object;

  timer->frequency = frequency;
  sctSetFrequency(&timer->base, timer->frequency);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow(const void *object)
{
  const struct SctTimer * const timer = object;
  const LPC_SCT_Type * const reg = timer->base.reg;
  const unsigned int part = timer->base.part == SCT_HIGH;

  return reg->MATCH_PART[timer->event][part] + 1;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflowUnified(const void *object)
{
  const struct SctTimer * const timer = object;
  return ((const LPC_SCT_Type *)timer->base.reg)->MATCH[timer->event] + 1;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *object, uint32_t overflow)
{
  struct SctTimer * const timer = object;
  LPC_SCT_Type * const reg = timer->base.reg;
  const unsigned int part = timer->base.part == SCT_HIGH;

  assert(overflow < (1 << 16));

  reg->CTRL_PART[part] |= CTRL_STOP;
  reg->MATCH_PART[timer->event][part] = overflow - 1;
  reg->CTRL_PART[part] &= ~CTRL_STOP;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflowUnified(void *object, uint32_t overflow)
{
  struct SctTimer * const timer = object;
  LPC_SCT_Type * const reg = timer->base.reg;

  reg->CTRL_PART[SCT_LOW] |= CTRL_STOP;
  reg->MATCH[timer->event] = overflow - 1;
  reg->CTRL_PART[SCT_LOW] &= ~CTRL_STOP;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object)
{
  const struct SctTimer * const timer = object;
  const LPC_SCT_Type * const reg = timer->base.reg;
  const unsigned int part = timer->base.part == SCT_HIGH;

  return reg->COUNT_PART[part];
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValueUnified(const void *object)
{
  const struct SctTimer * const timer = object;
  return ((const LPC_SCT_Type *)timer->base.reg)->COUNT;
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object, uint32_t value)
{
  struct SctTimer * const timer = object;
  LPC_SCT_Type * const reg = timer->base.reg;
  const unsigned int part = timer->base.part == SCT_HIGH;

  assert(value <= reg->MATCH_PART[timer->event][part]);

  reg->CTRL_PART[part] |= CTRL_STOP;
  reg->COUNT_PART[part] = value;
  reg->CTRL_PART[part] &= ~CTRL_STOP;
}
/*----------------------------------------------------------------------------*/
static void tmrSetValueUnified(void *object, uint32_t value)
{
  struct SctTimer * const timer = object;
  LPC_SCT_Type * const reg = timer->base.reg;

  assert(value <= reg->MATCH[timer->event]);

  reg->CTRL_PART[SCT_LOW] |= CTRL_STOP;
  reg->COUNT = value;
  reg->CTRL_PART[SCT_LOW] &= ~CTRL_STOP;
}
