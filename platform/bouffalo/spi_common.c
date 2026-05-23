/*
 * spi_common.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/bouffalo/spi_base.h>
#include <halm/platform/bouffalo/spi_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
uint8_t spiGetMode(const struct SpiBase *interface)
{
  const BL_SPI_Type * const reg = interface->reg;
  const uint32_t config = reg->CONFIG;
  uint8_t mode = 0;

  if (config & CONFIG_SCLKPH)
    mode |= 0x01;
  if (config & CONFIG_SCLKPOL)
    mode |= 0x02;

  return mode;
}
/*----------------------------------------------------------------------------*/
uint32_t spiGetRate(const struct SpiBase *interface)
{
  BL_SPI_Type * const reg = interface->reg;
  const uint32_t halfBitTime = PRD0_PRDPH0_VALUE(reg->PRD0) + 1;

  return spiGetClock(interface) / (halfBitTime << 1);
}
/*----------------------------------------------------------------------------*/
void spiSetMode(struct SpiBase *interface, uint8_t mode)
{
  BL_SPI_Type * const reg = interface->reg;
  uint32_t config = reg->CONFIG & ~(CONFIG_SCLKPOL | CONFIG_SCLKPH);

  if (mode & 0x01)
    config |= CONFIG_SCLKPH;

  if (mode & 0x02)
    config |= CONFIG_SCLKPOL;

  reg->CONFIG = config;
}
/*----------------------------------------------------------------------------*/
bool spiSetRate(struct SpiBase *interface, uint32_t rate)
{
  const uint32_t clock = spiGetClock(interface);

  if (!rate || !clock)
    return false;

  const uint32_t halfBitTime = (((clock + (rate - 1)) / rate) >> 1) - 1;

  if (halfBitTime > PRD0_PRD_MAX)
    return false;

  BL_SPI_Type * const reg = interface->reg;

  reg->PRD0 = PRD0_PRDS(halfBitTime) | PRD0_PRDP(halfBitTime)
      | PRD0_PRDPH0(halfBitTime) | PRD0_PRDPH1(halfBitTime);
  reg->PRD1 = PRD1_PRDI(0);

  return true;
}
