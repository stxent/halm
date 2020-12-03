/*
 * halm/platform/lpc/lpc17xx/vectors.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC17XX_VECTORS_H_
#define HALM_PLATFORM_LPC_LPC17XX_VECTORS_H_
/*----------------------------------------------------------------------------*/
enum
{
  /* Chip-specific interrupt sources */
  WDT_IRQ       = 0,
  TIMER0_IRQ    = 1,
  TIMER1_IRQ    = 2,
  TIMER2_IRQ    = 3,
  TIMER3_IRQ    = 4,
  UART0_IRQ     = 5,
  UART1_IRQ     = 6,
  UART2_IRQ     = 7,
  UART3_IRQ     = 8,
  PWM1_IRQ      = 9,
  I2C0_IRQ      = 10,
  I2C1_IRQ      = 11,
  I2C2_IRQ      = 12,
  SPI_IRQ       = 13,
  SSP0_IRQ      = 14,
  SSP1_IRQ      = 15,
  PLL0_IRQ      = 16,
  RTC_IRQ       = 17,
  EINT0_IRQ     = 18,
  EINT1_IRQ     = 19,
  EINT2_IRQ     = 20,
  EINT3_IRQ     = 21,
  ADC_IRQ       = 22,
  BOD_IRQ       = 23,
  USB_IRQ       = 24,
  CAN_IRQ       = 25,
  GPDMA_IRQ     = 26,
  I2S_IRQ       = 27,
  ETHERNET_IRQ  = 28,
  RIT_IRQ       = 29,
  MCPWM_IRQ     = 30,
  QEI_IRQ       = 31,
  PLL1_IRQ      = 32,
  USB_ACT_IRQ   = 33,
  CAN_ACT_IRQ   = 34
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC17XX_VECTORS_H_ */
