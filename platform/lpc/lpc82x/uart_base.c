/*
 * uart_base.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/system.h>
#include <halm/platform/lpc/uart_base.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void configPins(const struct UartBaseConfig *);
static bool setInstance(uint8_t, struct UartBase *);
/*----------------------------------------------------------------------------*/
static enum Result uartInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_UART_NO_DEINIT
static void uartDeinit(void *);
#else
#  define uartDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const UartBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = uartInit,
    .deinit = uartDeinit
};
/*----------------------------------------------------------------------------*/
static struct UartBase *instances[3] = {NULL};
/*----------------------------------------------------------------------------*/
static void configPins(const struct UartBaseConfig *config)
{
  if (config->rx)
  {
    /* Configure UART RX pin */
    const struct Pin pin = pinInit(config->rx);

    pinInput(pin);
    pinSetMux(pin, PINMUX_UART0_RXD + PINMUX_UART_STRIDE * config->channel);
  }

  if (config->tx)
  {
    /* Configure UART TX pin */
    const struct Pin pin = pinInit(config->tx);

    pinOutput(pin, true);
    pinSetMux(pin, PINMUX_UART0_TXD + PINMUX_UART_STRIDE * config->channel);
  }
}
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct UartBase *object)
{
  assert(channel < ARRAY_SIZE(instances));

  if (instances[channel] == NULL)
  {
    instances[channel] = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void UART0_ISR(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void UART1_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
void UART2_ISR(void)
{
  instances[2]->handler(instances[2]);
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetClock(const struct UartBase *)
{
  const uint32_t frequency = clockFrequency(SystemClock);
  const uint32_t divisor = LPC_SYSCON->UARTCLKDIV;

  return divisor ? frequency / divisor : 0;
}
/*----------------------------------------------------------------------------*/
static enum Result uartInit(void *object, const void *configBase)
{
  const struct UartBaseConfig * const config = configBase;
  struct UartBase * const interface = object;

  if (!setInstance(config->channel, interface))
    return E_BUSY;

  switch (config->channel)
  {
    case 0:
      interface->reg = LPC_USART0;
      break;

    case 1:
      interface->reg = LPC_USART1;
      break;

    case 2:
      interface->reg = LPC_USART2;
      break;

    default:
      return E_ERROR;
  }

  /* Configure input and output pins */
  configPins(config);

  sysClockEnable(CLK_UART0 + config->channel);
  sysResetPulse(RST_UART0 + config->channel);

  interface->channel = config->channel;
  interface->handler = NULL;
  interface->irq = UART0_IRQ + config->channel;

  /* Enable UART clock */
  if (!LPC_SYSCON->UARTCLKDIV)
  {
    sysResetPulse(RST_UARTFRG);

    LPC_SYSCON->UARTFRGMULT = 0;
    LPC_SYSCON->UARTFRGDIV = 255;
    LPC_SYSCON->UARTCLKDIV = 1;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_UART_NO_DEINIT
static void uartDeinit(void *object)
{
  const struct UartBase * const interface = object;
  size_t index;

  sysClockDisable(CLK_UART0 + interface->channel);
  instances[interface->channel] = NULL;

  for (index = 0; index < ARRAY_SIZE(instances); ++index)
  {
    if (instances[index] != NULL)
      break;
  }

  /* Disable UART clock when all peripherals are released */
  if (index == ARRAY_SIZE(instances))
    LPC_SYSCON->UARTCLKDIV = 0;
}
#endif
