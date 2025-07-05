/*
 * timer_factory.c
 * Copyright (C) 2016, 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/timer_factory.h>
#include <halm/irq.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct TimerFactoryEntryConfig
{
  struct TimerFactory *parent;
};
/*----------------------------------------------------------------------------*/
struct TimerFactoryEntry
{
  struct Timer base;

  struct TimerFactory *factory;
  struct TimerFactoryEntry *next;

  void (*callback)(void *);
  void *callbackArgument;

  uint32_t overflow;
  uint32_t timestamp;
  bool continuous;
  bool enabled;
};
/*----------------------------------------------------------------------------*/
static inline uint32_t distance(uint32_t a, uint32_t b);
static void insertTimer(struct TimerFactory *, struct TimerFactoryEntry *);
static void interruptHandler(void *);
static void removeTimer(struct TimerFactory *,
    const struct TimerFactoryEntry *);
/*----------------------------------------------------------------------------*/
static enum Result factoryInit(void *, const void *);
static void factoryDeinit(void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const TimerFactory =
    &(const struct EntityClass){
    .size = sizeof(struct TimerFactory),
    .init = factoryInit,
    .deinit = factoryDeinit
};
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);
static void tmrDeinit(void *);
static void tmrEnable(void *);
static void tmrDisable(void *);
static void tmrSetAutostop(void *, bool);
static void tmrSetCallback(void *, void (*)(void *), void *);
static uint32_t tmrGetFrequency(const void *);
static void tmrSetFrequency(void *, uint32_t);
static uint32_t tmrGetOverflow(const void *);
static void tmrSetOverflow(void *, uint32_t);
static uint32_t tmrGetValue(const void *);
static void tmrSetValue(void *, uint32_t);
/*----------------------------------------------------------------------------*/
const struct TimerClass * const TimerFactoryEntry = &(const struct TimerClass){
    .size = sizeof(struct TimerFactoryEntry),
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
/*----------------------------------------------------------------------------*/
static inline uint32_t distance(uint32_t a, uint32_t b)
{
  return b - a;
}
/*----------------------------------------------------------------------------*/
static void insertTimer(struct TimerFactory *factory,
    struct TimerFactoryEntry *timer)
{
  if (factory->head != NULL)
  {
    /* Time of the current timer is overflowed */
    const bool co = factory->counter >= timer->timestamp;
    /* Time of the current timer is greater or equal to the head timer */
    const bool hg = timer->timestamp <= factory->head->timestamp;
    /* Time of the head timer is overflowed */
    const bool ho = factory->counter >= factory->head->timestamp;

    if ((!co && (ho ^ hg)) || (co && ho && hg))
    {
      timer->next = factory->head;
      factory->head = timer;
    }
    else
    {
      struct TimerFactoryEntry *current = factory->head;

      while (current->next != NULL)
      {
        /* Time of the current timer is greater or equal to the next timer */
        const bool cg = timer->timestamp <= current->next->timestamp;
        /* Time of the next timer is overflowed */
        const bool no = factory->counter >= current->next->timestamp;

        if ((no && !co) || ((no || !co) && cg))
          break;

        current = current->next;
      }

      timer->next = current->next;
      current->next = timer;
    }
  }
  else
  {
    timer->next = NULL;
    factory->head = timer;
  }

  assert(factory->head == NULL || (factory->head != factory->head->next));
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct TimerFactory * const factory = object;
  struct TimerFactoryEntry *current;
  struct TimerFactoryEntry *head = NULL;

  ++factory->counter;

  current = factory->head;
  while (current != NULL && factory->counter == current->timestamp)
  {
    struct TimerFactoryEntry * const timer = current;
    current = current->next;

    if (!timer->continuous)
      timer->enabled = false;
    timer->next = head;
    head = timer;
  }
  factory->head = current;

  current = head;
  while (current != NULL)
  {
    struct TimerFactoryEntry * const timer = current;
    current = current->next;

    timer->callback(timer->callbackArgument);

    if (timer->enabled)
    {
      /* Append the periodic timer to the main list */
      timer->timestamp = factory->counter + timer->overflow;
      insertTimer(factory, timer);
    }
  }

  assert(factory->head == NULL || (factory->head != factory->head->next));
}
/*----------------------------------------------------------------------------*/
static void removeTimer(struct TimerFactory *factory,
    const struct TimerFactoryEntry *timer)
{
  struct TimerFactoryEntry **current = &factory->head;

  while (*current != NULL && *current != timer)
    current = &(*current)->next;

  if (*current != NULL)
    *current = timer->next;

  assert(factory->head == NULL || (factory->head != factory->head->next));
}
/*----------------------------------------------------------------------------*/
static enum Result factoryInit(void *object, const void *configBase)
{
  const struct TimerFactoryConfig * const config = configBase;
  assert(config != NULL);
  assert(config->timer != NULL);

  struct TimerFactory * const factory = object;

  factory->timer = config->timer;
  factory->head = NULL;
  factory->counter = 0;

  timerSetCallback(factory->timer, interruptHandler, factory);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void factoryDeinit(void *object)
{
  struct TimerFactory * const factory = object;

  assert(factory->head == NULL);
  timerSetCallback(factory->timer, NULL, NULL);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct TimerFactoryEntryConfig * const config = configBase;
  struct TimerFactoryEntry * const timer = object;

  timer->factory = config->parent;
  timer->callback = NULL;
  timer->overflow = 0;
  timer->timestamp = timer->factory->counter;
  timer->continuous = true;
  timer->enabled = false;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void tmrDeinit(void *object)
{
  tmrDisable(object);
}
/*----------------------------------------------------------------------------*/
static void tmrEnable(void *object)
{
  struct TimerFactoryEntry * const timer = object;
  struct TimerFactory * const factory = timer->factory;
  const IrqState state = irqSave();

  if (timer->enabled)
    removeTimer(factory, timer);

  timer->enabled = true;
  timer->timestamp = factory->counter + timer->overflow;
  insertTimer(factory, timer);

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object)
{
  struct TimerFactoryEntry * const timer = object;
  const IrqState state = irqSave();

  if (timer->enabled)
  {
    timer->enabled = false;
    removeTimer(timer->factory, timer);
  }

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static void tmrSetAutostop(void *object, bool state)
{
  struct TimerFactoryEntry * const timer = object;
  timer->continuous = !state;
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct TimerFactoryEntry * const timer = object;

  timer->callbackArgument = argument;
  timer->callback = callback;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object)
{
  const struct TimerFactoryEntry * const timer = object;
  struct TimerFactory * const factory = timer->factory;
  const uint32_t clock = timerGetFrequency(factory->timer);
  const uint32_t overflow = timerGetOverflow(factory->timer);

  assert(overflow != 0 && overflow <= clock);
  return clock / overflow;
}
/*----------------------------------------------------------------------------*/
static void tmrSetFrequency(void *object, uint32_t frequency)
{
  const struct TimerFactoryEntry * const timer = object;
  struct TimerFactory * const factory = timer->factory;
  const uint32_t clock = timerGetFrequency(factory->timer);

  assert(frequency != 0 && frequency <= clock);
  timerSetOverflow(factory->timer, clock / frequency);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow(const void *object)
{
  const struct TimerFactoryEntry * const timer = object;
  return timer->overflow;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *object, uint32_t overflow)
{
  struct TimerFactoryEntry * const timer = object;
  timer->overflow = overflow;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object)
{
  const struct TimerFactoryEntry * const timer = object;
  return distance(timer->factory->counter, timer->timestamp);
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object, uint32_t value)
{
  struct TimerFactoryEntry * const timer = object;
  const IrqState state = irqSave();

  const uint32_t current = distance(timer->factory->counter, timer->timestamp);
  timer->timestamp += current - value;

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
void *timerFactoryCreate(void *object)
{
  return init(TimerFactoryEntry, &(struct TimerFactoryEntryConfig){object});
}
