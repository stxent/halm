/*
 * work_queue_irq.c
 * Copyright (C) 2020, 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/work_queue_irq.h>
#include <xcore/containers/tg_array.h>
#include <xcore/containers/tg_queue.h>
/*----------------------------------------------------------------------------*/
struct WqTaskDescriptor
{
  void (*task)(void *);
  WqCounter count;

  struct
  {
    WqCounter max;
    WqCounter min;
    WqCounter total;
  } execution;
};

struct WqTask
{
  void (*callback)(void *);
  void *argument;

#ifdef CONFIG_GENERIC_WQ_IRQ_PROFILE
  struct WqTaskDescriptor *info;
  WqCounter timestamp;
#endif
};

DEFINE_ARRAY(struct WqTaskDescriptor, WqInfo, wqInfo)
DEFINE_QUEUE(struct WqTask, WqTask, wqTask)

struct WorkQueueIrq
{
  struct WorkQueue base;

  WqTaskQueue tasks;
  IrqNumber irq;

#ifdef CONFIG_GENERIC_WQ_IRQ_PROFILE
  struct
  {
    WqCounter max;
    WqCounter min;
  } latency;

  WqCounter timestamp;
  WqInfoArray info;

  size_t watermark;
#endif
};
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_GENERIC_WQ_IRQ_PROFILE
static struct WqTaskDescriptor *findTaskInfo(struct WorkQueueIrq *,
    void (*)(void *));
#endif
/*----------------------------------------------------------------------------*/
static enum Result workQueueInit(void *, const void *);
static enum Result workQueueAdd(void *, void (*)(void *), void *);
static enum Result workQueueStart(void *);

#ifdef CONFIG_GENERIC_WQ_IRQ_PROFILE
  static void workQueueProfile(void *, WqProfileCallback, void *);
  static void workQueueStatistics(void *, struct WqInfo *);
#else
#  define workQueueProfile NULL
#  define workQueueStatistics NULL
#endif

#ifndef CONFIG_GENERIC_WQ_IRQ_NONSTOP
  static void workQueueDeinit(void *);
  static void workQueueStop(void *);
