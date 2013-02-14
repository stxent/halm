/*
 * ssp.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SSP_H_
#define SSP_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <LPC13xx.h>
#include "gpio.h"
#include "interface.h"
/*----------------------------------------------------------------------------*/
struct SspClass;
/*----------------------------------------------------------------------------*/
extern const struct SspClass *Ssp;
/*----------------------------------------------------------------------------*/
/* TODO Add master/slave select */
struct SspConfig
{
  /* Mandatory arguments */
  uint8_t channel; /* Peripheral number */
  gpioKey sck, miso, mosi; /* Serial clock, master output */
  uint32_t rate;
  /* Optional arguments */
  uint8_t frame; /* Frame size, 8 bits by default */
  gpioKey cs; /* Chip select for slave operations */
};
/*----------------------------------------------------------------------------*/
struct SspClass
{
  struct InterfaceClass parent;

  /* Interrupt handler, arguments: SSP descriptor assigned to peripheral */
  void (*handler)(void *);
};
/*----------------------------------------------------------------------------*/
struct Ssp
{
  struct Interface parent;

  uint8_t channel; /* Peripheral number */
  struct Gpio sckPin, csPin, misoPin, mosiPin; /* Peripheral pins */

  LPC_SSP_TypeDef *reg; /* Pointer to SSP registers */
  IRQn_Type irq; /* IRQ identifier */
};
/*----------------------------------------------------------------------------*/
enum result sspSetDescriptor(uint8_t, void *);
enum result sspSetRate(struct Ssp *, uint32_t);
/*----------------------------------------------------------------------------*/
#endif /* SSP_H_ */
