/*
 * uart_common.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/uart_base.h>
#include <halm/platform/numicro/uart_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
extern const struct PinEntry uartPins[];
/*----------------------------------------------------------------------------*/
void uartConfigPins(const struct UartBaseConfig *config)
{
  /* Direction configuration is not needed for alternate function pins */

  if (config->rx)
  {
    /* Configure UART RX pin */
    const struct PinEntry * const pinEntry = pinFind(uartPins,
        config->rx, config->channel);
    assert(pinEntry != NULL);

    const struct Pin pin = pinInit(config->rx);

    pinInput(pin);
    pinSetFunction(pin, pinEntry->value);
  }

  if (config->tx)
  {
    /* Configure UART TX pin */
    const struct PinEntry * const pinEntry = pinFind(uartPins,
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
  const NM_UART_Type * const reg = interface->reg;
  const uint32_t line = reg->LINE;

  if (line & LINE_PBE)
  {
    if (line & LINE_EPE)
      return SERIAL_PARITY_EVEN;
    else
      return SERIAL_PARITY_ODD;
  }
  else
    return SERIAL_PARITY_NONE;
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetRate(const struct UartBase *interface)
{
  NM_UART_Type * const reg = interface->reg;
  const uint32_t clock = uartGetClock(interface);
  const uint32_t baud = reg->BAUD;
  const uint32_t brd = BAUD_BRD_VALUE(baud);

  switch (BAUD_BAUDM_VALUE(baud))
  {
    case BAUDM_MODE_0:
      return clock / ((brd + 2) * 16);

    case BAUDM_MODE_1:
      return clock / ((BAUD_EDIVM1_VALUE(baud) + 1) * (brd + 2));

    default:
      return clock / (brd + 2);
  }
}
/*----------------------------------------------------------------------------*/
void uartSetParity(struct UartBase *interface, enum SerialParity parity)
{
  NM_UART_Type * const reg = interface->reg;
  uint32_t line = reg->LINE & ~(LINE_PBE | LINE_EPE);

  if (parity != SERIAL_PARITY_NONE)
  {
    line |= LINE_PBE;

    if (parity == SERIAL_PARITY_EVEN)
      line |= LINE_EPE;
  }

  reg->LINE = line;
}
/*----------------------------------------------------------------------------*/
bool uartSetRate(struct UartBase *interface, uint32_t rate)
{
  if (!rate)
    return false;

  NM_UART_Type * const reg = interface->reg;
  const uint32_t clock = uartGetClock(interface);
  uint32_t divisor = ((clock >> 4) + (rate >> 1)) / rate;

  if (divisor >= 2 && divisor <= BAUD_BRD_MAX + 2)
  {
    reg->BAUD = BAUD_BRD(divisor - 2);
    return true;
  }
  else
    return false;
}
