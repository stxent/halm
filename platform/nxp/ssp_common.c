/*
 * ssp_common.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/ssp_base.h>
#include <platform/nxp/ssp_defs.h>
/*----------------------------------------------------------------------------*/
extern const struct GpioDescriptor sspPins[];
/*----------------------------------------------------------------------------*/
enum result sspSetupPins(struct SspBase *interface,
    const struct SspBaseConfig *config)
{
  /* TODO Pin configuration for SSP slave */
  const struct GpioDescriptor *pinDescriptor;
  struct Gpio pin;

  /* Configure MOSI pin */
  if (config->mosi)
  {
    if (!(pinDescriptor = gpioFind(sspPins, config->mosi, interface->channel)))
      return E_VALUE;
    gpioOutput((pin = gpioInit(config->mosi)), 0);
    gpioSetFunction(pin, pinDescriptor->value);
  }

  /* Configure MISO pin */
  if (config->miso)
  {
    if (!(pinDescriptor = gpioFind(sspPins, config->miso, interface->channel)))
      return E_VALUE;
    gpioInput((pin = gpioInit(config->miso)));
    gpioSetFunction(pin, pinDescriptor->value);
  }

  /* Configure SCK pin */
  if (config->sck)
  {
    if (!(pinDescriptor = gpioFind(sspPins, config->sck, interface->channel)))
      return E_VALUE;
    gpioOutput((pin = gpioInit(config->sck)), 0);
    gpioSetFunction(pin, pinDescriptor->value);
  }

  /* Configure Slave Select pin available only in slave mode */
/*  if (!(pinDescriptor = gpioFind(sspPins, config->cs, interface->channel)))
    return E_VALUE;
  gpioInput((pin = gpioInit(config->cs)));
  gpioSetFunction(pin, pinDescriptor->value); */

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
