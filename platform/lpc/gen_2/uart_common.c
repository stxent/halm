/*
 * uart_common.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gen_2/uart_defs.h>
#include <halm/platform/lpc/uart_base.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
void uartConfigPins(const struct UartBaseConfig *config,
    const struct PinEntry *map)
{
  if (config->rx)
  {
    /* Configure UART RX pin */
    const struct PinEntry * const pinEntry = pinFind(map,
        config->rx, config->channel);
    assert(pinEntry != NULL);

    const struct Pin pin = pinInit(config->rx);

    pinInput(pin);
    pinSetFunction(pin, pinEntry->value);
  }

  if (config->tx)
  {
    /* Configure UART TX pin */
    const struct PinEntry * const pinEntry = pinFind(map,
        config->tx, config->channel);
    assert(pinEntry != NULL);

    const struct Pin pin = pinInit(config->tx);

    pinInput(pin);
    pinSetFunction(pin, pinEntry->value);
  }
}
/*----------------------------------------------------------------------------*/
enum SerialParity uartGetParity(const struct UartBase *interface)
{
  const LPC_USART_Type * const reg = interface->reg;

  switch (CFG_PARITYSEL_VALUE(reg->CFG))
  {
    case PARITYSEL_EVEN:
      return SERIAL_PARITY_EVEN;

    case PARITYSEL_ODD:
      return SERIAL_PARITY_ODD;

    default:
      return SERIAL_PARITY_NONE;
  }
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetRate(const struct UartBase *interface)
{
  const LPC_USART_Type * const reg = interface->reg;
  const uint32_t clock = uartGetClock(interface);
  const uint32_t divisor = reg->BRG + 1;

  return (clock >> 4) / divisor;
}
/*----------------------------------------------------------------------------*/
enum SerialStopBits uartGetStopBits(const struct UartBase *interface)
{
  const LPC_USART_Type * const reg = interface->reg;
  return (reg->CFG & CFG_STOPLEN) ? SERIAL_STOPBITS_2 : SERIAL_STOPBITS_1;
}
/*----------------------------------------------------------------------------*/
void uartSetParity(struct UartBase *interface, enum SerialParity parity)
{
  LPC_USART_Type * const reg = interface->reg;
  uint32_t value = reg->CFG & ~(CFG_ENABLE | CFG_PARITYSEL_MASK);

  switch (parity)
  {
    case SERIAL_PARITY_ODD:
      value |= CFG_PARITYSEL(PARITYSEL_ODD);
      break;

    case SERIAL_PARITY_EVEN:
      value |= CFG_PARITYSEL(PARITYSEL_EVEN);
      break;

    default:
      value |= CFG_PARITYSEL(PARITYSEL_NONE);
      break;
  }

  reg->CFG &= ~CFG_ENABLE;
  reg->CFG = value;
  reg->CFG |= CFG_ENABLE;
}
/*----------------------------------------------------------------------------*/
bool uartSetRate(struct UartBase *interface, uint32_t rate)
{
  const uint32_t clock = uartGetClock(interface);
  const uint32_t divisor = ((clock >> 4) + (rate >> 1)) / rate;

  if (divisor <= 65536)
  {
    LPC_USART_Type * const reg = interface->reg;

    reg->CFG &= ~CFG_ENABLE;
    reg->BRG = divisor - 1;
    reg->CFG |= CFG_ENABLE;

    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void uartSetStopBits(struct UartBase *interface, enum SerialStopBits number)
{
  assert(number == SERIAL_STOPBITS_1 || number == SERIAL_STOPBITS_2);

  LPC_USART_Type * const reg = interface->reg;
  uint32_t value = reg->CFG & ~(CFG_ENABLE | CFG_STOPLEN);

  if (number == SERIAL_STOPBITS_2)
    value |= CFG_STOPLEN;

  reg->CFG &= ~CFG_ENABLE;
  reg->CFG = value;
  reg->CFG |= CFG_ENABLE;
}
