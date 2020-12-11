/*
 * halm/platform/lpc/lpc43xx/gpdma_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GPDMA_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC43XX_GPDMA_BASE_H_
#define HALM_PLATFORM_LPC_LPC43XX_GPDMA_BASE_H_
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
  GPDMA_I2S1_REQ1,
  GPDMA_I2S1_REQ2,
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
  GPDMA_ADC1,
  GPDMA_DAC,
  GPDMA_ADCHS_READ,
  GPDMA_ADCHS_WRITE,
  GPDMA_SGPIO14,
  GPDMA_SGPIO15,
  GPDMA_SCT_REQ0,
  GPDMA_SCT_REQ1,
  GPDMA_SCT_OUT2,
  GPDMA_SCT_OUT3,
  GPDMA_SPIFI,
  GPDMA_MEMORY,
  GPDMA_EVENT_END
} __attribute__((packed));

struct GpDmaMuxConfig
{
  uint32_t mask;
  uint32_t value;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_GPDMA_BASE_H_ */
