/*
 * uart.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
/*----------------------------------------------------------------------------*/
#include "uart.h"
#include "lpc13xx_defines.h"
/*----------------------------------------------------------------------------*/
#define FIFO_SIZE                       8
/* In LPC13xx UART clock divider is value from 1 to 255, 0 to disable */
#define DEFAULT_DIV                     1
#define DEFAULT_DIV_VALUE               1
/*----------------------------------------------------------------------------*/
/* UART register bits */
/* Modem signals, auto-baud and RS-485 not used */
/*------------------Line Control Register-------------------------------------*/
#define LCR_WORD_5BIT                   (0 << 0)
#define LCR_WORD_6BIT                   (1 << 0)
#define LCR_WORD_7BIT                   (2 << 0)
#define LCR_WORD_8BIT                   (3 << 0)
#define LCR_STOP_1BIT                   (0 << 2)
#define LCR_STOP_2BIT                   (1 << 2)
#define LCR_PARITY                      BIT(3)
#define LCR_PARITY_ODD                  (0 << 4)
#define LCR_PARITY_EVEN                 (1 << 4)
#define LCR_BREAK                       BIT(6)
#define LCR_DLAB                        BIT(7)
/*------------------Interrupt Enable Register---------------------------------*/
#define IER_RBR                         BIT(0)
#define IER_THRE                        BIT(1)
#define IER_RX_LINE                     BIT(2)
//#define IER_ABEO                        BIT(8) /* End of auto-baud */
//#define IER_ABTO                        BIT(9) /* Auto-baud timeout */
/*------------------Interrupt Identification Register-------------------------*/
#define IIR_INT_STATUS                  BIT(0) /* Status, active low */
#define IIR_INT_MASK                    (0x07 << 1)
#define IIR_INT_RLS                     (3 << 1) /* Receive line status */
#define IIR_INT_RDA                     (2 << 1) /* Receive data available */
#define IIR_INT_CTI                     (6 << 1) /* Character timeout */
#define IIR_INT_THRE                    (1 << 1) /* THRE interrupt */
//#define IIR_INT_MODEM                   (0 << 1) /* Modem interrupt */
/*------------------FIFO Control Register-------------------------------------*/
#define FCR_ENABLE                      BIT(0)
#define FCR_RX_RESET                    BIT(1)
#define FCR_TX_RESET                    BIT(2)
/*
 * Level 0:  1 character
 * Level 1:  4 characters
 * Level 2:  8 characters
 * Level 3: 14 characters
 */
#define FCR_RX_TRIGGER(level)           ((level) << 6)
/*------------------Line Status Register--------------------------------------*/
#define LSR_RDR                         BIT(0) /* Receiver data ready */
#define LSR_OE                          BIT(1) /* Overrun error */
#define LSR_PE                          BIT(2) /* Parity error */
#define LSR_FE                          BIT(3) /* Framing error */
#define LSR_BI                          BIT(4) /* Break interrupt */
/* Transmitter holding register empty */
#define LSR_THRE                        BIT(5)
#define LSR_TEMT                        BIT(6) /* Transmitter empty */
#define LSR_RXFE                        BIT(7) /* Error in RX FIFO */
/*------------------Transmit Enable Register----------------------------------*/
#define TER_TXEN                        BIT(7) /* Transmit enable */
/*----------------------------------------------------------------------------*/
/* UART pin function values */
static const struct GpioPinFunc uartPins[] = {
    {
        .key = GPIO_TO_PIN(1, 6),
        .func = 0x01
    },
    {
        .key = GPIO_TO_PIN(1, 7),
        .func = 0x01
    },
    {
        .key = -1,
        .func = 0x00
    }
};
/*----------------------------------------------------------------------------*/
static enum result uartInit(struct Interface *, const void *);
static void uartDeinit(struct Interface *);
static unsigned int uartRead(struct Interface *, uint8_t *, unsigned int);
static unsigned int uartWrite(struct Interface *, const uint8_t *,
    unsigned int);
