/*
 * halm/generic/work_queue.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_GENERIC_WORK_QUEUE_H_
#define HALM_GENERIC_WORK_QUEUE_H_
/*----------------------------------------------------------------------------*/
#include <stddef.h>
#include <xcore/error.h>
#include <xcore/helpers.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

enum Result workQueueAdd(void (*)(void *), void *);
enum Result workQueueInit(size_t);
void workQueueStart(void *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_WORK_QUEUE_H_ */
