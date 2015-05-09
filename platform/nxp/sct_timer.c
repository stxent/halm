/*
 * sct_timer.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/platform_defs.h>
#include <platform/nxp/sct_defs.h>
#include <platform/nxp/sct_timer.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static void setMatchValue(struct SctTimer *, uint32_t);
static enum result updateFrequency(struct SctTimer *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *, const void *);
static void tmrDeinit(void *);
static void tmrCallback(void *, void (*)(void *), void *);
static void tmrSetEnabled(void *, bool);
static enum result tmrSetFrequency(void *, uint32_t);
static enum result tmrSetOverflow(void *, uint32_t);
static enum result tmrSetValue(void *, uint32_t);
static uint32_t tmrValue(const void *);
/*----------------------------------------------------------------------------*/
static const struct TimerClass tmrTable = {
    .size = sizeof(struct SctTimer),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .callback = tmrCallback,
    .setEnabled = tmrSetEnabled,
    .setFrequency = tmrSetFrequency,
    .setOverflow = tmrSetOverflow,
    .setValue = tmrSetValue,
    .value = tmrValue
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
  LPC_SCT_Type * const reg = timer->parent.reg;

  if (timer->parent.part != SCT_UNIFIED)
  {
    const uint8_t offset = timer->parent.part == SCT_HIGH;

    reg->MATCH_PART[timer->event][offset] = (uint16_t)value;
  }
  else
    reg->MATCH[timer->event] = value;
}
/*----------------------------------------------------------------------------*/
static enum result updateFrequency(struct SctTimer *timer, uint32_t frequency)
{
  LPC_SCT_Type * const reg = timer->parent.reg;
  const uint8_t offset = timer->parent.part == SCT_HIGH;
  const uint32_t value = (reg->CTRL_PART[offset] & ~CTRL_PRE_MASK)
      | CTRL_CLRCTR;

  if (frequency)
  {
    /* TODO Check whether the clock is from internal source */
    const uint32_t baseClock = sctGetClock((struct SctBase *)timer);
    const uint16_t prescaler = baseClock / frequency - 1;

    if (prescaler >= 256)
      return E_VALUE;

    reg->CTRL_PART[offset] = value | CTRL_PRE(prescaler);
  }
  else
    reg->CTRL_PART[offset] = value;

  timer->frequency = frequency;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *object, const void *configBase)
{
  const struct SctTimerConfig * const config = configBase;
  const struct SctBaseConfig parentConfig = {
      .channel = config->channel,
      .input = SCT_INPUT_NONE,
      .part = config->part
  };
  struct SctTimer * const timer = object;
  enum result res;

  /* Call base class constructor */
  if ((res = SctBase->init(object, &parentConfig)) != E_OK)
    return res;

  const int8_t event = sctAllocateEvent((struct SctBase *)timer);

  if (event == -1)
    return E_BUSY;

  timer->parent.handler = interruptHandler;
  timer->callback = 0;
  timer->event = (uint8_t)event;

  LPC_SCT_Type * const reg = timer->parent.reg;
  const uint16_t eventMask = 1 << timer->event;
  const uint8_t offset = timer->parent.part == SCT_HIGH;

  timer->parent.mask = eventMask;
  reg->CTRL_PART[offset] = CTRL_HALT;

  /* Set desired timer frequency */
  if ((res = updateFrequency(timer, config->frequency)) != E_OK)
    return res;

  /* Disable match value reload and set current match register value */
  reg->CONFIG |= CONFIG_NORELOAD(offset);
  /* Should be called after event initialization */
  setMatchValue(timer, 0xFFFFFFFF);

  /* Configure event */
  reg->EV[timer->event].CTRL = EVCTRL_MATCHSEL(timer->event)
      | EVCTRL_COMBMODE(COMBMODE_MATCH) | EVCTRL_MATCHMEM
      | EVCTRL_DIRECTION(DIRECTION_INDEPENDENT);

  if (timer->parent.part == SCT_HIGH)
    reg->EV[timer->event].CTRL |= EVCTRL_HEVENT;

  /* Reset current state and enable allocated event in state 0 */
  reg->STATE_PART[offset] = 0;
  reg->EV[timer->event].STATE = 0x00000001;
  /* Enable timer clearing on allocated event */
  reg->LIMIT_PART[offset] = eventMask;

  if (!config->disabled)
    reg->CTRL_PART[offset] &= ~CTRL_HALT;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void tmrDeinit(void *object)
{
  struct SctTimer * const timer = object;
  LPC_SCT_Type * const reg = timer->parent.reg;
  const uint8_t offset = timer->parent.part == SCT_HIGH;

  /* Halt the timer */
  reg->CTRL_PART[offset] = CTRL_HALT;
  reg->EVEN &= ~timer->parent.mask;
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
static void tmrCallback(void *object, void (*callback)(void *), void *argument)
{
  struct SctTimer * const timer = object;
  LPC_SCT_Type * const reg = timer->parent.reg;
  const uint16_t eventMask = timer->parent.mask;

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
  LPC_SCT_Type * const reg = timer->parent.reg;
  const uint8_t offset = timer->parent.part == SCT_HIGH;

  if (state)
  {
    reg->EVFLAG = timer->parent.mask;
    reg->CTRL_PART[offset] &= ~CTRL_HALT;
  }
  else
    reg->CTRL_PART[offset] |= CTRL_HALT;
}
/*----------------------------------------------------------------------------*/
static enum result tmrSetFrequency(void *object, uint32_t frequency)
{
  struct SctTimer * const timer = object;

  return updateFrequency(timer, frequency);
}
/*----------------------------------------------------------------------------*/
static enum result tmrSetOverflow(void *object, uint32_t overflow)
{
  struct SctTimer * const timer = object;
  LPC_SCT_Type * const reg = timer->parent.reg;
  const uint8_t offset = timer->parent.part == SCT_HIGH;

  if (timer->parent.part != SCT_UNIFIED && overflow >= (1 << 16))
    return E_VALUE;

  reg->CTRL_PART[offset] |= CTRL_STOP;
  setMatchValue(timer, overflow - 1);
  reg->CTRL_PART[offset] &= ~CTRL_STOP;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result tmrSetValue(void *object, uint32_t value)
{
  struct SctTimer * const timer = object;
  LPC_SCT_Type * const reg = timer->parent.reg;
  const uint8_t offset = timer->parent.part == SCT_HIGH;
  enum result res = E_VALUE;

  reg->CTRL_PART[offset] |= CTRL_STOP;
  if (timer->parent.part == SCT_UNIFIED)
  {
    if (value <= reg->MATCH[timer->event])
    {
      reg->COUNT = value;
      res = E_OK;
    }
  }
  else
  {
    if (value <= reg->MATCH_PART[timer->event][offset])
    {
      reg->COUNT_PART[offset] = value;
      res = E_OK;
    }
  }
  reg->CTRL_PART[offset] &= ~CTRL_STOP;

  return res;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrValue(const void *object)
{
  const struct SctTimer * const timer = object;
  const LPC_SCT_Type * const reg = timer->parent.reg;
  const uint8_t offset = timer->parent.part == SCT_HIGH;

  return timer->parent.part == SCT_UNIFIED ? reg->COUNT
      : reg->COUNT_PART[offset];
}
