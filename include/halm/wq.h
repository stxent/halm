/*
 * halm/wq.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Abstract Work Queue class.
 */

#ifndef HALM_WQ_H_
#define HALM_WQ_H_
/*----------------------------------------------------------------------------*/
#include <xcore/entity.h>
#include <stdbool.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
extern void *WQ_DEFAULT;
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct WorkQueueClass
{
  CLASS_HEADER

  enum Result (*add)(void *, void (*)(void *), void *);
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

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_WQ_H_ */
