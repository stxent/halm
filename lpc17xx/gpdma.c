/*
 * gpdma.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "lpc17xx_sys.h"
#include "gpdma.h"
#include "gpdma_defs.h"
#include "mutex.h"
/*----------------------------------------------------------------------------*/
#define CHANNEL_COUNT 8
/*----------------------------------------------------------------------------*/
static inline LPC_GPDMACH_TypeDef *calcChannel(uint8_t);
static void setMux(struct Gpdma *, enum gpdmaLine);
void terminalHandler(struct Gpdma *);
void errorHandler(struct Gpdma *);
/*----------------------------------------------------------------------------*/
static enum result gpdmaInit(void *, const void *);
static void gpdmaDeinit(void *);
static enum result gpdmaStart(void *, void *, const void *, uint32_t);
static enum result gpdmaStartList(void *, const void *);
static void gpdmaStop(void *);
static void gpdmaHalt(void *);
static void gpdmaLinkItem(void *, void *, void *, void *, const void *,
    uint16_t);
/*----------------------------------------------------------------------------*/
static const struct DmaClass gpdmaTable = {
    .size = sizeof(struct Gpdma),
    .init = gpdmaInit,
    .deinit = gpdmaDeinit,

    .start = gpdmaStart,
    .startList = gpdmaStartList,
    .stop = gpdmaStop,
    .halt = gpdmaHalt,
    .linkItem = gpdmaLinkItem
};
/*----------------------------------------------------------------------------*/
const struct DmaClass *Gpdma = &gpdmaTable;
/*----------------------------------------------------------------------------*/
static void * volatile descriptors[] = {0, 0, 0, 0, 0, 0, 0, 0};
static Mutex lock = MUTEX_UNLOCKED;
/* Initialized descriptors count */
static uint16_t instances = 0;
/*----------------------------------------------------------------------------*/
/* TODO Replace return type */
enum result gpdmaSetDescriptor(uint8_t channel, void *descriptor)
{
  enum result res = E_ERROR;

  mutexLock(&lock);
  if (!descriptors[channel])
  {
    descriptors[channel] = descriptor;
    res = E_OK;
  }
  mutexUnlock(&lock);
  return res;
}
/*----------------------------------------------------------------------------*/
void terminalHandler(struct Gpdma *controller)
{
  controller->parent.active = false;
}
/*----------------------------------------------------------------------------*/
void errorHandler(struct Gpdma *controller)
{
  controller->parent.active = false;
}
/*----------------------------------------------------------------------------*/
void DMA_IRQHandler(void)
{
  int8_t counter = CHANNEL_COUNT - 1;
  uint8_t mask = 0x80;
  uint8_t terminalStat = LPC_GPDMA->DMACIntTCStat;
  uint8_t errorStat = LPC_GPDMA->DMACIntErrStat;

  for (; counter >= 0; counter--, mask >>= 1)
  {
    if (terminalStat & mask)
    {
      if (descriptors[counter])
      {
        terminalHandler(descriptors[counter]);
        descriptors[counter] = 0; /* Clear channel descriptor */
      }
      /* Clear terminal count interrupt flag */
      LPC_GPDMA->DMACIntTCClear = mask;
    }
    if (errorStat & mask)
    {
      if (descriptors[counter])
      {
        errorHandler(descriptors[counter]);
        descriptors[counter] = 0; /* Clear channel descriptor */
      }
      /* Clear error interrupt flag */
      LPC_GPDMA->DMACIntErrClr = mask;
    }
  }
}
/*----------------------------------------------------------------------------*/
static inline LPC_GPDMACH_TypeDef *calcChannel(uint8_t channel)
{
  return (LPC_GPDMACH_TypeDef *)((void *)LPC_GPDMACH0 +
      ((void *)LPC_GPDMACH1 - (void *)LPC_GPDMACH0) * channel);
}
/*----------------------------------------------------------------------------*/
static void setMux(struct Gpdma *controller, enum gpdmaLine line)
{
  uint8_t pos;

  if (line >= GPDMA_LINE_UART0_TX && line != GPDMA_LINE_MEMORY)
  {
    pos = (line >> 2) - 8;
    controller->muxMask &= ~(1 << pos);
    controller->muxValue |= (line & 1) << pos;
  }
}
/*----------------------------------------------------------------------------*/
static enum result gpdmaInit(void *object, const void *configPtr)
{
  const struct GpdmaConfig *config = configPtr;
  struct Gpdma *controller = object;

  if (!config || config->channel >= CHANNEL_COUNT)
    return E_ERROR;

  /* TODO Add channel allocation, when channel is -1 */
  controller->parent.channel = (uint8_t)config->channel;
  controller->parent.active = false;
  controller->direction = config->direction;
  controller->reg = calcChannel(controller->parent.channel);

  controller->control = C_CONTROL_INT | C_CONTROL_SRC_WIDTH(config->width) |
      C_CONTROL_DEST_WIDTH(config->width);
  /* LPC17xx does not support 2 bytes burst */
  if (config->burst >= DMA_BURST_4)
  {
    controller->control |= C_CONTROL_SRC_BURST(config->burst - 1) |
        C_CONTROL_DEST_BURST(config->burst - 1);
  }
  if (config->source.increment)
    controller->control |= C_CONTROL_SRC_INC;
  if (config->destination.increment)
    controller->control |= C_CONTROL_DEST_INC;

