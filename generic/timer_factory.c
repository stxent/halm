/*
 * timer_factory.c
 * Copyright (C) 2016, 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/timer_factory.h>
#include <halm/irq.h>
#include <assert.h>
#include <limits.h>
/*----------------------------------------------------------------------------*/
#define BASE_OVERFLOW UINT32_MAX
/*----------------------------------------------------------------------------*/
/* Timer Factory subclass descriptor */
struct TimerFactoryClass
{
  struct TimerClass base;
  struct Timer *(*create)(struct TimerFactory *);
};

struct TimerFactoryEntryConfig
{
  struct TimerFactory *parent;
};

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
static void insertTimer(struct TimerFactoryEntry **, struct TimerFactoryEntry *,
    uint32_t);
static void interruptHandler(void *);
static void interruptHandlerTickless(void *);
static void removeTimer(struct TimerFactory *,
    const struct TimerFactoryEntry *);
static inline void scheduleTimerEvent(struct TimerFactory *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum Result factoryInit(void *, const void *);
static void factoryDeinit(void *);
static void factoryEnable(void *);
static void factoryDisable(void *);
static uint32_t factoryGetFrequency(const void *);
static void factorySetFrequency(void *, uint32_t);
static uint32_t factoryGetOverflow(const void *);
static void factorySetOverflow(void *, uint32_t);
static uint32_t factoryGetValue(const void *);
static struct Timer *factoryCreate(struct TimerFactory *);

static enum Result factoryInitTickless(void *, const void *);
static void factoryEnableTickless(void *);
static uint32_t factoryGetOverflowTickless(const void *);
static uint32_t factoryGetValueTickless(const void *);
static struct Timer *factoryCreateTickless(struct TimerFactory *);
/*----------------------------------------------------------------------------*/
const struct TimerFactoryClass * const TimerFactoryImpl =
    &(const struct TimerFactoryClass){
    .base = {
        .size = sizeof(struct TimerFactory),
        .init = factoryInit,
        .deinit = factoryDeinit,

        .enable = factoryEnable,
        .disable = factoryDisable,
        .setAutostop = NULL,
        .setCallback = NULL,
        .getFrequency = factoryGetFrequency,
        .setFrequency = factorySetFrequency,
        .getOverflow = factoryGetOverflow,
        .setOverflow = factorySetOverflow,
        .getValue = factoryGetValue,
        .setValue = NULL
    },

    .create = factoryCreate
};

const struct TimerClass * const TimerFactory =
    (const struct TimerClass *)TimerFactoryImpl;

const struct TimerFactoryClass * const TicklessFactoryImpl =
    &(const struct TimerFactoryClass){
    .base = {
        .size = sizeof(struct TimerFactory),
        .init = factoryInitTickless,
        .deinit = factoryDeinit,

        .enable = factoryEnableTickless,
        .disable = factoryDisable,
        .setAutostop = NULL,
        .setCallback = NULL,
        .getFrequency = factoryGetFrequency,
        .setFrequency = factorySetFrequency,
        .getOverflow = factoryGetOverflowTickless,
        .setOverflow = NULL,
        .getValue = factoryGetValueTickless,
        .setValue = NULL
    },

    .create = factoryCreateTickless
};

const struct TimerClass * const TicklessFactory =
    (const struct TimerClass *)TicklessFactoryImpl;
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);
static void tmrDeinit(void *);
static void tmrEnable(void *);
static void tmrDisable(void *);
static void tmrSetAutostop(void *, bool);
static void tmrSetCallback(void *, void (*)(void *), void *);
static uint32_t tmrGetFrequency(const void *);
static uint32_t tmrGetOverflow(const void *);
static void tmrSetOverflow(void *, uint32_t);
static uint32_t tmrGetValue(const void *);
static void tmrSetValue(void *, uint32_t);

static void tmrEnableTickless(void *);
static uint32_t tmrGetFrequencyTickless(const void *);
/*----------------------------------------------------------------------------*/
const struct TimerClass * const TimerFactoryEntry =
    &(const struct TimerClass){
    .size = sizeof(struct TimerFactoryEntry),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .enable = tmrEnable,
    .disable = tmrDisable,
    .setAutostop = tmrSetAutostop,
    .setCallback = tmrSetCallback,
    .getFrequency = tmrGetFrequency,
    .setFrequency = NULL,
    .getOverflow = tmrGetOverflow,
    .setOverflow = tmrSetOverflow,
    .getValue = tmrGetValue,
    .setValue = tmrSetValue
};

const struct TimerClass * const TicklessFactoryEntry =
    &(const struct TimerClass){
    .size = sizeof(struct TimerFactoryEntry),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .enable = tmrEnableTickless,
    .disable = tmrDisable,
    .setAutostop = tmrSetAutostop,
    .setCallback = tmrSetCallback,
    .getFrequency = tmrGetFrequencyTickless,
    .setFrequency = NULL,
    .getOverflow = tmrGetOverflow,
    .setOverflow = tmrSetOverflow,
    .getValue = tmrGetValue,
    .setValue = tmrSetValue
};
/*----------------------------------------------------------------------------*/
static void insertTimer(struct TimerFactoryEntry **head,
    struct TimerFactoryEntry *timer, uint32_t counter)
{
  struct TimerFactoryEntry *current = *head;

