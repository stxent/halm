/*
 * cache.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/core/cortex/cache.h>
/*----------------------------------------------------------------------------*/
void dCacheClean(uintptr_t address __attribute__((unused)),
    size_t size __attribute__((unused)))
{
}
/*----------------------------------------------------------------------------*/
void dCacheCleanAll(void)
{
}
/*----------------------------------------------------------------------------*/
void dCacheDisable(void)
{
}
/*----------------------------------------------------------------------------*/
void dCacheEnable(void)
{
}
/*----------------------------------------------------------------------------*/
void dCacheFlush(uintptr_t address __attribute__((unused)),
    size_t size __attribute__((unused)))
{
}
/*----------------------------------------------------------------------------*/
void dCacheFlushAll(void)
{
}
/*----------------------------------------------------------------------------*/
void dCacheInvalidate(uintptr_t address __attribute__((unused)),
    size_t size __attribute__((unused)))
{
}
/*----------------------------------------------------------------------------*/
void dCacheInvalidateAll(void)
{
}
/*----------------------------------------------------------------------------*/
void iCacheDisable(void)
{
}
/*----------------------------------------------------------------------------*/
void iCacheEnable(void)
{
}
/*----------------------------------------------------------------------------*/
void iCacheInvalidateAll(void)
{
}
