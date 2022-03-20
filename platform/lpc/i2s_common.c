/*
 * i2s_common.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/i2s_base.h>
/*----------------------------------------------------------------------------*/
bool i2sCalcRate(struct I2SBase *interface, uint32_t clock,
    struct I2SRateConfig *config)
{
  const uint32_t apbClock = i2sGetClock(interface);
  const uint32_t ratio = ((uint64_t)clock << 16) / apbClock;
  uint32_t minError = UINT32_MAX;
  uint32_t yDiv = 0;

  for (uint8_t y = 255; y > 0; --y)
  {
    const uint32_t x = y * ratio;
    const uint32_t xDiv = x >> 16;

    if (!xDiv || xDiv >= 128)
      continue;

    const uint32_t delta = (x & UINT16_MAX) << 16;

    if (delta == 0)
    {
      yDiv = y;
      break;
    }

    const uint32_t error = delta / xDiv;

    if (error < minError)
    {
      minError = error;
      yDiv = y;
    }
  }

  if (yDiv != 0)
  {
    const uint16_t xDiv = yDiv * ratio >> 15;

    config->x = (uint8_t)xDiv;
    config->y = (uint8_t)yDiv;

    return true;
  }
  else
    return false;
}