  if (current != NULL)
  {
    int32_t distance;
    uint32_t timestamp = current->timestamp;

    /* Time of the current timer is overflowed */
    distance = timer->timestamp - counter;
    const bool co = distance <= 0;

    /* Time of the current timer is greater or equal to the head timer */
    distance = timestamp - timer->timestamp;
    const bool hg = distance >= 0;

    /* Time of the head timer is overflowed */
    distance = timestamp - counter;
    const bool ho = distance <= 0;

    if ((!co && (ho ^ hg)) || (co && ho && hg))
    {
      timer->next = current;
      *head = timer;
    }
    else
    {
      while (current->next != NULL)
      {
        timestamp = current->next->timestamp;

        /* Time of the current timer is greater or equal to the next timer */
        distance = timestamp - timer->timestamp;
        const bool cg = distance >= 0;

        /* Time of the next timer is overflowed */
        distance = timestamp - counter;
        const bool no = distance <= 0;

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
    *head = timer;
  }

  assert(*head == NULL || (*head != (*head)->next));
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct TimerFactory * const factory = object;
  struct TimerFactoryEntry *current;
  struct TimerFactoryEntry *head = NULL;
  const uint32_t counter = ++factory->counter;

  current = factory->head;
  while (current != NULL && counter == current->timestamp)
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

    timer->next = NULL;
    timer->callback(timer->callbackArgument);

    if (timer->enabled)
    {
      /* Assume that the timer is not rescheduled from another callback */
      assert(counter == timer->timestamp);

      /* Append the periodic timer to the main list */
      timer->timestamp = counter + timer->overflow;
      /* Timestamp cannot overflow to incorrect value in this mode */
      insertTimer(&factory->head, timer, counter);
    }
  }

  assert(factory->head == NULL || (factory->head != factory->head->next));
}
/*----------------------------------------------------------------------------*/
static void interruptHandlerTickless(void *object)
{
  struct TimerFactory * const factory = object;
  struct TimerFactoryEntry *current;
  struct TimerFactoryEntry *head = NULL;
  int32_t distance;

  do
  {
    const uint32_t counter = timerGetValue(factory->timer);

    current = factory->head;
    while (current != NULL && (int32_t)(counter - current->timestamp) >= 0)
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

      timer->next = NULL;
      timer->callback(timer->callbackArgument);

      if (timer->enabled)
      {
        /* Assume that the timer is not rescheduled from another callback */
        assert((int32_t)(counter - timer->timestamp) >= 0);

        /* Append the periodic timer to the main list */
        timer->timestamp = counter + timer->overflow;
        if (timer->timestamp > factory->overflow)
          timer->timestamp -= factory->overflow;
        insertTimer(&factory->head, timer, counter);
      }
    }

    assert(factory->head == NULL || (factory->head != factory->head->next));

    /* Configure time of the next timer interrupt */
    const uint32_t waketime = factory->head != NULL ?
        factory->head->timestamp : counter + factory->overflow / 2;

    scheduleTimerEvent(factory, waketime);
    distance = waketime - timerGetValue(factory->timer);
  }
  while (distance <= 0);
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
static inline void scheduleTimerEvent(struct TimerFactory *factory,
    uint32_t value)
{
  if (++value > factory->overflow)
    value -= factory->overflow;
  timerSetOverflow(factory->timer, value);
}
/*----------------------------------------------------------------------------*/
static enum Result factoryInit(void *object, const void *configBase)
{
  const struct TimerFactoryConfig * const config = configBase;
  assert(config != NULL);
  assert(config->timer != NULL);

  struct TimerFactory * const factory = object;

  factory->head = NULL;
  factory->timer = config->timer;
  factory->counter = 0;
  factory->overflow = BASE_OVERFLOW;

  timerSetCallback(factory->timer, interruptHandler, factory);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void factoryDeinit(void *object)
{
  struct TimerFactory * const factory = object;

  assert(factory->head == NULL);
  timerDisable(factory->timer);
  timerSetCallback(factory->timer, NULL, NULL);
}
/*----------------------------------------------------------------------------*/
static void factoryEnable(void *object)
{
  struct TimerFactory * const factory = object;

  timerSetValue(factory->timer, 0);
  timerEnable(factory->timer);
}
/*----------------------------------------------------------------------------*/
static void factoryDisable(void *object)
{
  struct TimerFactory * const factory = object;
  timerDisable(factory->timer);
}
/*----------------------------------------------------------------------------*/
static uint32_t factoryGetFrequency(const void *object)
{
  const struct TimerFactory * const factory = object;
  return timerGetFrequency(factory->timer);
}
/*----------------------------------------------------------------------------*/
static void factorySetFrequency(void *object, uint32_t frequency)
{
  struct TimerFactory * const factory = object;
  timerSetFrequency(factory->timer, frequency);
}
/*----------------------------------------------------------------------------*/
static uint32_t factoryGetOverflow(const void *object)
{
  const struct TimerFactory * const factory = object;
  return timerGetOverflow(factory->timer);
}
/*----------------------------------------------------------------------------*/
static void factorySetOverflow(void *object, uint32_t overflow)
{
  struct TimerFactory * const factory = object;
  timerSetOverflow(factory->timer, overflow);
}
/*----------------------------------------------------------------------------*/
static uint32_t factoryGetValue(const void *object)
{
  const struct TimerFactory * const factory = object;
  return factory->counter;
}
/*----------------------------------------------------------------------------*/
static struct Timer *factoryCreate(struct TimerFactory *factory)
{
  return init(TimerFactoryEntry, &(struct TimerFactoryEntryConfig){factory});
}
/*----------------------------------------------------------------------------*/
static enum Result factoryInitTickless(void *object, const void *configBase)
{
  const struct TimerFactoryConfig * const config = configBase;
  assert(config != NULL);
  assert(config->timer != NULL);

