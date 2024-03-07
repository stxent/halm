/*
 * halm/platform/stm32/stm32f0xx/bdma_base.h
 * Copyright (C) 2020, 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_BDMA_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F0XX_BDMA_BASE_H_
#define HALM_PLATFORM_STM32_STM32F0XX_BDMA_BASE_H_
/*----------------------------------------------------------------------------*/
/** Symbolic names for the DMA streams */
enum
{
  DMA1_STREAM1,
  DMA1_STREAM2,
  DMA1_STREAM3,
  DMA1_STREAM4,
  DMA1_STREAM5,
  DMA1_STREAM6,
  DMA1_STREAM7,

  DMA2_STREAM1,
  DMA2_STREAM2,
  DMA2_STREAM3,
  DMA2_STREAM4,
  DMA2_STREAM5
};

enum [[gnu::packed]] DmaEvent
{
  DMA_GENERIC,
  DMA_EVENT_END
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F0XX_BDMA_BASE_H_ */
