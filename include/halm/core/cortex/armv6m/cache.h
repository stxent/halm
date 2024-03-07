/*
 * halm/core/cortex/armv6m/cache.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_CORTEX_CACHE_H_
#error This header should not be included directly
#endif

#ifndef HALM_CORE_CORTEX_ARMV6M_CACHE_H_
#define HALM_CORE_CORTEX_ARMV6M_CACHE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
#include <stddef.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void dCacheClean(uintptr_t, size_t)
{
}

static inline void dCacheCleanAll(void)
{
}

static inline void dCacheDisable(void)
{
}

static inline void dCacheEnable(void)
{
}

static inline void dCacheFlush(uintptr_t, size_t)
{
}

static inline void dCacheFlushAll(void)
{
}

static inline void dCacheInvalidate(uintptr_t, size_t)
{
}

static inline void dCacheInvalidateAll(void)
{
}

static inline void iCacheDisable(void)
{
}

static inline void iCacheEnable(void)
{
}

static inline void iCacheInvalidateAll(void)
{
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_ARMV6M_CACHE_H_ */
