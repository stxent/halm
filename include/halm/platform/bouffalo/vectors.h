/*
 * halm/platform/bouffalo/vectors.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_RISCV_IRQ_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_BOUFFALO_VECTORS_H_
#define HALM_PLATFORM_BOUFFALO_VECTORS_H_
/*----------------------------------------------------------------------------*/
#include <halm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/vectors.h>
#include HEADER_PATH
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_VECTORS_H_ */
