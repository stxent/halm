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
struct PmHandlerEntry
{
  void *object;
  void (*callback)(void *, enum PmState);
};

DEFINE_LIST(struct PmHandlerEntry, Pm, pm)

struct PmHandler
{
  struct Entity parent;
  PmList objectList;
};
/*----------------------------------------------------------------------------*/
static bool entryComparator(const void *, void *);
static void notifyHandlerEntries(enum PmState);
static enum Result pmHandlerInit(void *, const void *);
/*----------------------------------------------------------------------------*/
extern enum Result pmCoreChangeState(enum PmState);
extern enum Result pmPlatformChangeState(enum PmState);
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const PmHandler = &(const struct EntityClass){
    .size = sizeof(struct PmHandler),
    .init = pmHandlerInit,
    .deinit = deletedDestructorTrap
};
/*----------------------------------------------------------------------------*/
static struct PmHandler *pmHandler = 0;
/*----------------------------------------------------------------------------*/
static bool entryComparator(const void *element, void *argument)
{
  const struct PmHandlerEntry * const entry = element;
  return entry->object == argument;
}
/*----------------------------------------------------------------------------*/
static void notifyHandlerEntries(enum PmState state)
{
  if (!pmHandler)
    return;

  PmListNode *current = pmListFront(&pmHandler->objectList);

  while (current)
  {
    const struct PmHandlerEntry * const entry = pmListData(current);

    entry->callback(entry->object, state);
    current = pmListNext(current);
  }
}
/*----------------------------------------------------------------------------*/
static enum Result pmHandlerInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct PmHandler * const handler = object;

  pmListInit(&handler->objectList);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
void pmChangeState(enum PmState state)
{
  if (state != PM_SLEEP)
    notifyHandlerEntries(state);

  /* Prepare specific platform features before entering other states */
  pmPlatformChangeState(state);

  /* Change current state */
  pmCoreChangeState(state);
}
/*----------------------------------------------------------------------------*/
enum Result pmRegister(void (*callback)(void *, enum PmState), void *object)
{
  if (!pmHandler)
    pmHandler = init(PmHandler, 0);
  if (!pmHandler)
    return E_ERROR;

  return pmListPushFront(&pmHandler->objectList,
      (struct PmHandlerEntry){object, callback}) ? E_OK : E_MEMORY;
}
/*----------------------------------------------------------------------------*/
void pmUnregister(const void *object)
{
  assert(pmHandler);
  assert(pmListFindIf(&pmHandler->objectList, (void *)object, entryComparator));

  pmListEraseIf(&pmHandler->objectList, (void *)object, entryComparator);
}
