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
struct DmaConfig
{
  int8_t channel;
  struct DmaSide source, destination;
  enum dmaDirection direction;
  enum dmaBurst burst;
  enum dmaWidth width;
};
/*----------------------------------------------------------------------------*/
struct DmaListItem /* Rewrite */
{
  uint32_t source;
  uint32_t destination;
  uint32_t next;
  uint32_t control;
};
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct DmaClass
{
  CLASS_GENERATOR(Dma)
};
/*----------------------------------------------------------------------------*/
struct Dma
{
  struct Entity parent;

  /* Transmission active flag */
  bool active;
  /* Channel may be assigned to multiple descriptors, but not at same time */
  uint8_t channel;
  enum dmaDirection direction;

  /* Precalculated values of DMA connection multiplexer register */
  uint8_t muxMask, muxValue;
  /* Precalculated values of channel control and configuration registers */
  uint32_t control, config;

  /* Pointer to DMA registers structure */
  LPC_GPDMACH_TypeDef *reg;
};
/*----------------------------------------------------------------------------*/
/* Start DMA transmission */
enum result dmaStart(struct Dma *, void *, const void *, uint16_t);
/* Start scatter-gather DMA transmission */
enum result dmaStartList(struct Dma *, struct DmaListItem *);
/* Disable a channel and lose data in the FIFO */
void dmaStop(struct Dma *);
/* Disable a channel without losing data in the FIFO */
void dmaHalt(struct Dma *);
/*----------------------------------------------------------------------------*/
/* Check whether the channel is enabled or not */
bool dmaIsActive(const struct Dma *);
/* Get the number of completed transfers */
uint16_t dmaGetCount(const struct Dma *); //FIXME
/*----------------------------------------------------------------------------*/
///* Clear first element of linked list */
//void dmaClearFirst(struct Dma *, struct DmaListItem *);
/* Link next element to list */
enum result dmaLinkItem(struct Dma *, struct DmaListItem *,
    struct DmaListItem *, void *, const void *, uint16_t);
///* Set pointer to start of the linked list for scatter-gather */
//void dmaSetList(struct Dma *, const struct DmaListItem *);
/*----------------------------------------------------------------------------*/
enum result dmaSetDescriptor(uint8_t, void *);
/*----------------------------------------------------------------------------*/
#endif /* DMA_H_ */
