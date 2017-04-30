/*
 * software_timer.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/generic/software_timer.h>
#include <halm/irq.h>
/*----------------------------------------------------------------------------*/
struct SoftwareTimerConfig
{
  struct SoftwareTimerFactory *parent;
};
/*----------------------------------------------------------------------------*/
struct SoftwareTimer
{
  struct Timer base;

  struct SoftwareTimerFactory *factory;

  void (*callback)(void *);
  void *callbackArgument;

  uint32_t period;
  uint32_t timestamp;

  struct SoftwareTimer *next;
};
/*----------------------------------------------------------------------------*/
static uint32_t distance(uint32_t a, uint32_t b);
static void insertTimer(struct SoftwareTimerFactory *, struct SoftwareTimer *);
static void interruptHandler(void *);
static void removeTimer(struct SoftwareTimerFactory *,
    const struct SoftwareTimer *);
/*----------------------------------------------------------------------------*/
static enum Result factoryInit(void *, const void *);
static void factoryDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass factoryTable = {
    .size = sizeof(struct SoftwareTimerFactory),
    .init = factoryInit,
    .deinit = factoryDeinit
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const SoftwareTimerFactory = &factoryTable;
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);
static void tmrDeinit(void *);
static void tmrEnable(void *);
static void tmrDisable(void *);
static void tmrSetCallback(void *, void (*)(void *), void *);
static uint32_t tmrGetFrequency(const void *);
static void tmrSetFrequency(void *, uint32_t);
static uint32_t tmrGetOverflow(const void *);
static void tmrSetOverflow(void *, uint32_t);
static uint32_t tmrGetValue(const void *);
static void tmrSetValue(void *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct TimerClass tmrTable = {
    .size = sizeof(struct SoftwareTimer),
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
/*----------------------------------------------------------------------------*/
static const struct TimerClass * const SoftwareTimer = &tmrTable;
/*----------------------------------------------------------------------------*/
static uint32_t distance(uint32_t a, uint32_t b)
{
  return b >= a ? b - a : 0xFFFFFF - (a - b);
}
/*----------------------------------------------------------------------------*/
static void insertTimer(struct SoftwareTimerFactory *factory,
    struct SoftwareTimer *timer)
{
  if (!factory->head)
  {
    timer->next = factory->head;
    factory->head = timer;
    return;
  }

  /* Time of the current element is overflowed */
  const bool co = factory->counter >= timer->timestamp;
  /* Time of the current element is greater or equal to the head element */
  const bool hg = timer->timestamp <= factory->head->timestamp;
  /* Time of the head element is overflowed */
  const bool ho = factory->counter >= factory->head->timestamp;

  if ((!co && (ho ^ hg)) || (co && ho && hg))
  {
    timer->next = factory->head;
    factory->head = timer;
  }
  else
  {
    struct SoftwareTimer *current = factory->head;

    while (current->next)
    {
      /* Time of the current element is greater or equal to the next element */
      const bool cg = timer->timestamp <= current->next->timestamp;
      /* Time of the next element is overflowed */
      const bool no = factory->counter >= current->next->timestamp;

      if ((no && !co) || ((no || !co) && cg))
        break;

      current = current->next;
    }

    timer->next = current->next;
    current->next = timer;
  }
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct SoftwareTimerFactory * const factory = object;
  struct SoftwareTimer *current = factory->head;
  struct SoftwareTimer *head = 0;

  ++factory->counter;

  while (current && factory->counter == current->timestamp)
  {
    struct SoftwareTimer * const timer = current;
    current = current->next;

    timer->callback(timer->callbackArgument);

    if (timer->period)
    {
      /* Append periodic timers to the temporary list */
      timer->next = head ? head : 0;
      head = timer;
    }
  }
  factory->head = current;

  /* Append periodic timers to the main list */
  while (head)
  {
    struct SoftwareTimer * const timer = head;
    head = head->next;

    timer->timestamp = factory->counter + timer->period;
    insertTimer(factory, timer);
  }
}
/*----------------------------------------------------------------------------*/
static void removeTimer(struct SoftwareTimerFactory *factory,
    const struct SoftwareTimer *timer)
{
  struct SoftwareTimer **current = &factory->head;

  while (*current != timer)
    current = &(*current)->next;
  assert(current);

  *current = timer->next;
}
/*----------------------------------------------------------------------------*/
static enum Result factoryInit(void *object, const void *configBase)
{
  const struct SoftwareTimerFactoryConfig * const config = configBase;
  struct SoftwareTimerFactory * const factory = object;

  assert(config->timer);

  factory->timer = config->timer;
  factory->head = 0;
  factory->counter = 0;

  timerSetCallback(factory->timer, interruptHandler, factory);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void factoryDeinit(void *object)
{
  struct SoftwareTimerFactory * const factory = object;

  assert(!factory->head);

  timerSetCallback(factory->timer, 0, 0);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct SoftwareTimerConfig * const config = configBase;
  struct SoftwareTimer * const timer = object;

  timer->factory = config->parent;
  timer->callback = 0;
  timer->period = 0;
  timer->timestamp = 0;
  timer->next = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void tmrDeinit(void *object)
{
  struct SoftwareTimer * const timer = object;
  const IrqState irq = irqSave();

  removeTimer(timer->factory, timer);

  irqRestore(irq);
}
/*----------------------------------------------------------------------------*/
static void tmrEnable(void *object)
{
  struct SoftwareTimer * const timer = object;
  struct SoftwareTimerFactory * const factory = timer->factory;
  const IrqState irq = irqSave();

  timer->timestamp = factory->counter + timer->period;
  insertTimer(factory, timer);

  irqRestore(irq);
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object)
{
  struct SoftwareTimer * const timer = object;
  const IrqState irq = irqSave();

  removeTimer(timer->factory, timer);
  timer->next = 0;
  timer->period = 0;

  irqRestore(irq);
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SoftwareTimer * const timer = object;

  timer->callbackArgument = argument;
  timer->callback = callback;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object)
{
  const struct SoftwareTimer * const timer = object;

  return timerGetFrequency(timer->factory->timer);
}
/*----------------------------------------------------------------------------*/
static void tmrSetFrequency(void *object, uint32_t frequency)
{
  const struct SoftwareTimer * const timer = object;

  timerSetFrequency(timer->factory->timer, frequency);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow(const void *object)
{
  const struct SoftwareTimer * const timer = object;

  return timer->period;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *object, uint32_t overflow)
{
  struct SoftwareTimer * const timer = object;

  timer->period = overflow;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object)
{
  const struct SoftwareTimer * const timer = object;

  return distance(timer->factory->counter, timer->timestamp);
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object, uint32_t value)
{
  struct SoftwareTimer * const timer = object;

  const IrqState irq = irqSave();
  const uint32_t current = distance(timer->factory->counter, timer->timestamp);

  if (current > value)
    timer->timestamp += current - value;
  else
    timer->timestamp -= value - current;

  irqRestore(irq);
}
/*----------------------------------------------------------------------------*/
void *softwareTimerCreate(void *object)
{
  const struct SoftwareTimerConfig config = {
      .parent = object
  };

  return init(SoftwareTimer, &config);
}
