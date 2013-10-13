/*
 * interrupts.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_
/*----------------------------------------------------------------------------*/
enum
{
  /* Core-specific interrupt sources */
  NMI_IRQ         = -14,
  MEMMANAGE_IRQ   = -12,
  BUSFAULT_IRQ    = -11,
  USAGEFAULT_IRQ  = -10,
  SVCALL_IRQ      = -5,
  DEBUGMON_IRQ    = -4,
  PENDSV_IRQ      = -2,
  SYSTICK_IRQ     = -1,

  /* Chip-specific IRQ handlers */
  WAKEUP_IRQ    = 0,
  /* TODO */
  CAN_IRQ       = 13,
  SSP1_IRQ      = 14,
  I2C_IRQ       = 15,
  TIMER16B0_IRQ = 16,
  TIMER16B1_IRQ = 17,
  TIMER32B0_IRQ = 18,
  TIMER32B1_IRQ = 19,
  SSP0_IRQ      = 20,
  UART_IRQ      = 21,
//  USB_IRQ       = 47,
//  USB_FIQ_IRQ   = 48,
  ADC_IRQ       = 24,
  WDT_IRQ       = 25,
  BOD_IRQ       = 26,
  PIOINT3_IRQ   = 28,
  PIOINT2_IRQ   = 29,
  PIOINT1_IRQ   = 30,
  PIOINT0_IRQ   = 31
};
/*----------------------------------------------------------------------------*/
#endif /* INTERRUPTS_H_ */
