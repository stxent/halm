/*
 * ssp_common.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/ssp_base.h>
#include <halm/platform/nxp/ssp_defs.h>
/*----------------------------------------------------------------------------*/
extern const struct PinEntry sspPins[];
/*----------------------------------------------------------------------------*/
void sspConfigPins(struct SspBase *interface,
    const struct SspBaseConfig *config)
{
  const PinNumber pinArray[] = {
      config->miso, config->mosi, config->sck, config->cs
  };

  /* Direction configuration is not needed for alternate function pins */
  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      const struct PinEntry * const pinEntry = pinFind(sspPins, pinArray[index],
          interface->channel);
      assert(pinEntry);

      const struct Pin pin = pinInit(pinArray[index]);

      pinInput(pin);
      pinSetFunction(pin, pinEntry->value);
    }
  }
}
/*----------------------------------------------------------------------------*/
void sspSetRate(struct SspBase *interface, uint32_t rate)
{
  assert(rate != 0);

  const uint32_t clock = sspGetClock(interface);
  uint32_t divisor = ((clock + (rate - 1)) >> 1) / rate - 1;

  if (divisor >= 127 * 256)
    divisor = 127 * 256 - 1;

  LPC_SSP_Type * const reg = interface->reg;
  const unsigned int prescaler = 1 + (divisor >> 8);

  reg->CPSR = prescaler << 1;
  reg->CR0 = (reg->CR0 & ~CR0_SCR_MASK) | CR0_SCR(divisor / prescaler);
}
/*----------------------------------------------------------------------------*/
uint32_t sspGetRate(const struct SspBase *interface)
{
  LPC_SSP_Type * const reg = interface->reg;

  if (!reg->CPSR)
    return 0;

  return sspGetClock(interface) / reg->CPSR / (CR0_SCR_VALUE(reg->CR0) + 1);
}
