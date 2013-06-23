/*
 * ssp.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SSP_H_
#define SSP_H_
/*----------------------------------------------------------------------------*/
#include <LPC17xx.h>
#include "gpio.h"
#include "interface.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *Ssp;
/*----------------------------------------------------------------------------*/
/* TODO Add master/slave select */
struct SspConfig
{
  uint32_t rate; /* Mandatory: serial data rate */
  gpioKey cs; /* Optional: chip select for slave mode */
  gpioKey sck, miso, mosi; /* Mandatory: peripheral pins */
  uint8_t channel; /* Mandatory: peripheral number */
  uint8_t frame; /* Optional: frame size, 8 bits by default */
};
/*----------------------------------------------------------------------------*/
struct Ssp
{
  struct Interface parent;

  void (*handler)(void *); /* Interrupt handler */

  LPC_SSP_TypeDef *reg; /* Pointer to SSP registers */
  IRQn_Type irq; /* IRQ identifier */

  struct Gpio sckPin, csPin, misoPin, mosiPin; /* Peripheral pins */
  uint8_t channel; /* Peripheral number */
};
/*----------------------------------------------------------------------------*/
enum result sspSetDescriptor(uint8_t, void *);
uint32_t sspGetRate(struct Ssp *);
void sspSetRate(struct Ssp *, uint32_t);
/*----------------------------------------------------------------------------*/
#endif /* SSP_H_ */