/*
 * software_timer.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/software_timer.h>
#include <halm/irq.h>
#include <assert.h>
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
  struct SoftwareTimer *next;

  void (*callback)(void *);
  void *callbackArgument;

  uint32_t overflow;
  uint32_t timestamp;
  bool continuous;
  bool enabled;
};
/*----------------------------------------------------------------------------*/
static inline uint32_t distance(uint32_t a, uint32_t b);
static void insertTimer(struct SoftwareTimerFactory *, struct SoftwareTimer *);
static void interruptHandler(void *);
static void removeTimer(struct SoftwareTimerFactory *,
    const struct SoftwareTimer *);
/*----------------------------------------------------------------------------*/
static enum Result factoryInit(void *, const void *);
static void factoryDeinit(void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const SoftwareTimerFactory =
    &(const struct EntityClass){
    .size = sizeof(struct SoftwareTimerFactory),
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
const struct TimerClass * const SoftwareTimer = &(const struct TimerClass){
    .size = sizeof(struct SoftwareTimer),
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
static void insertTimer(struct SoftwareTimerFactory *factory,
    struct SoftwareTimer *timer)
{
  if (factory->head)
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
      struct SoftwareTimer *current = factory->head;

      while (current->next)
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
    timer->next = 0;
    factory->head = timer;
  }

  assert(!factory->head || (factory->head != factory->head->next));
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct SoftwareTimerFactory * const factory = object;
  struct SoftwareTimer *current;
  struct SoftwareTimer *head = 0;

  ++factory->counter;

  current = factory->head;
  while (current && factory->counter == current->timestamp)
  {
    struct SoftwareTimer * const timer = current;
    current = current->next;

    if (!timer->continuous)
      timer->enabled = false;
    timer->next = head;
    head = timer;
  }
  factory->head = current;

  current = head;
  while (current)
  {
    struct SoftwareTimer * const timer = current;
    current = current->next;

    timer->callback(timer->callbackArgument);

    if (timer->enabled)
    {
      /* Append the periodic timer to the main list */
      timer->timestamp = factory->counter + timer->overflow;
      insertTimer(factory, timer);
    }
  }

  assert(!factory->head || (factory->head != factory->head->next));
}
/*----------------------------------------------------------------------------*/
static void removeTimer(struct SoftwareTimerFactory *factory,
    const struct SoftwareTimer *timer)
{
  struct SoftwareTimer **current = &factory->head;

  while (*current && *current != timer)
    current = &(*current)->next;

  if (*current)
    *current = timer->next;

  assert(!factory->head || (factory->head != factory->head->next));
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
  struct SoftwareTimer * const timer = object;
  struct SoftwareTimerFactory * const factory = timer->factory;
  const IrqState state = irqSave();

  if (timer->enabled)
    removeTimer(timer->factory, timer);

  timer->enabled = true;
  timer->timestamp = factory->counter + timer->overflow;
  insertTimer(factory, timer);

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *object)
{
  struct SoftwareTimer * const timer = object;

  if (timer->enabled)
  {
    timer->enabled = false;
    removeTimer(timer->factory, timer);
  }
}
/*----------------------------------------------------------------------------*/
static void tmrSetAutostop(void *object, bool state)
{
  struct SoftwareTimer * const timer = object;
  timer->continuous = !state;
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
  return timer->overflow;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *object, uint32_t overflow)
{
  struct SoftwareTimer * const timer = object;
  timer->overflow = overflow;
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
  const IrqState state = irqSave();

  const uint32_t current = distance(timer->factory->counter, timer->timestamp);
  timer->timestamp += current - value;

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
void *softwareTimerCreate(void *object)
{
  const struct SoftwareTimerConfig config = {
      .parent = object
  };
  return init(SoftwareTimer, &config);
}
