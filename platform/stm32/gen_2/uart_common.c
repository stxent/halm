/*
 * uart_common.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/uart_base.h>
#include <halm/platform/stm32/uart_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
extern const struct PinEntry uartPins[];
/*----------------------------------------------------------------------------*/
void uartConfigPins(const struct UartBaseConfig *config)
{
  const PinNumber pinArray[] = {
      config->rx, config->tx
  };

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      const struct PinEntry * const pinEntry = pinFind(uartPins,
          pinArray[index], CHANNEL_INDEX(config->channel, index));
      assert(pinEntry != NULL);

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
enum SerialStopBits uartGetStopBits(const struct UartBase *interface)
{
  const STM_USART_Type * const reg = interface->reg;
  enum SerialStopBits value;

  switch (CR2_STOP_VALUE(reg->CR2))
  {
    case STOP_1_BIT:
      value = SERIAL_STOPBITS_1;
      break;
    case STOP_0_5_BIT:
      value = SERIAL_STOPBITS_0P5;
      break;
    case STOP_2_BITS:
      value = SERIAL_STOPBITS_2;
      break;
    case STOP_1_5_BITS:
      value = SERIAL_STOPBITS_1P5;
      break;
  }

  return value;
}
/*----------------------------------------------------------------------------*/
void uartSetParity(struct UartBase *interface, enum SerialParity parity)
{
  STM_USART_Type * const reg = interface->reg;
  const uint32_t enabled = reg->CR1 & CR1_UE;
  uint32_t value = reg->CR1 & ~(CR1_PS | CR1_PCE | CR1_M0 | CR1_UE);

  if (parity != SERIAL_PARITY_NONE)
  {
    value |= CR1_PCE | CR1_M0;
    if (parity == SERIAL_PARITY_ODD)
      value |= CR1_PS;
  }

  reg->CR1 &= ~CR1_UE;
  reg->CR1 = value;
  reg->CR1 |= enabled;
}
/*----------------------------------------------------------------------------*/
bool uartSetRate(struct UartBase *interface, uint32_t rate)
{
  if (!rate)
    return false;

  STM_USART_Type * const reg = interface->reg;
  const uint32_t clock = uartGetClock(interface);
  const uint32_t divisor = (clock + (rate >> 1)) / rate;
  const uint32_t enabled = reg->CR1 & CR1_UE;

  if (divisor <= BRR_DIV_MAX)
  {
    reg->CR1 &= ~CR1_UE;
    reg->BRR = BRR_DIV(divisor);
    reg->CR1 |= enabled;

    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void uartSetStopBits(struct UartBase *interface, enum SerialStopBits number)
{
  STM_USART_Type * const reg = interface->reg;
  const uint32_t enabled = reg->CR1 & CR1_UE;
  uint32_t value = reg->CR2 & ~CR2_STOP_MASK;

  switch (number)
  {
    case SERIAL_STOPBITS_2:
      value |= STOP_2_BITS;
      break;
    case SERIAL_STOPBITS_0P5:
      value |= STOP_0_5_BIT;
      break;
    case SERIAL_STOPBITS_1P5:
      value |= STOP_1_5_BITS;
      break;
    default:
      value |= STOP_1_BIT;
      break;
  }

  reg->CR1 &= ~CR1_UE;
  reg->CR2 = value;
  reg->CR1 |= enabled;
}
