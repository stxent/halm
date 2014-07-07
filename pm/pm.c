/*
 * pm.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
#include <entity.h>
#include <list.h>
#include <pm.h>
/*----------------------------------------------------------------------------*/
struct PmHandlerEntry
{
  void *object;
  PmCallback callback;
};
/*----------------------------------------------------------------------------*/
struct PmHandler
{
  struct Entity parent;

  struct List objectList;
};
/*----------------------------------------------------------------------------*/
static enum result pmHandlerInit(void *, const void *);
/*----------------------------------------------------------------------------*/
extern enum result pmCoreChangeState(enum pmState);
extern enum result pmPlatformChangeState(enum pmState);
/*----------------------------------------------------------------------------*/
static const struct EntityClass handlerTable = {
    .size = sizeof(struct PmHandler),
    .init = pmHandlerInit,
    .deinit = 0
};
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const PmHandler = &handlerTable;
static struct PmHandler *pmHandler = 0;
/*----------------------------------------------------------------------------*/
static enum result pmHandlerInit(void *object,
    const void *configPtr __attribute__((unused)))
{
  struct PmHandler * const handler = object;

  return listInit(&handler->objectList, sizeof(struct PmHandlerEntry));
}
/*----------------------------------------------------------------------------*/
enum result pmChangeState(enum pmState state)
{
  enum result res;

  if (pmHandler)
  {
    struct PmHandlerEntry entry;
    const void *current = listFirst(&pmHandler->objectList);

    while (current)
    {
      listData(&pmHandler->objectList, current, &entry);
      if ((res = entry.callback(entry.object, state)) != E_OK)
        return res;
      current = listNext(current);
    }
  }

  /* Prepare specific platform features before entering other state */
  if ((res = pmPlatformChangeState(state)) != E_OK)
    return res;

  /* Change current state */
  if ((res = pmCoreChangeState(state)) != E_OK)
    return res;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
enum result pmRegister(void *object, PmCallback callback)
{
  if (!pmHandler)
  {
    if (!(pmHandler = init(PmHandler, 0)))
      return E_ERROR;
  }

  const struct PmHandlerEntry entry = {
      .object = object,
      .callback = callback
  };

  return listPush(&pmHandler->objectList, &entry);
}
/*----------------------------------------------------------------------------*/
void pmUnregister(const void *object)
{
  if (!pmHandler)
    return;

  struct PmHandlerEntry entry;
  void *current = listFirst(&pmHandler->objectList);

  while (current)
  {
    listData(&pmHandler->objectList, current, &entry);
    if (entry.object == object)
    {
      listErase(&pmHandler->objectList, current);
      return;
    }
    current = listNext(current);
  }
}
