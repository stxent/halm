/*
 * halm/platform/lpc/lpc11xx/vectors.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LPC_LPC11XX_VECTORS_H_
#define HALM_PLATFORM_LPC_LPC11XX_VECTORS_H_
/*----------------------------------------------------------------------------*/
enum
{
  /* Chip-specific interrupt sources */
  WAKEUP_IRQ  = 0, /* 13 start logic wake-up interrupts */
  CAN_IRQ     = 13,
  SSP1_IRQ    = 14,
  I2C_IRQ     = 15,
  CT16B0_IRQ  = 16,
  CT16B1_IRQ  = 17,
  CT32B0_IRQ  = 18,
  CT32B1_IRQ  = 19,
  SSP0_IRQ    = 20,
  UART_IRQ    = 21,
  ADC_IRQ     = 24,
  WDT_IRQ     = 25,
  BOD_IRQ     = 26,
  PIOINT3_IRQ = 28,
  PIOINT2_IRQ = 29,
  PIOINT1_IRQ = 30,
  PIOINT0_IRQ = 31
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC11XX_VECTORS_H_ */
