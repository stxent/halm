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
void i2sConfigClockPin(uint8_t channel, PinNumber key)
{
  const struct PinEntry * const pinEntry = pinFind(spiPins, key, channel);
  assert(pinEntry != NULL);

  const struct Pin pin = pinInit(key);

  pinOutput(pin, false);
  pinSetFunction(pin, pinEntry->value);
}
/*----------------------------------------------------------------------------*/
void spiConfigPins(const struct SpiBaseConfig *config)
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
          pinArray[index], config->channel);
      assert(pinEntry != NULL);

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
uint8_t spiGetMode(const struct SpiBase *interface)
{
  const STM_SPI_Type * const reg = interface->reg;
  const uint32_t cr1 = reg->CR1;
  uint8_t mode = 0;

  if (cr1 & CR1_CPHA)
    mode |= 0x01;
  if (cr1 & CR1_CPOL)
    mode |= 0x02;

  return mode;
}
/*----------------------------------------------------------------------------*/
uint32_t spiGetRate(const struct SpiBase *interface)
{
  STM_SPI_Type * const reg = interface->reg;
  return spiGetClock(interface) / (1UL << (CR1_BR_VALUE(reg->CR1) + 1));
}
/*----------------------------------------------------------------------------*/
void spiSetMode(struct SpiBase *interface, uint8_t mode)
{
  STM_SPI_Type * const reg = interface->reg;
  uint32_t cr1 = reg->CR1 & ~(CR1_CPHA | CR1_CPOL);

  if (mode & 0x01)
    cr1 |= CR1_CPHA;
  if (mode & 0x02)
    cr1 |= CR1_CPOL;
  reg->CR1 = cr1;
}
/*----------------------------------------------------------------------------*/
bool spiSetRate(struct SpiBase *interface, uint32_t rate)
{
  if (!rate)
    return false;

  const uint32_t clock = spiGetClock(interface);
  const uint32_t divisor = (clock + (rate - 1)) / rate;
  uint32_t prescalerExp = 31 - countLeadingZeros32(divisor);

  if (prescalerExp && !(divisor & (divisor - 1)))
  {
    /* Decrease exponent when the divisor is equal to the power of two */
    --prescalerExp;
  }

  if (prescalerExp > 7)
    return false;

  STM_SPI_Type * const reg = interface->reg;
  reg->CR1 = (reg->CR1 & ~CR1_BR_MASK) | CR1_BR(prescalerExp);

  return true;
}
