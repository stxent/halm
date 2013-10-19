/*
 * gpdma.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <irq.h>
#include <spinlock.h>
#include <platform/nxp/lpc17xx/gpdma.h>
#include <platform/nxp/lpc17xx/gpdma_defs.h>
#include <platform/nxp/lpc17xx/power.h>
/*----------------------------------------------------------------------------*/
#define CHANNEL_COUNT 8
/*----------------------------------------------------------------------------*/
static inline LPC_GPDMACH_TypeDef *calcPeripheral(uint8_t);
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t, struct GpDma *);
static void setMux(struct GpDma *, enum gpDmaLine);
static void errorHandler(struct GpDma *);
static void terminalHandler(struct GpDma *);
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
    .size = sizeof(struct GpDma),
    .init = gpdmaInit,
    .deinit = gpdmaDeinit,

    .start = gpdmaStart,
    .startList = gpdmaStartList,
    .stop = gpdmaStop,
    .halt = gpdmaHalt,
    .linkItem = gpdmaLinkItem
};
/*----------------------------------------------------------------------------*/
const struct DmaClass *GpDma = &gpdmaTable;
/*----------------------------------------------------------------------------*/
static struct GpDma * volatile descriptors[] = {0, 0, 0, 0, 0, 0, 0, 0};
/* Access to descriptor array */
static spinlock_t lock = SPIN_UNLOCKED;
/* Initialized descriptors count */
static uint16_t instances = 0;
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel, struct GpDma *controller)
{
  enum result res = E_BUSY;

  irqDisable(DMA_IRQ);
  if (spinTryLock(&lock))
  {
    if (!descriptors[channel])
    {
      descriptors[channel] = controller;
      res = E_OK;
    }
    spinUnlock(&lock);
  }
  irqEnable(DMA_IRQ);

  return res;
}
/*----------------------------------------------------------------------------*/
static void terminalHandler(struct GpDma *controller)
{
  controller->parent.active = false;
}
/*----------------------------------------------------------------------------*/
static void errorHandler(struct GpDma *controller)
{
  controller->parent.active = false;
}
/*----------------------------------------------------------------------------*/
void DMA_ISR(void)
{
  const uint8_t errorStat = LPC_GPDMA->DMACIntErrStat;
  const uint8_t terminalStat = LPC_GPDMA->DMACIntTCStat;
  uint8_t counter = 0, mask = 0x80;

  for (; counter < CHANNEL_COUNT; ++counter, mask <<= 1)
  {
    if (descriptors[counter] && (terminalStat | errorStat) & mask)
    {
      spinLock(&lock);
      descriptors[counter] = 0; /* Clear channel descriptor */
      spinUnlock(&lock);

      if (terminalStat & mask)
      {
        terminalHandler(descriptors[counter]);
        /* TODO Add callback */
      }
      else
        errorHandler(descriptors[counter]);
    }
  }
  LPC_GPDMA->DMACIntErrClr = errorStat;
  LPC_GPDMA->DMACIntTCClear = terminalStat;
}
/*----------------------------------------------------------------------------*/
static inline LPC_GPDMACH_TypeDef *calcPeripheral(uint8_t channel)
{
  return (LPC_GPDMACH_TypeDef *)((uint32_t)LPC_GPDMACH0
      + ((uint32_t)LPC_GPDMACH1 - (uint32_t)LPC_GPDMACH0) * channel);
}
/*----------------------------------------------------------------------------*/
static void setMux(struct GpDma *controller, enum gpDmaLine line)
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
  const struct GpDmaConfig * const config = configPtr;
  struct GpDma *controller = object;

  assert(config && config->channel < CHANNEL_COUNT);

  /* TODO Add channel allocation, when channel is -1 */
  controller->parent.channel = (uint8_t)config->channel;
  /* TODO Replace active flag in GPDMA with something else */
  controller->parent.active = false;
  controller->direction = config->direction;
  controller->reg = calcPeripheral(controller->parent.channel);

  controller->control = C_CONTROL_INT | C_CONTROL_SRC_WIDTH(config->width)
      | C_CONTROL_DEST_WIDTH(config->width);
  /* LPC17xx does not support 2 bytes burst */
  if (config->burst >= DMA_BURST_4)
  {
    controller->control |= C_CONTROL_SRC_BURST(config->burst - 1)
        | C_CONTROL_DEST_BURST(config->burst - 1);
  }
  if (config->source.increment)
    controller->control |= C_CONTROL_SRC_INC;
  if (config->destination.increment)
    controller->control |= C_CONTROL_DEST_INC;

  controller->config = C_CONFIG_TYPE(config->direction)
      | C_CONFIG_IE | C_CONFIG_ITC;
  if (config->source.line != GPDMA_LINE_MEMORY)
    controller->config |= C_CONFIG_SRC_PERIPH(config->source.line);
  if (config->destination.line != GPDMA_LINE_MEMORY)
    controller->config |= C_CONFIG_DEST_PERIPH(config->destination.line);

  /* Reset multiplexer register values */
  controller->muxMask = 0xFF;
  controller->muxValue = 0x00;
  /* Calculate new mask and value */
  /* TODO Rewrite function to return calculated value */
  setMux(controller, config->source.line);
  setMux(controller, config->destination.line);

  if (!instances++)
  {
    sysPowerEnable(PWR_GPDMA);
    LPC_GPDMA->DMACConfig |= DMA_ENABLE;
    irqEnable(DMA_IRQ);
    //TODO Add priority configuration
    //nvicSetPriority(device->irq, GET_PRIORITY(config->priority));
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void gpdmaDeinit(void *object)
{
  struct GpDma *controller = object; /* TODO Remove variable? */

  /* Disable DMA peripheral when no active descriptors exist */
  if (!--instances)
  {
    irqDisable(DMA_IRQ);
    LPC_GPDMA->DMACConfig &= ~DMA_ENABLE;
    sysPowerDisable(PWR_GPDMA);
  }
}
/*----------------------------------------------------------------------------*/
enum result gpdmaStart(void *object, void *dest, const void *src, uint32_t size)
{
  struct GpDma *controller = object;

  if (setDescriptor(controller->parent.channel, object) != E_OK)
    return E_ERROR;

  LPC_SC->DMAREQSEL &= controller->muxMask;
  LPC_SC->DMAREQSEL |= controller->muxValue;

  controller->reg->DMACCSrcAddr = (uint32_t)src;
  controller->reg->DMACCDestAddr = (uint32_t)dest;

  /* Transfer size is 12-bit width value */
  controller->reg->DMACCControl = controller->control | C_CONTROL_SIZE(size);

  controller->reg->DMACCLLI = 0;
  controller->reg->DMACCConfig = controller->config;

  /* Clear interrupt requests of the selected channel */
  LPC_GPDMA->DMACIntTCClear |= 1 << controller->parent.channel;
  LPC_GPDMA->DMACIntErrClr |= 1 << controller->parent.channel;

  controller->parent.active = true;
  controller->reg->DMACCConfig |= C_CONFIG_ENABLE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
enum result gpdmaStartList(void *object, const void *first)
{
  const struct GpDmaListItem *item = first;
  struct GpDma *controller = object;

  if (setDescriptor(controller->parent.channel, object) != E_OK)
    return E_ERROR;

  LPC_SC->DMAREQSEL &= controller->muxMask;
  LPC_SC->DMAREQSEL |= controller->muxValue;

  controller->reg->DMACCSrcAddr = item->source;
  controller->reg->DMACCDestAddr = item->destination;
  controller->reg->DMACCControl = item->control;
  controller->reg->DMACCLLI = item->next;

  controller->reg->DMACCConfig = controller->config;

  /* Clear interrupt requests of the selected channel */
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
  ((struct GpDma *)object)->reg->DMACCConfig &= ~C_CONFIG_ENABLE;
}
/*----------------------------------------------------------------------------*/
void gpdmaHalt(void *object)
{
  /* Ignore future requests */
  ((struct GpDma *)object)->reg->DMACCConfig |= C_CONFIG_HALT;
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
  struct GpDmaListItem *item = current;

  item->source = (uint32_t)src;
  item->destination = (uint32_t)dest;
  item->next = (uint32_t)next;
  item->control = ((struct GpDma *)object)->control | C_CONTROL_SIZE(size);
}
