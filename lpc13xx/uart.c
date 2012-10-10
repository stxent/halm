/*
 * uart.c
 *
 *  Created on: Aug 27, 2012
 *      Author: xen
 */

#include "uart.h"
#include "lpc13xx_defines.h"
/*----------------------------------------------------------------------------*/
/* UART pin modes */
static const struct GpioPinMode uartPins[] = {
    {
        .id = GPIO_TO_PIN(1, 6),
        .mode = 0x01
    },
    {
        .id = GPIO_TO_PIN(1, 7),
        .mode = 0x01
    },
    {
        .id = -1,
        .mode = 0x00
    }
};
/*----------------------------------------------------------------------------*/
/* UART register bits                                                         */
/* Modem signals, auto-baud and RS-485 not used                               */
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
static struct Uart *descriptor[] = {0};
/*----------------------------------------------------------------------------*/
static void uartBaseHandler(struct Uart *);
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
      while (queueSize(&desc->sendQueue) && counter++ < UART_FIFO_SIZE)
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
int uartRead(struct Uart *desc, uint8_t *buffer, int length)
{
  int read = 0;
  while (queueSize(&desc->receiveQueue) && (read++ < length))
  {
    NVIC_DisableIRQ(desc->irq);
    *buffer++ = queuePop(&desc->receiveQueue);
    NVIC_EnableIRQ(desc->irq);
  }
  return read;
}
/*----------------------------------------------------------------------------*/
int uartWrite(struct Uart *desc, const uint8_t *buffer, int length)
{
  int written = 0;
  if (!desc->active)
  {
    desc->active = true;
    desc->block->THR = *buffer++;
    written++;
  }
  while ((queueSize(&desc->sendQueue) < QUEUE_SIZE) && (written++ < length))
  {
    NVIC_DisableIRQ(desc->irq);
    queuePush(&desc->sendQueue, *buffer++);
    NVIC_EnableIRQ(desc->irq);
  }
  return written;
}
/*----------------------------------------------------------------------------*/
void uartDeinit(struct Uart *desc)
{
  NVIC_DisableIRQ(desc->irq); /* Disable interrupt */
  LPC_SYSCON->UARTCLKDIV = 0;
  /* Processor-specific register */
  LPC_SYSCON->SYSAHBCLKCTRL &= ~SYSAHBCLKCTRL_UART;
  /* Release pins */
  gpioReleasePin(&desc->rxPin);
  gpioReleasePin(&desc->txPin);
}
/*----------------------------------------------------------------------------*/
int uartInit(struct Uart *desc, uint8_t channel,
    const struct UartConfig *config, uint32_t rate)
{
  uint8_t divider, func;

  queueInit(&desc->sendQueue);
  queueInit(&desc->receiveQueue);
  desc->active = false;

  descriptor[channel] = desc;
  desc->channel = channel;
  desc->block = LPC_UART;
  desc->irq = UART_IRQn;

  /* Select UART function and pull-up */
  /* TODO check output mode assignment */
  desc->rxPin = gpioInitPin(config->rx, GPIO_INPUT);
  func = gpioFindMode(uartPins, config->rx);
  if ((gpioGetId(&desc->rxPin) == -1) || !func)
    return -1;
  gpioSetMode(&desc->rxPin, func);
  desc->txPin = gpioInitPin(config->tx, GPIO_OUTPUT);
  func = gpioFindMode(uartPins, config->tx);
  if ((gpioGetId(&desc->txPin) == -1) || !func)
    return -1;
  gpioSetMode(&desc->txPin, func);

  /* Processor-specific registers */
  LPC_SYSCON->SYSAHBCLKCTRL |= SYSAHBCLKCTRL_UART;
  LPC_SYSCON->UARTCLKDIV = 1; /* Divide AHB clock by 1 */

  divider = (SystemCoreClock / LPC_SYSCON->SYSAHBCLKDIV) / (rate << 4);

  /* Set 8-bit length and enable DLAB access */
  desc->block->LCR = LCR_WORD_8BIT | LCR_DLAB;
  /* Set divisor of the baud rate generator */
  desc->block->DLL = (uint8_t)divider;
  desc->block->DLM = (uint8_t)(divider >> 8);
  desc->block->LCR &= ~LCR_DLAB; /* Clear DLAB */
  /* Enable and clear FIFO */
  desc->block->FCR = FCR_ENABLE;
  /* TODO add RX line interrupt */
  /* Enable RBR and THRE interrupts */
  desc->block->IER = IER_RBR | IER_THRE;
  desc->block->TER = TER_TXEN;

  NVIC_EnableIRQ(desc->irq); /* Enable interrupt */
  return 0;
}
