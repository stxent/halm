/*
 * cache.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/core/core_defs.h>
#include <halm/core/cortex/cache.h>
#include <halm/core/cortex/scb_defs.h>
#include <xcore/accel.h>
#include <xcore/asm.h>
/*----------------------------------------------------------------------------*/
void dCacheClean(uintptr_t address, size_t size)
{
#if defined(CONFIG_CORE_CORTEX_DCACHE) && !defined(CONFIG_CORE_CORTEX_DCACHE_WT)
  const uint32_t ccsidr = SCB->CCSIDR;
  const size_t lineSize = 1 << (CCSIDR_LINESIZE_VALUE(ccsidr) + 4);

  const uintptr_t end = address + size;
  uintptr_t begin = address & ~(lineSize - 1);

  __dsb();

  do
  {
    SCB->DCCMVAC = begin;
    begin += lineSize;
  }
  while (begin < end);

  __dsb();
  __isb();
#else
  (void)address;
  (void)size;
#endif
}
/*----------------------------------------------------------------------------*/
void dCacheCleanAll(void)
{
#if defined(CONFIG_CORE_CORTEX_DCACHE) && !defined(CONFIG_CORE_CORTEX_DCACHE_WT)
  const uint32_t ccsidr = SCB->CCSIDR;
  const unsigned int lineSizePow = CCSIDR_LINESIZE_VALUE(ccsidr) + 4;
  const unsigned int nWays = CCSIDR_ASSOCIATIVITY_VALUE(ccsidr);
  const unsigned int nWaysPow = countLeadingZeros32(nWays);
  unsigned int nSets = CCSIDR_NUMSETS_VALUE(ccsidr);

  __dsb();

  /* Disable the D-Cache */
  SCB->CCR &= ~CCR_DC;

  do
  {
    unsigned int index = nWays;

    do
    {
      SCB->DCCSW = (index << nWaysPow) | (nSets << lineSizePow);
    }
    while (index--);
  }
  while (nSets--);

  __dsb();
  __isb();
#endif
}
/*----------------------------------------------------------------------------*/
void dCacheDisable(void)
{
#ifdef CONFIG_CORE_CORTEX_DCACHE
  const uint32_t ccsidr = SCB->CCSIDR;
  const unsigned int lineSizePow = CCSIDR_LINESIZE_VALUE(ccsidr) + 4;
  const unsigned int nWays = CCSIDR_ASSOCIATIVITY_VALUE(ccsidr);
  const unsigned int nWaysPow = countLeadingZeros32(nWays);
  unsigned int nSets = CCSIDR_NUMSETS_VALUE(ccsidr);

  __dsb();

  /* Disable the D-Cache */
  SCB->CCR &= ~CCR_DC;

  do
  {
    unsigned int index = nWays;

    do
    {
      SCB->DCCISW = (index << nWaysPow) | (nSets << lineSizePow);
    }
    while (index--);
  }
  while (nSets--);

  __dsb();
  __isb();
#endif
}
/*----------------------------------------------------------------------------*/
void dCacheEnable(void)
{
#ifdef CONFIG_CORE_CORTEX_DCACHE
  const uint32_t ccsidr = SCB->CCSIDR;
  const unsigned int lineSizePow = CCSIDR_LINESIZE_VALUE(ccsidr) + 4;
  const unsigned int nWays = CCSIDR_ASSOCIATIVITY_VALUE(ccsidr);
  const unsigned int nWaysPow = countLeadingZeros32(nWays);
  unsigned int nSets = CCSIDR_NUMSETS_VALUE(ccsidr);

  __dsb();

  do
  {
    unsigned int index = nWays;

    do
    {
      SCB->DCISW = (index << nWaysPow) | (nSets << lineSizePow);
    }
    while (index--);
  }
  while (nSets--);

  __dsb();

#  ifdef CONFIG_CORE_CORTEX_DCACHE_WT
  SCB->CACR |= CACR_FORCEWT;
#  endif /* CONFIG_CORE_CORTEX_DCACHE_WT */

  /* Enable the D-Cache */
  SCB->CCR |= CCR_DC;

  __dsb();
  __isb();
#endif /* CONFIG_CORE_CORTEX_DCACHE */
}
/*----------------------------------------------------------------------------*/
void dCacheFlush(uintptr_t address, size_t size)
{
#if defined(CONFIG_CORE_CORTEX_DCACHE) && !defined(CONFIG_CORE_CORTEX_DCACHE_WT)
  const uint32_t ccsidr = SCB->CCSIDR;
  const size_t lineSize = 1 << (CCSIDR_LINESIZE_VALUE(ccsidr) + 4);

  const uintptr_t end = address + size;
  uintptr_t begin = address & ~(lineSize - 1);

  __dsb();

  do
  {
    SCB->DCCIMVAC = begin;
    begin += lineSize;
  }
  while (begin < end);

  __dsb();
  __isb();
#else
  (void)address;
  (void)size;
#endif
}
/*----------------------------------------------------------------------------*/
void dCacheFlushAll(void)
{
#if defined(CONFIG_CORE_CORTEX_DCACHE) && !defined(CONFIG_CORE_CORTEX_DCACHE_WT)
  const uint32_t ccsidr = SCB->CCSIDR;
  const unsigned int lineSizePow = CCSIDR_LINESIZE_VALUE(ccsidr) + 4;
  const unsigned int nWays = CCSIDR_ASSOCIATIVITY_VALUE(ccsidr);
  const unsigned int nWaysPow = countLeadingZeros32(nWays);
  unsigned int nSets = CCSIDR_NUMSETS_VALUE(ccsidr);

  __dsb();

  /* Disable the D-Cache */
  SCB->CCR &= ~CCR_DC;

  do
  {
    unsigned int index = nWays;

    do
    {
      SCB->DCCISW = (index << nWaysPow) | (nSets << lineSizePow);
    }
    while (index--);
  }
  while (nSets--);

  __dsb();
  __isb();
#else
  dCacheInvalidateAll();
#endif
}
/*----------------------------------------------------------------------------*/
void dCacheInvalidate(uintptr_t address, size_t size)
{
#ifdef CONFIG_CORE_CORTEX_DCACHE
  const uint32_t ccsidr = SCB->CCSIDR;
  const size_t lineSize = 1 << (CCSIDR_LINESIZE_VALUE(ccsidr) + 4);

  const uintptr_t end = address + size;
  uintptr_t begin = address & ~(lineSize - 1);

  __dsb();

  if (address - begin)
  {
    SCB->DCCIMVAC = begin;
    address += lineSize;
  }

  if (begin < end)
  {
    while (begin <= end - lineSize)
    {
      SCB->DCIMVAC = begin;
      begin += lineSize;
    }
  }

  if (begin < end)
    SCB->DCCIMVAC = begin;

  __dsb();
  __isb();
#else
  (void)address;
  (void)size;
#endif
}
/*----------------------------------------------------------------------------*/
void dCacheInvalidateAll(void)
{
#ifdef CONFIG_CORE_CORTEX_DCACHE
  const uint32_t ccsidr = SCB->CCSIDR;
  const unsigned int lineSizePow = CCSIDR_LINESIZE_VALUE(ccsidr) + 4;
  const unsigned int nWays = CCSIDR_ASSOCIATIVITY_VALUE(ccsidr);
  const unsigned int nWaysPow = countLeadingZeros32(nWays);
  unsigned int nSets = CCSIDR_NUMSETS_VALUE(ccsidr);

  __dsb();

  /* Disable the D-Cache */
  SCB->CCR &= ~CCR_DC;

  do
  {
    unsigned int index = nWays;

    do
    {
      SCB->DCISW = (index << nWaysPow) | (nSets << lineSizePow);
    }
    while (index--);
  }
  while (nSets--);

  __dsb();
  __isb();
#endif
}
/*----------------------------------------------------------------------------*/
void iCacheDisable(void)
{
#ifdef CONFIG_CORE_CORTEX_ICACHE
  __dsb();
  __isb();

  /* Disable the I-Cache */
  SCB->CCR &= ~CCR_IC;
  /* Enable the entire I-Cache */
  SCB->ICIALLU = 0;

  __dsb();
  __isb();
#endif
}
/*----------------------------------------------------------------------------*/
void iCacheEnable(void)
{
#ifdef CONFIG_CORE_CORTEX_ICACHE
  __dsb();
  __isb();

  /* Enable the entire I-Cache */
  SCB->ICIALLU = 0;
  /* Enable the I-Cache */
  SCB->CCR |= CCR_IC;

  __dsb();
  __isb();
#endif
}
/*----------------------------------------------------------------------------*/
void iCacheInvalidateAll(void)
{
#ifdef CONFIG_CORE_CORTEX_ICACHE
  __dsb();
  __isb();

  /* Enable the entire I-Cache */
  SCB->ICIALLU = 0;

  __dsb();
  __isb();
#endif
}
