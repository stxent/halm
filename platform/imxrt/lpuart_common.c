/*
 * lpuart_common.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/imxrt/lpuart_base.h>
#include <halm/platform/imxrt/lpuart_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
extern const struct PinEntry lpUartPins[];
/*----------------------------------------------------------------------------*/
void uartConfigPins(const struct LpUartBaseConfig *config,
    enum PinDaisyIndex rxDaisy, enum PinDaisyIndex txDaisy)
{
  if (config->rx)
  {
    /* Configure UART RX pin */
    const struct PinEntry * const pinEntry = pinFind(lpUartPins,
        config->rx, config->channel);
    assert(pinEntry != NULL);

    const struct Pin pin = pinInit(config->rx);

    pinInput(pin);
    pinSetFunction(pin, UNPACK_FUNCTION(pinEntry->value));
    pinSetDaisyChain(rxDaisy, UNPACK_DAISY(pinEntry->value));
  }

  if (config->tx)
  {
    /* Configure UART TX pin */
    const struct PinEntry * const pinEntry = pinFind(lpUartPins,
        config->tx, config->channel);
    assert(pinEntry != NULL);

    const struct Pin pin = pinInit(config->tx);

    pinOutput(pin, true);
    pinSetFunction(pin, UNPACK_FUNCTION(pinEntry->value));
    pinSetDaisyChain(txDaisy, UNPACK_DAISY(pinEntry->value));
  }
}
/*----------------------------------------------------------------------------*/
enum SerialParity uartGetParity(const struct LpUartBase *interface)
{
  const IMX_LPUART_Type * const reg = interface->reg;
  const uint32_t control = reg->CTRL;

  if (control & CTRL_PE)
    return (control & CTRL_PT) ? SERIAL_PARITY_ODD : SERIAL_PARITY_EVEN;
  else
    return SERIAL_PARITY_NONE;
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetRate(const struct LpUartBase *interface)
{
  const IMX_LPUART_Type * const reg = interface->reg;
  const uint32_t baud = reg->BAUD;
  const uint32_t divisor = (BAUD_OSR_VALUE(baud) + 1) * BAUD_SBR_VALUE(baud);

  return divisor ? (uartGetClock(interface) / divisor) : 0;
}
/*----------------------------------------------------------------------------*/
void uartSetParity(struct LpUartBase *interface, enum SerialParity parity)
{
  IMX_LPUART_Type * const reg = interface->reg;
  uint32_t control = reg->CTRL & ~(CTRL_PT | CTRL_PE | CTRL_M);

  if (parity != SERIAL_PARITY_NONE)
  {
    control |= CTRL_PE | CTRL_M;
    if (parity == SERIAL_PARITY_ODD)
      control |= CTRL_PT;
  }

  reg->CTRL = control;
}
/*----------------------------------------------------------------------------*/
bool uartSetRate(struct LpUartBase *interface, uint32_t rate)
{
  if (!rate)
    return false;

  IMX_LPUART_Type * const reg = interface->reg;
  const uint32_t baud = reg->BAUD & ~BAUD_SBR_MASK;
  const uint32_t enabled = reg->CTRL & (CTRL_RE | CTRL_TE);

  const uint32_t clock = uartGetClock(interface);
  const uint32_t oversampling = BAUD_OSR_VALUE(baud) + 1;
  const uint32_t sbr = (clock / oversampling + (rate >> 1)) / rate;

  if (sbr <= BAUD_SBR_MAX)
  {
    reg->CTRL &= ~(CTRL_RE | CTRL_TE);
    reg->BAUD = baud | BAUD_SBR(sbr);
    reg->CTRL |= enabled;

    return true;
  }
  else
    return false;
}
