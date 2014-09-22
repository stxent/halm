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
enum result sspSetupPins(struct SspBase *interface,
    const struct SspBaseConfig *config)
{
  const struct PinEntry *pinEntry;
  struct Pin pin;

  /* Direction configuration is not needed for alternate function pins */

  /* Configure MOSI pin */
  if (config->mosi)
  {
    if (!(pinEntry = pinFind(sspPins, config->mosi, interface->channel)))
      return E_VALUE;
    pinInput((pin = pinInit(config->mosi)));
    pinSetFunction(pin, pinEntry->value);
  }

  /* Configure MISO pin */
  if (config->miso)
  {
    if (!(pinEntry = pinFind(sspPins, config->miso, interface->channel)))
      return E_VALUE;
    pinInput((pin = pinInit(config->miso)));
    pinSetFunction(pin, pinEntry->value);
  }

  /* Configure SCK pin */
  if (config->sck)
  {
    if (!(pinEntry = pinFind(sspPins, config->sck, interface->channel)))
      return E_VALUE;
    pinInput((pin = pinInit(config->sck)));
    pinSetFunction(pin, pinEntry->value);
  }

  /* Slave Select pin configuration needed only in slave mode */
  if (config->cs)
  {
    if (!(pinEntry = pinFind(sspPins, config->cs, interface->channel)))
      return E_VALUE;
    pinInput((pin = pinInit(config->cs)));
    pinSetFunction(pin, pinEntry->value);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
enum result sspSetRate(struct SspBase *interface, uint32_t rate)
{
  if (!rate)
    return E_VALUE;

  const uint32_t divisor = ((sspGetClock(interface) + (rate >> 1)) >> 1)
      / rate - 1;

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
