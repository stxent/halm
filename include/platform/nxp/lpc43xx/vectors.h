/*
 * platform/nxp/lpc43xx/vectors.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC43XX_VECTORS_H_
#define HALM_PLATFORM_NXP_LPC43XX_VECTORS_H_
/*----------------------------------------------------------------------------*/
enum
{
  /* Chip-specific interrupt sources */
  DAC_IRQ         = 0,
  M0APP_IRQ       = 1,
  GPDMA_IRQ       = 2,
  EEPROM_IRQ      = 4,
  ETHERNET_IRQ    = 5,
  SDIO_IRQ        = 6,
  LCD_IRQ         = 7,
  USB0_IRQ        = 8,
  USB1_IRQ        = 9,
  SCT_IRQ         = 10,
  RIT_IRQ         = 11,
  TIMER0_IRQ      = 12,
  TIMER1_IRQ      = 13,
  TIMER2_IRQ      = 14,
  TIMER3_IRQ      = 15,
  MCPWM_IRQ       = 16,
  ADC0_IRQ        = 17,
  I2C0_IRQ        = 18,
  I2C1_IRQ        = 19,
  SPI_IRQ         = 20,
  ADC1_IRQ        = 21,
  SSP0_IRQ        = 22,
  SSP1_IRQ        = 23,
  USART0_IRQ      = 24,
  UART1_IRQ       = 25,
  USART2_IRQ      = 26,
  USART3_IRQ      = 27,
  I2S0_IRQ        = 28,
  I2S1_IRQ        = 29,
  SPIFI_IRQ       = 30,
  SGPIO_IRQ       = 31,
  PIN_INT0_IRQ    = 32,
  PIN_INT1_IRQ    = 33,
  PIN_INT2_IRQ    = 34,
  PIN_INT3_IRQ    = 35,
  PIN_INT4_IRQ    = 36,
  PIN_INT5_IRQ    = 37,
  PIN_INT6_IRQ    = 38,
  PIN_INT7_IRQ    = 39,
  PIN_GINT0_IRQ   = 40,
  PIN_GINT1_IRQ   = 41,
  EVENTROUTER_IRQ = 42,
  CAN1_IRQ        = 43,
  ADCHS_IRQ       = 45,
  ATIMER_IRQ      = 46,
  RTC_IRQ         = 47,
  WWDT_IRQ        = 49,
  M0SUB_IRQ       = 50,
  CAN0_IRQ        = 51,
  QEI_IRQ         = 52
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC43XX_VECTORS_H_ */
