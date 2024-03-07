/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/pm.h>
#include <xcore/containers/tg_list.h>
#include <xcore/entity.h>
#include <assert.h>
#include <stdlib.h>
/*----------------------------------------------------------------------------*/
struct PowerManagerObserver
{
  void *object;
  void (*callback)(void *, enum PmState);
};

DEFINE_LIST(struct PowerManagerObserver, Pmo, pmo)

struct PowerManager
{
  struct Entity parent;

  /* List of power event observers */
  PmoList observers;
};
/*----------------------------------------------------------------------------*/
static bool entryComparator(const void *, void *);
static void notifyObservers(enum PmState);

static enum Result pmInit(void *, const void *);
/*----------------------------------------------------------------------------*/
extern enum Result pmCoreChangeState(enum PmState);
extern enum Result pmPlatformChangeState(enum PmState);
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const PowerManager =
    &(const struct EntityClass){
    .size = sizeof(struct PowerManager),
    .init = pmInit,
    .deinit = deletedDestructorTrap
};
/*----------------------------------------------------------------------------*/
static struct PowerManager *pmInstance = NULL;
/*----------------------------------------------------------------------------*/
static bool entryComparator(const void *element, void *argument)
{
  const struct PowerManagerObserver * const entry = element;
  return entry->object == argument;
}
/*----------------------------------------------------------------------------*/
static void notifyObservers(enum PmState state)
{
  if (pmInstance == NULL)
    return;

  PmoListNode *current = pmoListFront(&pmInstance->observers);

  while (current != NULL)
  {
    const struct PowerManagerObserver * const entry = pmoListData(current);

    entry->callback(entry->object, state);
    current = pmoListNext(current);
  }
}
/*----------------------------------------------------------------------------*/
static enum Result pmInit(void *object, const void *)
{
  struct PowerManager * const handler = object;

  pmoListInit(&handler->observers);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
void pmChangeState(enum PmState state)
{
  if (state != PM_SLEEP)
    notifyObservers(state);

  /* Prepare specific platform features before entering other states */
  pmPlatformChangeState(state);

  /* Change current state */
  pmCoreChangeState(state);
}
/*----------------------------------------------------------------------------*/
enum Result pmRegister(void (*callback)(void *, enum PmState), void *object)
{
  if (pmInstance == NULL)
    pmInstance = init(PowerManager, NULL);
  if (pmInstance == NULL)
    return E_ERROR;

  return pmoListPushFront(&pmInstance->observers,
      (struct PowerManagerObserver){object, callback}) ? E_OK : E_MEMORY;
}
/*----------------------------------------------------------------------------*/
void pmUnregister(const void *object)
{
  assert(pmInstance != NULL);
  assert(pmoListFindIf(&pmInstance->observers,
      (void *)object, entryComparator) != NULL);

  pmoListEraseIf(&pmInstance->observers, (void *)object, entryComparator);
}
