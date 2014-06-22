/*
 * platform/nxp/lpc17xx/gpdma_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPDMA_BASE_H_
#define GPDMA_BASE_H_
/*----------------------------------------------------------------------------*/
/** Direct Memory Access controller connections. */
enum gpDmaEvent
{
  GPDMA_SSP0_RX,
  GPDMA_SSP1_RX,
  GPDMA_SSP0_TX,
  GPDMA_SSP1_TX,
  GPDMA_I2S0,
  GPDMA_I2S1,
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
  GPDMA_ADC,
  GPDMA_DAC,
  GPDMA_MEMORY,
  GPDMA_EVENT_END
};
/*----------------------------------------------------------------------------*/
struct GpDmaMuxConfig
{
  uint8_t mask;
  uint8_t value;
};
/*----------------------------------------------------------------------------*/
#endif /* GPDMA_BASE_H_ */