#else
#  define workQueueDeinit deletedDestructorTrap
#  define workQueueStop NULL
#endif
/*----------------------------------------------------------------------------*/
const struct WorkQueueClass * const WorkQueueIrq =
    &(const struct WorkQueueClass){
    .size = sizeof(struct WorkQueueIrq),
    .init = workQueueInit,
    .deinit = workQueueDeinit,

    .add = workQueueAdd,
    .profile = workQueueProfile,
    .statistics = workQueueStatistics,
    .start = workQueueStart,
    .stop = workQueueStop
};
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_GENERIC_WQ_IRQ_PROFILE
static struct WqTaskDescriptor *findTaskInfo(struct WorkQueueIrq *wq,
    void (*task)(void *))
{
  for (size_t index = 0; index < wqInfoArraySize(&wq->info); ++index)
  {
    struct WqTaskDescriptor * const current = wqInfoArrayAt(&wq->info, index);

    if (current->task == task)
      return current;
  }

  return NULL;
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result workQueueInit(void *object, const void *configBase)
{
  const struct WorkQueueIrqConfig * const config = configBase;
  assert(config != NULL);
  assert(config->size);

  struct WorkQueueIrq * const wq = object;

  irqSetPriority(config->irq, config->priority);
  wq->irq = config->irq;

#ifdef CONFIG_GENERIC_WQ_IRQ_PROFILE
  if (!wqInfoArrayInit(&wq->info, config->size))
    return E_MEMORY;
#endif

  if (!wqTaskQueueInit(&wq->tasks, config->size))
    return E_MEMORY;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_GENERIC_WQ_IRQ_NONSTOP
static void workQueueDeinit(void *object)
{
  struct WorkQueueIrq * const wq = object;

  wqStop(wq);
  wqTaskQueueDeinit(&wq->tasks);

#ifdef CONFIG_GENERIC_WQ_IRQ_PROFILE
  wqInfoArrayDeinit(&wq->info);
#endif /* CONFIG_GENERIC_WQ_IRQ_PROFILE */
}
#endif /* CONFIG_GENERIC_WQ_IRQ_NONSTOP */
/*----------------------------------------------------------------------------*/
static enum Result workQueueAdd(void *object, void (*callback)(void *),
    void *argument)
{
  assert(callback != NULL);

  struct WorkQueueIrq * const wq = object;
  const IrqState state = irqSave();
  enum Result res;

  if (!wqTaskQueueFull(&wq->tasks))
  {
#ifdef CONFIG_GENERIC_WQ_IRQ_PROFILE
    const size_t watermark = wqTaskQueueSize(&wq->tasks) + 1;
    struct WqTaskDescriptor *entry = findTaskInfo(wq, callback);

    if (entry == NULL)
    {
      const struct WqTaskDescriptor info = {
          .task = callback,
          .count = 0,
          .execution = {0, WQ_COUNTER_MAX, 0}
      };

      wqInfoArrayPushBack(&wq->info, info);
      entry = wqInfoArrayAt(&wq->info, wqInfoArraySize(&wq->info) - 1);
    }

    if (wq->watermark < watermark)
      wq->watermark = watermark;

    const struct WqTask task = {
        .callback = callback,
        .argument = argument,
        .info = entry,
        .timestamp = wqGetTime()
    };
#else
    const struct WqTask task = {
        .callback = callback,
        .argument = argument
    };
#endif

    wqTaskQueuePushBack(&wq->tasks, task);
    irqSetPending(wq->irq);
    res = E_OK;
  }
  else
    res = E_FULL;

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_GENERIC_WQ_IRQ_PROFILE
static void workQueueProfile(void *object, WqProfileCallback callback,
    void *argument)
{
  struct WorkQueueIrq * const wq = object;

  for (size_t index = 0; index < wqInfoArraySize(&wq->info); ++index)
  {
    struct WqTaskDescriptor * const entry = wqInfoArrayAt(&wq->info, index);
    const IrqState state = irqSave();

    const struct WqTaskInfo info = {
        .task = entry->task,
        .count = entry->count,
        .execution = {
            entry->execution.max,
            entry->execution.min != WQ_COUNTER_MAX ? entry->execution.min : 0,
            entry->execution.total
        }
    };

    irqRestore(state);
    callback(argument, &info);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result workQueueStart(void *object)
{
  struct WorkQueueIrq * const wq = object;

#ifdef CONFIG_GENERIC_WQ_IRQ_PROFILE
  const IrqState state = irqSave();

  wqInfoArrayClear(&wq->info);
  wq->latency.max = 0;
  wq->latency.min = WQ_COUNTER_MAX;
  wq->watermark = 0;
  wq->timestamp = wqGetTime();

  irqRestore(state);
#endif

  irqEnable(wq->irq);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_GENERIC_WQ_IRQ_PROFILE
static void workQueueStatistics(void *object, struct WqInfo *statistics)
{
  struct WorkQueueIrq * const wq = object;
  const IrqState state = irqSave();

  statistics->watermark = wq->watermark;
  statistics->uptime = wqGetTime() - wq->timestamp;
  statistics->latency.max = wq->latency.max;
  statistics->latency.min = wq->latency.min != WQ_COUNTER_MAX ?
      wq->latency.min : 0;

  irqRestore(state);
}
#endif
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_GENERIC_WQ_IRQ_NONSTOP
static void workQueueStop(void *object)
{
  struct WorkQueueIrq * const wq = object;
  irqDisable(wq->irq);
}
#endif
/*----------------------------------------------------------------------------*/
void wqIrqUpdate(void *object)
{
  struct WorkQueueIrq * const wq = object;

  while (!wqTaskQueueEmpty(&wq->tasks))
  {
    const struct WqTask task = wqTaskQueueFront(&wq->tasks);
    IrqState state;

#ifdef CONFIG_GENERIC_WQ_IRQ_PROFILE
    const WqCounter begin = wqGetTime();
#endif

    /* Critical section begin */
    state = irqSave();
    wqTaskQueuePopFront(&wq->tasks);
    irqRestore(state);
    /* Critical section end */

    task.callback(task.argument);

#ifdef CONFIG_GENERIC_WQ_IRQ_PROFILE
    const WqCounter end = wqGetTime();
    const WqCounter execution = end - begin;
    const WqCounter latency = begin - task.timestamp;

    /* Critical section begin */
    state = irqSave();

    if (wq->latency.min > latency)
      wq->latency.min = latency;
    if (wq->latency.max < latency)
      wq->latency.max = latency;

    if (task.info->execution.min > execution)
      task.info->execution.min = execution;
    if (task.info->execution.max < execution)
      task.info->execution.max = execution;

    task.info->execution.total += execution;
    ++task.info->count;

    irqRestore(state);
    /* Critical section end */
#endif
  }
}
