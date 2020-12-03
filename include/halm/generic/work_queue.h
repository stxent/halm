/*
 * halm/generic/work_queue.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_WORK_QUEUE_H_
#define HALM_GENERIC_WORK_QUEUE_H_
/*----------------------------------------------------------------------------*/
#include <halm/wq.h>
/*----------------------------------------------------------------------------*/
extern const struct WorkQueueClass * const WorkQueue;

struct WorkQueueConfig
{
  size_t size;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_WORK_QUEUE_H_ */
