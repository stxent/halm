/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <xcore/containers/list.h>
#include <xcore/entity.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
struct PmHandlerEntry
{
  void *object;
  void (*callback)(void *, enum PmState);
};
/*----------------------------------------------------------------------------*/
struct PmHandler
{
  struct Entity parent;

  struct List objectList;
};
/*----------------------------------------------------------------------------*/
static void notifyHandlerEntries(enum PmState);
static enum Result pmHandlerInit(void *, const void *);
/*----------------------------------------------------------------------------*/
extern enum Result pmCoreChangeState(enum PmState);
extern enum Result pmPlatformChangeState(enum PmState);
/*----------------------------------------------------------------------------*/
static const struct EntityClass handlerTable = {
    .size = sizeof(struct PmHandler),
    .init = pmHandlerInit,
    .deinit = deletedDestructorTrap
};
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const PmHandler = &handlerTable;
static struct PmHandler *pmHandler = 0;
/*----------------------------------------------------------------------------*/
static void notifyHandlerEntries(enum PmState state)
{
  if (!pmHandler)
    return;

  const struct ListNode *current = listFirst(&pmHandler->objectList);
  struct PmHandlerEntry entry;

  while (current)
  {
    listData(&pmHandler->objectList, current, &entry);
    entry.callback(entry.object, state);
    current = listNext(current);
  }
}
/*----------------------------------------------------------------------------*/
static enum Result pmHandlerInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct PmHandler * const handler = object;
  return listInit(&handler->objectList, sizeof(struct PmHandlerEntry));
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

  const struct PmHandlerEntry entry = {
      .object = object,
      .callback = callback
  };

  return listPush(&pmHandler->objectList, &entry);
}
/*----------------------------------------------------------------------------*/
void pmUnregister(const void *object)
{
  assert(pmHandler);

  struct ListNode *current = listFirst(&pmHandler->objectList);
  struct PmHandlerEntry entry;

  while (current)
  {
    listData(&pmHandler->objectList, current, &entry);
    if (entry.object == object)
      break;
    current = listNext(current);
  }
  assert(current);

  listErase(&pmHandler->objectList, current);
}
