/*
 * event_router.c
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/irq.h>
#include <halm/platform/platform_defs.h>
#include <xcore/containers/tg_list.h>
#include <xcore/entity.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct EventRouterObserver
{
  void *object;
  void (*callback)(void *);
  uint32_t events;
};

DEFINE_LIST(struct EventRouterObserver, Ero, ero)

struct EventRouter
{
  struct Entity base;

  /* List of observers */
  EroList observers;
};
/*----------------------------------------------------------------------------*/
static bool entryComparator(const void *, void *);
static void notifyObservers(uint32_t);

static enum Result erInit(void *, const void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const EventRouter =
    &(const struct EntityClass){
    .size = sizeof(struct EventRouter),
    .init = erInit,
    .deinit = deletedDestructorTrap
};
/*----------------------------------------------------------------------------*/
static struct EventRouter *erInstance = 0;
/*----------------------------------------------------------------------------*/
static bool entryComparator(const void *element, void *argument)
{
  const struct EventRouterObserver * const entry = element;
  return entry->object == argument;
}
/*----------------------------------------------------------------------------*/
static void notifyObservers(uint32_t status)
{
  if (!erInstance)
    return;

  EroListNode *current = eroListFront(&erInstance->observers);

  while (current)
  {
    const struct EventRouterObserver * const entry = eroListData(current);

    if (entry->events & status)
      entry->callback(entry->object);
    current = eroListNext(current);
  }
}
/*----------------------------------------------------------------------------*/
void EVENTROUTER_ISR(void)
{
  const uint32_t status = LPC_EVENTROUTER->STATUS;

  LPC_EVENTROUTER->CLR_STAT = status;
  notifyObservers(status);
}
/*----------------------------------------------------------------------------*/
static enum Result erInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct EventRouter * const handler = object;

  eroListInit(&handler->observers);

  irqSetPriority(EVENTROUTER_IRQ, CONFIG_PLATFORM_LPC_EVENT_ROUTER_PRIORITY);
  irqEnable(EVENTROUTER_IRQ);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
enum Result erRegister(void (*callback)(void *), void *object, uint32_t events)
{
  if (events == 0)
    return E_VALUE;

  if (!erInstance)
    erInstance = init(EventRouter, 0);
  if (!erInstance)
    return E_ERROR;

  LPC_EVENTROUTER->CLR_STAT = events & ~LPC_EVENTROUTER->ENABLE;
  LPC_EVENTROUTER->SET_EN = events;

  LPC_EVENTROUTER->HILO |= events;
  LPC_EVENTROUTER->EDGE |= events;

  return eroListPushFront(&erInstance->observers,
      (struct EventRouterObserver){object, callback, events}) ? E_OK : E_MEMORY;
}
/*----------------------------------------------------------------------------*/
void erUnregister(const void *object)
{
  assert(erInstance);

  EroListNode *current = eroListFront(&erInstance->observers);
  uint32_t others = 0;
  uint32_t target = 0;

  while (current)
  {
    const struct EventRouterObserver * const entry = eroListData(current);

    if (entry->object != object)
      others |= entry->events;
    else
      target = entry->events;

    current = eroListNext(current);
  }
  /* The entry associated with the object should exist */
  assert(target != 0);

  LPC_EVENTROUTER->CLR_EN = target & ~others;
  eroListEraseIf(&erInstance->observers, (void *)object, entryComparator);
}
