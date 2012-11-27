/*
 * dma.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "dma.h"
#include "lpc17xx_defines.h"
#include "lpc17xx_sys.h"
/*----------------------------------------------------------------------------*/
/* TODO Add linked lists support */
/*----------------------------------------------------------------------------*/
#define CHANNEL_COUNT                   8
/*----------------------------------------------------------------------------*/
/*------------------DMA configuration register--------------------------------*/
#define DMA_ENABLE                      BIT(0)
/* 0 for little-endian, 1 for big-endian */
#define DMA_ENDIANNESS                  BIT(1)
/*------------------DMA Channel Control register------------------------------*/
#define C_CONTROL_SIZE(size)            (size)
#define C_CONTROL_SRC_BURST(burst)      ((burst) << 12)
#define C_CONTROL_DEST_BURST(burst)     ((burst) << 15)
#define C_CONTROL_SRC_WIDTH(width)      ((width) << 18)
#define C_CONTROL_DEST_WIDTH(width)     ((width) << 21)
#define C_CONTROL_SRC_INC               BIT(26) /* Source increment */
#define C_CONTROL_DEST_INC              BIT(27) /* Destination increment */
//#define DMA_CONTROL_PRIVILEGED          BIT(28)
//#define DMA_CONTROL_BUFFERABLE          BIT(29)
//#define DMA_CONTROL_CACHEABLE           BIT(30)
#define C_CONTROL_INT                   BIT(31) /* Terminal count interrupt */
/*------------------DMA Channel Configuration register------------------------*/
#define C_CONFIG_ENABLE                 BIT(0)
#define C_CONFIG_SRC_PERIPH(periph)     ((periph >> 2) << 1)
#define C_CONFIG_DEST_PERIPH(periph)    ((periph >> 2) << 6)
/* Transfer type */
#define C_CONFIG_TYPE(type)             ((type) << 11)
/* Interrupt error mask */
#define C_CONFIG_IE                     BIT(14)
/* Terminal count interrupt mask */
#define C_CONFIG_ITC                    BIT(15)
/* Locked transfer, not used on LPC17xx */
#define C_CONFIG_LOCK                   BIT(16)
/* Indicates whether FIFO not empty */
#define C_CONFIG_ACTIVE                 BIT(17)
#define C_CONFIG_HALT                   BIT(18)
/*----------------------------------------------------------------------------*/
enum result gpdmaInit(struct Dma *, const void *);
void gpdmaDeinit(struct Dma *);
enum result gpdmaStart(struct Dma *, void *, void *, uint16_t);
void gpdmaStop(struct Dma *);
/*----------------------------------------------------------------------------*/
bool dmaIsActive(struct Dma *);
void dmaLinkList(struct Dma *, const struct DmaListItem *);
/*----------------------------------------------------------------------------*/
static inline LPC_GPDMACH_TypeDef *calcChannel(uint8_t);
static void setMux(struct Dma *, enum dmaLine);
/*----------------------------------------------------------------------------*/
static const struct DmaClass gpdmaTable = {
    .size = sizeof(struct Dma),
    .init = gpdmaInit,
    .deinit = gpdmaDeinit,

    .start = gpdmaStart,
    .stop = gpdmaStop
};
/*----------------------------------------------------------------------------*/
const struct DmaClass *Dma = &gpdmaTable;
/*----------------------------------------------------------------------------*/
void *dmaDescriptors[] = {0, 0, 0, 0, 0, 0, 0, 0};
/* Initialized descriptors count */
/* TODO Replace with semaphore */
static uint16_t instances = 0;
/*----------------------------------------------------------------------------*/
static inline LPC_GPDMACH_TypeDef *calcChannel(uint8_t channel)
{
  /* FIXME Rewrite */
  return (LPC_GPDMACH_TypeDef *)((void *)LPC_GPDMACH0 + channel * 0x20);
}
/*----------------------------------------------------------------------------*/
static void setMux(struct Dma *dma, enum dmaLine line)
{
  uint8_t pos;

  if (line >= DMA_LINE_UART0_TX && line != DMA_LINE_MEMORY)
  {
    pos = (line >> 2) - 8;
    dma->muxMask &= ~(1 << pos);
    dma->muxValue |= (line & 1) << pos;
  }
}
/*----------------------------------------------------------------------------*/
void DMA_IRQHandler(void)
{
  uint8_t counter = 0;
  uint32_t stat = LPC_GPDMA->DMACIntTCStat;

  for (; counter < CHANNEL_COUNT; counter++)
  {
    if (stat & (1 << counter))
    {
      if (dmaDescriptors[counter])
      {
        /* Clear channel descriptor */
        ((struct Dma **)dmaDescriptors)[counter]->active = false;
        ((struct Dma **)dmaDescriptors)[counter] = 0;
      }
      /* Clear terminal count interrupt flag */
      LPC_GPDMA->DMACIntTCClear = 1 << counter;
    }
  }
  LPC_GPDMA->DMACIntErrClr = (1 << CHANNEL_COUNT) - 1;
}
/*----------------------------------------------------------------------------*/
enum result gpdmaInit(struct Dma *gpdma, const void *cdata)
{
  const struct DmaConfig *config = (const struct DmaConfig *)cdata;
//  struct Dma *gpdma = (struct Dma *)dma;

