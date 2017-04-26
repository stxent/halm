/*
 * sct_timer.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/sct_defs.h>
#include <halm/platform/nxp/sct_timer.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static void setMatchValue(struct SctTimer *, uint32_t);
static void updateFrequency(struct SctTimer *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *, const void *);
static void tmrDeinit(void *);
static void tmrSetCallback(void *, void (*)(void *), void *);
static void tmrSetEnabled(void *, bool);
static uint32_t tmrGetFrequency(const void *);
static void tmrSetFrequency(void *, uint32_t);
static uint32_t tmrGetOverflow(const void *);
static void tmrSetOverflow(void *, uint32_t);
static uint32_t tmrGetValue(const void *);
static void tmrSetValue(void *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct TimerClass tmrTable = {
    .size = sizeof(struct SctTimer),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .setCallback = tmrSetCallback,
    .setEnabled = tmrSetEnabled,
    .getFrequency = tmrGetFrequency,
    .setFrequency = tmrSetFrequency,
    .getOverflow = tmrGetOverflow,
    .setOverflow = tmrSetOverflow,
    .getValue = tmrGetValue,
    .setValue = tmrSetValue
};
/*----------------------------------------------------------------------------*/
const struct TimerClass * const SctTimer = &tmrTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct SctTimer * const timer = object;

  if (timer->callback)
    timer->callback(timer->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void setMatchValue(struct SctTimer *timer, uint32_t value)
{
  LPC_SCT_Type * const reg = timer->base.reg;

  if (timer->base.part != SCT_UNIFIED)
  {
    const unsigned int offset = timer->base.part == SCT_HIGH;

    reg->MATCH_PART[timer->event][offset] = (uint16_t)value;
  }
  else
    reg->MATCH[timer->event] = value;
}
/*----------------------------------------------------------------------------*/
static void updateFrequency(struct SctTimer *timer, uint32_t frequency)
{
  LPC_SCT_Type * const reg = timer->base.reg;
  const unsigned int offset = timer->base.part == SCT_HIGH;
  const uint32_t value =
      (reg->CTRL_PART[offset] & ~CTRL_PRE_MASK) | CTRL_CLRCTR;

  if (frequency)
  {
    /* TODO Check whether the clock is from the internal source */
    const uint32_t baseClock = sctGetClock((struct SctBase *)timer);
    const uint16_t prescaler = baseClock / frequency - 1;

    assert(prescaler < 256);

    reg->CTRL_PART[offset] = value | CTRL_PRE(prescaler);
  }
  else
    reg->CTRL_PART[offset] = value;

  timer->frequency = frequency;
}
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *object, const void *configBase)
{
  const struct SctTimerConfig * const config = configBase;
  assert(config);

  const struct SctBaseConfig baseConfig = {
      .channel = config->channel,
      .input = SCT_INPUT_NONE,
      .part = config->part
  };
  struct SctTimer * const timer = object;
  enum result res;

  /* Call base class constructor */
  if ((res = SctBase->init(object, &baseConfig)) != E_OK)
    return res;

  const int event = sctAllocateEvent((struct SctBase *)timer);

  if (event == -1)
    return E_BUSY;

  timer->base.handler = interruptHandler;
  timer->callback = 0;
  timer->event = event;

  LPC_SCT_Type * const reg = timer->base.reg;
  const unsigned int offset = timer->base.part == SCT_HIGH;
  const uint16_t eventMask = 1 << timer->event;

  timer->base.mask = eventMask;
  reg->CTRL_PART[offset] = CTRL_HALT;

  /* Set desired timer frequency */
  updateFrequency(timer, config->frequency);

  /* Disable match value reload and set current match register value */
  reg->CONFIG |= CONFIG_NORELOAD(offset);
  /* Should be called after event initialization */
  setMatchValue(timer, 0xFFFFFFFF);

  /* Configure event */
  reg->EV[timer->event].CTRL = EVCTRL_MATCHSEL(timer->event)
      | EVCTRL_COMBMODE(COMBMODE_MATCH) | EVCTRL_MATCHMEM
      | EVCTRL_DIRECTION(DIRECTION_INDEPENDENT);

  if (timer->base.part == SCT_HIGH)
    reg->EV[timer->event].CTRL |= EVCTRL_HEVENT;

  /* Reset current state and enable allocated event in state 0 */
  reg->STATE_PART[offset] = 0;
  reg->EV[timer->event].STATE = 0x00000001;
  /* Enable timer clearing on allocated event */
  reg->LIMIT_PART[offset] = eventMask;

  /* Timer is left disabled by default */

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void tmrDeinit(void *object)
{
  struct SctTimer * const timer = object;
  LPC_SCT_Type * const reg = timer->base.reg;
  const unsigned int offset = timer->base.part == SCT_HIGH;

  /* Halt the timer */
  reg->CTRL_PART[offset] = CTRL_HALT;
  reg->EVEN &= ~timer->base.mask;
  reg->LIMIT_PART[offset] = 0;

  /* Disable allocated event */
  reg->EV[timer->event].CTRL = 0;
  reg->EV[timer->event].STATE = 0;
  sctReleaseEvent((struct SctBase *)timer, timer->event);

  /* Reset to default state */
  reg->CONFIG &= ~CONFIG_NORELOAD(offset);

  SctBase->deinit(timer);
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *), void *argument)
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
static void tmrSetEnabled(void *object, bool state)
{
  struct SctTimer * const timer = object;
  LPC_SCT_Type * const reg = timer->base.reg;
  const unsigned int offset = timer->base.part == SCT_HIGH;

  if (state)
  {
    reg->EVFLAG = timer->base.mask;
    reg->CTRL_PART[offset] &= ~CTRL_HALT;
  }
  else
  {
    reg->CTRL_PART[offset] |= CTRL_HALT;
    reg->EVFLAG = timer->base.mask;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object)
{
  const struct SctTimer * const timer = object;
  const LPC_SCT_Type * const reg = timer->base.reg;
  const unsigned int offset = timer->base.part == SCT_HIGH;
  const uint32_t baseClock = sctGetClock((const struct SctBase *)timer);
  const uint16_t prescaler = CTRL_PRE_VALUE(reg->CTRL_PART[offset]) + 1;

  return baseClock / prescaler;
}
/*----------------------------------------------------------------------------*/
static void tmrSetFrequency(void *object, uint32_t frequency)
{
  updateFrequency(object, frequency);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow(const void *object)
{
  const struct SctTimer * const timer = object;
  const LPC_SCT_Type * const reg = timer->base.reg;
  const unsigned int offset = timer->base.part == SCT_HIGH;

  if (timer->base.part != SCT_UNIFIED)
    return (uint16_t)(reg->MATCH_PART[timer->event][offset] + 1);
  else
    return reg->MATCH[timer->event] + 1;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *object, uint32_t overflow)
{
  struct SctTimer * const timer = object;
  LPC_SCT_Type * const reg = timer->base.reg;
  const unsigned int offset = timer->base.part == SCT_HIGH;

  if (timer->base.part != SCT_UNIFIED)
    assert(overflow < (1 << 16));

  reg->CTRL_PART[offset] |= CTRL_STOP;
  setMatchValue(timer, overflow - 1);
  reg->CTRL_PART[offset] &= ~CTRL_STOP;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object)
{
  const struct SctTimer * const timer = object;
  const LPC_SCT_Type * const reg = timer->base.reg;
  const unsigned int offset = timer->base.part == SCT_HIGH;

  return timer->base.part == SCT_UNIFIED ? reg->COUNT : reg->COUNT_PART[offset];
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object, uint32_t value)
{
  struct SctTimer * const timer = object;
  LPC_SCT_Type * const reg = timer->base.reg;
  const unsigned int offset = timer->base.part == SCT_HIGH;

  reg->CTRL_PART[offset] |= CTRL_STOP;
  if (timer->base.part == SCT_UNIFIED)
  {
    assert(value <= reg->MATCH[timer->event]);
    reg->COUNT = value;
  }
  else
  {
    assert(value <= reg->MATCH_PART[timer->event][offset]);
    reg->COUNT_PART[offset] = value;
  }
  reg->CTRL_PART[offset] &= ~CTRL_STOP;
}
