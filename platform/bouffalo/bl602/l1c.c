/*
 * l1c.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/bouffalo/bl602/l1c.h>
#include <halm/platform/bouffalo/bl602/l1c_defs.h>
#include <halm/platform/platform_defs.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
void cacheControlAccess2T(bool enabled)
{
  if (enabled)
    BL_L1C->L1C_CONFIG |= L1C_CONFIG_IROM_2T_ACCESS;
  else
    BL_L1C->L1C_CONFIG &= ~L1C_CONFIG_IROM_2T_ACCESS;
}
/*----------------------------------------------------------------------------*/
void cacheDisable(void)
{
  cacheEnable(ICACHE_NONE);
}
/*----------------------------------------------------------------------------*/
void cacheEnable(enum CacheSize size)
{
  uint32_t config = BL_L1C->L1C_CONFIG;

  config &= ~L1C_CONFIG_CACHEABLE;
  config |= L1C_CONFIG_WAYDIS_MASK | L1C_CONFIG_IROM_2T_ACCESS;

  /* Reset cache settings, all cache ways are disabled */
  BL_L1C->L1C_CONFIG = config;

  if (size != ICACHE_NONE)
  {
    config &= ~L1C_CONFIG_WAYDIS_MASK;
    config |= L1C_CONFIG_CACHEABLE | L1C_CONFIG_WAYDIS(MASK(4) & ~MASK(size));

    /* Apply new cache settings */
    BL_L1C->L1C_CONFIG = config;
  }
}
