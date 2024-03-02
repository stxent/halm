/*
 * uart_common.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gen_1/uart_defs.h>
#include <halm/platform/lpc/uart_base.h>
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
bool uartCalcRate(const struct UartBase *interface, uint32_t rate,
    struct UartRateConfig *output)
{
  if (!rate)
    return false;

  const uint32_t clock = uartGetClock(interface);
  const uint32_t divisor = ((clock >> 4) + (rate >> 1)) / rate;

  if (divisor && divisor < (1 << 16))
  {
    output->divisor = (uint16_t)divisor;
    output->fraction = 0x10;
    /* TODO Add fractional part calculation */

    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
enum SerialParity uartGetParity(const struct UartBase *interface)
{
  const LPC_UART_Type * const reg = interface->reg;
  const uint32_t lcr = reg->LCR;

  if (lcr & LCR_PE)
  {
    switch (LCR_PS_VALUE(lcr))
    {
      case PS_ODD:
        return SERIAL_PARITY_ODD;

      case PS_EVEN:
        return SERIAL_PARITY_EVEN;

      default:
        return SERIAL_PARITY_NONE;
    }
  }
  else
    return SERIAL_PARITY_NONE;
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetRate(const struct UartBase *interface)
{
  LPC_UART_Type * const reg = interface->reg;
  const uint32_t rate = uartGetClock(interface) >> 4;
  struct UartRateConfig config;

  reg->LCR |= LCR_DLAB; /* Enable DLAB access */
  config.divisor = ((reg->DLM & 0xFF) << 8) | (reg->DLL & 0xFF);
  config.fraction = reg->FDR;
  reg->LCR &= ~LCR_DLAB; /* Disable DLAB access */

  return !config.divisor ? 0 : rate / config.divisor;
}
/*----------------------------------------------------------------------------*/
void uartSetParity(struct UartBase *interface, enum SerialParity parity)
{
  LPC_UART_Type * const reg = interface->reg;
  uint32_t value = reg->LCR & ~(LCR_PE | LCR_PS_MASK);

  if (parity != SERIAL_PARITY_NONE)
  {
    value |= LCR_PE;
    value |= parity == SERIAL_PARITY_EVEN ? LCR_PS(PS_EVEN) : LCR_PS(PS_ODD);
  }

  reg->LCR = value;
}
/*----------------------------------------------------------------------------*/
void uartSetRate(struct UartBase *interface, struct UartRateConfig config)
{
  LPC_UART_Type * const reg = interface->reg;

  reg->LCR |= LCR_DLAB; /* Enable DLAB access */
  reg->DLL = config.divisor & 0xFF;
  reg->DLM = config.divisor >> 8;
  reg->FDR = config.fraction;
  reg->LCR &= ~LCR_DLAB; /* Disable DLAB access */
}
