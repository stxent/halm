/*
 * ssp_common.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "platform/nxp/ssp_base.h"
#include "platform/nxp/ssp_defs.h"
/*----------------------------------------------------------------------------*/
extern const struct GpioDescriptor sspPins[];
/*----------------------------------------------------------------------------*/
enum result sspSetupPins(struct SspBase *interface,
    const struct SspBaseConfig *config)
{
  const struct GpioDescriptor *pin;

  /* Setup MISO pin */
  if (!(pin = gpioFind(sspPins, config->miso, interface->channel)))
    return E_VALUE;
  interface->misoPin = gpioInit(config->miso, GPIO_INPUT);
  gpioSetFunction(&interface->misoPin, pin->value);

  /* Setup MOSI pin */
  if (!(pin = gpioFind(sspPins, config->mosi, interface->channel)))
    return E_VALUE;
  interface->mosiPin = gpioInit(config->mosi, GPIO_OUTPUT);
  gpioSetFunction(&interface->mosiPin, pin->value);

  /* Setup Serial Clock pin */
  if (!(pin = gpioFind(sspPins, config->sck, interface->channel)))
    return E_VALUE;
  interface->sckPin = gpioInit(config->sck, GPIO_OUTPUT);
  gpioSetFunction(&interface->sckPin, pin->value);

  /* Setup CS pin, only in slave mode */
//  if (!(pin = gpioFind(sspPins, config->cs, interface->channel)))
//    return E_VALUE;
//  interface->csPin = gpioInit(config->cs, GPIO_INPUT);
//  gpioSetFunction(&interface->csPin, pin->value);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
void sspSetRate(struct SspBase *interface, uint32_t rate)
{
  LPC_SSP_TypeDef *reg = interface->reg;
  uint16_t divider;

  divider = (sspGetClock(interface) >> 1) / rate - 1;
  /* FIXME Add more precise calculation of SSP dividers */
  reg->CPSR = 2;
  reg->CR0 &= ~CR0_SCR_MASK;
  reg->CR0 |= CR0_SCR(divider);
}
/*----------------------------------------------------------------------------*/
uint32_t sspGetRate(struct SspBase *interface)
{
  uint32_t rate;
  uint16_t divider;

  /* FIXME Add more precise calculation of SSP rate */
  divider = CR0_SCR_VALUE(((LPC_SSP_TypeDef *)interface->reg)->CR0);
  rate = (sspGetClock(interface) >> 1) / (divider + 1);
  return rate;
}
