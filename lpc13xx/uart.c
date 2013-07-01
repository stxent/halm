/*
 * uart.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include "mutex.h"
#include "system.h"
#include "uart.h"
#include "uart_defs.h"
/*----------------------------------------------------------------------------*/
/* In LPC13xx UART clock divisor is number from 1 to 255, 0 to disable */
#define DEFAULT_DIV         1
#define DEFAULT_DIV_VALUE   1
/*----------------------------------------------------------------------------*/
static const struct GpioDescriptor uartPins[] = {
    {
        .key = GPIO_TO_PIN(1, 6), /* UART_RX */
        .value = 1
    }, {
        .key = GPIO_TO_PIN(1, 7), /* UART_TX */
        .value = 1
    }, {
        /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t, struct Uart *);
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
void UART_ISR(void)
{
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
enum result uartCalcRate(struct UartRateConfig *config, uint32_t rate)
{
  uint32_t divisor;

  if (!rate)
    return E_ERROR;
  divisor = ((sysCoreClock / DEFAULT_DIV_VALUE) >> 4) / rate;
  if (!divisor || divisor >= (1 << 16))
    return E_ERROR;

  config->high = (uint8_t)(divisor >> 8);
  config->low = (uint8_t)divisor;
  config->fraction = 0;
  /* TODO Add fractional part calculation */

  return E_OK;
}
/*----------------------------------------------------------------------------*/
enum result setDescriptor(uint8_t channel, struct Uart *device)
{
  enum result res = E_BUSY;

  assert(channel < sizeof(descriptors));

  mutexLock(&lock);
  if (!descriptors[channel])
  {
    descriptors[channel] = device;
    res = E_OK;
  }
  mutexUnlock(&lock);
  return res;
}
/*----------------------------------------------------------------------------*/
void uartSetRate(struct Uart *device, struct UartRateConfig rate)
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
  const struct GpioDescriptor *pin;
  const struct UartConfig * const config = configPtr;
  struct Uart *device = object;
  struct UartRateConfig rate;
  enum result res;

  /* Check device configuration data */
  assert(config);

  /* Calculate and check baud rate value */
  if ((res = uartCalcRate(&rate, config->rate)) != E_OK)
    return res;

  /* Try to set peripheral descriptor */
  device->channel = config->channel;
  if ((res = setDescriptor(device->channel, device)) != E_OK)
    return res;

  /* Reset pointer to interrupt handler function */
  device->handler = 0;

  /* Setup UART RX pin */
  pin = gpioFind(uartPins, config->rx, device->channel);
  assert(pin);
  device->rxPin = gpioInit(config->rx, GPIO_INPUT);
  gpioSetFunction(&device->rxPin, pin->value);

  /* Setup UART TX pin */
  pin = gpioFind(uartPins, config->tx, device->channel);
  assert(pin);
  device->txPin = gpioInit(config->tx, GPIO_OUTPUT);
  gpioSetFunction(&device->txPin, pin->value);

  switch (config->channel)
  {
    case 0:
      sysClockEnable(CLK_UART);
      LPC_SYSCON->UARTCLKDIV = DEFAULT_DIV; /* Divide AHB clock */
      device->reg = LPC_UART;
      device->irq = UART_IRQ;
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
  setDescriptor(device->channel, 0);
}