  controller->config = C_CONFIG_TYPE(config->direction) |
      C_CONFIG_IE | C_CONFIG_ITC;
  if (config->source.line != GPDMA_LINE_MEMORY)
    controller->config |= C_CONFIG_SRC_PERIPH(config->source.line);
  if (config->destination.line != GPDMA_LINE_MEMORY)
    controller->config |= C_CONFIG_DEST_PERIPH(config->destination.line);

  /* Reset multiplexer register values */
  controller->muxMask = 0xFF;
  controller->muxValue = 0x00;
  /* Calculate new mask and value */
  setMux(controller, config->source.line);
  setMux(controller, config->destination.line);

  mutexLock(&lock);
  if (!instances)
  {
    sysPowerEnable(PCON_GPDMA);
    LPC_GPDMA->DMACConfig |= DMA_ENABLE;
    NVIC_EnableIRQ(DMA_IRQn);
    //TODO add priority config
    //NVIC_SetPriority(device->irq, GET_PRIORITY(config->priority));
  }
  instances++;
  mutexUnlock(&lock);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void gpdmaDeinit(void *object)
{
  struct Gpdma *controller = object;

  mutexLock(&lock);
  instances--;
  /* Disable DMA peripheral when no active descriptors exist */
  if (!instances)
  {
    NVIC_DisableIRQ(DMA_IRQn);
    LPC_GPDMA->DMACConfig &= ~DMA_ENABLE;
    sysPowerDisable(PCON_GPDMA);
  }
  mutexUnlock(&lock);
}
/*----------------------------------------------------------------------------*/
enum result gpdmaStart(void *object, void *dest, const void *src, uint32_t size)
{
  struct Gpdma *controller = object;

  if (gpdmaSetDescriptor(controller->parent.channel, object) != E_OK)
    return E_ERROR;

  LPC_SC->DMAREQSEL &= controller->muxMask;
  LPC_SC->DMAREQSEL |= controller->muxValue;

  controller->reg->DMACCSrcAddr = (uint32_t)src;
  controller->reg->DMACCDestAddr = (uint32_t)dest;

  /* Transfer size is 12-bit width value */
  controller->reg->DMACCControl = controller->control | C_CONTROL_SIZE(size);

  controller->reg->DMACCLLI = 0;
  controller->reg->DMACCConfig = controller->config;

  /* Clear interrupt requests */
  LPC_GPDMA->DMACIntTCClear |= 1 << controller->parent.channel;
  LPC_GPDMA->DMACIntErrClr |= 1 << controller->parent.channel;

  controller->parent.active = true;
  controller->reg->DMACCConfig |= C_CONFIG_ENABLE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
enum result gpdmaStartList(void *object, const void *first)
{
  struct Gpdma *controller = object;
  const struct GpdmaListItem *item = first;

  if (gpdmaSetDescriptor(controller->parent.channel, object) != E_OK)
    return E_ERROR;

  LPC_SC->DMAREQSEL &= controller->muxMask;
  LPC_SC->DMAREQSEL |= controller->muxValue;

  controller->reg->DMACCSrcAddr = item->source;
  controller->reg->DMACCDestAddr = item->destination;
  controller->reg->DMACCControl = item->control;
  controller->reg->DMACCLLI = item->next;

  controller->reg->DMACCConfig = controller->config;

  /* Clear interrupt requests */
  LPC_GPDMA->DMACIntTCClear |= 1 << controller->parent.channel;
  LPC_GPDMA->DMACIntErrClr |= 1 << controller->parent.channel;

  controller->parent.active = true;
  controller->reg->DMACCConfig |= C_CONFIG_ENABLE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
void gpdmaStop(void *object)
{
  /* Disable channel */
  ((struct Gpdma *)object)->reg->DMACCConfig &= ~C_CONFIG_ENABLE;
}
/*----------------------------------------------------------------------------*/
void gpdmaHalt(void *object)
{
  /* Ignore future requests */
  ((struct Gpdma *)object)->reg->DMACCConfig |= C_CONFIG_HALT;
  //FIXME Clear enable bit?
}
/*----------------------------------------------------------------------------*/
//uint16_t dmaGetCount(const struct Dma *device)
//{
//  /*
//   * Reading transfer size when the channel is active does not give
//   * useful information
//   */
//  //FIXME Check work with linked lists
//  if (!dmaIsActive(device))
//  {
//    /* Return transferred bytes count */
//    return device->reg->DMACCControl & C_CONTROL_SIZE_MASK;
//  }
//  return 0;
//}
/*----------------------------------------------------------------------------*/
static void gpdmaLinkItem(void *object, void *current, void *next,
    void *dest, const void *src, uint16_t size)
{
  struct GpdmaListItem *item = current;

  item->source = (uint32_t)src;
  item->destination = (uint32_t)dest;
  item->next = (uint32_t)next;
  item->control = ((struct Gpdma *)object)->control | C_CONTROL_SIZE(size);
}
