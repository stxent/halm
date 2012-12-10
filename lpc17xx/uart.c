/*
 * uart.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "lpc17xx_defs.h"
#include "lpc17xx_sys.h"
#include "uart.h"
#include "uart_defs.h"
#include "mutex.h"
/*----------------------------------------------------------------------------*/
/* UART settings */
#define FRACTION_VALUE                  15 /* Divisor from 1 to 15 */
#define DEFAULT_DIV                     PCLK_DIV1
#define DEFAULT_DIV_VALUE               1
/*----------------------------------------------------------------------------*/
/* UART pin function values */
static const struct GpioPinFunc uartPins[] = {
    {
        .key = GPIO_TO_PIN(0, 0),
        .func = 0x02
    },
    {
        .key = GPIO_TO_PIN(0, 1),
        .func = 0x02
    },
    {
        .key = GPIO_TO_PIN(0, 2),
        .func = 0x01
    },
    {
        .key = GPIO_TO_PIN(0, 3),
        .func = 0x01
    },
    {
        .key = GPIO_TO_PIN(0, 10),
        .func = 0x01
    },
    {
        .key = GPIO_TO_PIN(0, 11),
        .func = 0x01
    },
    {
        .key = GPIO_TO_PIN(0, 15),
        .func = 0x01
    },
    {
        .key = GPIO_TO_PIN(0, 16),
        .func = 0x01
    },
    {
        .key = GPIO_TO_PIN(0, 25),
        .func = 0x03
    },
    {
        .key = GPIO_TO_PIN(0, 26),
        .func = 0x03
    },
    {
        .key = GPIO_TO_PIN(2, 0),
        .func = 0x02
    },
    {
        .key = GPIO_TO_PIN(2, 1),
        .func = 0x02
    },
    {
        .key = GPIO_TO_PIN(2, 8),
        .func = 0x02
    },
    {
        .key = GPIO_TO_PIN(2, 9),
        .func = 0x02
    },
    {
        .key = GPIO_TO_PIN(4, 28),
        .func = 0x03
    },
    {
        .key = GPIO_TO_PIN(4, 29),
        .func = 0x03
    },
    {
        .key = -1,
        .func = 0x00
    }
};
/*----------------------------------------------------------------------------*/
static enum result uartInit(struct Interface *, const void *);
static void uartDeinit(struct Interface *);
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
  uint32_t divisor;
  struct UartConfigRate result = {
      .high = 0,
      .low = 0,
      .fraction = 0
  };

  divisor = ((SystemCoreClock / DEFAULT_DIV_VALUE) >> 4) / rate;
  if (!(divisor >= (1 << 16) || !divisor))
  {
    result.high = (uint8_t)(divisor >> 8);
    result.low = (uint8_t)divisor;
    /* TODO Add fractional part calculation */
    /* if (result.high > 0 || result.low > 1)
      result.fraction = 0; */
  }
  return result;
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
static void uartDeinit(struct Interface *iface)
{
  struct Uart *device = (struct Uart *)iface;

  /* Disable UART peripheral power */
  LPC_SC->PCONP &= ~PCONP_PCUART0;
  /* Release pins */
  gpioDeinit(&device->txPin);
  gpioDeinit(&device->rxPin);
  /* Reset UART descriptor */
  uartSetDescriptor(device->channel, 0);
}
/*----------------------------------------------------------------------------*/
static enum result uartInit(struct Interface *iface, const void *configPtr)
{
  /* Set pointer to device configuration data */
  const struct UartConfig *config = (const struct UartConfig *)configPtr;
  struct Uart *device = (struct Uart *)iface;
  uint8_t rxFunc, txFunc;

  /* Check device configuration data and availability */
  if (!config || uartSetDescriptor(config->channel, device) != E_OK)
    return E_ERROR;

  /* Check pin mapping */
  rxFunc = gpioFindFunc(uartPins, config->rx);
  txFunc = gpioFindFunc(uartPins, config->tx);
  if (!rxFunc || !txFunc)
    return E_ERROR;

  /* Setup UART pins */
  device->rxPin = gpioInit(config->rx, GPIO_INPUT);
  if (gpioGetKey(&device->rxPin) == -1)
    return E_ERROR;

  device->txPin = gpioInit(config->tx, GPIO_OUTPUT);
  if (gpioGetKey(&device->txPin) == -1)
  {
    gpioDeinit(&device->rxPin);
    return E_ERROR;
  }

  device->channel = config->channel;

  //FIXME Remove FIFO level from CMSIS
  switch (config->channel)
  {
    case 0:
      LPC_SC->PCONP |= PCONP_PCUART0;
      sysSetPeriphDiv(PCLK_UART0, DEFAULT_DIV);
      //FIXME Replace with LPC_UART_TypeDef in CMSIS
      device->reg = (LPC_UART_TypeDef *)LPC_UART0;
      device->irq = UART0_IRQn;
      break;
    case 1:
      LPC_SC->PCONP |= PCONP_PCUART1;
      sysSetPeriphDiv(PCLK_UART1, DEFAULT_DIV);
      //FIXME Rewrite TER type
      device->reg = (LPC_UART_TypeDef *)LPC_UART1;
      device->irq = UART1_IRQn;
      break;
    case 2:
      LPC_SC->PCONP |= PCONP_PCUART2;
      sysSetPeriphDiv(PCLK_UART2, DEFAULT_DIV);
      device->reg = LPC_UART2;
      device->irq = UART2_IRQn;
      break;
    case 3:
      LPC_SC->PCONP |= PCONP_PCUART3;
      sysSetPeriphDiv(PCLK_UART3, DEFAULT_DIV);
      device->reg = LPC_UART3;
      device->irq = UART3_IRQn;
      break;
    default:
      break;
  }

  /* Select the UART function of pins */
  gpioSetFunc(&device->rxPin, rxFunc);
  gpioSetFunc(&device->txPin, txFunc);

  /* Set interrupt priority, lowest by default */
  NVIC_SetPriority(device->irq, GET_PRIORITY(config->priority));
  return E_OK;
}
