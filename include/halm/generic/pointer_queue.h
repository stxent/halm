/*
 * halm/generic/pointer_queue.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_GENERIC_POINTER_QUEUE_H_
#define HALM_GENERIC_POINTER_QUEUE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/containers/tg_queue.h>
#include <xcore/helpers.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

DEFINE_QUEUE(void *, Pointer, pointer)

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_POINTER_QUEUE_H_ */
