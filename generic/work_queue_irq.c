/*
 * work_queue_irq.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/work_queue_defs.h>
#include <halm/generic/work_queue_irq.h>
/*----------------------------------------------------------------------------*/
struct WorkQueueIrq
{
  struct WorkQueue base;
  WqTaskQueue queue;
  IrqNumber irq;
};
/*----------------------------------------------------------------------------*/
static enum Result workQueueInit(void *, const void *);
static enum Result workQueueAdd(void *, void (*)(void *), void *);
static enum Result workQueueStart(void *);
static void workQueueStop(void *);
/*----------------------------------------------------------------------------*/
const struct WorkQueueClass * const WorkQueueIrq =
    &(const struct WorkQueueClass){
    .size = sizeof(struct WorkQueueIrq),
    .init = workQueueInit,
    .deinit = deletedDestructorTrap,

    .add = workQueueAdd,
    .start = workQueueStart,
    .stop = workQueueStop
};
/*----------------------------------------------------------------------------*/
static enum Result workQueueInit(void *object, const void *configBase)
{
  const struct WorkQueueIrqConfig * const config = configBase;
  assert(config);
  assert(config->size);

  struct WorkQueueIrq * const wq = object;

  irqSetPriority(config->irq, config->priority);
  wq->irq = config->irq;

  return wqTaskQueueInit(&wq->queue, config->size) ? E_OK : E_MEMORY;
}
/*----------------------------------------------------------------------------*/
static enum Result workQueueAdd(void *object, void (*callback)(void *),
    void *argument)
{
  assert(callback != 0);

  struct WorkQueueIrq * const wq = object;
  enum Result res;

  /* Critical section */
  const IrqState state = irqSave();

  if (!wqTaskQueueFull(&wq->queue))
  {
    wqTaskQueuePushBack(&wq->queue, (struct WqTask){callback, argument});
    irqSetPending(wq->irq);
    res = E_OK;
  }
  else
    res = E_FULL;

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result workQueueStart(void *object)
{
  struct WorkQueueIrq * const wq = object;

  irqEnable(wq->irq);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void workQueueStop(void *object)
{
  struct WorkQueueIrq * const wq = object;
  irqDisable(wq->irq);
}
/*----------------------------------------------------------------------------*/
void wqIrqUpdate(void *object)
{
  struct WorkQueueIrq * const wq = object;

  while (!wqTaskQueueEmpty(&wq->queue))
  {
    const struct WqTask task = wqTaskQueueFront(&wq->queue);

    /* Critical section */
    const IrqState state = irqSave();
    wqTaskQueuePopFront(&wq->queue);
    irqRestore(state);

    task.callback(task.argument);
  }
}
