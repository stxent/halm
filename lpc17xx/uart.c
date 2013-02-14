/*
 * uart.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "lpc17xx_sys.h"
#include "mutex.h"
#include "uart.h"
#include "uart_defs.h"
/*----------------------------------------------------------------------------*/
/* UART settings */
#define DEFAULT_DIV                     CLK_DIV1
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
        .key = GPIO_TO_PIN(0, 0),
        .func = 2
    },
    {
        .key = GPIO_TO_PIN(0, 1),
        .func = 2
    },
    {
        .key = GPIO_TO_PIN(0, 2),
        .func = 1
    },
    {
        .key = GPIO_TO_PIN(0, 3),
        .func = 1
    },
    {
        .key = GPIO_TO_PIN(0, 10),
        .func = 1
    },
    {
        .key = GPIO_TO_PIN(0, 11),
        .func = 1
    },
    {
        .key = GPIO_TO_PIN(0, 15),
        .func = 1
    },
    {
        .key = GPIO_TO_PIN(0, 16),
        .func = 1
    },
    {
        .key = GPIO_TO_PIN(0, 25),
        .func = 3
    },
    {
        .key = GPIO_TO_PIN(0, 26),
        .func = 3
    },
    {
        .key = GPIO_TO_PIN(2, 0),
        .func = 2
    },
    {
        .key = GPIO_TO_PIN(2, 1),
        .func = 2
    },
    {
        .key = GPIO_TO_PIN(2, 8),
        .func = 2
    },
    {
        .key = GPIO_TO_PIN(2, 9),
        .func = 2
    },
    {
        .key = GPIO_TO_PIN(4, 28),
        .func = 3
    },
    {
        .key = GPIO_TO_PIN(4, 29),
        .func = 3
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
static void * volatile descriptors[] = {0, 0, 0, 0};
static Mutex lock = MUTEX_UNLOCKED;
/*----------------------------------------------------------------------------*/
static void uartCleanup(struct Uart *device, enum cleanup step)
{
  const enum sysPowerDevice uartPower[] = {
      PWR_UART0, PWR_UART1, PWR_UART2, PWR_UART3
  };

  switch (step)
  {
    case FREE_ALL:
    case FREE_PERIPHERAL:
      /* Disable UART peripheral power */
      sysPowerDisable(uartPower[device->channel]);
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
/* TODO Replace return type */
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
void UART0_IRQHandler(void)
{
  if (descriptors[0])
    ((struct UartClass *)CLASS(descriptors[0]))->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void UART1_IRQHandler(void)
{
  if (descriptors[1])
    ((struct UartClass *)CLASS(descriptors[1]))->handler(descriptors[1]);
}
/*----------------------------------------------------------------------------*/
void UART2_IRQHandler(void)
{
  if (descriptors[2])
    ((struct UartClass *)CLASS(descriptors[2]))->handler(descriptors[2]);
}
/*----------------------------------------------------------------------------*/
void UART3_IRQHandler(void)
{
  if (descriptors[3])
    ((struct UartClass *)CLASS(descriptors[3]))->handler(descriptors[3]);
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

  /* Check device configuration data and availability */
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

  //FIXME Remove FIFO level from CMSIS
  switch (device->channel)
  {
    case 0:
      sysPowerEnable(PWR_UART0);
      sysClockControl(CLK_UART0, DEFAULT_DIV);
      //FIXME Replace with LPC_UART_TypeDef in CMSIS
      device->reg = (LPC_UART_TypeDef *)LPC_UART0;
      device->irq = UART0_IRQn;
      break;
    case 1:
      sysPowerEnable(PWR_UART1);
      sysClockControl(CLK_UART1, DEFAULT_DIV);
      //FIXME Rewrite TER type
      device->reg = (LPC_UART_TypeDef *)LPC_UART1;
      device->irq = UART1_IRQn;
      break;
    case 2:
      sysPowerEnable(PWR_UART2);
      sysClockControl(CLK_UART2, DEFAULT_DIV);
      device->reg = LPC_UART2;
      device->irq = UART2_IRQn;
      break;
    case 3:
      sysPowerEnable(PWR_UART3);
      sysClockControl(CLK_UART3, DEFAULT_DIV);
      device->reg = LPC_UART3;
      device->irq = UART3_IRQn;
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
