/*
 * task_sequence.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/task_sequence.h>
#include <halm/irq.h>
#include <halm/timer.h>
#include <halm/wq.h>
/*----------------------------------------------------------------------------*/
static void onProcessEvent(void *);
static void onTimerEvent(void *);

static enum Result taskSequenceInit(void *, const void *);
static void taskSequenceDeinit(void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const TaskSequence = &(const struct EntityClass){
    .size = sizeof(struct TaskSequence),
    .init = taskSequenceInit,
    .deinit = taskSequenceDeinit
};
/*----------------------------------------------------------------------------*/
static void onProcessEvent(void *argument)
{
  struct TaskSequence * const seq = argument;

  if (seq->state != TS_ACTIVE)
  {
    /* Sequence is disabled asynchronously */
    return;
  }

  const IrqState state = irqSave();
  const struct TsTask * const task = tsTaskArrayAt(&seq->tasks, seq->index);
  irqRestore(state);

  task->callback(task->argument);
  ++seq->index;

  if (!task->delay)
  {
    if (seq->index >= tsTaskArraySize(&seq->tasks))
    {
      seq->state = TS_COMPLETED;
    }
    else if (wqAdd(seq->wq, onProcessEvent, seq) != E_OK)
    {
      seq->state = TS_ERROR;
    }
  }
  else
  {
    timerSetOverflow(seq->timer, (uint32_t)task->delay);
    timerEnable(seq->timer);
  }
}
/*----------------------------------------------------------------------------*/
static void onTimerEvent(void *argument)
{
  struct TaskSequence * const seq = argument;

  if (seq->index >= tsTaskArraySize(&seq->tasks))
  {
    seq->state = TS_COMPLETED;
  }
  else if (wqAdd(seq->wq, onProcessEvent, seq) != E_OK)
  {
    seq->state = TS_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result taskSequenceInit(void *object, const void *configBase)
{
  const struct TaskSequenceConfig * const config = configBase;
  assert(config != NULL);
  assert(config->timer != NULL && config->wq != NULL);
  assert(config->size);

  struct TaskSequence * const seq = object;

  if (!tsTaskArrayInit(&seq->tasks, config->size))
    return E_MEMORY;

  seq->timer = config->timer;
  seq->wq = config->wq;
  seq->index = 0;
  seq->state = TS_IDLE;

  timerSetCallback(seq->timer, onTimerEvent, seq);
  timerSetAutostop(seq->timer, true);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void taskSequenceDeinit(void *object)
{
  struct TaskSequence * const seq = object;

  timerDisable(seq->timer);
  timerSetCallback(seq->timer, NULL, NULL);
  tsTaskArrayDeinit(&seq->tasks);
}
/*----------------------------------------------------------------------------*/
enum Result tsAdd(struct TaskSequence *seq, void (*callback)(void *),
    void *argument, unsigned long delay)
{
  assert(callback != NULL);

  if (!tsTaskArrayFull(&seq->tasks))
  {
    const struct TsTask task = {
        .callback = callback,
        .argument = argument,
        .delay = delay
    };

    const IrqState state = irqSave();
    tsTaskArrayPushBack(&seq->tasks, task);
    irqRestore(state);

    return E_OK;
  }
  else
    return E_FULL;
}
/*----------------------------------------------------------------------------*/
void tsClear(struct TaskSequence *seq)
{
  if (seq->state == TS_ACTIVE)
    tsStop(seq);

  const IrqState state = irqSave();
  tsTaskArrayClear(&seq->tasks);
  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
enum Result tsStart(struct TaskSequence *seq)
{
  if (tsTaskArrayEmpty(&seq->tasks))
    return E_EMPTY;
  if (seq->state == TS_ACTIVE)
    return E_BUSY;

  seq->index = 0;
  seq->state = TS_ACTIVE;

  return wqAdd(seq->wq, onProcessEvent, seq);
}
/*----------------------------------------------------------------------------*/
void tsStop(struct TaskSequence *seq)
{
  timerDisable(seq->timer);
  seq->state = TS_IDLE;
}
