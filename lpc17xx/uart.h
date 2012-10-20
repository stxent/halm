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
#include "LPC17xx.h"
/*----------------------------------------------------------------------------*/
#include "interface.h"
#include "queue.h"
#include "gpio.h"
/*----------------------------------------------------------------------------*/
struct UartConfig
{
  gpioKey rx;
  gpioKey tx;
  uint8_t channel;
  uint32_t rate;
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
enum ifResult uartInit(struct Interface *, const void *);
void uartDeinit(struct Interface *);
unsigned int uartRead(struct Interface *, uint8_t *, unsigned int);
unsigned int uartWrite(struct Interface *, const uint8_t *, unsigned int);
enum ifResult uartGetOpt(struct Interface *, enum ifOption, void *);
enum ifResult uartSetOpt(struct Interface *, enum ifOption, const void *);
/*----------------------------------------------------------------------------*/
#endif /* UART_H_ */
