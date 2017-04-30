/*
 * uart_common.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/stm/uart_base.h>
#include <halm/platform/stm/uart_defs.h>
/*----------------------------------------------------------------------------*/
extern const struct PinEntry uartPins[];
/*----------------------------------------------------------------------------*/
void uartConfigPins(struct UartBase *interface,
    const struct UartBaseConfig *config)
{
  const struct PinEntry *pinEntry;
  struct Pin pin;

  /* Configure UART RX pin */
  pinEntry = pinFind(uartPins, config->rx, interface->channel);
  assert(pinEntry);
  pinInput((pin = pinInit(config->rx)));
  pinSetFunction(pin, pinEntry->value);

  /* Configure UART TX pin */
  pinEntry = pinFind(uartPins, config->tx, interface->channel);
  assert(pinEntry);
  pinOutput((pin = pinInit(config->tx)), true);
  pinSetFunction(pin, pinEntry->value);
}
/*----------------------------------------------------------------------------*/
void uartSetParity(struct UartBase *interface, enum UartParity parity)
{
  STM_USART_Type * const reg = interface->reg;
  uint32_t value = reg->CR1 & ~(CR1_PS | CR1_PCE | CR1_M);

  if (parity != UART_PARITY_NONE)
  {
    value |= CR1_PCE | CR1_M;
    if (parity == UART_PARITY_ODD)
      value |= CR1_PS;
  }

  reg->CR1 = value;
}
/*----------------------------------------------------------------------------*/
void uartSetRate(struct UartBase *interface, uint32_t rate)
{
  assert(rate);

  STM_USART_Type * const reg = interface->reg;
  const uint32_t clock = uartGetClock(interface);

  reg->BRR = (clock + rate / 2) / rate;
}
