/*
 * work_queue_unique.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/work_queue.h>
#include <halm/irq.h>
#include <halm/pm.h>
#include <xcore/asm.h>
#include <xcore/containers/tg_array.h>
#include <xcore/containers/tg_queue.h>
/*----------------------------------------------------------------------------*/
struct WqTask
{
  void (*callback)(void *);
  void *argument;
};

struct WqTaskBucket
{
  struct WqTask task;

#ifdef CONFIG_GENERIC_WQ_UNIQUE_PROFILE
  WqCounter count;
  WqCounter timestamp;

  struct
  {
    WqCounter max;
    WqCounter min;
    WqCounter total;
  } execution;
#endif

  bool pending;
};

DEFINE_ARRAY(struct WqTaskBucket *, WqTask, wqTask)
DEFINE_QUEUE(struct WqTaskBucket *, WqTask, wqTask)

struct WorkQueueUnique
{
  struct WorkQueue base;

  struct WqTaskBucket *pool;
  WqTaskArray buckets;
  WqTaskQueue tasks;

#ifdef CONFIG_GENERIC_WQ_UNIQUE_PROFILE
  struct
  {
    WqCounter max;
    WqCounter min;
  } latency;

  WqCounter timestamp;
#endif

#ifdef CONFIG_GENERIC_WQ_UNIQUE_LOAD
  WqCounter loops;
  WqCounter previous;
#endif

#ifndef CONFIG_GENERIC_WQ_UNIQUE_NONSTOP
  bool stop;
#endif
};
/*----------------------------------------------------------------------------*/
static bool findTaskBucket(struct WorkQueueUnique *, const struct WqTask *,
    size_t *);
static int taskComparator(const struct WqTask *, const struct WqTask *);
/*----------------------------------------------------------------------------*/
static enum Result workQueueInit(void *, const void *);
static enum Result workQueueAdd(void *, void (*)(void *), void *);
static enum Result workQueueStart(void *);

#if defined(CONFIG_GENERIC_WQ_UNIQUE_PROFILE) \
    || defined(CONFIG_GENERIC_WQ_UNIQUE_LOAD)
  static void workQueueStatistics(void *, struct WqInfo *);
#else
#  define workQueueStatistics NULL
#endif

#ifdef CONFIG_GENERIC_WQ_UNIQUE_PROFILE
  static void workQueueProfile(void *, WqProfileCallback, void *);
#else
#  define workQueueProfile NULL
#endif

#ifndef CONFIG_GENERIC_WQ_UNIQUE_NONSTOP
  static void workQueueDeinit(void *);
  static void workQueueStop(void *);
#  define WQ_RUNNING(object) ((object)->stop == false)
#else
#  define workQueueDeinit deletedDestructorTrap
#  define workQueueStop NULL
#  define WQ_RUNNING(object) (true)
#endif
/*----------------------------------------------------------------------------*/
const struct WorkQueueClass * const WorkQueueUnique =
    &(const struct WorkQueueClass){
    .size = sizeof(struct WorkQueueUnique),
    .init = workQueueInit,
    .deinit = workQueueDeinit,

    .add = workQueueAdd,
    .profile = workQueueProfile,
    .statistics = workQueueStatistics,
    .start = workQueueStart,
    .stop = workQueueStop
};
/*----------------------------------------------------------------------------*/
static bool findTaskBucket(struct WorkQueueUnique *wq,
    const struct WqTask *task, size_t *index)
{
  WqTaskArray * const buckets = &wq->buckets;
  size_t left = 0;
  size_t right = wqTaskArraySize(buckets);

  while (left != right)
  {
    const size_t middle = (left + right) / 2;
    const struct WqTaskBucket * const current = *wqTaskArrayAt(buckets, middle);
    const int result = taskComparator(&current->task, task);

    if (result > 0)
    {
      right = middle;
    }
    else if (result < 0)
    {
      left = middle + 1;
    }
    else
    {
      *index = middle;
      return true;
    }
  }

  *index = left;
  return false;
}
/*----------------------------------------------------------------------------*/
static int taskComparator(const struct WqTask *a, const struct WqTask *b)
{
  if ((uintptr_t)a->callback > (uintptr_t)b->callback)
    return 1;
  if ((uintptr_t)a->callback < (uintptr_t)b->callback)
    return -1;

  if (a->argument == b->argument)
    return 0;
  else
    return ((uintptr_t)a->argument > (uintptr_t)b->argument) ? 1 : -1;
}
/*----------------------------------------------------------------------------*/
static enum Result workQueueInit(void *object, const void *configBase)
{
  const struct WorkQueueConfig * const config = configBase;
  assert(config != NULL);
  assert(config->size);

  struct WorkQueueUnique * const wq = object;

  wq->pool = malloc(sizeof(struct WqTaskBucket) * config->size);
  if (wq->pool == NULL)
    return E_MEMORY;

  if (!wqTaskArrayInit(&wq->buckets, config->size))
    return E_MEMORY;

