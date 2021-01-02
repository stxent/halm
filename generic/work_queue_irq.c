/*
 * work_queue_irq.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/work_queue_irq.h>
#include <xcore/containers/tg_queue.h>
/*----------------------------------------------------------------------------*/
struct WqTask
{
  void (*callback)(void *);
  void *argument;
};

DEFINE_QUEUE(struct WqTask, WqTask, wqTask)

struct WorkQueueIrq
{
  struct WorkQueue base;
  WqTaskQueue tasks;
  IrqNumber irq;
};
/*----------------------------------------------------------------------------*/
static enum Result workQueueInit(void *, const void *);
static void workQueueDeinit(void *);
static enum Result workQueueAdd(void *, void (*)(void *), void *);
static enum Result workQueueStart(void *);
static void workQueueStop(void *);
/*----------------------------------------------------------------------------*/
const struct WorkQueueClass * const WorkQueueIrq =
    &(const struct WorkQueueClass){
    .size = sizeof(struct WorkQueueIrq),
    .init = workQueueInit,
    .deinit = workQueueDeinit,

    .add = workQueueAdd,
    .profile = 0,
    .statistics = 0,
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

  return wqTaskQueueInit(&wq->tasks, config->size) ? E_OK : E_MEMORY;
}
/*----------------------------------------------------------------------------*/
static void workQueueDeinit(void *object)
{
  struct WorkQueueIrq * const wq = object;

  wqStop(wq);
  wqTaskQueueDeinit(&wq->tasks);
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

  if (!wqTaskQueueFull(&wq->tasks))
  {
    wqTaskQueuePushBack(&wq->tasks, (struct WqTask){callback, argument});
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

  while (!wqTaskQueueEmpty(&wq->tasks))
  {
    const struct WqTask task = wqTaskQueueFront(&wq->tasks);

    /* Critical section */
    const IrqState state = irqSave();
    wqTaskQueuePopFront(&wq->tasks);
    irqRestore(state);

    task.callback(task.argument);
  }
}
