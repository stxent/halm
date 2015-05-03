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
  /* Implemented algorithm is from NXP software examples */
  const uint32_t periphClock = i2sGetClock(interface);
  uint64_t divider;
  uint32_t x, y;
  uint16_t xDiv, yDiv;
  uint16_t delta;
  uint16_t error, minError = 0xFFFF;

  divider = ((uint64_t)clock << 16) / periphClock;

  for (y = 255; y > 0; y--)
  {
    x = y * divider;
    if (x >= (1 << 24))
      continue;

    delta = x & 0xFFFF;
    error = delta > 0x8000 ? 0x10000 - delta : delta;

    if (!error)
    {
      yDiv = y;
      break;
    }
    else if (error < minError)
    {
      minError = error;
      yDiv = y;
    }
  }
  xDiv = ((uint64_t)yDiv * clock * 2) / periphClock;

  if (!xDiv || xDiv >= 256)
    return E_VALUE;

  config->x = xDiv;
  config->y = yDiv;

  return E_OK;
}
