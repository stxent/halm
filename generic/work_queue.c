/*
 * work_queue.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdbool.h>
#include <xcore/asm.h>
#include <xcore/containers/queue.h>
#include <xcore/entity.h>
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
static enum Result wqInit(void *, const void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass wqTable = {
    .size = sizeof(struct SimpleWorkQueue),
    .init = wqInit,
    .deinit = deletedDestructorTrap
};
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const SimpleWorkQueue = &wqTable;
static struct SimpleWorkQueue *instance = 0;
/*----------------------------------------------------------------------------*/
enum Result workQueueAdd(void (*callback)(void *), void *argument)
{
  assert(instance != 0);

  if (!callback)
    return E_VALUE;
  if (queueFull(&instance->queue))
    return E_FULL;

  const struct WorkDescriptor descriptor = {
      .callback = callback,
      .argument = argument
  };

  /* Critical section */
  const IrqState state = irqSave();
  queuePush(&instance->queue, &descriptor);
  instance->event = true;
  irqRestore(state);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
enum Result workQueueInit(size_t size)
{
  assert(instance == 0);

  const struct SimpleWorkQueueConfig config = {
      .size = size
  };
  instance = init(SimpleWorkQueue, &config);

  return instance != 0 ? E_OK : E_ERROR;
}
/*----------------------------------------------------------------------------*/
void workQueueStart(void *argument __attribute__((unused)))
{
  assert(instance != 0);

  while (1)
  {
    while (!instance->event)
    {
      pmChangeState(PM_SLEEP);
      barrier();
    }
    instance->event = false;

    while (!queueEmpty(&instance->queue))
    {
      struct WorkDescriptor descriptor;

      /* Critical section */
      const IrqState state = irqSave();
      queuePop(&instance->queue, &descriptor);
      irqRestore(state);

      descriptor.callback(descriptor.argument);
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum Result wqInit(void *object, const void *configBase)
{
  const struct SimpleWorkQueueConfig * const config = configBase;
  assert(config);

  struct SimpleWorkQueue * const wq = object;
  const enum Result res = queueInit(&wq->queue,
      sizeof(struct WorkDescriptor), config->size);

  if (res == E_OK)
    wq->event = false;

  return res;
}
