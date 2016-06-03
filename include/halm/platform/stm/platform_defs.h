/*
 * halm/platform/stm/platform_defs.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_STM_PLATFORM_DEFS_H_
#define HALM_PLATFORM_STM_PLATFORM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
/* No effect or reserved registers */
#define __ne__ __attribute__((deprecated))
/* Registers with read and write access types */
#define __rw__ volatile
/* Read-only registers */
#define __ro__ const volatile
/* Write-only registers */
#define __wo__ volatile
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/platform_defs.h>
#include HEADER_PATH
/*----------------------------------------------------------------------------*/
#undef __wo__
#undef __ro__
#undef __rw__
#undef __ne__
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM_PLATFORM_DEFS_H_ */
