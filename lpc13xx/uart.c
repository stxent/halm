/*
 * uart.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include "lpc13xx_sys.h"
#include "mutex.h"
#include "uart.h"
#include "uart_defs.h"
/*----------------------------------------------------------------------------*/
/* In LPC13xx UART clock divisor is number from 1 to 255, 0 to disable */
#define DEFAULT_DIV         1
#define DEFAULT_DIV_VALUE   1
/*----------------------------------------------------------------------------*/
/* UART pin function values */
static const struct GpioPinFunc uartPins[] = {
    {
        .key = GPIO_TO_PIN(1, 6),
        .func = 1
    },
    {
        .key = GPIO_TO_PIN(1, 7),
        .func = 1
    },
    {} /* End of pin function association list */
};
/*----------------------------------------------------------------------------*/
static enum result uartInit(void *, const void *);
static void uartDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass uartTable = {
    .size = 0, /* Abstract class */
    .init = uartInit,
    .deinit = uartDeinit,

    .read = 0,
    .write = 0,
    .get = 0,
    .set = 0
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass *Uart = &uartTable;
/*----------------------------------------------------------------------------*/
static struct Uart *descriptors[] = {0};
static Mutex lock = MUTEX_UNLOCKED;
/*----------------------------------------------------------------------------*/
void UART_IRQHandler(void)
{
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
enum result uartCalcRate(struct UartConfigRate *config, uint32_t rate)
{
  uint32_t divisor;

  if (!rate)
    return E_ERROR;
  divisor = ((SystemCoreClock / DEFAULT_DIV_VALUE) >> 4) / rate;
  if (!divisor || divisor >= (1 << 16))
    return E_ERROR;

  config->high = (uint8_t)(divisor >> 8);
  config->low = (uint8_t)divisor;
  config->fraction = 0;
  /* TODO Add fractional part calculation */

  return E_OK;
}
/*----------------------------------------------------------------------------*/
enum result uartSetDescriptor(uint8_t channel, void *descriptor)
{
  enum result res = E_ERROR; /* TODO Create special error code */

  assert(channel < sizeof(descriptors));

  mutexLock(&lock);
  if (!descriptors[channel])
  {
    descriptors[channel] = descriptor;
    res = E_OK;
  }
  mutexUnlock(&lock);
  return res;
}
/*----------------------------------------------------------------------------*/
void uartSetRate(struct Uart *device, struct UartConfigRate rate)
{
  /* Enable DLAB access */
  device->reg->LCR |= LCR_DLAB;
  /* Set divisor of the baud rate generator */
  device->reg->DLL = rate.low;
  device->reg->DLM = rate.high;
  /* Set fractional divisor */
  device->reg->FDR = rate.fraction;
  /* Disable DLAB access */
  device->reg->LCR &= ~LCR_DLAB;
}
/*----------------------------------------------------------------------------*/
static enum result uartInit(void *object, const void *configPtr)
{
  /* Set pointer to device configuration data */
  const struct UartConfig *config = configPtr;
  struct Uart *device = object;
  struct UartConfigRate rate;
  gpioFunc func;
  enum result res;

  /* Check device configuration data */
  assert(config);

  /* Calculate and check baud rate value */
  if ((res = uartCalcRate(&rate, config->rate)) != E_OK)
    return res;

  /* Try to set peripheral descriptor */
  device->channel = config->channel;
  if ((res = uartSetDescriptor(device->channel, device)) != E_OK)
    return res;

  /* Reset pointer to interrupt handler function */
  device->handler = 0;

  /* Setup UART RX pin */
  func = gpioFindFunc(uartPins, config->rx);
  assert(func != -1);
  device->rxPin = gpioInit(config->rx, GPIO_INPUT);
  gpioSetFunc(&device->rxPin, func);

  /* Setup UART TX pin */
  func = gpioFindFunc(uartPins, config->tx);
  assert(func != -1);
  device->txPin = gpioInit(config->tx, GPIO_OUTPUT);
  gpioSetFunc(&device->txPin, func);

  switch (config->channel)
  {
    case 0:
      sysClockEnable(CLK_UART);
      LPC_SYSCON->UARTCLKDIV = DEFAULT_DIV; /* Divide AHB clock */
      device->reg = LPC_UART;
      device->irq = UART_IRQn;
      break;
  }

  device->reg->FCR = 0;
  device->reg->IER = 0;
  /* Set 8-bit length */
  device->reg->LCR = LCR_WORD_8BIT;
  /* Set parity */
  if (config->parity != UART_PARITY_NONE)
  {
    device->reg->LCR |= LCR_PARITY;
    if (config->parity == UART_PARITY_EVEN)
      device->reg->LCR |= LCR_PARITY_EVEN;
    else
      device->reg->LCR |= LCR_PARITY_ODD;
  }

  uartSetRate(object, rate);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void uartDeinit(void *object)
{
  struct Uart *device = object;

  /* Disable UART peripheral power */
  LPC_SYSCON->UARTCLKDIV = 0;
  sysClockDisable(CLK_UART);
  gpioDeinit(&device->txPin);
  gpioDeinit(&device->rxPin);
  /* Reset UART descriptor */
  uartSetDescriptor(device->channel, 0);
}
