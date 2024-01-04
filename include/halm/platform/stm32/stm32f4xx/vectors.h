/*
 * halm/platform/stm32/stm32f4xx/vectors.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_VECTORS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F4XX_VECTORS_H_
#define HALM_PLATFORM_STM32_STM32F4XX_VECTORS_H_
/*----------------------------------------------------------------------------*/
enum
{
  /* Chip-specific interrupt sources */
  WWDG_IRQ                = 0,
  PVD_IRQ                 = 1,
  TAMP_STAMP_IRQ          = 2,
  RTC_WKUP_IRQ            = 3,
  FLASH_IRQ               = 4,
  RCC_IRQ                 = 5,
  EXTI0_IRQ               = 6,
  EXTI1_IRQ               = 7,
  EXTI2_IRQ               = 8,
  EXTI3_IRQ               = 9,
  EXTI4_IRQ               = 10,
  DMA1_STREAM0_IRQ        = 11,
  DMA1_STREAM1_IRQ        = 12,
  DMA1_STREAM2_IRQ        = 13,
  DMA1_STREAM3_IRQ        = 14,
  DMA1_STREAM4_IRQ        = 15,
  DMA1_STREAM5_IRQ        = 16,
  DMA1_STREAM6_IRQ        = 17,
  ADC_IRQ                 = 18,
  CAN1_TX_IRQ             = 19,
  CAN1_RX0_IRQ            = 20,
  CAN1_RX1_IRQ            = 21,
  CAN1_SCE_IRQ            = 22,
  EXTI9_5_IRQ             = 23,
  TIM1_BRK_TIM9_IRQ       = 24,
  TIM1_UP_TIM10_IRQ       = 25,
  TIM1_TRG_COM_TIM11_IRQ  = 26,
  TIM1_CC_IRQ             = 27,
  TIM2_IRQ                = 28,
  TIM3_IRQ                = 29,
  TIM4_IRQ                = 30,
  I2C1_EV_IRQ             = 31,
  I2C1_ER_IRQ             = 32,
  I2C2_EV_IRQ             = 33,
  I2C2_ER_IRQ             = 34,
  SPI1_IRQ                = 35,
  SPI2_IRQ                = 36,
  USART1_IRQ              = 37,
  USART2_IRQ              = 38,
  USART3_IRQ              = 39,
  EXTI15_10_IRQ           = 40,
  RTC_ALARM_IRQ           = 41,
  OTG_FS_WKUP_IRQ         = 42,
  TIM8_BRK_TIM12_IRQ      = 43,
  TIM8_UP_TIM13_IRQ       = 44,
  TIM8_TRG_COM_TIM14_IRQ  = 45,
  TIM8_CC_IRQ             = 46,
  DMA1_STREAM7_IRQ        = 47,
  FSMC_IRQ                = 48,
  SDIO_IRQ                = 49,
  TIM5_IRQ                = 50,
  SPI3_IRQ                = 51,
  UART4_IRQ               = 52,
  UART5_IRQ               = 53,
  TIM6_DAC_IRQ            = 54,
  TIM7_IRQ                = 55,
  DMA2_STREAM0_IRQ        = 56,
  DMA2_STREAM1_IRQ        = 57,
  DMA2_STREAM2_IRQ        = 58,
  DMA2_STREAM3_IRQ        = 59,
  DMA2_STREAM4_IRQ        = 60,
  ETH_IRQ                 = 61,
  ETH_WKUP_IRQ            = 62,
  CAN2_TX_IRQ             = 63,
  CAN2_RX0_IRQ            = 64,
  CAN2_RX1_IRQ            = 65,
  CAN2_SCE_IRQ            = 66,
  OTG_FS_IRQ              = 67,
  DMA2_STREAM5_IRQ        = 68,
  DMA2_STREAM6_IRQ        = 69,
  DMA2_STREAM7_IRQ        = 70,
  USART6_IRQ              = 71,
  I2C3_EV_IRQ             = 72,
  I2C3_ER_IRQ             = 73,
  OTG_HS_EP1_OUT_IRQ      = 74,
  OTG_HS_EP1_IN_IRQ       = 75,
  OTG_HS_WKUP_IRQ         = 76,
  OTG_HS_IRQ              = 77,
  DCMI_IRQ                = 78,
  CRYP_IRQ                = 79,
  HASH_RNG_IRQ            = 80,
  FPU_IRQ                 = 81
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F4XX_VECTORS_H_ */