  if (!wqTaskQueueInit(&wq->tasks, config->size))
    return E_MEMORY;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_GENERIC_WQ_UNIQUE_NONSTOP
static void workQueueDeinit(void *object)
{
  struct WorkQueueUnique * const wq = object;

  wqStop(wq);
  wqTaskQueueDeinit(&wq->tasks);
  wqTaskArrayDeinit(&wq->buckets);
  free(wq->pool);
}
#endif /* CONFIG_GENERIC_WQ_UNIQUE_NONSTOP */
/*----------------------------------------------------------------------------*/
static enum Result workQueueAdd(void *object, void (*callback)(void *),
    void *argument)
{
  assert(callback != NULL);

  struct WorkQueueUnique * const wq = object;
  struct WqTaskBucket *bucket;
  size_t index;

  const struct WqTask task = {
      .callback = callback,
      .argument = argument
  };
  const IrqState state = irqSave();

  if (!findTaskBucket(wq, &task, &index))
  {
    if (!wqTaskArrayFull(&wq->buckets))
    {
      bucket = &wq->pool[wqTaskArraySize(&wq->buckets)];

      bucket->task = task;
      bucket->pending = false;

#ifdef CONFIG_GENERIC_WQ_UNIQUE_PROFILE
      bucket->count = 0;
      bucket->timestamp = wqGetTime();
      bucket->execution.max = 0;
      bucket->execution.min = WQ_COUNTER_MAX;
      bucket->execution.total = 0;
#endif

      wqTaskArrayInsert(&wq->buckets, index, bucket);
    }
    else
    {
      bucket = NULL;
    }
  }
  else
  {
    bucket = *wqTaskArrayAt(&wq->buckets, index);
  }

  enum Result res;

  if (bucket != NULL)
  {
    if (!bucket->pending)
    {
      bucket->pending = true;
      wqTaskQueuePushBack(&wq->tasks, bucket);

      res = E_OK;
    }
    else
      res = E_BUSY;
  }
  else
    res = E_FULL;

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_GENERIC_WQ_UNIQUE_PROFILE
static void workQueueProfile(void *object, WqProfileCallback callback,
    void *argument)
{
  struct WorkQueueUnique * const wq = object;

  for (size_t index = 0; index < wqTaskArraySize(&wq->buckets); ++index)
  {
    struct WqTaskBucket * const bucket = *wqTaskArrayAt(&wq->buckets, index);
    const IrqState state = irqSave();

    const struct WqTaskInfo info = {
        .task = bucket->task.callback,
        .count = bucket->count,
        .execution = {
            bucket->execution.max,
            bucket->execution.min != WQ_COUNTER_MAX ? bucket->execution.min : 0,
            bucket->execution.total
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
  struct WorkQueueUnique * const wq = object;
  IrqState state;

#ifndef CONFIG_GENERIC_WQ_UNIQUE_NONSTOP
  wq->stop = false;
#endif

#ifdef CONFIG_GENERIC_WQ_UNIQUE_PROFILE
  state = irqSave();

  wq->latency.max = 0;
  wq->latency.min = WQ_COUNTER_MAX;
  wq->timestamp = wqGetTime();

  irqRestore(state);
#endif

  while (WQ_RUNNING(wq))
  {
#if defined(CONFIG_GENERIC_WQ_UNIQUE_PM) && \
    !defined(CONFIG_GENERIC_WQ_UNIQUE_LOAD)
    /*
     * Disable interrupts to avoid entering sleep mode when interrupt is fired
     * between size comparison and sleep instruction.
     */
    state = irqSave();
    if (wqTaskQueueEmpty(&wq->tasks))
      pmChangeState(PM_SLEEP);
    irqRestore(state);
#endif

#ifdef CONFIG_GENERIC_WQ_UNIQUE_LOAD
    ++wq->loops;
#endif

    /* Reload queue size */
    barrier();

    while (!wqTaskQueueEmpty(&wq->tasks))
    {
      struct WqTaskBucket * const bucket = wqTaskQueueFront(&wq->tasks);

#ifdef CONFIG_GENERIC_WQ_UNIQUE_PROFILE
      const WqCounter begin = wqGetTime();
#endif

      /* Critical section begin */
      state = irqSave();
      wqTaskQueuePopFront(&wq->tasks);
      bucket->pending = false;
      irqRestore(state);
      /* Critical section end */

      bucket->task.callback(bucket->task.argument);

#ifdef CONFIG_GENERIC_WQ_UNIQUE_PROFILE
      const WqCounter end = wqGetTime();
      const WqCounter execution = end - begin;
      const WqCounter latency = begin - bucket->timestamp;

      /* Critical section begin */
      state = irqSave();

      if (wq->latency.min > latency)
        wq->latency.min = latency;
      if (wq->latency.max < latency)
        wq->latency.max = latency;

      if (bucket->execution.min > execution)
        bucket->execution.min = execution;
      if (bucket->execution.max < execution)
        bucket->execution.max = execution;

      bucket->execution.total += execution;
      ++bucket->count;

      irqRestore(state);
      /* Critical section end */
#endif
    }
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#if defined(CONFIG_GENERIC_WQ_UNIQUE_PROFILE) \
    || defined(CONFIG_GENERIC_WQ_UNIQUE_LOAD)
static void workQueueStatistics(void *object, struct WqInfo *statistics)
{
  struct WorkQueueUnique * const wq = object;

#ifdef CONFIG_GENERIC_WQ_UNIQUE_PROFILE
  const IrqState state = irqSave();

  statistics->watermark = wqTaskArraySize(&wq->buckets);
  statistics->uptime = wqGetTime() - wq->timestamp;
  statistics->latency.max = wq->latency.max;
  statistics->latency.min = wq->latency.min != WQ_COUNTER_MAX ?
      wq->latency.min : 0;

  irqRestore(state);
#endif

#ifdef CONFIG_GENERIC_WQ_UNIQUE_LOAD
  const WqCounter loops = wq->loops;

  statistics->loops = loops - wq->previous;
  wq->previous = loops;
#endif
}
#endif
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_GENERIC_WQ_UNIQUE_NONSTOP
static void workQueueStop(void *object)
{
  struct WorkQueueUnique * const wq = object;
  wq->stop = true;
}
#endif
