/*
 * spi_common.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/m03x/spi_defs.h>
#include <halm/platform/numicro/spi_base.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
extern const struct PinEntry spiPins[];
/*----------------------------------------------------------------------------*/
void spiConfigPins(struct SpiBase *interface,
    const struct SpiBaseConfig *config)
{
  const PinNumber pinArray[] = {
      config->miso, config->mosi, config->sck, config->cs
  };

  /* Direction configuration is not needed for alternate function pins */
  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      const struct PinEntry * const pinEntry = pinFind(spiPins,
          pinArray[index], interface->channel);
      assert(pinEntry);

      const struct Pin pin = pinInit(pinArray[index]);

      pinInput(pin);
      pinSetFunction(pin, pinEntry->value);
    }
  }
}
/*----------------------------------------------------------------------------*/
uint32_t spiGetRate(const struct SpiBase *interface)
{
  NM_SPI_Type * const reg = interface->reg;
  return spiGetClock(interface) / (reg->CLKDIV + 1);
}
/*----------------------------------------------------------------------------*/
void spiSetRate(struct SpiBase *interface, uint32_t rate)
{
  const uint32_t clock = spiGetClock(interface);
  NM_SPI_Type * const reg = interface->reg;

  if (rate && clock)
  {
    uint32_t divisor = (clock + (rate - 1)) / rate - 1;

    if (divisor > CLKDIV_DIVIDER_MASK)
      divisor = CLKDIV_DIVIDER_MASK;

    /*
    * The time interval must be larger than or equal 8 PCLK cycles between
    * releasing SPI IP software reset and setting this clock divider register.
    */
    reg->CLKDIV = divisor;
  }
  else
    reg->CLKDIV = 0;
}
