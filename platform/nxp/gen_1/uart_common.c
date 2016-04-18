/*
 * uart_common.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/gen_1/uart_base.h>
#include <platform/nxp/gen_1/uart_defs.h>
/*----------------------------------------------------------------------------*/
extern const struct PinEntry uartPins[];
/*----------------------------------------------------------------------------*/
void uartConfigPins(struct UartBase *interface,
    const struct UartBaseConfig *config)
{
  const struct PinEntry *pinEntry;
  struct Pin pin;

  /* Direction configuration is not needed for alternate function pins */

  /* Configure UART RX pin */
  pinEntry = pinFind(uartPins, config->rx, interface->channel);
  assert(pinEntry);
  pinInput((pin = pinInit(config->rx)));
  pinSetFunction(pin, pinEntry->value);

  /* Configure UART TX pin */
  pinEntry = pinFind(uartPins, config->tx, interface->channel);
  assert(pinEntry);
  pinInput((pin = pinInit(config->tx)));
  pinSetFunction(pin, pinEntry->value);
}
/*----------------------------------------------------------------------------*/
enum result uartCalcRate(const struct UartBase *interface, uint32_t rate,
    struct UartRateConfig *output)
{
  if (!rate)
    return E_VALUE;

  const uint32_t divisor =
      ((uartGetClock(interface) + (rate >> 1)) >> 4) / rate;

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
void uartSetParity(struct UartBase *interface, enum uartParity parity)
{
  LPC_UART_Type * const reg = interface->reg;
  uint32_t value = reg->LCR & ~(LCR_PARITY | LCR_PARITY_MASK);

  if (parity != UART_PARITY_NONE)
  {
    value |= LCR_PARITY;
    value |= parity == UART_PARITY_EVEN ? LCR_PARITY_EVEN : LCR_PARITY_ODD;
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
