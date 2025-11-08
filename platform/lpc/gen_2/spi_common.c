/*
 * spi_common.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/spi_base.h>
#include <halm/platform/lpc/spi_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
void spiConfigPins(const struct SpiBaseConfig *config,
    const struct PinEntry *map)
{
  const PinNumber pinArray[] = {
      config->miso, config->mosi, config->sck, config->cs
  };

  /* Direction configuration is not needed for alternate function pins */
  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      const struct PinEntry * const pinEntry = pinFind(map,
          pinArray[index], config->channel);
      assert(pinEntry != NULL);

      const struct Pin pin = pinInit(pinArray[index]);

      pinInput(pin);
      pinSetFunction(pin, pinEntry->value);
    }
  }
}
/*----------------------------------------------------------------------------*/
uint8_t spiGetMode(const struct SpiBase *interface)
{
  const LPC_SPI_Type * const reg = interface->reg;
  const uint32_t cfg = reg->CFG;
  uint8_t mode = 0;

  if (cfg & CFG_CPHA)
    mode |= 0x01;
  if (cfg & CFG_CPOL)
    mode |= 0x02;

  return mode;
}
/*----------------------------------------------------------------------------*/
uint32_t spiGetRate(const struct SpiBase *interface)
{
  const LPC_SPI_Type * const reg = interface->reg;
  const uint32_t clock = spiGetClock(interface);
  const uint32_t divisor = reg->DIV + 1;

  return clock / divisor;
}
/*----------------------------------------------------------------------------*/
void spiSetMode(struct SpiBase *interface, uint8_t mode)
{
  LPC_SPI_Type * const reg = interface->reg;
  uint32_t cfg = reg->CFG & ~(CFG_CPHA | CFG_CPOL);

  if (mode & 0x01)
    cfg |= CFG_CPHA;
  if (mode & 0x02)
    cfg |= CFG_CPOL;

  reg->CFG = cfg;
}
/*----------------------------------------------------------------------------*/
bool spiSetRate(struct SpiBase *interface, uint32_t rate)
{
  if (!rate)
    return false;

  LPC_SPI_Type * const reg = interface->reg;
  const uint32_t clock = spiGetClock(interface);
  const uint32_t divisor = (clock + (rate - 1)) / rate;

  if (divisor > 0 && divisor <= DIV_DIVVAL_MAX)
  {
    reg->DIV = divisor - 1;
    return true;
  }
  else
    return false;
}
