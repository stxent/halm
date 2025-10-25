/*
 * halm/platform/stm32/stm32f4xx/dma_base.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_DMA_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F4XX_DMA_BASE_H_
#define HALM_PLATFORM_STM32_STM32F4XX_DMA_BASE_H_
/*----------------------------------------------------------------------------*/
/** Symbolic names for the DMA streams */
enum
{
  DMA1_STREAM0,
  DMA1_STREAM1,
  DMA1_STREAM2,
  DMA1_STREAM3,
  DMA1_STREAM4,
  DMA1_STREAM5,
  DMA1_STREAM6,
  DMA1_STREAM7,

  DMA2_STREAM0,
  DMA2_STREAM1,
  DMA2_STREAM2,
  DMA2_STREAM3,
  DMA2_STREAM4,
  DMA2_STREAM5,
  DMA2_STREAM6,
  DMA2_STREAM7
};

enum [[gnu::packed]] DmaEvent
{
  DMA_SPI1_RX,
  DMA_SPI1_TX,
  DMA_SPI2_RX,
  DMA_SPI2_TX,
  DMA_SPI3_RX,
  DMA_SPI3_TX,
  DMA_I2C1_RX,
  DMA_I2C1_TX,
  DMA_I2C2_RX,
  DMA_I2C2_TX,
  DMA_I2C3_RX,
  DMA_I2C3_TX,
  DMA_I2S2EXT_RX,
  DMA_I2S2EXT_TX,
  DMA_I2S3EXT_RX,
  DMA_I2S3EXT_TX,
  DMA_USART1_RX,
  DMA_USART1_TX,
  DMA_USART2_RX,
  DMA_USART2_TX,
  DMA_USART3_RX,
  DMA_USART3_TX,
  DMA_UART4_RX,
  DMA_UART4_TX,
  DMA_UART5_RX,
  DMA_UART5_TX,
  DMA_USART6_RX,
  DMA_USART6_TX,
  DMA_ADC1,
  DMA_ADC2,
  DMA_ADC3,
  DMA_DAC1,
  DMA_DAC2,
  DMA_DCMI,
  DMA_SDIO,
  DMA_CRYP_IN,
  DMA_CRYP_OUT,
  DMA_HASH_IN,

  DMA_TIM1_CH1,
  DMA_TIM1_CH1_CH2_CH3,
  DMA_TIM1_CH2,
  DMA_TIM1_CH3,
  DMA_TIM1_CH4_TRIG_COM,
  DMA_TIM1_TRIG,
  DMA_TIM1_UP,

  DMA_TIM2_CH1,
  DMA_TIM2_CH2_CH4,
  DMA_TIM2_CH3_UP,
  DMA_TIM2_CH4_UP,

  DMA_TIM3_CH1_TRIG,
  DMA_TIM3_CH2,
  DMA_TIM3_CH3,
  DMA_TIM3_CH4_UP,

  DMA_TIM4_CH1,
  DMA_TIM4_CH2,
  DMA_TIM4_CH3,
  DMA_TIM4_UP,

  DMA_TIM5_CH1,
  DMA_TIM5_CH2,
  DMA_TIM5_CH3_UP,
  DMA_TIM5_CH4_TRIG,
  DMA_TIM5_UP,

  DMA_TIM6_UP,
  DMA_TIM7_UP,

  DMA_TIM8_CH1,
  DMA_TIM8_CH1_CH2_CH3,
  DMA_TIM8_CH2,
  DMA_TIM8_CH3,
  DMA_TIM8_CH4_TRIG_COM,
  DMA_TIM8_UP,

  DMA_MEMORY,
  DMA_EVENT_END
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline enum DmaEvent dmaGetEventAdc(uint8_t channel)
{
  return DMA_ADC1 + channel;
}

static inline enum DmaEvent dmaGetEventI2CRx(uint8_t channel)
{
  return DMA_I2C1_RX + channel * 2;
}

static inline enum DmaEvent dmaGetEventI2CTx(uint8_t channel)
{
  return DMA_I2C1_TX + channel * 2;
}

static inline enum DmaEvent dmaGetEventI2SRx(uint8_t channel)
{
  return DMA_I2S2EXT_RX + (channel - 1) * 2;
}

static inline enum DmaEvent dmaGetEventI2STx(uint8_t channel)
{
  return DMA_I2S2EXT_TX + (channel - 1) * 2;
}

static inline enum DmaEvent dmaGetEventSdio(void)
{
  return DMA_SDIO;
}

static inline enum DmaEvent dmaGetEventSpiRx(uint8_t channel)
{
  return DMA_SPI1_RX + channel * 2;
}

static inline enum DmaEvent dmaGetEventSpiTx(uint8_t channel)
{
  return DMA_SPI1_TX + channel * 2;
}

static inline enum DmaEvent dmaGetEventUartRx(uint8_t channel)
{
  return DMA_USART1_RX + channel * 2;
}

static inline enum DmaEvent dmaGetEventUartTx(uint8_t channel)
{
  return DMA_USART1_TX + channel * 2;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F4XX_DMA_BASE_H_ */
