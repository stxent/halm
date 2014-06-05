/*
 * uart_common.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/nxp/uart_base.h>
#include <platform/nxp/uart_defs.h>
/*----------------------------------------------------------------------------*/
extern const struct GpioDescriptor uartPins[];
/*----------------------------------------------------------------------------*/
enum result uartSetupPins(struct UartBase *interface,
    const struct UartBaseConfig *config)
{
  const struct GpioDescriptor *pinDescriptor;
  struct Gpio pin;

  /* Setup UART RX pin */
  if (!(pinDescriptor = gpioFind(uartPins, config->rx, interface->channel)))
    return E_VALUE;
  gpioInput((pin = gpioInit(config->rx)));
  gpioSetFunction(pin, pinDescriptor->value);

  /* Setup UART TX pin */
  if (!(pinDescriptor = gpioFind(uartPins, config->tx, interface->channel)))
    return E_VALUE;
  gpioOutput((pin = gpioInit(config->tx)), 1);
  gpioSetFunction(pin, pinDescriptor->value);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
enum result uartCalcRate(struct UartBase *interface, uint32_t rate,
    struct UartRateConfig *output)
{
  if (!rate)
    return E_VALUE;

  uint32_t divisor = (uartGetClock(interface) >> 4) / rate;

  if (divisor && divisor < (1 << 16))
  {
    output->divisor = (uint16_t)divisor;
    output->fraction = 0x10;
    /* TODO Add fractional part calculation */

    return E_OK;
  }
  else
    return E_ERROR;
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetRate(struct UartBase *interface)
{
  LPC_UART_Type *reg = interface->reg;
  uint32_t rate = uartGetClock(interface) >> 4;
  struct UartRateConfig config;

  reg->LCR |= LCR_DLAB; /* Enable DLAB access */
  config.divisor = ((reg->DLM & 0xFF) << 8) | (reg->DLL & 0xFF);
  config.fraction = reg->FDR;
  reg->LCR &= ~LCR_DLAB; /* Disable DLAB access */

  return !config.divisor ? 0 : rate / config.divisor;
}
/*----------------------------------------------------------------------------*/
void uartSetParity(struct UartBase *interface, enum uartParity parity)
{
  LPC_UART_Type *reg = interface->reg;

  if (parity != UART_PARITY_NONE)
  {
    reg->LCR |= LCR_PARITY;
    reg->LCR |= parity == UART_PARITY_EVEN ? LCR_PARITY_EVEN : LCR_PARITY_ODD;
  }
}
/*----------------------------------------------------------------------------*/
void uartSetRate(struct UartBase *interface, struct UartRateConfig config)
{
  LPC_UART_Type *reg = interface->reg;

  reg->LCR |= LCR_DLAB; /* Enable DLAB access */
  reg->DLL = config.divisor & 0xFF;
  reg->DLM = config.divisor >> 8;
  reg->FDR = config.fraction;
  reg->LCR &= ~LCR_DLAB; /* Disable DLAB access */
}
