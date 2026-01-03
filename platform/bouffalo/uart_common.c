/*
 * uart_common.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/bouffalo/uart_base.h>
#include <halm/platform/bouffalo/uart_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
enum SerialParity uartGetParity(const struct UartBase *interface)
{
  const BL_UART_Type * const reg = interface->reg;
  const uint32_t utxConfig = reg->UTX_CONFIG;

  if (utxConfig & UTX_CONFIG_TXPREN)
  {
    return (utxConfig & UTX_CONFIG_TXPRSEL) ?
        SERIAL_PARITY_ODD : SERIAL_PARITY_EVEN;
  }
  else
    return SERIAL_PARITY_NONE;
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetRate(const struct UartBase *interface)
{
  const BL_UART_Type * const reg = interface->reg;
  const uint32_t prescaler = BIT_PRD_TBITPRD_VALUE(reg->BIT_PRD) + 1;

  return uartGetClock(interface) / prescaler;
}
/*----------------------------------------------------------------------------*/
enum SerialStopBits uartGetStopBits(const struct UartBase *interface)
{
  const BL_UART_Type * const reg = interface->reg;
  enum SerialStopBits value;

  switch (UTX_CONFIG_TXBCNTP_VALUE(reg->UTX_CONFIG))
  {
    case BCNTP_0_5_BIT:
      value = SERIAL_STOPBITS_0P5;
      break;
    case BCNTP_1_BIT:
      value = SERIAL_STOPBITS_1;
      break;
    case BCNTP_1_5_BITS:
      value = SERIAL_STOPBITS_1P5;
      break;
    case BCNTP_2_BITS:
      value = SERIAL_STOPBITS_2;
      break;
  }

  return value;
}
/*----------------------------------------------------------------------------*/
void uartSetParity(struct UartBase *interface, enum SerialParity parity)
{
  BL_UART_Type * const reg = interface->reg;
  uint32_t urxConfig =
      reg->URX_CONFIG & ~(URX_CONFIG_RXPREN | URX_CONFIG_RXPRSEL);
  uint32_t utxConfig =
      reg->UTX_CONFIG & ~(UTX_CONFIG_TXPREN | UTX_CONFIG_TXPRSEL);

  if (parity != SERIAL_PARITY_NONE)
  {
    urxConfig |= URX_CONFIG_RXPREN;
    utxConfig |= UTX_CONFIG_TXPREN;

    if (parity == SERIAL_PARITY_ODD)
    {
      urxConfig |= URX_CONFIG_RXPRSEL;
      utxConfig |= UTX_CONFIG_TXPRSEL;
    }
  }

  reg->URX_CONFIG = urxConfig;
  reg->UTX_CONFIG = utxConfig;
}
/*----------------------------------------------------------------------------*/
bool uartSetRate(struct UartBase *interface, uint32_t rate)
{
  if (!rate)
    return false;

  BL_UART_Type * const reg = interface->reg;
  const uint32_t clock = uartGetClock(interface);
  const uint32_t prescaler = (clock + (rate >> 1)) / rate;

  if (prescaler <= BIT_PRD_BITPRD_MAX)
  {
    reg->BIT_PRD = BIT_PRD_TBITPRD(prescaler) | BIT_PRD_RBITPRD(prescaler);
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void uartSetStopBits(struct UartBase *interface, enum SerialStopBits number)
{
  BL_UART_Type * const reg = interface->reg;
  uint32_t utxConfig = reg->UTX_CONFIG & ~UTX_CONFIG_TXBCNTP_MASK;

  switch (number)
  {
    case SERIAL_STOPBITS_2:
      utxConfig |= BCNTP_2_BITS;
      break;
    case SERIAL_STOPBITS_0P5:
      utxConfig |= BCNTP_0_5_BIT;
      break;
    case SERIAL_STOPBITS_1P5:
      utxConfig |= BCNTP_1_5_BITS;
      break;
    default:
      utxConfig |= BCNTP_1_BIT;
      break;
  }

  reg->UTX_CONFIG = utxConfig;
}
