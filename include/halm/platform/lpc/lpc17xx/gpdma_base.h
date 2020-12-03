/*
 * halm/platform/lpc/lpc17xx/gpdma_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC17XX_GPDMA_BASE_H_
#define HALM_PLATFORM_LPC_LPC17XX_GPDMA_BASE_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
/** Direct Memory Access controller connections. */
enum GpDmaEvent
{
  GPDMA_SSP0_RX,
  GPDMA_SSP1_RX,
  GPDMA_SSP0_TX,
  GPDMA_SSP1_TX,
  GPDMA_I2S0_REQ1,
  GPDMA_I2S0_REQ2,
  GPDMA_UART0_RX,
  GPDMA_UART1_RX,
  GPDMA_UART2_RX,
  GPDMA_UART3_RX,
  GPDMA_UART0_TX,
  GPDMA_UART1_TX,
  GPDMA_UART2_TX,
  GPDMA_UART3_TX,
  GPDMA_MAT0_0,
  GPDMA_MAT0_1,
  GPDMA_MAT1_0,
  GPDMA_MAT1_1,
  GPDMA_MAT2_0,
  GPDMA_MAT2_1,
  GPDMA_MAT3_0,
  GPDMA_MAT3_1,
  GPDMA_ADC0,
  GPDMA_DAC,
  GPDMA_MEMORY,
  GPDMA_EVENT_END
} __attribute__((packed));

struct GpDmaMuxConfig
{
  uint8_t mask;
  uint8_t value;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC17XX_GPDMA_BASE_H_ */
