/*
 * halm/platform/bouffalo/bl602/dma_base.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_DMA_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_BOUFFALO_BL602_DMA_BASE_H_
#define HALM_PLATFORM_BOUFFALO_BL602_DMA_BASE_H_
/*----------------------------------------------------------------------------*/
/** Direct Memory Access controller connections. */
enum [[gnu::packed]] DmaEvent
{
  DMA_UART0_RX,
  DMA_UART0_TX,
  DMA_UART1_RX,
  DMA_UART1_TX,
  DMA_I2C_RX,
  DMA_I2C_TX,
  SPI_RX,
  SPI_TX,
  DMA_ADC,
  DMA_DAC,
  DMA_MEMORY,
  DMA_EVENT_END
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_BL602_DMA_BASE_H_ */
