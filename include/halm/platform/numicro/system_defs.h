/*
 * halm/platform/numicro/system_defs.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_SYSTEM_DEFS_H_
#define HALM_PLATFORM_NUMICRO_SYSTEM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <halm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/system_defs.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_SYSTEM_DEFS_H_ */
