/*
 * halm/platform/stm32/stm32f0xx/vectors.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_VECTORS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F0XX_VECTORS_H_
#define HALM_PLATFORM_STM32_STM32F0XX_VECTORS_H_
/*----------------------------------------------------------------------------*/
enum
{
  /* Chip-specific interrupt sources */
  WWDG_IRQ                            = 0,
  PVD_VDDIO2_IRQ                      = 1,
  RTC_IRQ                             = 2,
  FLASH_IRQ                           = 3,
  RCC_CRS_IRQ                         = 4,
  EXTI0_1_IRQ                         = 5,
  EXTI2_3_IRQ                         = 6,
  EXTI4_15_IRQ                        = 7,
  TSC_IRQ                             = 8,
  DMA1_CHANNEL1_IRQ                   = 9,
  DMA1_CHANNEL2_3_DMA2_CHANNEL1_2_IRQ = 10,
  DMA1_CHANNEL4_7_DMA2_CHANNEL3_5_IRQ = 11,
  ADC1_COMP_IRQ                       = 12,
  TIM1_BRK_UP_TRG_COM_IRQ             = 13,
  TIM1_CC_IRQ                         = 14,
  TIM2_IRQ                            = 15,
  TIM3_IRQ                            = 16,
  TIM6_DAC_IRQ                        = 17,
  TIM7_IRQ                            = 18,
  TIM14_IRQ                           = 19,
  TIM15_IRQ                           = 20,
  TIM16_IRQ                           = 21,
  TIM17_IRQ                           = 22,
  I2C1_IRQ                            = 23,
  I2C2_IRQ                            = 24,
  SPI1_IRQ                            = 25,
  SPI2_IRQ                            = 26,
  USART1_IRQ                          = 27,
  USART2_IRQ                          = 28,
  USART3_8_IRQ                        = 29,
  CEC_CAN_IRQ                         = 30,
  USB_IRQ                             = 31
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F0XX_VECTORS_H_ */
