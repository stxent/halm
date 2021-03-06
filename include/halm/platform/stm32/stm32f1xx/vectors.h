/*
 * halm/platform/stm32/stm32f1xx/vectors.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_VECTORS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F1XX_VECTORS_H_
#define HALM_PLATFORM_STM32_STM32F1XX_VECTORS_H_
/*----------------------------------------------------------------------------*/
enum
{
  /* Chip-specific interrupt sources */
  WWDG_IRQ               = 0,
  PVD_IRQ                = 1,
  TAMPER_IRQ             = 2,
  RTC_IRQ                = 3,
  FLASH_IRQ              = 4,
  RCC_IRQ                = 5,
  EXTI0_IRQ              = 6,
  EXTI1_IRQ              = 7,
  EXTI2_IRQ              = 8,
  EXTI3_IRQ              = 9,
  EXTI4_IRQ              = 10,
  DMA1_CHANNEL1_IRQ      = 11,
  DMA1_CHANNEL2_IRQ      = 12,
  DMA1_CHANNEL3_IRQ      = 13,
  DMA1_CHANNEL4_IRQ      = 14,
  DMA1_CHANNEL5_IRQ      = 15,
  DMA1_CHANNEL6_IRQ      = 16,
  DMA1_CHANNEL7_IRQ      = 17,
  ADC1_2_IRQ             = 18,
  USB_HP_CAN1_TX_IRQ     = 19,
  USB_LP_CAN1_RX0_IRQ    = 20,
  CAN1_RX1_IRQ           = 21,
  CAN1_SCE_IRQ           = 22,
  EXTI9_5_IRQ            = 23,
  TIM1_BRK_TIM15_IRQ     = 24,
  TIM1_UP_TIM16_IRQ      = 25,
  TIM1_TRG_COM_TIM17_IRQ = 26,
  TIM1_CC_IRQ            = 27,
  TIM2_IRQ               = 28,
  TIM3_IRQ               = 29,
  TIM4_IRQ               = 30,
  I2C1_EV_IRQ            = 31,
  I2C1_ER_IRQ            = 32,
  I2C2_EV_IRQ            = 33,
  I2C2_ER_IRQ            = 34,
  SPI1_IRQ               = 35,
  SPI2_IRQ               = 36,
  USART1_IRQ             = 37,
  USART2_IRQ             = 38,
  USART3_IRQ             = 39,
  EXTI15_10_IRQ          = 40,
  RTC_ALARM_IRQ          = 41,
  USB_WAKEUP_CEC_IRQ     = 42,
  TIM8_BRK_TIM12_IRQ     = 43,
  TIM8_UP_TIM13_IRQ      = 44,
  TIM8_TRG_COM_TIM14_IRQ = 45,
  TIM8_CC_IRQ            = 46,
  ADC3_IRQ               = 47,
  FSMC_IRQ               = 48,
  SDIO_IRQ               = 49,
  TIM5_IRQ               = 50,
  SPI3_IRQ               = 51,
  UART4_IRQ              = 52,
  UART5_IRQ              = 53,
  DAC_TIM6_IRQ           = 54,
  TIM7_IRQ               = 55,
  DMA2_CHANNEL1_IRQ      = 56,
  DMA2_CHANNEL2_IRQ      = 57,
  DMA2_CHANNEL3_IRQ      = 58,
  DMA2_CHANNEL4_5_IRQ    = 59,
  DMA2_CHANNEL5_IRQ      = 60,
  ETH_IRQ                = 61,
  ETH_WKUP_IRQ           = 62,
  CAN2_TX_IRQ            = 63,
  CAN2_RX0_IRQ           = 64,
  CAN2_RX1_IRQ           = 65,
  CAN2_SCE_IRQ           = 66,
  OTG_FS_IRQ             = 67
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F1XX_VECTORS_H_ */
