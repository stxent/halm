/*
 * halm/platform/stm32/stm32f1xx/pin_remap.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_STM32F1XX_PIN_REMAP_H_
#define HALM_PLATFORM_STM32_STM32F1XX_PIN_REMAP_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#define PACK_REMAP(type, value) (((value) << 5) | (type))
/*----------------------------------------------------------------------------*/
enum RemapGroup
{
  REMAP_UNUSED,

  /* AFIO_MAPR */
  REMAP_SPI1,
  REMAP_I2C1,
  REMAP_USART1,
  REMAP_USART2,
  REMAP_USART3,
  REMAP_USART3_PARTIAL,
  REMAP_TIM1,
  REMAP_TIM1_PARTIAL,
  REMAP_TIM2_LOWER,
  REMAP_TIM2_UPPER,
  REMAP_TIM3,
  REMAP_TIM3_PARTIAL,
  REMAP_TIM4,
  REMAP_CAN1,
  REMAP_OSC,
  REMAP_TIM5,
  REMAP_ETH,
  REMAP_CAN2,
  REMAP_SPI3,
  REMAP_SWJ,

  /* AFIO_MAPR2 */
  REMAP_TIM9,
  REMAP_TIM10,
  REMAP_TIM11,
  REMAP_TIM13,
  REMAP_TIM14
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void pinRemapApply(uint8_t);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F1XX_PIN_REMAP_H_ */
