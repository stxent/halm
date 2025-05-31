/*
 * sct_timer.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/sct_defs.h>
#include <halm/platform/lpc/sct_timer.h>
#include <halm/pm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static enum Result genericTimerInit(void *, uint8_t, enum SctPart,
    PinNumber, enum InputEvent, enum SctOutput, IrqPriority, uint32_t);

#ifdef CONFIG_PLATFORM_LPC_SCT_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);
static enum Result tmrInitCounter(void *, const void *);
static enum Result tmrInitUnified(void *, const void *);
static enum Result tmrInitUnifiedCounter(void *, const void *);
static void tmrEnable(void *);
static void tmrDisable(void *);
static void tmrSetAutostop(void *, bool);
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

#ifndef CONFIG_PLATFORM_LPC_SCT_NO_DEINIT
static void tmrDeinit(void *);
#else
#  define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct TimerClass * const SctTimer = &(const struct TimerClass){
    .size = sizeof(struct SctTimer),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .enable = tmrEnable,
    .disable = tmrDisable,
    .setAutostop = tmrSetAutostop,
    .setCallback = tmrSetCallback,
    .getFrequency = tmrGetFrequency,
    .setFrequency = tmrSetFrequency,
    .getOverflow = tmrGetOverflow,
    .setOverflow = tmrSetOverflow,
    .getValue = tmrGetValue,
    .setValue = tmrSetValue
};

const struct TimerClass * const SctUnifiedTimer = &(const struct TimerClass){
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

const struct TimerClass * const SctCounter =
    &(const struct TimerClass){
    .size = sizeof(struct SctTimer),
    .init = tmrInitCounter,
    .deinit = tmrDeinit,

    .enable = tmrEnable,
    .disable = tmrDisable,
    .setAutostop = tmrSetAutostop,
    .setCallback = tmrSetCallback,
    .getFrequency = tmrGetFrequency,
    .setFrequency = tmrSetFrequency,
    .getOverflow = tmrGetOverflow,
    .setOverflow = tmrSetOverflow,
    .getValue = tmrGetValue,
    .setValue = tmrSetValue
};

const struct TimerClass * const SctUnifiedCounter =
    &(const struct TimerClass){
    .size = sizeof(struct SctTimer),
    .init = tmrInitUnifiedCounter,
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
static void interruptHandler(void *object)
{
  struct SctTimer * const timer = object;
  timer->callback(timer->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result genericTimerInit(void *object, uint8_t channel,
    enum SctPart segment, PinNumber clock, enum InputEvent edge,
    enum SctOutput output, IrqPriority priority, uint32_t frequency)
{
  const struct SctBaseConfig baseConfig = {
      .channel = channel,
      .part = segment
  };
  struct SctTimer * const timer = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = SctBase->init(timer, &baseConfig)) != E_OK)
    return res;

  if (!sctAllocateEvent(&timer->base, &timer->event))
    return E_BUSY;

  LPC_SCT_Type * const reg = timer->base.reg;
  const unsigned int part = timer->base.part == SCT_HIGH;
  const uint16_t eventMask = 1 << timer->event;

  /* Configure clock input */
  if (clock)
  {
    timer->input = sctAllocateInputChannel(&timer->base, clock);
    if (timer->input == SCT_INPUT_NONE)
      return E_BUSY;

    sctConfigInputPin(object, timer->input, clock, PIN_NOPULL);

    /* Configure timer clock source */
    if (edge == INPUT_RISING)
    {
      reg->CONFIG |= CONFIG_CLKMODE(CLKMODE_INPUT_HP)
          | CONFIG_CKSEL_RISING(timer->input - 1);
    }
    else
    {
      reg->CONFIG |= CONFIG_CLKMODE(CLKMODE_INPUT_HP)
          | CONFIG_CKSEL_FALLING(timer->input - 1);
    }
  }
  else
    timer->input = SCT_INPUT_NONE;

  timer->base.handler = interruptHandler;
  timer->base.mask = eventMask;
  timer->callback = NULL;

  /* Disable the timer before any configuration is done */
  reg->CTRL_PART[part] = CTRL_HALT;

  /* Set desired timer frequency */
  timer->frequency = frequency;
  sctSetFrequency(&timer->base, timer->frequency);

  /* Disable match value reload and set current match register value */
  reg->CONFIG |= CONFIG_NORELOAD(part);

  /* Match value must be configured after event initialization */
  if (timer->base.part != SCT_UNIFIED)
    reg->MATCH_PART[timer->event][part] = 0xFFFF;
  else
    reg->MATCH[timer->event] = 0xFFFFFFFFUL;

  /* Enable match mode for match/capture registers */
  reg->REGMODE_PART[part] &= ~eventMask;

  /* Configure event */
  reg->EV[timer->event].CTRL =
      (timer->base.part == SCT_HIGH ? EVCTRL_HEVENT : 0)
      | EVCTRL_MATCHSEL(timer->event)
      | EVCTRL_COMBMODE(COMBMODE_MATCH)
      | EVCTRL_DIRECTION(DIRECTION_INDEPENDENT);

  /* Configure event output */
  timer->output = output;
  if (timer->output != SCT_OUTPUT_NONE)
  {
    reg->OUTPUTDIRCTRL &= ~OUTPUTDIRCTRL_SETCLR_MASK(timer->output - 1);
    reg->RES = (reg->RES & ~RES_OUTPUT_MASK(timer->output - 1))
        | RES_OUTPUT(timer->output - 1, OUTPUT_TOGGLE);
    reg->OUT[timer->output - 1].CLR = eventMask;
    reg->OUT[timer->output - 1].SET = eventMask;
  }

  /* Reset current state and enable allocated event in state 0 */
  reg->STATE_PART[part] = 0;
  reg->EV[timer->event].STATE = 0x00000001;
  /* Enable timer clearing on allocated event */
  reg->LIMIT_PART[part] = eventMask;

