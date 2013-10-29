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
struct UartRateConfig uartCalcRate(struct UartBase *interface, uint32_t rate)
{
  uint32_t divisor;
  struct UartRateConfig config = {0, 0, 0x10};

  if (!rate)
    return config;

  divisor = (uartGetClock(interface) >> 4) / rate;
  if (divisor && divisor < (1 << 16))
  {
    config.high = (uint8_t)(divisor >> 8);
    config.low = (uint8_t)divisor;
    /* TODO Add fractional part calculation */
  }
  return config;
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetRate(struct UartBase *interface)
{
  LPC_UART_Type *reg = interface->reg;
  uint32_t rate = uartGetClock(interface) >> 4;
  struct UartRateConfig config;

  /* UART should not be used during this command sequence */
  reg->LCR |= LCR_DLAB; /* Enable DLAB access */
  config.low = reg->DLL;
  config.high = reg->DLM;
  config.fraction = reg->FDR;
  reg->LCR &= ~LCR_DLAB; /* Disable DLAB access */

  if (!(config.low || config.high))
    return 0;
  rate /= ((uint16_t)config.high << 8) | (uint16_t)config.low;

  return rate;
}
/*----------------------------------------------------------------------------*/
void uartSetParity(struct UartBase *interface, enum uartParity parity)
{
  LPC_UART_Type *reg = interface->reg;

  if (parity != UART_PARITY_NONE)
  {
    reg->LCR |= LCR_PARITY;
    if (parity == UART_PARITY_EVEN)
      reg->LCR |= LCR_PARITY_EVEN;
    else
      reg->LCR |= LCR_PARITY_ODD;
  }
}
/*----------------------------------------------------------------------------*/
void uartSetRate(struct UartBase *interface, struct UartRateConfig config)
{
  LPC_UART_Type *reg = interface->reg;

  /* UART should not be used during this command sequence */
  reg->LCR |= LCR_DLAB; /* Enable DLAB access */
  reg->DLL = config.low;
  reg->DLM = config.high;
  reg->FDR = config.fraction;
  reg->LCR &= ~LCR_DLAB; /* Disable DLAB access */
}
