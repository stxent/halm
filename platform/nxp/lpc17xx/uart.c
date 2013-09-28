/*
 * uart.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include "platform/nxp/system.h"
#include "platform/nxp/uart.h"
#include "platform/nxp/uart_defs.h"
#include "platform/nxp/lpc17xx/interrupts.h"
#include "platform/nxp/lpc17xx/power.h"
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV       CLK_DIV1
#define DEFAULT_DIV_VALUE 1
/*----------------------------------------------------------------------------*/
static const struct GpioDescriptor uartPins[] = {
    {
        .key = GPIO_TO_PIN(0, 0), /* UART_TXD3 */
        .channel = 3,
        .value = 2
    }, {
        .key = GPIO_TO_PIN(0, 1), /* UART_RXD3 */
        .channel = 3,
        .value = 2
    }, {
        .key = GPIO_TO_PIN(0, 2), /* UART_TXD0 */
        .channel = 0,
        .value = 1
    }, {
        .key = GPIO_TO_PIN(0, 3), /* UART_RXD0 */
        .channel = 0,
        .value = 1
    }, {
        .key = GPIO_TO_PIN(0, 10), /* UART_TXD2 */
        .channel = 2,
        .value = 1
    }, {
        .key = GPIO_TO_PIN(0, 11), /* UART_RXD2 */
        .channel = 2,
        .value = 1
    }, {
        .key = GPIO_TO_PIN(0, 15), /* UART_TXD1 */
        .channel = 1,
        .value = 1
    }, {
        .key = GPIO_TO_PIN(0, 16), /* UART_RXD1 */
        .channel = 1,
        .value = 1
    }, {
        .key = GPIO_TO_PIN(0, 25), /* UART_TXD3 */
        .channel = 3,
        .value = 3
    }, {
        .key = GPIO_TO_PIN(0, 26), /* UART_RXD3 */
        .channel = 3,
        .value = 3
    }, {
        .key = GPIO_TO_PIN(2, 0), /* UART_TXD1 */
        .channel = 1,
        .value = 2
    }, {
        .key = GPIO_TO_PIN(2, 1), /* UART_RXD1 */
        .channel = 1,
        .value = 2
    }, {
        .key = GPIO_TO_PIN(2, 8), /* UART_TXD2 */
        .channel = 2,
        .value = 2
    }, {
        .key = GPIO_TO_PIN(2, 9), /* UART_RXD2 */
        .channel = 2,
        .value = 2
    }, {
        .key = GPIO_TO_PIN(4, 28), /* UART_TXD3 */
        .channel = 3,
        .value = 3
    }, {
        .key = GPIO_TO_PIN(4, 29), /* UART_RXD3 */
        .channel = 3,
        .value = 3
    }, {
        /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t, struct Uart *);
static inline enum result setupPins(struct Uart *, const struct UartConfig *);
/*----------------------------------------------------------------------------*/
static enum result uartInit(void *, const void *);
static void uartDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass uartTable = {
    .size = 0, /* Abstract class */
    .init = uartInit,
    .deinit = uartDeinit,

    .callback = 0,
    .get = 0,
    .set = 0,
    .read = 0,
    .write = 0
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass *Uart = &uartTable;
/*----------------------------------------------------------------------------*/
static struct Uart *descriptors[] = {0, 0, 0, 0};
/*----------------------------------------------------------------------------*/
void UART0_ISR(void)
{
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void UART1_ISR(void)
{
  if (descriptors[1])
    descriptors[1]->handler(descriptors[1]);
}
/*----------------------------------------------------------------------------*/
void UART2_ISR(void)
{
  if (descriptors[2])
    descriptors[2]->handler(descriptors[2]);
}
/*----------------------------------------------------------------------------*/
void UART3_ISR(void)
{
  if (descriptors[3])
    descriptors[3]->handler(descriptors[3]);
}
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel, struct Uart *interface)
{
  assert(channel < sizeof(descriptors));

  if (descriptors[channel])
    return E_BUSY;

  descriptors[channel] = interface;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static inline enum result setupPins(struct Uart *interface,
    const struct UartConfig *config)
{
  const struct GpioDescriptor *pin;

  /* Setup UART RX pin */
  if (!(pin = gpioFind(uartPins, config->rx, interface->channel)))
    return E_VALUE;
  interface->rxPin = gpioInit(config->rx, GPIO_INPUT);
  gpioSetFunction(&interface->rxPin, pin->value);

  /* Setup UART TX pin */
  if (!(pin = gpioFind(uartPins, config->tx, interface->channel)))
    return E_VALUE;
  interface->txPin = gpioInit(config->tx, GPIO_OUTPUT);
  gpioSetFunction(&interface->txPin, pin->value);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
struct UartRateConfig uartCalcRate(uint32_t rate)
{
  uint32_t divisor;
  struct UartRateConfig config = {0, 0, 0x10};

  if (!rate)
    return config;

  divisor = ((sysCoreClock / DEFAULT_DIV_VALUE) >> 4) / rate;
  if (divisor && divisor < (1 << 16))
  {
    config.high = (uint8_t)(divisor >> 8);
    config.low = (uint8_t)divisor;
    /* TODO Add fractional part calculation */
  }
  return config;
}
/*----------------------------------------------------------------------------*/
void uartSetRate(struct Uart *interface, struct UartRateConfig config)
{
  LPC_UART_TypeDef *reg = interface->reg;

  /* UART should not be used during this command sequence */
  reg->LCR |= LCR_DLAB; /* Enable DLAB access */
  reg->DLL = config.low;
  reg->DLM = config.high;
  reg->FDR = config.fraction;
  reg->LCR &= ~LCR_DLAB; /* Disable DLAB access */
}
/*----------------------------------------------------------------------------*/
static enum result uartInit(void *object, const void *configPtr)
{
  const struct UartConfig * const config = configPtr;
  struct Uart *interface = object;
  enum result res;

  /* Try to set peripheral descriptor */
  interface->channel = config->channel;
  if ((res = setDescriptor(interface->channel, interface)) != E_OK)
    return res;

  if ((res = setupPins(interface, config)) != E_OK)
    return res;

  interface->handler = 0;

  switch (interface->channel)
  {
    case 0:
      sysPowerEnable(PWR_UART0);
      sysClockControl(CLK_UART0, DEFAULT_DIV);
      interface->reg = LPC_UART0;
      interface->irq = UART0_IRQ;
      break;
    case 1:
      sysPowerEnable(PWR_UART1);
      sysClockControl(CLK_UART1, DEFAULT_DIV);
      interface->reg = (LPC_UART_TypeDef *)LPC_UART1;
      interface->irq = UART1_IRQ;
      break;
    case 2:
      sysPowerEnable(PWR_UART2);
      sysClockControl(CLK_UART2, DEFAULT_DIV);
      interface->reg = LPC_UART2;
      interface->irq = UART2_IRQ;
      break;
    case 3:
      sysPowerEnable(PWR_UART3);
      sysClockControl(CLK_UART3, DEFAULT_DIV);
      interface->reg = LPC_UART3;
      interface->irq = UART3_IRQ;
      break;
  }

  /* Initialize UART block */
  LPC_UART_TypeDef *reg = interface->reg;

  /* Set initial values for UART registers */
  reg->FCR = FCR_ENABLE;
  reg->IER = 0;
  reg->TER = TER_TXEN;
  reg->LCR = LCR_WORD_8BIT; /* Set 8-bit length */

  /* Set parity */
  if (config->parity != UART_PARITY_NONE)
  {
    reg->LCR |= LCR_PARITY;
    if (config->parity == UART_PARITY_EVEN)
      reg->LCR |= LCR_PARITY_EVEN;
    else
      reg->LCR |= LCR_PARITY_ODD;
  }

  uartSetRate(object, uartCalcRate(config->rate));

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void uartDeinit(void *object)
{
  const enum sysPowerDevice uartPower[] = {
      PWR_UART0, PWR_UART1, PWR_UART2, PWR_UART3
  };
  struct Uart *interface = object;

  /* Disable UART peripheral power */
  sysPowerDisable(uartPower[interface->channel]);
  gpioDeinit(&interface->txPin);
  gpioDeinit(&interface->rxPin);
  /* Reset UART descriptor */
  setDescriptor(interface->channel, 0);
}
