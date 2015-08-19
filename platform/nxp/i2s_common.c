/*
 * i2s_common.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/nxp/i2s_base.h>
/*----------------------------------------------------------------------------*/
enum result i2sCalcRate(struct I2sBase *interface, uint32_t clock,
    struct I2sRateConfig *config)
{
  /* Algorithm based on NXP software examples */
  const uint32_t periphClock = i2sGetClock(interface);
  const uint64_t divider = ((uint64_t)clock << 16) / periphClock;
  uint16_t minError = 0xFFFF;
  uint16_t yDiv = 0;

  for (uint32_t y = 255; y > 0; --y)
  {
    const uint32_t x = y * divider;

    if (x >= (1 << 24))
      continue;

    const uint16_t delta = x & 0xFFFF;
    const uint16_t error = delta > 0x8000 ? 0x10000 - delta : delta;

    if (error < minError)
    {
      minError = error;
      yDiv = y;

      if (!minError)
        break;
    }
  }

  const uint16_t xDiv = ((uint64_t)yDiv * clock * 2) / periphClock;

  if (!xDiv || xDiv >= 256)
    return E_VALUE;

  config->x = xDiv;
  config->y = yDiv;

  return E_OK;
}
