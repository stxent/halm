/*
 * work_queue.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <xcore/asm.h>
#include <xcore/containers/tg_queue.h>
#include <xcore/entity.h>
#include <halm/generic/work_queue.h>
#include <halm/irq.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
struct Task
{
  void (*callback)(void *);
  void *argument;
};

DEFINE_QUEUE(struct Task, Task, task)

struct SimpleWorkQueueConfig
{
  size_t size;
};

struct SimpleWorkQueue
{
  struct Entity base;

  TaskQueue queue;
  bool event;
};
/*----------------------------------------------------------------------------*/
static enum Result wqInit(void *, const void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const SimpleWorkQueue =
    &(const struct EntityClass){
    .size = sizeof(struct SimpleWorkQueue),
    .init = wqInit,
    .deinit = deletedDestructorTrap
};
/*----------------------------------------------------------------------------*/
static struct SimpleWorkQueue *instance = 0;
/*----------------------------------------------------------------------------*/
enum Result workQueueAdd(void (*callback)(void *), void *argument)
{
  assert(instance != 0);

  if (!callback)
    return E_VALUE;

  /* Critical section */
  const IrqState state = irqSave();
  enum Result res;

  if (!taskQueueFull(&instance->queue))
  {
    taskQueuePushBack(&instance->queue, (struct Task){callback, argument});
    instance->event = true;
    res = E_OK;
  }
  else
    res = E_FULL;

  irqRestore(state);
  return res;
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

    while (!taskQueueEmpty(&instance->queue))
    {
      /* Critical section */
      const IrqState state = irqSave();
      const struct Task task = taskQueueFront(&instance->queue);
      taskQueuePopFront(&instance->queue);
      irqRestore(state);

      task.callback(task.argument);
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum Result wqInit(void *object, const void *configBase)
{
  const struct SimpleWorkQueueConfig * const config = configBase;
  assert(config);

  struct SimpleWorkQueue * const wq = object;

  if (taskQueueInit(&wq->queue, config->size))
  {
    wq->event = false;
    return E_OK;
  }
  else
    return E_MEMORY;
}
