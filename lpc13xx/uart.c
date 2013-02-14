/*
 * uart.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "lpc13xx_sys.h"
#include "mutex.h"
#include "uart.h"
#include "uart_defs.h"
/*----------------------------------------------------------------------------*/
/* UART settings */
/* In LPC13xx UART clock divisor is number from 1 to 255, 0 to disable */
#define DEFAULT_DIV                     1
#define DEFAULT_DIV_VALUE               1
/*----------------------------------------------------------------------------*/
enum cleanup
{
  FREE_NONE = 0,
  FREE_DESCRIPTOR,
  FREE_RX_PIN,
  FREE_TX_PIN,
  FREE_PERIPHERAL,
  FREE_ALL
};
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
static void uartCleanup(struct Uart *, enum cleanup);
/*----------------------------------------------------------------------------*/
static enum result uartInit(void *, const void *);
static void uartDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct UartClass uartTable = {
    .parent = {
        .size = 0, /* Abstract class */
        .init = uartInit,
        .deinit = uartDeinit,

        .read = 0,
        .write = 0,
        .getopt = 0,
        .setopt = 0
    },
    .handler = 0
};
/*----------------------------------------------------------------------------*/
const struct UartClass *Uart = &uartTable;
/*----------------------------------------------------------------------------*/
static void * volatile descriptors[] = {0};
static Mutex lock = MUTEX_UNLOCKED;
/*----------------------------------------------------------------------------*/
static void uartCleanup(struct Uart *device, enum cleanup step)
{
  switch (step)
  {
    case FREE_ALL:
    case FREE_PERIPHERAL:
      /* Disable UART peripheral power */
      LPC_SYSCON->UARTCLKDIV = 0;
      sysClockDisable(CLK_UART);
      break;
    case FREE_TX_PIN:
      gpioDeinit(&device->txPin);
    case FREE_RX_PIN:
      gpioDeinit(&device->rxPin);
    case FREE_DESCRIPTOR:
      /* Reset UART descriptor */
      uartSetDescriptor(device->channel, 0);
    default:
      break;
  }
}
/*----------------------------------------------------------------------------*/
enum result uartSetDescriptor(uint8_t channel, void *descriptor)
{
  enum result res = E_ERROR;

  if (channel < sizeof(descriptors))
  {
    mutexLock(&lock);
    if (!descriptors[channel])
    {
      descriptors[channel] = descriptor;
      res = E_OK;
    }
    mutexUnlock(&lock);
  }
  return res;
}
/*----------------------------------------------------------------------------*/
void UART_IRQHandler(void)
{
  if (descriptors[0])
    ((struct UartClass *)CLASS(descriptors[0]))->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
struct UartConfigRate uartCalcRate(uint32_t rate)
{
  struct UartConfigRate config = {
      .high = 0,
      .low = 0,
      .fraction = 0
  };
  uint32_t divisor;

  divisor = ((SystemCoreClock / DEFAULT_DIV_VALUE) >> 4) / rate;
  if (!(divisor >= (1 << 16) || !divisor))
  {
    config.high = (uint8_t)(divisor >> 8);
    config.low = (uint8_t)divisor;
    /* TODO Add fractional part calculation */
    /* if (config.high > 0 || config.low > 1)
      config.fraction = 0; */
  }
  return config;
}
/*----------------------------------------------------------------------------*/
enum result uartSetRate(struct Uart *device, struct UartConfigRate rate)
{
  if (!rate.high && !rate.low)
    return E_ERROR;
  /* Enable DLAB access */
  device->reg->LCR |= LCR_DLAB;
  /* Set divisor of the baud rate generator */
  device->reg->DLL = rate.low;
  device->reg->DLM = rate.high;
  /* Set fractional divisor */
  device->reg->FDR = rate.fraction;
  /* Disable DLAB access */
  device->reg->LCR &= ~LCR_DLAB;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result uartInit(void *object, const void *configPtr)
{
  /* Set pointer to device configuration data */
  const struct UartConfig *config = configPtr;
  struct Uart *device = object;
  gpioFunc func;
  enum result res;

  /* Check device configuration data */
  if (!config)
    return E_ERROR;

  /* Try to set peripheral descriptor */
  device->channel = config->channel;
  if ((res = uartSetDescriptor(device->channel, device)) != E_OK)
    return res;

  /* Setup UART RX pin */
  func = gpioFindFunc(uartPins, config->rx);
  device->rxPin = gpioInit(config->rx, GPIO_INPUT);
  if (func == -1 || !gpioGetKey(&device->rxPin))
  {
    uartCleanup(device, FREE_DESCRIPTOR);
    return E_ERROR;
  }
  gpioSetFunc(&device->rxPin, func);

  /* Setup UART TX pin */
  func = gpioFindFunc(uartPins, config->tx);
  device->txPin = gpioInit(config->tx, GPIO_OUTPUT);
  if (func == -1 || !gpioGetKey(&device->rxPin))
  {
    uartCleanup(device, FREE_RX_PIN);
    return E_ERROR;
  }
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

  if ((res = uartSetRate(object, uartCalcRate(config->rate))) != E_OK)
  {
    uartCleanup(device, FREE_TX_PIN);
    return res;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void uartDeinit(void *object)
{
  struct Uart *device = object;

  uartCleanup(device, FREE_ALL);
}
