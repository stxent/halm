/*
 * spi_common.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/spi_base.h>
#include <halm/platform/stm32/spi_defs.h>
#include <xcore/accel.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
extern const struct PinEntry spiPins[];
/*----------------------------------------------------------------------------*/
void spiConfigPins(struct SpiBase *interface,
    const struct SpiBaseConfig *config)
{
  enum
  {
    MISO_INDEX,
    MOSI_INDEX,
    SCK_INDEX,
    CS_INDEX
  };

  const PinNumber pinArray[] = {
      [MISO_INDEX] = config->miso,
      [MOSI_INDEX] = config->mosi,
      [SCK_INDEX]  = config->sck,
      [CS_INDEX]   = config->cs
  };

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      const struct PinEntry * const pinEntry = pinFind(spiPins,
          pinArray[index], interface->channel);
      assert(pinEntry);

      const struct Pin pin = pinInit(pinArray[index]);
      const bool isOutput = (index != MISO_INDEX) ^ (config->slave == true);

      if (isOutput)
        pinOutput(pin, false);
      else
        pinInput(pin);

      pinSetFunction(pin, pinEntry->value);
    }
  }
}
/*----------------------------------------------------------------------------*/
uint32_t spiGetRate(const struct SpiBase *interface)
{
  STM_SPI_Type * const reg = interface->reg;
  return spiGetClock(interface) / (1UL << (CR1_BR_VALUE(reg->CR1) + 1));
}
/*----------------------------------------------------------------------------*/
void spiSetRate(struct SpiBase *interface, uint32_t rate)
{
  assert(rate != 0);

  const uint32_t clock = spiGetClock(interface);
  uint32_t divisor = (clock + (rate - 1)) / rate;
  uint32_t prescalerExp = 31 - countLeadingZeros32(divisor);

  if (prescalerExp && !(divisor & (divisor - 1)))
  {
    /* Decrease exponent when the divisor is equal to the power of two */
    --prescalerExp;
  }

  /* Limit the prescaler by PCLK / 256 */
  if (prescalerExp > 7)
    prescalerExp = 7;

  STM_SPI_Type * const reg = interface->reg;
  reg->CR1 = (reg->CR1 & ~CR1_BR_MASK) | CR1_BR(prescalerExp);
}
