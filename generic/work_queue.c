/*
 * work_queue.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/generic/work_queue.h>
#include <halm/generic/work_queue_defs.h>
#include <halm/irq.h>
#include <halm/pm.h>
#include <xcore/asm.h>
/*----------------------------------------------------------------------------*/
struct WorkQueue
{
  struct WorkQueueBase base;
  WqTaskQueue queue;
};
/*----------------------------------------------------------------------------*/
static enum Result workQueueInit(void *, const void *);
static enum Result workQueueAdd(void *, void (*)(void *), void *);
static enum Result workQueueStart(void *);
/*----------------------------------------------------------------------------*/
const struct WorkQueueClass * const WorkQueue =
    &(const struct WorkQueueClass){
    .size = sizeof(struct WorkQueue),
    .init = workQueueInit,
    .deinit = deletedDestructorTrap,

    .add = workQueueAdd,
    .start = workQueueStart,
    .stop = 0
};
/*----------------------------------------------------------------------------*/
static enum Result workQueueInit(void *object, const void *configBase)
{
  const struct WorkQueueConfig * const config = configBase;
  assert(config);
  assert(config->size);

  struct WorkQueue * const wq = object;
  return wqTaskQueueInit(&wq->queue, config->size) ? E_OK : E_MEMORY;
}
/*----------------------------------------------------------------------------*/
static enum Result workQueueAdd(void *object, void (*callback)(void *),
    void *argument)
{
  assert(callback != 0);

  struct WorkQueue * const wq = object;
  enum Result res;

  /* Critical section */
  const IrqState state = irqSave();

  if (!wqTaskQueueFull(&wq->queue))
  {
    wqTaskQueuePushBack(&wq->queue, (struct WqTask){callback, argument});
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
  struct WorkQueue * const wq = object;

  while (1)
  {
    IrqState state;

#ifdef CONFIG_GENERIC_WQ_PM
    /*
     * Disable interrupts to avoid entering sleep mode when interrupt is fired
     * between size comparison and sleep instruction.
     */
    state = irqSave();
    if (wqTaskQueueEmpty(&wq->queue))
      pmChangeState(PM_SLEEP);
    irqRestore(state);
#endif

    /* Reload queue size */
    barrier();

    while (!wqTaskQueueEmpty(&wq->queue))
    {
      const struct WqTask task = wqTaskQueueFront(&wq->queue);

      /* Critical section */
      state = irqSave();
      wqTaskQueuePopFront(&wq->queue);
      irqRestore(state);

      task.callback(task.argument);
    }
  }

  return E_OK;
}
