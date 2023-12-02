/*
 * halm/generic/work_queue_unique.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_WORK_QUEUE_UNIQUE_H_
#define HALM_GENERIC_WORK_QUEUE_UNIQUE_H_
/*----------------------------------------------------------------------------*/
#include <halm/wq.h>
/*----------------------------------------------------------------------------*/
extern const struct WorkQueueClass * const WorkQueueUnique;

struct WorkQueueUniqueConfig
{
  /** Mandatory: number of queued tasks. */
  size_t size;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_WORK_QUEUE_UNIQUE_H_ */
