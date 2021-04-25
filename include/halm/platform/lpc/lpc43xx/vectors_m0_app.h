/*
 * halm/platform/lpc/lpc43xx/vectors_m0_app.h
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC43XX_VECTORS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC43XX_VECTORS_M0_APP_H_
#define HALM_PLATFORM_LPC_LPC43XX_VECTORS_M0_APP_H_
/*----------------------------------------------------------------------------*/
enum
{
  /* Chip-specific interrupt sources */

  RTC_IRQ           = 0,
  M4CORE_IRQ        = 1,
  GPDMA_IRQ         = 2,
  /* Reserved */
  EEPROM_ATIMER_IRQ = 4,
  ETHERNET_IRQ      = 5,
  SDIO_IRQ          = 6,
  LCD_IRQ           = 7,
  USB0_IRQ          = 8,
  USB1_IRQ          = 9,
  SCT_IRQ           = 10,
  RIT_WWDT_IRQ      = 11,
  TIMER0_IRQ        = 12,
  PIN_GINT1_IRQ     = 13,
  PIN_INT4_IRQ      = 14,
  TIMER3_IRQ        = 15,
  MCPWM_IRQ         = 16,
  ADC0_IRQ          = 17,
  I2C0_I2C1_IRQ     = 18,
  SGPIO_IRQ         = 19,
  SPI_DAC_IRQ       = 20,
  ADC1_IRQ          = 21,
  SSP0_SSP1_IRQ     = 22,
  EVENTROUTER_IRQ   = 23,
  USART0_IRQ        = 24,
  UART1_IRQ         = 25,
  USART2_CAN1_IRQ   = 26,
  USART3_IRQ        = 27,
  I2S0_I2S1_QEI_IRQ = 28,
  CAN0_IRQ          = 29,
  SPIFI_ADCHS_IRQ   = 30,
  M0SUB_IRQ         = 31,

  /* Virtual interrupt sources */

  EEPROM_IRQ        = EEPROM_ATIMER_IRQ,
  ATIMER_IRQ        = EEPROM_ATIMER_IRQ,
  RIT_IRQ           = RIT_WWDT_IRQ,
  WWDT_IRQ          = RIT_WWDT_IRQ,
  I2C0_IRQ          = I2C0_I2C1_IRQ,
  I2C1_IRQ          = I2C0_I2C1_IRQ,
  SPI_IRQ           = SPI_DAC_IRQ,
  DAC_IRQ           = SPI_DAC_IRQ,
  SSP0_IRQ          = SSP0_SSP1_IRQ,
  SSP1_IRQ          = SSP0_SSP1_IRQ,
  USART2_IRQ        = USART2_CAN1_IRQ,
  CAN1_IRQ          = USART2_CAN1_IRQ,
  I2S0_IRQ          = I2S0_I2S1_QEI_IRQ,
  I2S1_IRQ          = I2S0_I2S1_QEI_IRQ,
  QEI_IRQ           = I2S0_I2S1_QEI_IRQ,
  SPIFI_IRQ         = SPIFI_ADCHS_IRQ,
  ADCHS_IRQ         = SPIFI_ADCHS_IRQ,

  /* Stubs */

  TIMER1_IRQ        = -1,
  TIMER2_IRQ        = -1,
  PIN_INT0_IRQ      = -1,
  PIN_INT1_IRQ      = -1,
  PIN_INT2_IRQ      = -1,
  PIN_INT3_IRQ      = -1,
  PIN_INT5_IRQ      = -1,
  PIN_INT6_IRQ      = -1,
  PIN_INT7_IRQ      = -1,
  PIN_GINT0_IRQ     = -1
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_VECTORS_M0_APP_H_ */
