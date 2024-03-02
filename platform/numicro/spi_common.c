/*
 * spi_common.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/spi_base.h>
#include <halm/platform/numicro/spi_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
extern const struct PinEntry spiPins[];
/*----------------------------------------------------------------------------*/
void spiConfigPins(const struct SpiBaseConfig *config)
{
  const PinNumber pinArray[] = {
      config->miso, config->mosi, config->sck, config->cs
  };

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      const struct PinEntry * const pinEntry = pinFind(spiPins,
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
  const NM_SPI_Type * const reg = interface->reg;
  const uint32_t ctl = reg->CTL;
  uint8_t mode = 0;

  if (ctl & CTL_RXNEG)
    mode |= 0x01;
  if (ctl & CTL_CLKPOL)
    mode |= 0x02;

  return mode;
}
/*----------------------------------------------------------------------------*/
uint32_t spiGetRate(const struct SpiBase *interface)
{
  NM_SPI_Type * const reg = interface->reg;
  return spiGetClock(interface) / (reg->CLKDIV + 1);
}
/*----------------------------------------------------------------------------*/
void spiSetMode(struct SpiBase *interface, uint8_t mode)
{
  NM_SPI_Type * const reg = interface->reg;
  uint32_t ctl = reg->CTL & ~(CTL_RXNEG | CTL_TXNEG | CTL_CLKPOL);

  if (mode & 0x01)
    ctl |= CTL_RXNEG; /* CPHA = 1 */
  else
    ctl |= CTL_TXNEG; /* CPHA = 0 */

  if (mode & 0x02)
    ctl |= CTL_CLKPOL;

  reg->CTL = ctl;
}
/*----------------------------------------------------------------------------*/
bool spiSetRate(struct SpiBase *interface, uint32_t rate)
{
  const uint32_t clock = spiGetClock(interface);

  if (!rate || !clock)
    return false;

  NM_SPI_Type * const reg = interface->reg;
  uint32_t divisor = (clock + (rate - 1)) / rate - 1;

  if (divisor > CLKDIV_DIVIDER_MASK)
    return false;

  /*
  * The time interval must be larger than or equal 8 PCLK cycles between
  * releasing SPI IP software reset and setting this clock divider register.
  */
  reg->CLKDIV = divisor;
  return true;
}
