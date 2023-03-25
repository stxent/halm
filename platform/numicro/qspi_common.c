/*
 * qspi_common.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/qspi_base.h>
#include <halm/platform/numicro/qspi_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
extern const struct PinEntry qspiPins[];
/*----------------------------------------------------------------------------*/
void qspiConfigPins(const struct QspiBaseConfig *config)
{
  const PinNumber pinArray[] = {
      config->cs,
      config->io0, /* MOSI0 */
      config->io1, /* MISO0 */
      config->io2, /* MOSI1 */
      config->io3, /* MISO1 */
      config->sck
  };

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      const struct PinEntry * const pinEntry = pinFind(qspiPins,
          pinArray[index], config->channel);
      assert(pinEntry);

      const struct Pin pin = pinInit(pinArray[index]);

      pinInput(pin);
      pinSetFunction(pin, pinEntry->value);
    }
  }
}
/*----------------------------------------------------------------------------*/
uint8_t qspiGetMode(const struct QspiBase *interface)
{
  const NM_QSPI_Type * const reg = interface->reg;
  const uint32_t ctl = reg->CTL;
  uint8_t mode = 0;

  if (ctl & CTL_RXNEG)
    mode |= 0x01;
  if (ctl & CTL_CLKPOL)
    mode |= 0x02;

  return mode;
}
/*----------------------------------------------------------------------------*/
uint32_t qspiGetRate(const struct QspiBase *interface)
{
  NM_QSPI_Type * const reg = interface->reg;
  return qspiGetClock(interface) / (reg->CLKDIV + 1);
}
/*----------------------------------------------------------------------------*/
void qspiSetMode(struct QspiBase *interface, uint8_t mode)
{
  NM_QSPI_Type * const reg = interface->reg;
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
void qspiSetRate(struct QspiBase *interface, uint32_t rate)
{
  const uint32_t clock = qspiGetClock(interface);
  NM_QSPI_Type * const reg = interface->reg;

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
