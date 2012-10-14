/*
 * uart.h
 *
 *  Created on: Aug 27, 2012
 *      Author: xen
 */

#ifndef UART_H_
#define UART_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
#ifdef __LPC13XX
#include "LPC13xx.h"

#define UART_FUNC_

#elif defined __LPC17XX

#endif
/*----------------------------------------------------------------------------*/
#include "queue.h"
#include "gpio.h"
/*----------------------------------------------------------------------------*/
#define UART_FIFO_SIZE 8
/*----------------------------------------------------------------------------*/
struct UartConfig
{
  int16_t rx;
  int16_t tx;
};
/*----------------------------------------------------------------------------*/
struct Uart
{
  LPC_UART_TypeDef *block;
  IRQn_Type irq;
  struct Queue sendQueue, receiveQueue;
  struct Gpio rxPin, txPin;
  uint8_t channel;
  bool active;
};
/*----------------------------------------------------------------------------*/
int uartInit(struct Uart *, uint8_t, const struct UartConfig *, uint32_t);
void uartDeinit(struct Uart *);
int uartRead(struct Uart *, uint8_t *, int);
int uartWrite(struct Uart *, const uint8_t *, int);
/*----------------------------------------------------------------------------*/
#endif /* UART_H_ */
