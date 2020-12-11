/*
 * pin_remap.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/platform_defs.h>
#include <halm/platform/stm32/stm32f1xx/pin_defs.h>
#include <halm/platform/stm32/stm32f1xx/pin_remap.h>
#include <halm/platform/stm32/stm32f1xx/pin_remap_defs.h>
#include <halm/platform/stm32/system.h>
#include <assert.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
#define UNPACK_TYPE(pair)  ((pair) & 0x1F)
#define UNPACK_VALUE(pair) (((pair) >> 5) & 0x07)
/*----------------------------------------------------------------------------*/
struct RemapEntry
{
  uint8_t mask;
  uint8_t offset;
};
/*----------------------------------------------------------------------------*/
static const struct RemapEntry remapTable[] = {
    [REMAP_SPI1]            = {MAPR_SPI1_MASK,            MAPR_SPI1_OFFSET},
    [REMAP_I2C1]            = {MAPR_I2C1_MASK,            MAPR_I2C1_OFFSET},
    [REMAP_USART1]          = {MAPR_USART1_MASK,          MAPR_USART1_OFFSET},
    [REMAP_USART2]          = {MAPR_USART2_MASK,          MAPR_USART2_OFFSET},
    [REMAP_USART3]          = {MAPR_USART3_MASK,          MAPR_USART3_OFFSET},
    [REMAP_USART3_PARTIAL]  = {MAPR_USART3_MASK_PARTIAL,  MAPR_USART3_OFFSET},
    [REMAP_TIM1]            = {MAPR_TIM1_MASK,            MAPR_TIM1_OFFSET},
    [REMAP_TIM1_PARTIAL]    = {MAPR_TIM1_MASK_PARTIAL,    MAPR_TIM1_OFFSET},
    [REMAP_TIM2_LOWER]      = {MAPR_TIM2_MASK_LOWER,      MAPR_TIM2_OFFSET},
    [REMAP_TIM2_UPPER]      = {MAPR_TIM2_MASK_UPPER,      MAPR_TIM2_OFFSET},
    [REMAP_TIM3]            = {MAPR_TIM3_MASK,            MAPR_TIM3_OFFSET},
    [REMAP_TIM3_PARTIAL]    = {MAPR_TIM3_MASK_PARTIAL,    MAPR_TIM3_OFFSET},
    [REMAP_TIM4]            = {MAPR_TIM4_MASK,            MAPR_TIM4_OFFSET},
    [REMAP_CAN1]            = {MAPR_CAN1_MASK,            MAPR_CAN1_OFFSET},
    [REMAP_OSC]             = {MAPR_PD01_MASK,            MAPR_PD01_OFFSET},
    [REMAP_TIM5]            = {MAPR_TIM5CH4_MASK,         MAPR_TIM5CH4_OFFSET},
    [REMAP_ETH]             = {MAPR_ETH_MASK,             MAPR_ETH_OFFSET},
    [REMAP_CAN2]            = {MAPR_CAN2_MASK,            MAPR_CAN2_OFFSET},
    [REMAP_SPI3]            = {MAPR_SPI3_MASK,            MAPR_SPI3_OFFSET},

    /* AFIO_MAPR2 */
    [REMAP_TIM9]            = {MAPR2_TIM9_MASK,           MAPR2_TIM9_OFFSET},
    [REMAP_TIM10]           = {MAPR2_TIM10_MASK,          MAPR2_TIM10_OFFSET},
    [REMAP_TIM11]           = {MAPR2_TIM11_MASK,          MAPR2_TIM11_OFFSET},
    [REMAP_TIM13]           = {MAPR2_TIM13_MASK,          MAPR2_TIM13_OFFSET},
    [REMAP_TIM14]           = {MAPR2_TIM14_MASK,          MAPR2_TIM14_OFFSET}
};
/*----------------------------------------------------------------------------*/
void pinRemapApply(uint8_t function)
{
  const size_t index = UNPACK_TYPE(function);
  assert(index != REMAP_UNUSED);

  const uint32_t mask = remapTable[index].mask << remapTable[index].offset;
  const uint32_t value = UNPACK_VALUE(function) << remapTable[index].offset;

  if (!sysClockStatus(CLK_AFIO))
    sysClockEnable(CLK_AFIO);

  volatile uint32_t * const reg = index < REMAP_TIM9 ?
      &STM_AFIO->MAPR : &STM_AFIO->MAPR2;
  const uint32_t cachedValue = *reg;

  if ((cachedValue & mask) != (value & mask))
    *reg = (cachedValue & ~mask) | value;
}
