/*
 * halm/core/cortex/armv7m/cache.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_CORTEX_CACHE_H_
#error This header should not be included directly
#endif

#ifndef HALM_CORE_CORTEX_ARMV7M_CACHE_H_
#define HALM_CORE_CORTEX_ARMV7M_CACHE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
#include <stddef.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void dCacheClean(uintptr_t, size_t);
void dCacheCleanAll(void);
void dCacheDisable(void);
void dCacheEnable(void);
void dCacheFlush(uintptr_t, size_t);
void dCacheFlushAll(void);
void dCacheInvalidate(uintptr_t, size_t);
void dCacheInvalidateAll(void);
void iCacheDisable(void);
void iCacheEnable(void);
void iCacheInvalidateAll(void);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_ARMV7M_CACHE_H_ */
