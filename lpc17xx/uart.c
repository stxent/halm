/*
 * uart.c
 *
 *  Created on: Aug 27, 2012
 *      Author: xen
 */

#include "uart.h"
#include "lpc17xx_defines.h"
#include "lpc17xx_sys.h"
//FIXME replace return values with something else
/*----------------------------------------------------------------------------*/
#define FIFO_SIZE                       8
#define DEFAULT_DIV                     PCLK_DIV1
#define DEFAULT_DIV_VALUE               1
/*----------------------------------------------------------------------------*/
/* UART register bits */
/* IrDA, Modem signals, auto-baud and RS-485 not used */
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
static struct Uart *descriptor[] = {0, 0, 0, 0};
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
void UART0_IRQHandler(void)
{
  uartBaseHandler(descriptor[0]);
}
/*----------------------------------------------------------------------------*/
void UART1_IRQHandler(void)
{
  uartBaseHandler(descriptor[1]);
}
/*----------------------------------------------------------------------------*/
void UART2_IRQHandler(void)
{
  uartBaseHandler(descriptor[2]);
}
/*----------------------------------------------------------------------------*/
void UART3_IRQHandler(void)
{
  uartBaseHandler(descriptor[3]);
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
  /* Processor-specific register */
  LPC_SC->PCONP &= ~PCONP_PCUART0;
  /* Release pins */
  gpioReleasePin(&desc->rxPin);
  gpioReleasePin(&desc->txPin);
}
/*----------------------------------------------------------------------------*/
int uartInit(struct Uart *desc, uint8_t channel,
    const struct UartConfig *config, uint32_t rate)
{
  uint32_t divider;
  uint8_t func;

  queueInit(&desc->sendQueue);
  queueInit(&desc->receiveQueue);
  desc->active = false;

  //FIXME rewrite definitions, fix TER size
  switch (channel)
  {
    case 0:
      LPC_SC->PCONP |= PCONP_PCUART0;
      sysSetPeriphDiv(PCLK_UART0, DEFAULT_DIV);
      desc->block = (LPC_UART_TypeDef *)LPC_UART0; //FIXME
      desc->irq = UART0_IRQn;
      break;
    case 1:
      LPC_SC->PCONP |= PCONP_PCUART1;
      sysSetPeriphDiv(PCLK_UART1, DEFAULT_DIV);
      desc->block = (LPC_UART_TypeDef *)LPC_UART1; //FIXME
      desc->irq = UART1_IRQn;
      break;
    case 2:
      LPC_SC->PCONP |= PCONP_PCUART2;
      sysSetPeriphDiv(PCLK_UART2, DEFAULT_DIV);
      desc->block = LPC_UART2;
      desc->irq = UART2_IRQn;
      break;
    case 3:
      LPC_SC->PCONP |= PCONP_PCUART3;
      sysSetPeriphDiv(PCLK_UART3, DEFAULT_DIV);
      desc->block = LPC_UART3;
      desc->irq = UART3_IRQn;
      break;
    default:
      return -1;
  }

  divider = (SystemCoreClock / DEFAULT_DIV_VALUE) / (rate << 4);
  if (divider > 0xFFFF)
    return -1;
  //TODO Add fractional divider calculation

  /* Select UART function and pull-up */
  desc->rxPin = gpioInitPin(config->rx, GPIO_INPUT);
  func = gpioFindFunc(uartPins, config->rx);
  if ((gpioGetKey(&desc->rxPin) == -1) || !func)
    return -1;
  gpioSetFunc(&desc->rxPin, func);
  desc->txPin = gpioInitPin(config->tx, GPIO_OUTPUT);
  func = gpioFindFunc(uartPins, config->tx);
  if ((gpioGetKey(&desc->txPin) == -1) || !func)
    return -1;
  gpioSetFunc(&desc->txPin, func);

  desc->channel = channel;
  descriptor[channel] = desc;

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
