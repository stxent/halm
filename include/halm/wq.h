/*
 * halm/wq.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * Abstract Work Queue class.
 */

#ifndef HALM_WQ_H_
#define HALM_WQ_H_
/*----------------------------------------------------------------------------*/
#include <xcore/entity.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
extern void *WQ_DEFAULT;
/*----------------------------------------------------------------------------*/
typedef uint32_t WqCounter;

#define WQ_COUNTER_MAX_GENERIC(value) _Generic((value), \
    unsigned long long: ULLONG_MAX, \
    unsigned long: ULONG_MAX, \
    unsigned int: UINT_MAX, \
    unsigned short: USHRT_MAX \
)
#define WQ_COUNTER_MAX WQ_COUNTER_MAX_GENERIC((WqCounter)0)

struct WqInfo
{
  size_t watermark;
  WqCounter loops;
  WqCounter uptime;

  struct
  {
    WqCounter max;
    WqCounter min;
  } latency;
};

struct WqTaskInfo
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

typedef void (*WqProfileCallback)(void *, const struct WqTaskInfo *);

/* Class descriptor */
struct WorkQueueClass
{
  CLASS_HEADER

  enum Result (*add)(void *, void (*)(void *), void *);
  void (*profile)(void *, WqProfileCallback, void *);
  void (*statistics)(void *, struct WqInfo *);
  enum Result (*start)(void *);
  void (*stop)(void *);
};

struct WorkQueue
{
  struct Entity base;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/**
 * Add a task to the work queue.
 * @param wq Pointer to a Work Queue object.
 * @param callback Callback function.
 * @param argument Callback function argument.
 * @return @b E_OK on success.
 */
static inline enum Result wqAdd(void *wq, void (*callback)(void *),
    void *argument)
{
  return ((const struct WorkQueueClass *)CLASS(wq))->add(wq, callback,
      argument);
}

/**
 * Request information about execution times.
 * Function invokes user callback for each task descriptor.
 * @param wq Pointer to a Work Queue object.
 * @param callback Pointer to the callback function.
 * @param argument Callback argument.
 * @return Information about Work Queue execution.
 */
static inline void wqProfile(void *wq, WqProfileCallback callback,
    void *argument)
{
  ((const struct WorkQueueClass *)CLASS(wq))->profile(wq, callback, argument);
}

/**
 * Request global information about Work Queue.
 * @param wq Pointer to a Work Queue object.
 * @param statistics Pointer to a statistics structure to be filled.
 */
static inline void wqStatistics(void *wq, struct WqInfo *statistics)
{
  ((const struct WorkQueueClass *)CLASS(wq))->statistics(wq, statistics);
}

/**
 * Start the work queue.
 * @param wq Pointer to a Work Queue object.
 * @return @b E_OK on success.
 */
static inline enum Result wqStart(void *wq)
{
  return ((const struct WorkQueueClass *)CLASS(wq))->start(wq);
}

/**
 * Stop the work queue.
 * @param wq Pointer to a Work Queue object.
 */
static inline void wqStop(void *wq)
{
  ((const struct WorkQueueClass *)CLASS(wq))->stop(wq);
}

/**
 * Work Queue helper function for a timestamp generation.
 * When profiling is enabled, the function must be defined in user code.
 * The time format should be chosen depending on the target platform.
 * @return Current time.
 */
WqCounter wqGetTime(void);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_WQ_H_ */
