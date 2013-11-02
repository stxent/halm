/*
 * gpdma.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <irq.h>
#include <spinlock.h>
#include <platform/nxp/gpdma_defs.h>
#include <platform/nxp/lpc17xx/gpdma.h>
#include <platform/nxp/lpc17xx/power.h>
/*----------------------------------------------------------------------------*/
#define CHANNEL_COUNT 8
/*----------------------------------------------------------------------------*/
static inline void *calcPeripheral(uint8_t);
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t, struct GpDma *);
static void setMux(struct GpDma *, enum gpDmaLine);
/*----------------------------------------------------------------------------*/
static enum result gpdmaInit(void *, const void *);
static void gpdmaDeinit(void *);
static bool gpdmaActive(void *);
static void gpdmaCallback(void *, void (*)(void *), void *);
static void gpdmaLink(void *, void *, void *, void *, const void *, uint32_t);
static enum result gpdmaStart(void *, void *, const void *, uint32_t);
static void gpdmaStop(void *);
/*----------------------------------------------------------------------------*/
static const struct DmaClass gpdmaTable = {
    .size = sizeof(struct GpDma),
    .init = gpdmaInit,
    .deinit = gpdmaDeinit,

    .active = gpdmaActive,
    .callback = gpdmaCallback,
    .link = gpdmaLink,
    .start = gpdmaStart,
    .stop = gpdmaStop
};
/*----------------------------------------------------------------------------*/
const struct DmaClass *GpDma = &gpdmaTable;
/*----------------------------------------------------------------------------*/
static struct GpDma * volatile descriptors[8] = {0};
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
void DMA_ISR(void)
{
  const uint8_t errorStat = LPC_GPDMA->INTERRSTAT;
  const uint8_t terminalStat = LPC_GPDMA->INTTCSTAT;
  uint8_t counter = 0, mask = 0x01;

  for (; counter < CHANNEL_COUNT; ++counter, mask <<= 1)
  {
    if (descriptors[counter] && (terminalStat | errorStat) & mask)
    {
      struct GpDma * volatile channel = descriptors[counter];

      spinLock(&lock);
      descriptors[counter] = 0; /* Clear channel descriptor */
      spinUnlock(&lock);

      /* TODO Invoke callback on errors */
      if ((terminalStat & mask) && channel->callback)
        channel->callback(channel->callbackArgument);
    }
  }
  LPC_GPDMA->INTERRCLEAR = errorStat;
  LPC_GPDMA->INTTCCLEAR = terminalStat;
}
/*----------------------------------------------------------------------------*/
static inline void *calcPeripheral(uint8_t channel)
{
  return (void *)((uint32_t)LPC_GPDMACH0 + ((uint32_t)LPC_GPDMACH1
      - (uint32_t)LPC_GPDMACH0) * channel);
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
  struct GpDma *channel = object;

  assert(config && config->channel < CHANNEL_COUNT);

  channel->callback = 0;
  channel->number = (uint8_t)config->channel;
  channel->reg = calcPeripheral(channel->number);

  channel->control = C_CONTROL_INT | C_CONTROL_SRC_WIDTH(config->width)
      | C_CONTROL_DST_WIDTH(config->width);
  /* LPC17xx does not support 2 bytes burst */
  if (config->burst >= DMA_BURST_4)
  {
    channel->control |= C_CONTROL_SRC_BURST(config->burst - 1)
        | C_CONTROL_DST_BURST(config->burst - 1);
  }
  if (config->source.increment)
    channel->control |= C_CONTROL_SRC_INC;
  if (config->destination.increment)
    channel->control |= C_CONTROL_DST_INC;

  channel->config = C_CONFIG_TYPE(config->direction)
      | C_CONFIG_IE | C_CONFIG_ITC;
  if (config->source.line != GPDMA_LINE_MEMORY)
    channel->config |= C_CONFIG_SRC_PERIPH(config->source.line);
  if (config->destination.line != GPDMA_LINE_MEMORY)
    channel->config |= C_CONFIG_DST_PERIPH(config->destination.line);

  /* Reset multiplexer register values */
  channel->muxMask = 0xFF;
  channel->muxValue = 0x00;
  /* Calculate new mask and value */
  /* TODO Rewrite function to return calculated value */
  setMux(channel, config->source.line);
  setMux(channel, config->destination.line);

  if (!instances++)
  {
    sysPowerEnable(PWR_GPDMA);
    LPC_GPDMA->CONFIG |= DMA_ENABLE;
    irqEnable(DMA_IRQ);
    //TODO Add priority configuration
    //setPriority(DMA_IRQ, config->priority);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void gpdmaDeinit(void *object __attribute__((unused)))
{
  /* TODO Stop all transactions? */

  /* Disable DMA peripheral when no active descriptors exist */
  if (!--instances)
  {
    irqDisable(DMA_IRQ);
    LPC_GPDMA->CONFIG &= ~DMA_ENABLE;
    sysPowerDisable(PWR_GPDMA);
  }
}
/*----------------------------------------------------------------------------*/
static bool gpdmaActive(void *object)
{
  struct GpDma *channel = object;
  LPC_GPDMACH_Type *reg = channel->reg;

  return descriptors[channel->number] == channel
      && (reg->CONFIG & C_CONFIG_ENABLE);
}
/*----------------------------------------------------------------------------*/
static void gpdmaCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpDma *channel = object;

  channel->callback = callback;
  channel->callbackArgument = argument;
}
/*----------------------------------------------------------------------------*/
static void gpdmaLink(void *object, void *current, void *next,
    void *dest, const void *src, uint32_t size)
{
  struct GpDmaListItem *item = current;

  item->source = (uint32_t)src;
  item->destination = (uint32_t)dest;
  item->next = (uint32_t)next;
  item->control = ((struct GpDma *)object)->control | C_CONTROL_SIZE(size);
}
/*----------------------------------------------------------------------------*/
enum result gpdmaStart(void *object, void *dest, const void *src, uint32_t size)
{
  struct GpDma *channel = object;

  if (setDescriptor(channel->number, object) != E_OK)
    return E_ERROR;

  LPC_SC->DMAREQSEL = (LPC_SC->DMAREQSEL & channel->muxMask)
      | channel->muxValue;

  LPC_GPDMACH_Type *reg = channel->reg;

  reg->SRCADDR = (uint32_t)src;
  reg->DESTADDR = (uint32_t)dest;
  reg->CONTROL = channel->control | C_CONTROL_SIZE(size);
  reg->LLI = 0; //FIXME
  reg->CONFIG = channel->config;

  /* Clear interrupt requests of the selected channel */
  LPC_GPDMA->INTTCCLEAR |= 1 << channel->number;
  LPC_GPDMA->INTERRCLEAR |= 1 << channel->number;

  reg->CONFIG |= C_CONFIG_ENABLE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
void gpdmaStop(void *object)
{
//  /* Ignore future requests */
//  ((LPC_GPDMACH_Type *)((struct GpDma *)object)->reg)->CONFIG |= C_CONFIG_HALT;
  /* Disable channel */
  ((LPC_GPDMACH_Type *)((struct GpDma *)object)->reg)->CONFIG &=
      ~C_CONFIG_ENABLE;
}
