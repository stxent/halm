/*
 * halm/generic/work_queue_defs.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_WORK_QUEUE_DEFS_H_
#define HALM_GENERIC_WORK_QUEUE_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/containers/tg_queue.h>
/*----------------------------------------------------------------------------*/
struct WqTask
{
  void (*callback)(void *);
  void *argument;
};

DEFINE_QUEUE(struct WqTask, WqTask, wqTask)
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_WORK_QUEUE_DEFS_H_ */
