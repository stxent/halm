/*
 * platform/nxp/lpc11exx/vectors.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VECTORS_H_
#define VECTORS_H_
/*----------------------------------------------------------------------------*/
enum
{
  /* Chip-specific interrupt sources */
  PIN_INT0      = 0,
  PIN_INT1      = 1,
  PIN_INT2      = 2,
  PIN_INT3      = 3,
  PIN_INT4      = 4,
  PIN_INT5      = 5,
  PIN_INT6      = 6,
  PIN_INT7      = 7,
  PIN_GINT0     = 8,
  PIN_GINT1     = 9,
  SSP1_IRQ      = 14,
  I2C_IRQ       = 15,
  TIMER16B0_IRQ = 16,
  TIMER16B1_IRQ = 17,
  TIMER32B0_IRQ = 18,
  TIMER32B1_IRQ = 19,
  SSP0_IRQ      = 20,
  UART_IRQ      = 21,
  ADC_IRQ       = 24,
  WDT_IRQ       = 25,
  BOD_IRQ       = 26,
  FLASH_IRQ     = 27,
  IOH_IRQ       = 31
};
/*----------------------------------------------------------------------------*/
#endif /* VECTORS_H_ */