  struct TimerFactory * const factory = object;

  factory->head = NULL;
  factory->timer = config->timer;
  factory->counter = 0;
  factory->overflow = timerGetOverflow(factory->timer) - 1;

  timerSetOverflow(factory->timer, factory->overflow / 2);
  timerSetCallback(factory->timer, interruptHandlerTickless, factory);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void factoryEnableTickless(void *object)
{
  struct TimerFactory * const factory = object;
  timerEnable(factory->timer);
}
/*----------------------------------------------------------------------------*/
static uint32_t factoryGetOverflowTickless(const void *object)
{
  const struct TimerFactory * const factory = object;
  return factory->overflow + 1;
}
/*----------------------------------------------------------------------------*/
static uint32_t factoryGetValueTickless(const void *object)
{
  const struct TimerFactory * const factory = object;
  return timerGetValue(factory->timer);
}
/*----------------------------------------------------------------------------*/
static struct Timer *factoryCreateTickless(struct TimerFactory *factory)
{
  return init(TicklessFactoryEntry, &(struct TimerFactoryEntryConfig){factory});
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct TimerFactoryEntryConfig * const config = configBase;
  struct TimerFactoryEntry * const timer = object;

  timer->factory = config->parent;
  timer->next = NULL;
  timer->callback = NULL;
  timer->callbackArgument = NULL;
  timer->overflow = 0;
  timer->timestamp = timerGetValue(timer->factory);
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

  assert(timer->callback != NULL);

  const IrqState state = irqSave();

  if (timer->enabled)
    removeTimer(factory, timer);
  timer->timestamp = factory->counter + timer->overflow;
  insertTimer(&factory->head, timer, factory->counter);
  timer->enabled = true;

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

  assert(!timer->enabled);
  timer->callbackArgument = argument;
  timer->callback = callback;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *object)
{
  const struct TimerFactoryEntry * const timer = object;
  const struct TimerFactory * const factory = timer->factory;
  const uint32_t frequency = timerGetFrequency(factory->timer);
  const uint32_t overflow = timerGetOverflow(factory->timer);

  assert(overflow != 0 && overflow <= frequency);
  return frequency / overflow;
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

  /*
   * Overflow values greater than half of the base timer overflow value
   * will lead to incorrect timestamp comparisons.
   */
  assert(overflow <= timer->factory->overflow / 2);
  /* Timer should be disabled to avoid timer rescheduling */
  assert(!timer->enabled);

  timer->overflow = overflow;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *object)
{
  const struct TimerFactoryEntry * const timer = object;

  if (!timer->enabled)
    return 0;

  const struct TimerFactory * const factory = timer->factory;
  const int32_t left = timer->timestamp - timerGetValue(factory);

  return timer->overflow - (left < 0 ?
      (uint32_t)left + factory->overflow / 2 : (uint32_t)left);
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *object, [[maybe_unused]] uint32_t value)
{
  struct TimerFactoryEntry * const timer = object;

  /* Timer value is read-only, writing 0 is implemented using reset */
  assert(value == 0);

  if (timer->enabled)
    tmrEnable(timer);
}
/*----------------------------------------------------------------------------*/
static void tmrEnableTickless(void *object)
{
  struct TimerFactoryEntry * const timer = object;
  struct TimerFactory * const factory = timer->factory;

  assert(timer->callback != NULL);

  const IrqState state = irqSave();
  const uint32_t counter = timerGetValue(timer->factory);

  if (timer->enabled)
    removeTimer(factory, timer);

  timer->timestamp = counter + timer->overflow;
  if (timer->timestamp > factory->overflow)
    timer->timestamp -= factory->overflow;
  insertTimer(&factory->head, timer, counter);
  timer->enabled = true;

  /* The timer queue was empty, or the new timer has the nearest wake-up time */
  if (factory->head == timer)
    scheduleTimerEvent(factory, timer->timestamp);

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequencyTickless(const void *object)
{
  const struct TimerFactoryEntry * const timer = object;
  const struct TimerFactory * const factory = timer->factory;

  return timerGetFrequency(factory->timer);
}
/*----------------------------------------------------------------------------*/
void *timerFactoryCreate(void *object)
{
  return ((const struct TimerFactoryClass *)CLASS(object))->create(object);
}
