/*
 * sct_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <spinlock.h>
#include <platform/platform_defs.h>
#include <platform/nxp/sct_base.h>
#include <platform/nxp/sct_defs.h>
#include <platform/nxp/lpc43xx/clocking.h>
#include <platform/nxp/lpc43xx/system.h>
/*----------------------------------------------------------------------------*/
#define CHANNEL_COUNT 1
/*----------------------------------------------------------------------------*/
struct TimerHandlerConfig
{
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct TimerHandler
{
  struct Entity parent;

  /* Pointer to peripheral registers */
  LPC_SCT_Type *reg;
  /* Timer descriptors */
  struct SctBase *descriptors[2];
  /* Allocated events */
  uint16_t events;
};
/*----------------------------------------------------------------------------*/
static bool timerHandlerActive(uint8_t);
static enum result timerHandlerAttach(uint8_t, enum sctPart, struct SctBase *);
static void timerHandlerDetach(uint8_t, enum sctPart);
static void timerHandlerInstantiate(uint8_t channel);
static void timerHandlerProcess(struct TimerHandler *);
static enum result timerHandlerInit(void *, const void *);
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *, const void *);
static void tmrDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass handlerTable = {
    .size = sizeof(struct TimerHandler),
    .init = timerHandlerInit,
    .deinit = 0
};
/*----------------------------------------------------------------------------*/
static const struct EntityClass timerTable = {
    .size = 0, /* Abstract class */
    .init = tmrInit,
    .deinit = tmrDeinit
};
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const TimerHandler = &handlerTable;
const struct EntityClass * const SctBase = &timerTable;
static struct TimerHandler *handlers[CHANNEL_COUNT] = {0};
static spinlock_t spinlocks[CHANNEL_COUNT] = {SPIN_UNLOCKED};
/*----------------------------------------------------------------------------*/
static bool timerHandlerActive(uint8_t channel)
{
  bool result = false;

  spinLock(&spinlocks[channel]);
  timerHandlerInstantiate(channel);

  result = handlers[channel]->descriptors[0]
      || handlers[channel]->descriptors[1];

  spinUnlock(&spinlocks[channel]);
  return result;
}
/*----------------------------------------------------------------------------*/
static enum result timerHandlerAttach(uint8_t channel, enum sctPart part,
    struct SctBase *timer)
{
  spinLock(&spinlocks[channel]);
  timerHandlerInstantiate(channel);

  struct TimerHandler * const handler = handlers[channel];
  enum result res = E_OK;

  if (part == SCT_UNIFIED)
  {
    if (!handler->descriptors[0] && !handler->descriptors[1])
      handler->descriptors[0] = timer;
    else
      res = E_BUSY;
  }
  else
  {
    const uint8_t offset = part == SCT_HIGH;

    if (!handler->descriptors[offset])
      handler->descriptors[offset] = timer;
    else
      res = E_BUSY;
  }

  spinUnlock(&spinlocks[channel]);
  return res;
}
/*----------------------------------------------------------------------------*/
static void timerHandlerDetach(uint8_t channel, enum sctPart part)
{
  spinLock(&spinlocks[channel]);
  handlers[channel]->descriptors[part == SCT_HIGH] = 0;
  spinUnlock(&spinlocks[channel]);
}
/*----------------------------------------------------------------------------*/
static void timerHandlerInstantiate(uint8_t channel)
{
  if (!handlers[channel])
  {
    const struct TimerHandlerConfig config = {
        .channel = channel
    };

    handlers[channel] = init(TimerHandler, &config);
  }
  assert(handlers[channel]);
}
/*----------------------------------------------------------------------------*/
static void timerHandlerProcess(struct TimerHandler *handler)
{
  const uint16_t state = handler->reg->EVFLAG;

  for (uint8_t index = 0; index < 2; ++index)
  {
    struct SctBase * const descriptor = handler->descriptors[index];

    if (descriptor && descriptor->mask & state)
      descriptor->handler(descriptor);
  }

  /* Clear interrupt flags */
  handler->reg->EVFLAG = state;
}
/*----------------------------------------------------------------------------*/
static enum result timerHandlerInit(void *object, const void *configBase)
{
  const struct TimerHandlerConfig * const config = configBase;
  struct TimerHandler * const handler = object;

