/*
 * dma.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef DMA_H_
#define DMA_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
#include "LPC17xx.h"
/*----------------------------------------------------------------------------*/
#include "error.h"
#include "entity.h"
/*----------------------------------------------------------------------------*/
struct Dma;
struct DmaClass;
/*----------------------------------------------------------------------------*/
extern const struct DmaClass *Dma;
extern void *dmaDescriptors[];
/*----------------------------------------------------------------------------*/
/* Controller-specific lines */
enum dmaLine
{
  DMA_LINE_SSP0_TX  = 0x00,
  DMA_LINE_SSP0_RX  = 0x04,
  DMA_LINE_SSP1_TX  = 0x08,
  DMA_LINE_SSP1_RX  = 0x0C,
  DMA_LINE_ADC      = 0x10,
  DMA_LINE_I2S0     = 0x14,
  DMA_LINE_I2S1     = 0x18,
  DMA_LINE_DAC      = 0x1C,
  DMA_LINE_UART0_TX = 0x20,
  DMA_LINE_MAT0_0   = 0x21,
  DMA_LINE_UART0_RX = 0x24,
  DMA_LINE_MAT0_1   = 0x25,
  DMA_LINE_UART1_TX = 0x28,
  DMA_LINE_MAT1_0   = 0x29,
  DMA_LINE_UART1_RX = 0x2C,
  DMA_LINE_MAT1_1   = 0x2D,
  DMA_LINE_UART2_TX = 0x30,
  DMA_LINE_MAT2_0   = 0x31,
  DMA_LINE_UART2_RX = 0x34,
  DMA_LINE_MAT2_1   = 0x35,
  DMA_LINE_UART3_TX = 0x38,
  DMA_LINE_MAT3_0   = 0x39,
  DMA_LINE_UART3_RX = 0x3C,
  DMA_LINE_MAT3_1   = 0x3D,
  DMA_LINE_MEMORY   = 0x40 /* Special case: reserved value */
};
/*----------------------------------------------------------------------------*/
enum dmaDirection
{
  DMA_DIR_M2M = 0,    /* Memory to memory */
  DMA_DIR_M2P,        /* Memory to peripheral */
  DMA_DIR_P2M,        /* Peripheral to memory */
  DMA_DIR_P2P,        /* Peripheral to peripheral, controlled by DMA */
  /* Reserved on LPC17xx */
  DMA_DIR_P2P_DEST,   /* Peripheral to peripheral, controlled by destination */
  DMA_DIR_M2P_PERIPH, /* Memory to peripheral, controlled by peripheral */
  DMA_DIR_P2M_PERIPH, /* Peripheral to memory, controlled by peripheral */
  DMA_DIR_P2P_SRC,    /* Peripheral to peripheral, controlled by source */
};
/*----------------------------------------------------------------------------*/
enum dmaBurst
{
  DMA_BURST_1 = 0,
  DMA_BURST_4,
  DMA_BURST_8,
  DMA_BURST_16,
  DMA_BURST_32,
  DMA_BURST_64,
  DMA_BURST_128,
  DMA_BURST_256
};
/*----------------------------------------------------------------------------*/
enum dmaWidth
{
  DMA_WIDTH_BYTE = 0,
  DMA_WIDTH_HALFWORD,
  DMA_WIDTH_WORD
};
/*----------------------------------------------------------------------------*/
/* Source or sink descriptor */
struct DmaSide
{
  enum dmaLine line;
  bool increment;
};
/*----------------------------------------------------------------------------*/
struct DmaParameters
{
  bool enabled;
  int8_t channel;
};
/*----------------------------------------------------------------------------*/
struct DmaConfig
{
  struct DmaSide source, destination;
  enum dmaBurst burst;
  enum dmaWidth width;
  enum dmaDirection direction;
  int8_t channel;
};
/*----------------------------------------------------------------------------*/
struct DmaListItem
{
  void *source;
  void *destination;
  void *next;
  uint32_t control;
};
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct DmaClass
{
  CLASS_GENERATOR(Dma)

  /* Start DMA transmission */
  enum result (*start)(struct Dma *, void *, void *, uint16_t);
  /* Disable a channel and lose data in the FIFO */
  uint16_t (*stop)(struct Dma *);
  /* Disable a channel without losing data in the FIFO */
  uint16_t (*halt)(struct Dma *);
};
/*----------------------------------------------------------------------------*/
struct Dma
{
  enum dmaDirection direction;
  bool active;
  uint8_t channel;

  /* Precalculated values of DMA connection multiplexer register */
  uint8_t muxMask, muxValue;
  /* Precalculated values of channel control and configuration registers */
  uint32_t control, config;

  /* Device-specific data */
  LPC_GPDMACH_TypeDef *reg;
};
/*----------------------------------------------------------------------------*/
bool dmaIsActive(struct Dma *); /* TODO Add const */
void dmaLinkList(struct Dma *, const struct DmaListItem *);
/*----------------------------------------------------------------------------*/
/*------------------Virtual functions-----------------------------------------*/
enum result dmaStart(struct Dma *, void *, void *, uint16_t);
uint16_t dmaStop(struct Dma *);
uint16_t dmaHalt(struct Dma *);
/*----------------------------------------------------------------------------*/
#endif /* DMA_H_ */
