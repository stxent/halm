/*
 * halm/generic/pointer_list.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_GENERIC_POINTER_LIST_H_
#define HALM_GENERIC_POINTER_LIST_H_
/*----------------------------------------------------------------------------*/
#include <xcore/containers/tg_list.h>
#include <xcore/helpers.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

DEFINE_LIST(void *, Pointer, pointer)

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_POINTER_LIST_H_ */
