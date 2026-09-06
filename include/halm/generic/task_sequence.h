/*
 * halm/generic/task_sequence.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_TASK_SEQUENCE_H_
#define HALM_GENERIC_TASK_SEQUENCE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/containers/tg_array.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const TaskSequence;

enum [[gnu::packed]] TsState
{
  TS_IDLE,
  TS_ACTIVE,
  TS_COMPLETED,
  TS_ERROR
};

struct TsTask
{
  void (*callback)(void *);
  void *argument;
  unsigned long delay;
};

DEFINE_ARRAY(struct TsTask, TsTask, tsTask)

struct TaskSequenceConfig
{
  /** Mandatory: pointer to a timer instance. */
  void *timer;
  /** Mandatory: Work Queue for task execution. */
  void *wq;
  /** Mandatory: number of queued tasks. */
  size_t size;
};

struct TaskSequence
{
  struct Entity base;

  /* Pointer to a timer instance */
  struct Timer *timer;
  /* Work Queue for task execution */
  struct WorkQueue *wq;
  /* Currently pending tasks */
  TsTaskArray tasks;
  /* Current entry */
  size_t index;
  /* Current state */
  enum TsState state;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

enum Result tsAdd(struct TaskSequence *, void (*)(void *), void *,
    unsigned long);
void tsClear(struct TaskSequence *);
enum Result tsStart(struct TaskSequence *);
void tsStop(struct TaskSequence *);

END_DECLS
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline bool tsIsActive(const struct TaskSequence *seq)
{
  return seq->state == TS_ACTIVE;
}

static inline bool tsIsCompleted(const struct TaskSequence *seq)
{
  return seq->state == TS_COMPLETED;
}

static inline bool tsIsFailed(const struct TaskSequence *seq)
{
  return seq->state == TS_ERROR;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_TASK_SEQUENCE_H_ */