  if (!config || config->channel >= CHANNEL_COUNT)
    return E_ERROR;

  /* TODO Add channel allocation, when channel is -1 */
  gpdma->channel = (uint8_t)config->channel;
  gpdma->active = false;
  gpdma->direction = config->direction;
  gpdma->reg = calcChannel(gpdma->channel);

  gpdma->control = C_CONTROL_INT |
      C_CONTROL_SRC_BURST(config->burst) | C_CONTROL_DEST_BURST(config->burst) |
      C_CONTROL_SRC_WIDTH(config->width) | C_CONTROL_DEST_WIDTH(config->width);
  if (config->source.increment)
    gpdma->control |= C_CONTROL_SRC_INC;
  if (config->destination.increment)
    gpdma->control |= C_CONTROL_DEST_INC;

  gpdma->config = C_CONFIG_TYPE(config->direction) | C_CONFIG_IE | C_CONFIG_ITC;
  if (config->source.line != DMA_LINE_MEMORY)
    gpdma->config |= C_CONFIG_SRC_PERIPH(config->source.line);
  if (config->destination.line != DMA_LINE_MEMORY)
    gpdma->config |= C_CONFIG_DEST_PERIPH(config->destination.line);

  /* Reset multiplexer register values */
  gpdma->muxMask = 0xFF;
  gpdma->muxValue = 0x00;
  /* Calculate new mask and value */
  setMux(gpdma, config->source.line);
  setMux(gpdma, config->destination.line);

  if (!instances)
  {
    LPC_SC->PCONP |= PCONP_PCGPDMA;
    LPC_GPDMA->DMACConfig |= DMA_ENABLE;
    NVIC_EnableIRQ(DMA_IRQn);
  }
  instances++;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
void gpdmaDeinit(struct Dma *dma)
{
  instances--;
  /* Disable DMA peripheral when no active descriptors exist */
  if (!instances)
  {
    NVIC_DisableIRQ(DMA_IRQn);
    LPC_GPDMA->DMACConfig &= ~DMA_ENABLE;
    LPC_SC->PCONP &= ~PCONP_PCGPDMA;
  }
}
/*----------------------------------------------------------------------------*/
enum result gpdmaStart(struct Dma *dma, void *src, void *dest, uint16_t size)
{
  if (dmaDescriptors[dma->channel])
    return E_ERROR;

  LPC_SC->DMAREQSEL &= dma->muxMask;
  LPC_SC->DMAREQSEL |= dma->muxValue;

  dma->reg->DMACCSrcAddr = (uint32_t)src;
  dma->reg->DMACCDestAddr = (uint32_t)dest;

  /* Transfer size is 12-bit width value */
  dma->reg->DMACCControl = dma->control | C_CONTROL_SIZE(size);

  dma->reg->DMACCLLI = 0;
  dma->reg->DMACCConfig = dma->config;

  /* Clear interrupt requests */
  LPC_GPDMA->DMACIntTCClear |= 1 << dma->channel;
  LPC_GPDMA->DMACIntErrClr |= 1 << dma->channel;

  dma->active = true;
  dmaDescriptors[dma->channel] = dma;
  dma->reg->DMACCConfig |= C_CONFIG_ENABLE;
}
/*----------------------------------------------------------------------------*/
void gpdmaStop(struct Dma *dma)
{

}
/*----------------------------------------------------------------------------*/
bool dmaIsActive(struct Dma *dma)
{
  return LPC_GPDMA->DMACEnbldChns & (1 << dma->channel) != 0;
}
/*----------------------------------------------------------------------------*/
void dmaLinkList(struct Dma *dma, const struct DmaListItem *list)
{
  /* Linked list items must be aligned at 4-byte boundary */
  dma->reg->DMACCLLI = (uint32_t)list;
}
