/*
 * halm/platform/stm32/stm32f1xx/pin_remap_defs.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_STM32F1XX_PIN_REMAP_DEFS_H_
#define HALM_PLATFORM_STM32_STM32F1XX_PIN_REMAP_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#define MAPR_SPI1_OFFSET          0
#define MAPR_SPI1_MASK            0x01
#define MAPR_I2C1_OFFSET          1
#define MAPR_I2C1_MASK            0x01
#define MAPR_USART1_OFFSET        2
#define MAPR_USART1_MASK          0x01
#define MAPR_USART2_OFFSET        3
#define MAPR_USART2_MASK          0x01
#define MAPR_USART3_OFFSET        4
#define MAPR_USART3_MASK          0x03
#define MAPR_USART3_MASK_PARTIAL  0x02
#define MAPR_TIM1_OFFSET          6
#define MAPR_TIM1_MASK            0x03
#define MAPR_TIM1_MASK_PARTIAL    0x02
#define MAPR_TIM2_OFFSET          8
#define MAPR_TIM2_MASK            0x03
#define MAPR_TIM2_MASK_LOWER      0x01
#define MAPR_TIM2_MASK_UPPER      0x02
#define MAPR_TIM3_OFFSET          10
#define MAPR_TIM3_MASK            0x01
#define MAPR_TIM3_MASK_PARTIAL    0x02
#define MAPR_TIM4_OFFSET          12
#define MAPR_TIM4_MASK            0x01
#define MAPR_CAN1_OFFSET          13
#define MAPR_CAN1_MASK            0x03
#define MAPR_PD01_OFFSET          15
#define MAPR_PD01_MASK            0x01
#define MAPR_TIM5CH4_OFFSET       16
#define MAPR_TIM5CH4_MASK         0x01
#define MAPR_ADC1_ETRGINJ_OFFSET  17
#define MAPR_ADC1_ETRGINJ_MASK    0x01
#define MAPR_ADC1_ETRGREG_OFFSET  18
#define MAPR_ADC1_ETRGREG_MASK    0x01
#define MAPR_ADC2_ETRGINJ_OFFSET  19
#define MAPR_ADC2_ETRGINJ_MASK    0x01
#define MAPR_ADC2_ETRGREG_OFFSET  20
#define MAPR_ADC2_ETRGREG_MASK    0x01
#define MAPR_ETH_OFFSET           21
#define MAPR_ETH_MASK             0x01
#define MAPR_CAN2_OFFSET          22
#define MAPR_CAN2_MASK            0x01
#define MAPR_MII_RMII_SEL_OFFSET  23
#define MAPR_MII_RMII_SEL_MASK    0x01
#define MAPR_SWJ_OFFSET           24
#define MAPR_SWJ_MASK             0x07
#define MAPR_SPI3_OFFSET          28
#define MAPR_SPI3_MASK            0x01
#define MAPR_TIM2ITR1_OFFSET      29
#define MAPR_TIM2ITR1_MASK        0x01
#define MAPR_PTP_PPS_OFFSET       30
#define MAPR_PTP_PPS_MASK         0x01
/*----------------------------------------------------------------------------*/
#define MAPR2_TIM9_OFFSET         5
#define MAPR2_TIM9_MASK           0x01
#define MAPR2_TIM10_OFFSET        6
#define MAPR2_TIM10_MASK          0x01
#define MAPR2_TIM11_OFFSET        7
#define MAPR2_TIM11_MASK          0x01
#define MAPR2_TIM13_OFFSET        8
#define MAPR2_TIM13_MASK          0x01
#define MAPR2_TIM14_OFFSET        9
#define MAPR2_TIM14_MASK          0x01
#define MAPR2_FSMC_NADV_OFFSET    10
#define MAPR2_FSMC_NADV_MASK      0x01
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F1XX_PIN_REMAP_DEFS_H_ */
