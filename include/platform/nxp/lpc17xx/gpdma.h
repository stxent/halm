/*
 * platform/nxp/lpc17xx/gpdma.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPDMA_H_
#define GPDMA_H_
/*----------------------------------------------------------------------------*/
/* GPDMA connections */
enum
{
  GPDMA_SSP0_RX = 0,
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
  GPDMA_EVENT_END
};
/*----------------------------------------------------------------------------*/
#endif /* GPDMA_H_ */