static enum result uartGetOpt(struct Interface *, enum ifOption, void *);
static enum result uartSetOpt(struct Interface *, enum ifOption, const void *);
/*----------------------------------------------------------------------------*/
static void uartBaseHandler(struct Uart *);
static enum result uartSetRate(struct Uart *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass uartTable = {
    .size = sizeof(struct Uart),
    .init = uartInit,
    .deinit = uartDeinit,

    .start = 0,
    .stop = 0,
    .read = uartRead,
    .write = uartWrite,
    .getopt = uartGetOpt,
    .setopt = uartSetOpt
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass *Uart = &uartTable;
/*----------------------------------------------------------------------------*/
static struct Uart *descriptor[] = {0};
/*----------------------------------------------------------------------------*/
static void uartBaseHandler(struct Uart *desc)
{
  uint8_t content, counter;
  if (!desc)
    return;

  switch (desc->block->IIR & IIR_INT_MASK)
  {
    case IIR_INT_THRE:
      /* Fill FIFO with 8 bytes or less */
      counter = 0;
      while (queueSize(&desc->sendQueue) && counter++ < FIFO_SIZE)
        desc->block->THR = queuePop(&desc->sendQueue);
      if (!queueSize(&desc->sendQueue))
        desc->active = false;
      break;
    case IIR_INT_RDA:
      content = desc->block->RBR;
      queuePush(&desc->receiveQueue, content);
      break;
    default:
      break;
  }
}
/*----------------------------------------------------------------------------*/
void UART_IRQHandler(void)
{
  uartBaseHandler(descriptor[0]);
}
/*----------------------------------------------------------------------------*/
static enum result uartSetRate(struct Uart *device, uint32_t rate)
{
  uint32_t divider;

  divider = (SystemCoreClock / DEFAULT_DIV_VALUE) / (rate << 4);
  if (divider >= (1 << 16) || !divider)
    return E_ERROR;
  //TODO Add fractional divider calculation
  /* Set 8-bit length and enable DLAB access */
  device->block->LCR = LCR_DLAB;
  /* Set divisor of the baud rate generator */
  device->block->DLL = (uint8_t)divider;
  device->block->DLM = (uint8_t)(divider >> 8);
  device->block->LCR &= ~LCR_DLAB; /* Clear DLAB */
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result uartGetOpt(struct Interface *iface, enum ifOption option,
    void *data)
{
  struct Uart *device = (struct Uart *)iface;

  switch (option)
  {
    case IF_SPEED:
      /* TODO */
      return E_OK;
    case IF_QUEUE_RX:
      *(uint8_t *)data = queueSize(&device->receiveQueue);
      return E_OK;
    case IF_QUEUE_TX:
      *(uint8_t *)data = queueSize(&device->sendQueue);
      return E_OK;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result uartSetOpt(struct Interface *iface, enum ifOption option,
    const void *data)
{
  struct Uart *device = (struct Uart *)iface;
  uint32_t rate;

  switch (option)
  {
    case IF_SPEED:
      rate = *(uint32_t *)data;
      return uartSetRate(device, rate);
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static unsigned int uartRead(struct Interface *iface, uint8_t *buffer,
    unsigned int length)
{
  struct Uart *device = (struct Uart *)iface;
  int read = 0;

  mutexLock(&device->lock);
  while (queueSize(&device->receiveQueue) && (read++ < length))
  {
    NVIC_DisableIRQ(device->irq);
    *buffer++ = queuePop(&device->receiveQueue);
    NVIC_EnableIRQ(device->irq);
  }
  mutexUnlock(&device->lock);
  return read;
}
/*----------------------------------------------------------------------------*/
static unsigned int uartWrite(struct Interface *iface, const uint8_t *buffer,
    unsigned int length)
{
  struct Uart *device = (struct Uart *)iface;
  int written = 0;

  mutexLock(&device->lock);
  if (!device->active)
  {
    device->active = true;
    device->block->THR = *buffer++;
    written++;
  }
  while ((queueSize(&device->sendQueue) < QUEUE_SIZE) &&
      (written++ < length))
  {
    NVIC_DisableIRQ(device->irq);
    queuePush(&device->sendQueue, *buffer++);
    NVIC_EnableIRQ(device->irq);
  }
  mutexUnlock(&device->lock);
  return written;
}
/*----------------------------------------------------------------------------*/
static void uartDeinit(struct Interface *iface)
{
  struct Uart *device = (struct Uart *)iface;

  NVIC_DisableIRQ(device->irq); /* Disable interrupt */
  LPC_SYSCON->UARTCLKDIV = 0;
  /* Processor-specific register */
  LPC_SYSCON->SYSAHBCLKCTRL &= ~SYSAHBCLKCTRL_UART;
  /* Release pins */
  gpioReleasePin(&device->rxPin);
  gpioReleasePin(&device->txPin);
}
/*----------------------------------------------------------------------------*/
static enum result uartInit(struct Interface *iface, const void *cdata)
{
  /* Set pointer to device configuration data */
  struct UartConfig *config = (struct UartConfig *)cdata;
  struct Uart *device = (struct Uart *)iface;
  uint8_t func;

  /* Check device configuration data */
  if (!config)
    return E_ERROR;

  /* Device already initialized */
  if (descriptor[config->channel])
    return E_ERROR;

  switch (config->channel)
  {
    case 0:
      LPC_SYSCON->SYSAHBCLKCTRL |= SYSAHBCLKCTRL_UART;
      LPC_SYSCON->UARTCLKDIV = DEFAULT_DIV; /* Divide AHB clock */
      device->block = LPC_UART;
      device->irq = UART_IRQn;
      break;
    default:
      return E_ERROR;
  }

  if (uartSetRate(device, config->rate) != E_OK)
    return E_ERROR;

  /* Setup UART pins */
  device->rxPin = gpioInitPin(config->rx, GPIO_INPUT);
  func = gpioFindFunc(uartPins, config->rx);
  if ((gpioGetKey(&device->rxPin) == -1) || !func)
    return E_ERROR;
  gpioSetFunc(&device->rxPin, func);
  device->txPin = gpioInitPin(config->tx, GPIO_OUTPUT);
  func = gpioFindFunc(uartPins, config->tx);
  if ((gpioGetKey(&device->txPin) == -1) || !func)
    return E_ERROR;
  gpioSetFunc(&device->txPin, func);

  /* Initialize RX and TX queues */
  queueInit(&device->sendQueue);
  queueInit(&device->receiveQueue);
  /* Initialize mutex */
  mutexInit(&device->lock);

  device->active = false;
  device->channel = config->channel;

  descriptor[config->channel] = device;

  /* Set 8-bit length */
  device->block->LCR = LCR_WORD_8BIT;
  /* Enable and clear FIFO */
  device->block->FCR = FCR_ENABLE;
  /* TODO add RX line interrupt */
  /* Enable RBR and THRE interrupts */
  device->block->IER = IER_RBR | IER_THRE;
  device->block->TER = TER_TXEN;

  NVIC_EnableIRQ(device->irq); /* Enable interrupt */
  return E_OK;
}
