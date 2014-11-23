/*
 * ssp_common.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/ssp_base.h>
#include <platform/nxp/ssp_defs.h>
/*----------------------------------------------------------------------------*/
extern const struct PinEntry sspPins[];
/*----------------------------------------------------------------------------*/
enum result sspConfigPins(struct SspBase *interface,
    const struct SspBaseConfig *config)
{
  const pin_t pinArray[4] = {
      config->miso, config->mosi, config->sck, config->cs
  };
  const struct PinEntry *pinEntry;
  struct Pin pin;

  /* Direction configuration is not needed for alternate function pins */
  for (uint8_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      if (!(pinEntry = pinFind(sspPins, pinArray[index], interface->channel)))
        return E_VALUE;
      pinInput((pin = pinInit(pinArray[index])));
      pinSetFunction(pin, pinEntry->value);
    }
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
enum result sspSetRate(struct SspBase *interface, uint32_t rate)
{
  if (!rate)
    return E_VALUE;

  const uint32_t divisor =
      ((sspGetClock(interface) + (rate >> 1)) >> 1) / rate - 1;

  assert(divisor && divisor < 127 * 256);

  LPC_SSP_Type * const reg = interface->reg;
  const uint8_t prescaler = 1 + (divisor >> 8);

  reg->CPSR = prescaler << 1;
  reg->CR0 = (reg->CR0 & ~CR0_SCR_MASK) | CR0_SCR(divisor / prescaler);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
uint32_t sspGetRate(const struct SspBase *interface)
{
  LPC_SSP_Type * const reg = interface->reg;

  if (!reg->CPSR)
    return 0;

  return sspGetClock(interface) / reg->CPSR / (CR0_SCR_VALUE(reg->CR0) + 1);
}
