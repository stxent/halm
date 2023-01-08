/*
 * halm/platform/numicro/platform_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_PLATFORM_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_PLATFORM_DEFS_H_
#define HALM_PLATFORM_NUMICRO_PLATFORM_DEFS_H_
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
#endif /* HALM_PLATFORM_NUMICRO_PLATFORM_DEFS_H_ */
