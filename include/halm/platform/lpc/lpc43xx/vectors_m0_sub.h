/*
 * halm/platform/lpc/lpc43xx/vectors_m0_sub.h
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC43XX_VECTORS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC43XX_VECTORS_M0_SUB_H_
#define HALM_PLATFORM_LPC_LPC43XX_VECTORS_M0_SUB_H_
/*----------------------------------------------------------------------------*/
enum
{
  /* Chip-specific interrupt sources */

  DAC_IRQ           = 0,
  M4CORE_IRQ        = 1,
  GPDMA_IRQ         = 2,
  /* Reserved */
  SGPIO_INPUT_IRQ   = 4,
  SGPIO_MATCH_IRQ   = 5,
  SGPIO_SHIFT_IRQ   = 6,
  SGPIO_POS_IRQ     = 7,
  USB0_IRQ          = 8,
  USB1_IRQ          = 9,
  SCT_IRQ           = 10,
  RIT_IRQ           = 11,
  PIN_GINT1_IRQ     = 12,
  TIMER1_IRQ        = 13,
  TIMER2_IRQ        = 14,
  PIN_INT5_IRQ      = 15,
  MCPWM_IRQ         = 16,
  ADC0_IRQ          = 17,
  I2C0_IRQ          = 18,
  I2C1_IRQ          = 19,
  SPI_IRQ           = 20,
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
  M0APP_IRQ         = 31,

  /* Virtual interrupt sources */

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

  EEPROM_IRQ        = IRQ_RESERVED,
  ETHERNET_IRQ      = IRQ_RESERVED,
  SDIO_IRQ          = IRQ_RESERVED,
  LCD_IRQ           = IRQ_RESERVED,
  TIMER0_IRQ        = IRQ_RESERVED,
  TIMER3_IRQ        = IRQ_RESERVED,
  SGPIO_IRQ         = IRQ_RESERVED,
  PIN_INT0_IRQ      = IRQ_RESERVED,
  PIN_INT1_IRQ      = IRQ_RESERVED,
  PIN_INT2_IRQ      = IRQ_RESERVED,
  PIN_INT3_IRQ      = IRQ_RESERVED,
  PIN_INT4_IRQ      = IRQ_RESERVED,
  PIN_INT6_IRQ      = IRQ_RESERVED,
  PIN_INT7_IRQ      = IRQ_RESERVED,
  PIN_GINT0_IRQ     = IRQ_RESERVED,
  ATIMER_IRQ        = IRQ_RESERVED,
  RTC_IRQ           = IRQ_RESERVED,
  WWDT_IRQ          = IRQ_RESERVED
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_VECTORS_M0_SUB_H_ */
