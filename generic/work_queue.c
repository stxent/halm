/*
 * work_queue.c
 * Copyright (C) 2016, 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/work_queue.h>
#include <halm/irq.h>
#include <halm/pm.h>
#include <xcore/asm.h>
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

#ifdef CONFIG_GENERIC_WQ_PROFILE
  struct WqTaskDescriptor *info;
  WqCounter timestamp;
#endif
};

DEFINE_ARRAY(struct WqTaskDescriptor, WqInfo, wqInfo)
DEFINE_QUEUE(struct WqTask, WqTask, wqTask)

struct WorkQueueDefault
{
  struct WorkQueue base;
  WqTaskQueue tasks;

#ifndef CONFIG_GENERIC_WQ_NONSTOP
  bool stop;
#endif

#ifdef CONFIG_GENERIC_WQ_PROFILE
  size_t watermark;
  WqCounter timestamp;

  struct
  {
    WqCounter max;
    WqCounter min;
  } latency;

  WqInfoArray info;
#endif
};
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_GENERIC_WQ_PROFILE
struct WqTaskDescriptor *findTaskInfo(struct WorkQueueDefault *,
    void (*)(void *));
#endif
/*----------------------------------------------------------------------------*/
static enum Result workQueueInit(void *, const void *);
static enum Result workQueueAdd(void *, void (*)(void *), void *);
static enum Result workQueueStart(void *);

#ifdef CONFIG_GENERIC_WQ_PROFILE
  static void workQueueProfile(void *, WqProfileCallback, void *);
  static void workQueueStatistics(void *, struct WqInfo *);
#else
  #define workQueueProfile 0
  #define workQueueStatistics 0
#endif

#ifndef CONFIG_GENERIC_WQ_NONSTOP
  static void workQueueDeinit(void *);
  static void workQueueStop(void *);
  #define WQ_RUNNING(object) ((object)->stop == false)
#else
  #define workQueueDeinit deletedDestructorTrap
  #define workQueueStop 0
  #define WQ_RUNNING(object) (true)
#endif
/*----------------------------------------------------------------------------*/
const struct WorkQueueClass * const WorkQueue =
    &(const struct WorkQueueClass){
    .size = sizeof(struct WorkQueueDefault),
    .init = workQueueInit,
    .deinit = workQueueDeinit,

    .add = workQueueAdd,
    .profile = workQueueProfile,
    .statistics = workQueueStatistics,
    .start = workQueueStart,
    .stop = workQueueStop
};
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_GENERIC_WQ_PROFILE
struct WqTaskDescriptor *findTaskInfo(struct WorkQueueDefault *wq,
    void (*task)(void *))
{
  for (size_t index = 0; index < wqInfoArraySize(&wq->info); ++index)
  {
    struct WqTaskDescriptor * const current = wqInfoArrayAt(&wq->info, index);

    if (current->task == task)
      return current;
  }

  return 0;
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result workQueueInit(void *object, const void *configBase)
{
  const struct WorkQueueConfig * const config = configBase;
  assert(config);
  assert(config->size);

  struct WorkQueueDefault * const wq = object;

#ifdef CONFIG_GENERIC_WQ_PROFILE
  if (!wqInfoArrayInit(&wq->info, config->size))
    return E_MEMORY;
#endif

  if (!wqTaskQueueInit(&wq->tasks, config->size))
    return E_MEMORY;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_GENERIC_WQ_NONSTOP
static void workQueueDeinit(void *object)
{
  struct WorkQueueDefault * const wq = object;

  wqStop(wq);
  wqTaskQueueDeinit(&wq->tasks);

#ifdef CONFIG_GENERIC_WQ_PROFILE
  wqInfoArrayDeinit(&wq->info);
#endif /* CONFIG_GENERIC_WQ_PROFILE */
}
#endif /* CONFIG_GENERIC_WQ_NONSTOP */
/*----------------------------------------------------------------------------*/
static enum Result workQueueAdd(void *object, void (*callback)(void *),
    void *argument)
{
  assert(callback != 0);

  struct WorkQueueDefault * const wq = object;
  const IrqState state = irqSave();
  enum Result res;

  if (!wqTaskQueueFull(&wq->tasks))
  {
#ifdef CONFIG_GENERIC_WQ_PROFILE
    const size_t watermark = wqTaskQueueSize(&wq->tasks) + 1;
    struct WqTaskDescriptor *entry = findTaskInfo(wq, callback);

    if (!entry)
    {
      const struct WqTaskDescriptor info = {
          .task = callback,
          .count = 0,
          .execution = {0, (WqCounter)-1, 0}
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
    res = E_OK;
  }
  else
    res = E_FULL;

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_GENERIC_WQ_PROFILE
static void workQueueProfile(void *object, WqProfileCallback callback,
    void *argument)
{
  struct WorkQueueDefault * const wq = object;

  for (size_t index = 0; index < wqInfoArraySize(&wq->info); ++index)
  {
    struct WqTaskDescriptor * const entry = wqInfoArrayAt(&wq->info, index);
    const IrqState state = irqSave();

    const struct WqTaskInfo info = {
        .task = entry->task,
        .count = entry->count,
        .execution = {
            entry->execution.max,
            entry->count ? entry->execution.min : 0,
            entry->execution.total
        }
    };

    irqRestore(state);
    callback(argument, &info);
  }
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_GENERIC_WQ_PROFILE
static void workQueueStatistics(void *object, struct WqInfo *statistics)
{
  struct WorkQueueDefault * const wq = object;
  const IrqState state = irqSave();

  statistics->watermark = wq->watermark;
  statistics->uptime = wqGetTime() - wq->timestamp;
  statistics->latency.max = wq->latency.max;
  statistics->latency.min = wq->latency.min;

  irqRestore(state);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result workQueueStart(void *object)
{
  struct WorkQueueDefault * const wq = object;
  IrqState state;

#ifndef CONFIG_GENERIC_WQ_NONSTOP
  wq->stop = false;
#endif

#ifdef CONFIG_GENERIC_WQ_PROFILE
  state = irqSave();

  wqInfoArrayClear(&wq->info);
  wq->watermark = 0;
  wq->latency.max = 0;
  wq->latency.min = (WqCounter)-1;
  wq->timestamp = wqGetTime();

  irqRestore(state);
#endif

  while (WQ_RUNNING(wq))
  {
#ifdef CONFIG_GENERIC_WQ_PM
    /*
     * Disable interrupts to avoid entering sleep mode when interrupt is fired
     * between size comparison and sleep instruction.
     */
    state = irqSave();
    if (wqTaskQueueEmpty(&wq->tasks))
      pmChangeState(PM_SLEEP);
    irqRestore(state);
#endif

    /* Reload queue size */
    barrier();

    while (!wqTaskQueueEmpty(&wq->tasks))
    {
      const struct WqTask task = wqTaskQueueFront(&wq->tasks);

#ifdef CONFIG_GENERIC_WQ_PROFILE
      const WqCounter begin = wqGetTime();
#endif

      /* Critical section begin */
      state = irqSave();
      wqTaskQueuePopFront(&wq->tasks);
      irqRestore(state);
      /* Critical section end */

      task.callback(task.argument);

#ifdef CONFIG_GENERIC_WQ_PROFILE
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

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_GENERIC_WQ_NONSTOP
static void workQueueStop(void *object)
{
  struct WorkQueueDefault * const wq = object;
  wq->stop = true;
}
#endif