  switch (config->channel)
  {
    case 0:
      handler->reg = LPC_SCT;
      break;

    default:
      return E_VALUE;
  }

  handler->descriptors[0] = handler->descriptors[1] = 0;
  handler->events = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
void SCT_ISR(void)
{
  timerHandlerProcess(handlers[0]);
}
/*----------------------------------------------------------------------------*/
int8_t sctAllocateEvent(struct SctBase *timer)
{
  spinLock(&spinlocks[timer->channel]);

  const uint16_t used = handlers[timer->channel]->events;
  int8_t pos = 16; /* Each timer has 16 possible events */

  while (--pos >= 0 && used & (1 << pos));
  if (pos >= 0)
    handlers[timer->channel]->events |= 1 << pos;

  spinUnlock(&spinlocks[timer->channel]);
  return pos;
}
/*----------------------------------------------------------------------------*/
uint32_t sctGetClock(const struct SctBase *timer __attribute__((unused)))
{
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
void sctReleaseEvent(struct SctBase *timer, uint8_t channel)
{
  spinLock(&spinlocks[timer->channel]);
  handlers[timer->channel]->events &= ~(1 << channel);
  spinUnlock(&spinlocks[timer->channel]);
}
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *object, const void *configBase)
{
  const uint32_t configMask = CONFIG_UNIFY | CONFIG_CLKMODE_MASK
      | CONFIG_CKSEL_MASK;
  const struct SctBaseConfig * const config = configBase;
  struct SctBase * const timer = object;
  uint32_t desiredConfig = 0;
  enum result res;

  assert(config->edge < PIN_TOGGLE);
  assert(config->input < SCT_INPUT_END);

  timer->channel = config->channel;
  timer->part = config->part;

  /* Check whether the timer is divided into two separate parts */
  if (timer->part == SCT_UNIFIED)
    desiredConfig |= CONFIG_UNIFY;

  /* Configure timer clock source */
  if (config->input != SCT_INPUT_NONE)
  {
    desiredConfig |= CONFIG_CLKMODE(CONFIG_CLKMODE_INPUT);
    if (config->edge == PIN_RISING)
      desiredConfig |= CONFIG_CKSEL_RISING(config->input);
    else
      desiredConfig |= CONFIG_CKSEL_FALLING(config->input);
  }

  LPC_SCT_Type * const reg = LPC_SCT;
  const bool enabled = timerHandlerActive(timer->channel);

  if (enabled)
  {
    /*
     * Compare current timer configuration with proposed one
     * when timer is already enabled.
     */
    if (desiredConfig != (reg->CONFIG & configMask))
      return E_BUSY;
  }

  if ((res = timerHandlerAttach(timer->channel, timer->part, timer)) != E_OK)
    return res;

  timer->handler = 0;
  timer->irq = SCT_IRQ;
  timer->mask = 0;
  timer->reg = reg;

  if (!enabled)
  {
    /* Enable clock to peripheral */
    sysClockEnable(CLK_M4_SCT);
    /* Reset registers to default values */
    sysResetEnable(RST_SCT);
    /* Enable common interrupt */
    irqEnable(timer->irq);
  }

  reg->CONFIG = (reg->CONFIG & ~configMask) | desiredConfig;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void tmrDeinit(void *object)
{
  struct SctBase * const timer = object;

  timerHandlerDetach(timer->channel, timer->part);
  if (!timerHandlerActive(timer->channel))
  {
    /* Disable common interrupt */
    irqDisable(timer->irq);
    /* Disable clock when all timer parts are inactive */
    sysClockDisable(CLK_M4_SCT);
  }
}
