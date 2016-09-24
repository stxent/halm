/*
 * work_queue.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdbool.h>
#include <xcore/containers/queue.h>
#include <xcore/entity.h>
#include <xcore/memory.h>
#include <halm/generic/work_queue.h>
#include <halm/irq.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
struct SimpleWorkQueueConfig
{
  size_t size;
};
/*----------------------------------------------------------------------------*/
struct SimpleWorkQueue
{
  struct Entity base;

  struct Queue queue;

  bool event;
};
/*----------------------------------------------------------------------------*/
struct WorkDescriptor
{
  void (*callback)(void *);
  void *argument;
};
/*----------------------------------------------------------------------------*/
static enum result wqInit(void *, const void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass wqTable = {
    .size = sizeof(struct SimpleWorkQueue),
    .init = wqInit,
    .deinit = 0
};
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const SimpleWorkQueue = &wqTable;
static struct SimpleWorkQueue *wqHandler = 0;
/*----------------------------------------------------------------------------*/
enum result workQueueAdd(void (*callback)(void *), void *argument)
{
  assert(wqHandler != 0);

  if (!callback)
    return E_VALUE;
  if (queueFull(&wqHandler->queue))
    return E_FULL;

  const struct WorkDescriptor descriptor = {
      .callback = callback,
      .argument = argument
  };
  irqState state;

  state = irqSave();
  queuePush(&wqHandler->queue, &descriptor);
  wqHandler->event = true;
  irqRestore(state);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
enum result workQueueInit(size_t size)
{
  assert(wqHandler == 0);

  const struct SimpleWorkQueueConfig config = {
      .size = size
  };

  wqHandler = init(SimpleWorkQueue, &config);

  return wqHandler != 0 ? E_OK : E_ERROR;
}
/*----------------------------------------------------------------------------*/
void workQueueStart(void *argument __attribute__((unused)))
{
  assert(wqHandler != 0);

  while (1)
  {
    while (!wqHandler->event)
    {
      pmChangeState(PM_SLEEP);
      barrier();
    }
    wqHandler->event = false;

    while (!queueEmpty(&wqHandler->queue))
    {
      struct WorkDescriptor descriptor;
      irqState state;

      state = irqSave();
      queuePop(&wqHandler->queue, &descriptor);
      irqRestore(state);

      descriptor.callback(descriptor.argument);
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum result wqInit(void *object, const void *configBase)
{
  const struct SimpleWorkQueueConfig * const config = configBase;
  struct SimpleWorkQueue * const wq = object;
  enum result res;

  res = queueInit(&wq->queue, sizeof(struct WorkDescriptor), config->size);
  if (res != E_OK)
    return res;

  wq->event = false;

  return E_OK;
}
