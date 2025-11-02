/*
 * sct_base.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/sct_base.h>
#include <halm/platform/lpc/sct_defs.h>
#include <halm/platform/lpc/system.h>
#include <halm/platform/platform_defs.h>
#include <xcore/accel.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define PACK_VALUE(function, channel) (((channel) << 4) | (function))
/*----------------------------------------------------------------------------*/
struct TimerHandler
{
  /* Timer descriptors */
  struct SctBase *parts[2];
  /* Free events */
  uint16_t events;
  /* Free inputs */
  uint8_t inputs;
  /* Free outputs */
  uint8_t outputs;
};
/*----------------------------------------------------------------------------*/
static bool timerHandlerActive(void);
static bool timerHandlerAttach(enum SctPart, struct SctBase *);

#ifndef CONFIG_PLATFORM_LPC_SCT_NO_DEINIT
static void timerHandlerDetach(enum SctPart);
#endif
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_SCT_NO_DEINIT
static void tmrDeinit(void *);
#else
#  define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const SctBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = tmrInit,
    .deinit = tmrDeinit
};
/*----------------------------------------------------------------------------*/
static struct TimerHandler instance = {
    .parts = {NULL, NULL},
    .events = MASK(SCT_EVENT_COUNT),
    .inputs = MASK(SCT_INPUT_END - 1),
    .outputs = MASK(SCT_OUTPUT_END - 1)
};
/*----------------------------------------------------------------------------*/
static bool timerHandlerActive(void)
{
  return instance.parts[0] != NULL || instance.parts[1] != NULL;
}
/*----------------------------------------------------------------------------*/
static bool timerHandlerAttach(enum SctPart timerPart, struct SctBase *timer)
{
  bool attached = false;

  if (timerPart == SCT_UNIFIED)
  {
    if (instance.parts[0] == NULL && instance.parts[1] == NULL)
    {
      instance.parts[0] = timer;
      attached = true;
    }
  }
  else
  {
    const unsigned int part = timerPart == SCT_HIGH;

    /*
     * If the SCT is operating as two 16-bit counters, events can only modify
     * the state of the outputs when neither counter is halted.
     */
    if (instance.parts[part] == NULL)
    {
      instance.parts[part] = timer;
      attached = true;
    }
  }

  return attached;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_SCT_NO_DEINIT
static void timerHandlerDetach(enum SctPart timerPart)
{
  const unsigned int part = timerPart == SCT_HIGH;
  instance.parts[part] = NULL;
}
#endif
/*----------------------------------------------------------------------------*/
void SCT_ISR(void)
{
  const uint16_t state = LPC_SCT->EVFLAG;

  if (instance.parts[0] != NULL && (instance.parts[0]->mask & state))
    instance.parts[0]->handler(instance.parts[0]);

  if (instance.parts[1] != NULL && (instance.parts[1]->mask & state))
    instance.parts[1]->handler(instance.parts[1]);

  /* Clear interrupt flags */
  LPC_SCT->EVFLAG = state;
}
/*----------------------------------------------------------------------------*/
bool sctAllocateEvent(struct SctBase *, uint8_t *event)
{
  if (instance.events)
  {
    *event = 31 - countLeadingZeros32(instance.events);
    instance.events &= ~(1 << *event);
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
enum SctInput sctAllocateInputChannel(struct SctBase *, PinNumber)
{
  if (instance.inputs)
  {
    const unsigned int route = 31 - countLeadingZeros32(instance.inputs);

    instance.inputs &= ~(1 << route);
    return (enum SctInput)(route + 1);
  }
  else
    return SCT_INPUT_NONE;
}
/*----------------------------------------------------------------------------*/
enum SctOutput sctAllocateOutputChannel(struct SctBase *, PinNumber)
{
  if (instance.outputs)
  {
    const unsigned int route = 31 - countLeadingZeros32(instance.outputs);

    instance.outputs &= ~(1 << route);
    return (enum SctOutput)(route + 1);
  }
  else
    return SCT_OUTPUT_NONE;
}
/*----------------------------------------------------------------------------*/
void sctConfigInputPin(struct SctBase *, enum SctInput route, PinNumber key,
    enum PinPull pull)
{
  const struct Pin pin = pinInit(key);

  pinInput(pin);
  pinSetMux(pin, PINMUX_SCT_PIN0 - 1 + route);
  pinSetPull(pin, pull);
}
/*----------------------------------------------------------------------------*/
void sctConfigOutputPin(struct SctBase *, enum SctOutput route, PinNumber key,
    bool value)
{
  const struct Pin pin = pinInit(key);

  pinOutput(pin, value);
  pinSetMux(pin, PINMUX_SCT_OUT0 - 1 + route);
}
/*----------------------------------------------------------------------------*/
uint32_t sctGetClock(const struct SctBase *)
{
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
void sctReleaseEvent(struct SctBase *, uint8_t event)
{
  instance.events |= 1 << event;
}
/*----------------------------------------------------------------------------*/
void sctReleaseInputChannel(struct SctBase *, enum SctInput route)
{
  instance.inputs |= 1 << (route - 1);
}
/*----------------------------------------------------------------------------*/
void sctReleaseOutputChannel(struct SctBase *, enum SctOutput route)
{
  instance.outputs |= 1 << (route - 1);
}
/*----------------------------------------------------------------------------*/
bool sctReserveInputChannel(struct SctBase *, enum SctInput route)
{
  if (instance.inputs & (1 << (route - 1)))
  {
    instance.inputs &= ~(1 << (route - 1));
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
bool sctReserveOutputChannel(struct SctBase *, enum SctOutput route)
{
  if (instance.outputs & (1 << (route - 1)))
  {
    instance.outputs &= ~(1 << (route - 1));
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct SctBaseConfig * const config = configBase;
  struct SctBase * const timer = object;
  uint32_t value = 0;

  /* Check whether the timer is divided into two separate parts */
  if (config->part == SCT_UNIFIED)
    value |= CONFIG_UNIFY;

  const bool enabled = timerHandlerActive();
  LPC_SCT_Type * const reg = LPC_SCT;

  if (enabled)
  {
    /*
     * Compare current timer configuration with proposed one
     * if the timer is already enabled.
     */
    if (value != (reg->CONFIG & CONFIG_SHARED_MASK))
      return E_BUSY;
  }

  if (timerHandlerAttach(config->part, timer))
  {
    timer->channel = config->channel;
    timer->handler = NULL;
    timer->irq = SCT_IRQ;
    timer->mask = 0;
    timer->part = config->part;
    timer->reg = reg;

    if (!enabled)
    {
      /* Enable clock to peripheral */
      sysClockEnable(CLK_SCT);
      /* Reset registers to default values */
      sysResetPulse(RST_SCT);
      /* Enable common interrupt */
      irqEnable(timer->irq);
    }

    reg->CONFIG = (reg->CONFIG & ~CONFIG_SHARED_MASK) | value;
    return E_OK;
  }

  return E_BUSY;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_SCT_NO_DEINIT
static void tmrDeinit(void *object)
{
  struct SctBase * const timer = object;

  timerHandlerDetach(timer->part);

  if (!timerHandlerActive())
  {
    /* Disable common interrupt */
    irqDisable(timer->irq);
    /* Disable clock when all timer parts are inactive */
    sysClockDisable(CLK_SCT);
  }
}
#endif
