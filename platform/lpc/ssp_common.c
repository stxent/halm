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
void sspConfigPins(const struct SspBaseConfig *config)
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
          pinArray[index], config->channel);
      assert(pinEntry != NULL);

      const struct Pin pin = pinInit(pinArray[index]);

      pinInput(pin);
      pinSetFunction(pin, pinEntry->value);
    }
  }
}
/*----------------------------------------------------------------------------*/
uint8_t sspGetMode(const struct SspBase *interface)
{
  const LPC_SSP_Type * const reg = interface->reg;
  const uint32_t cr0 = reg->CR0;
  uint8_t mode = 0;

  if (cr0 & CR0_CPHA)
    mode |= 0x01;
  if (cr0 & CR0_CPOL)
    mode |= 0x02;

  return mode;
}
/*----------------------------------------------------------------------------*/
uint32_t sspGetRate(const struct SspBase *interface)
{
  const LPC_SSP_Type * const reg = interface->reg;
  const uint32_t cpsr = reg->CPSR;

  return cpsr ?
      (sspGetClock(interface) / cpsr / (CR0_SCR_VALUE(reg->CR0) + 1)) : 0;
}
/*----------------------------------------------------------------------------*/
void sspSetMode(struct SspBase *interface, uint8_t mode)
{
  LPC_SSP_Type * const reg = interface->reg;
  uint32_t cr0 = reg->CR0 & ~(CR0_CPHA | CR0_CPOL);

  if (mode & 0x01)
    cr0 |= CR0_CPHA;
  if (mode & 0x02)
    cr0 |= CR0_CPOL;

  reg->CR0 = cr0;
}
/*----------------------------------------------------------------------------*/
bool sspSetRate(struct SspBase *interface, uint32_t rate)
{
  if (!rate)
    return false;

  const uint32_t clock = sspGetClock(interface);
  const uint32_t divisor = (clock + (rate - 1)) / rate;

  if (divisor < 2 || divisor > CPSR_MAX * (CR0_SCR_MAX + 1))
    return false;

  uint32_t prescaler0 = (divisor + CR0_SCR_MAX) / (CR0_SCR_MAX + 1);

  if (prescaler0 < CPSR_MIN)
    prescaler0 = CPSR_MIN;
  prescaler0 = (prescaler0 + 1) & 0xFE;

  LPC_SSP_Type * const reg = interface->reg;
  const uint32_t prescaler1 = (divisor + (prescaler0 - 1)) / prescaler0;

  reg->CPSR = prescaler0;
  reg->CR0 = (reg->CR0 & ~CR0_SCR_MASK) | CR0_SCR(prescaler1 - 1);
  return true;
}