#ifdef CONFIG_PLATFORM_LPC_SCT_PM
  if ((res = pmRegister(powerStateHandler, timer)) != E_OK)
    return res;
#endif

  /* Priority is same for both timer parts */
  irqSetPriority(timer->base.irq, priority);

  /* The timer remains disabled */
  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_LPC_SCT_PM
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
  assert(config != NULL);
  assert(config->part != SCT_UNIFIED);

  return genericTimerInit(object, config->channel, config->part,
      0, INPUT_RISING, config->output, config->priority, config->frequency);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInitCounter(void *object, const void *configBase)
{
  const struct SctCounterConfig * const config = configBase;
  assert(config != NULL);
  assert(config->part != SCT_UNIFIED);

  return genericTimerInit(object, config->channel, config->part,
      config->pin, config->edge, config->output, config->priority, 0);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInitUnified(void *object, const void *configBase)
{
  const struct SctTimerConfig * const config = configBase;
  assert(config != NULL);
  assert(config->part == SCT_UNIFIED);

  return genericTimerInit(object, config->channel, SCT_UNIFIED,
      0, INPUT_RISING, config->output, config->priority, config->frequency);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInitUnifiedCounter(void *object, const void *configBase)
{
  const struct SctCounterConfig * const config = configBase;
  assert(config != NULL);
  assert(config->part == SCT_UNIFIED);

  return genericTimerInit(object, config->channel, SCT_UNIFIED,
      config->pin, config->edge, config->output, config->priority, 0);
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_SCT_NO_DEINIT
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

  /* Release allocated input */
  if (timer->input != SCT_INPUT_NONE)
    sctReleaseInputChannel(&timer->base, timer->input);

  /* Disable output event generation */
  if (timer->output != SCT_OUTPUT_NONE)
  {
    reg->OUT[timer->output - 1].CLR = 0;
    reg->OUT[timer->output - 1].SET = 0;
  }

  /* Reset to default state */
  reg->CONFIG &= ~CONFIG_NORELOAD(part);

#ifdef CONFIG_PLATFORM_LPC_SCT_PM
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
static void tmrSetAutostop(void *object, bool state)
{
  struct SctTimer * const timer = object;
  LPC_SCT_Type * const reg = timer->base.reg;
  const unsigned int part = timer->base.part == SCT_HIGH;

  if (state)
    reg->HALT_PART[part] = 1 << timer->event;
  else
    reg->HALT_PART[part] = 0;
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

  if (timer->callback != NULL)
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
  const LPC_SCT_Type * const reg = timer->base.reg;

  return reg->MATCH[timer->event] + 1;
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
