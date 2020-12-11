/*
 * uart_common.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/uart_base.h>
#include <halm/platform/stm32/gen_2/uart_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
extern const struct PinEntry uartPins[];
/*----------------------------------------------------------------------------*/
void uartConfigPins(struct UartBase *interface,
    const struct UartBaseConfig *config)
{
  const PinNumber pinArray[] = {
      config->rx, config->tx
  };

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      const struct PinEntry * const pinEntry = pinFind(uartPins,
          pinArray[index], CHANNEL_INDEX(interface->channel, index));
      assert(pinEntry);

      const struct Pin pin = pinInit(pinArray[index]);

      pinInput(pin);
      pinSetFunction(pin, pinEntry->value);
    }
  }
}
/*----------------------------------------------------------------------------*/
enum SerialParity uartGetParity(const struct UartBase *interface)
{
  const STM_USART_Type * const reg = interface->reg;
  const uint32_t value = reg->CR1;

  if (value & CR1_PCE)
    return (value & CR1_PS) ? SERIAL_PARITY_ODD : SERIAL_PARITY_EVEN;
  else
    return SERIAL_PARITY_NONE;
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetRate(const struct UartBase *interface)
{
  const STM_USART_Type * const reg = interface->reg;
  const uint32_t value = reg->BRR;

  return value ? (uartGetClock(interface) / value) : 0;
}
/*----------------------------------------------------------------------------*/
void uartSetParity(struct UartBase *interface, enum SerialParity parity)
{
  STM_USART_Type * const reg = interface->reg;
  uint32_t value = reg->CR1 & ~(CR1_PS | CR1_PCE | CR1_M0);

  if (parity != SERIAL_PARITY_NONE)
  {
    value |= CR1_PCE | CR1_M0;
    if (parity == SERIAL_PARITY_ODD)
      value |= CR1_PS;
  }

  reg->CR1 = value;
}
/*----------------------------------------------------------------------------*/
void uartSetRate(struct UartBase *interface, uint32_t rate)
{
  STM_USART_Type * const reg = interface->reg;
  const uint32_t clock = uartGetClock(interface);
  const uint32_t enabled = reg->CR1 & CR1_UE;

  reg->CR1 &= ~CR1_UE;
  reg->BRR = rate ? ((clock + rate / 2) / rate) : 0;
  reg->CR1 |= enabled;
}
