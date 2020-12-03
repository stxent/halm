/*
 * ssp_common.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/ssp_base.h>
#include <halm/platform/lpc/ssp_defs.h>
#include <assert.h>
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
      const struct PinEntry * const pinEntry = pinFind(sspPins,
          pinArray[index], interface->channel);
      assert(pinEntry);

      const struct Pin pin = pinInit(pinArray[index]);

      pinInput(pin);
      pinSetFunction(pin, pinEntry->value);
    }
  }
}
/*----------------------------------------------------------------------------*/
uint32_t sspGetRate(const struct SspBase *interface)
{
  LPC_SSP_Type * const reg = interface->reg;
  const uint32_t value = reg->CPSR;

  return value ?
      (sspGetClock(interface) / value / (CR0_SCR_VALUE(reg->CR0) + 1)) : 0;
}
/*----------------------------------------------------------------------------*/
void sspSetRate(struct SspBase *interface, uint32_t rate)
{
  LPC_SSP_Type * const reg = interface->reg;
  uint32_t clockPrescaleReg;
  uint32_t controlReg0 = reg->CR0 & ~CR0_SCR_MASK;

  if (rate)
  {
    const uint32_t clock = sspGetClock(interface);
    uint32_t divisor = ((clock + (rate - 1)) >> 1) / rate - 1;

    if (divisor >= 127 * 256)
      divisor = 127 * 256 - 1;

    const uint32_t prescaler = 1 + (divisor >> 8);

    clockPrescaleReg = prescaler << 1;
    controlReg0 |= CR0_SCR(divisor / prescaler);
  }
  else
    clockPrescaleReg = 0;

  reg->CPSR = clockPrescaleReg;
  reg->CR0 = controlReg0;
}
