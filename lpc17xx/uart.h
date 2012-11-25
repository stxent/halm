/*
 * uart.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef UART_H_
#define UART_H_
/*----------------------------------------------------------------------------*/
/* TODO Move to makefile */
#define UART_DMA
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
#include "LPC17xx.h"
/*----------------------------------------------------------------------------*/
#include "queue.h"
#include "interface.h"
#include "mutex.h"
#include "gpio.h"
/*----------------------------------------------------------------------------*/
#ifdef UART_DMA
#include "dma.h"
#endif
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *Uart;
/*----------------------------------------------------------------------------*/
struct UartConfig
{
  /* Basic parameters */
  gpioKey rx, tx;
  uint16_t rxLength, txLength;
  uint8_t channel;
  uint32_t rate;
  uint8_t priority; /* UART interrupt priority */

  /* DMA related parameters */
#ifdef UART_DMA
  struct DmaParameters dma;
#endif
};
/*----------------------------------------------------------------------------*/
struct Uart
{
  struct Interface parent;
  struct Mutex lock;
  struct Gpio rxPin, txPin;
  struct Queue rxQueue, txQueue;
  uint8_t channel;
  bool active;

  /* DMA related members */
#ifdef UART_DMA
  struct Dma txDma; /* TODO replace with pointer, add RX channel */
  bool dma;
#endif

  /* Device-specific data */
  LPC_UART_TypeDef *block;
  IRQn_Type irq;
};
/*----------------------------------------------------------------------------*/
#endif /* UART_H_ */
