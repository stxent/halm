/*
 * gpdma.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPDMA_H_
#define GPDMA_H_
/*----------------------------------------------------------------------------*/
#include "LPC17xx.h"
/*----------------------------------------------------------------------------*/
#include "dma.h"
/*----------------------------------------------------------------------------*/
extern const struct DmaClass *Gpdma;
/*----------------------------------------------------------------------------*/
/* GPDMA connections */
enum gpdmaLine
{
  GPDMA_LINE_SSP0_TX  = 0x00,
  GPDMA_LINE_SSP0_RX  = 0x04,
  GPDMA_LINE_SSP1_TX  = 0x08,
  GPDMA_LINE_SSP1_RX  = 0x0C,
  GPDMA_LINE_ADC      = 0x10,
  GPDMA_LINE_I2S0     = 0x14,
  GPDMA_LINE_I2S1     = 0x18,
  GPDMA_LINE_DAC      = 0x1C,
  GPDMA_LINE_UART0_TX = 0x20,
  GPDMA_LINE_MAT0_0   = 0x21,
  GPDMA_LINE_UART0_RX = 0x24,
  GPDMA_LINE_MAT0_1   = 0x25,
  GPDMA_LINE_UART1_TX = 0x28,
  GPDMA_LINE_MAT1_0   = 0x29,
  GPDMA_LINE_UART1_RX = 0x2C,
  GPDMA_LINE_MAT1_1   = 0x2D,
  GPDMA_LINE_UART2_TX = 0x30,
  GPDMA_LINE_MAT2_0   = 0x31,
  GPDMA_LINE_UART2_RX = 0x34,
  GPDMA_LINE_MAT2_1   = 0x35,
  GPDMA_LINE_UART3_TX = 0x38,
  GPDMA_LINE_MAT3_0   = 0x39,
  GPDMA_LINE_UART3_RX = 0x3C,
  GPDMA_LINE_MAT3_1   = 0x3D,
  GPDMA_LINE_MEMORY   = 0x40 /* Special case: reserved value */
};
/*----------------------------------------------------------------------------*/
enum gpdmaDirection
{
  GPDMA_DIR_M2M = 0, /* Memory to memory */
  GPDMA_DIR_M2P,     /* Memory to peripheral */
  GPDMA_DIR_P2M,     /* Peripheral to memory */
  GPDMA_DIR_P2P,     /* Peripheral to peripheral, controlled by DMA */
};
/*----------------------------------------------------------------------------*/
/* Source or sink descriptor */
struct GpdmaSide
{
  enum gpdmaLine line;
  bool increment;
};
/*----------------------------------------------------------------------------*/
struct GpdmaConfig
{
  /* Channel may be assigned to multiple descriptors, but not at same time */
  int8_t channel;
  struct GpdmaSide source, destination;
  enum gpdmaDirection direction;
  enum dmaBurst burst;
  enum dmaWidth width;
};
/*----------------------------------------------------------------------------*/
struct GpdmaListItem
{
  uint32_t source;
  uint32_t destination;
  uint32_t next;
  uint32_t control;
};
/*----------------------------------------------------------------------------*/
struct Gpdma
{
  struct Dma parent;

  /* Transfer direction */
  enum gpdmaDirection direction;

  /* Precalculated values of DMA connection multiplexer register */
  uint8_t muxMask, muxValue;
  /* Precalculated values of channel control and configuration registers */
  uint32_t control, config;

  /* Pointer to DMA registers structure */
  LPC_GPDMACH_TypeDef *reg;
};
/*----------------------------------------------------------------------------*/
enum result gpdmaSetDescriptor(uint8_t, void *);
/*----------------------------------------------------------------------------*/
#endif /* GPDMA_H_ */
