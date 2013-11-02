/*
 * gpdma.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <irq.h>
#include <spinlock.h>
#include <platform/nxp/gpdma.h>
#include <platform/nxp/gpdma_defs.h>
#include <platform/nxp/platform_defs.h>
#include <platform/nxp/lpc17xx/power.h>
/*----------------------------------------------------------------------------*/
#define CHANNEL_COUNT 8
/*----------------------------------------------------------------------------*/
struct GpDmaListItem
{
  uint32_t source;
  uint32_t destination;
  uint32_t next;
  uint32_t control;
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
static inline void *calcPeripheral(uint8_t);
/*----------------------------------------------------------------------------*/
static uint8_t eventToPeripheral(dma_event_t);
static enum result setDescriptor(uint8_t, struct GpDma *);
static void setEventMux(struct GpDma *, uint8_t);
/*----------------------------------------------------------------------------*/
static enum result gpdmaInit(void *, const void *);
static void gpdmaDeinit(void *);
static bool gpdmaActive(void *);
static void gpdmaCallback(void *, void (*)(void *), void *);
static enum result gpdmaStart(void *, void *, const void *, uint32_t);
static void gpdmaStop(void *);

static void *gpdmaListAllocate(void *, uint32_t);
static void gpdmaListAppend(void *, void *, uint32_t, void *,
    const void *, uint32_t);
static enum result gpdmaListStart(void *, const void *);
/*----------------------------------------------------------------------------*/
static const struct DmaClass gpdmaTable = {
    .size = sizeof(struct GpDma),
    .init = gpdmaInit,
    .deinit = gpdmaDeinit,

    .active = gpdmaActive,
    .callback = gpdmaCallback,
    .start = gpdmaStart,
    .stop = gpdmaStop,

    .listAllocate = gpdmaListAllocate,
    .listAppend = gpdmaListAppend,
    .listStart = gpdmaListStart
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
static inline void *calcPeripheral(uint8_t channel)
{
  return (void *)((uint32_t)LPC_GPDMACH0 + ((uint32_t)LPC_GPDMACH1
      - (uint32_t)LPC_GPDMACH0) * channel);
}
/*----------------------------------------------------------------------------*/
static uint8_t eventToPeripheral(dma_event_t event)
{
  assert(event < GPDMA_EVENT_END);

  if (event >= GPDMA_MAT0_0 && event <= GPDMA_MAT3_1)
    return 8 + (event - GPDMA_MAT0_0);

//  if (event >= GPDMA_UART0_RX && event <= GPDMA_UART3_RX)
//    return 9 + ((event - GPDMA_UART0_RX) << 1);
//
//  if (event >= GPDMA_UART0_TX && event <= GPDMA_UART3_TX)
//    return 8 + ((event - GPDMA_UART0_TX) << 1);

  switch (event)
  {
    case GPDMA_SSP0_RX:
    case GPDMA_SSP1_RX:
      return 1 + ((event - GPDMA_SSP0_RX) << 1);
    case GPDMA_SSP0_TX:
    case GPDMA_SSP1_TX:
      return 0 + ((event - GPDMA_SSP0_TX) << 1);
    case GPDMA_I2S0:
    case GPDMA_I2S1:
      return 5 + (event - GPDMA_I2S0);
    case GPDMA_UART0_RX:
    case GPDMA_UART1_RX:
    case GPDMA_UART2_RX:
    case GPDMA_UART3_RX:
      return 9 + ((event - GPDMA_UART0_RX) << 1);
    case GPDMA_UART0_TX:
    case GPDMA_UART1_TX:
    case GPDMA_UART2_TX:
    case GPDMA_UART3_TX:
      return 8 + ((event - GPDMA_UART0_TX) << 1);
    case GPDMA_ADC:
      return 4;
    case GPDMA_DAC:
      return 7;
  }

  return 0xFF;
}
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
static void setEventMux(struct GpDma *channel, uint8_t peripheral)
{
  if (peripheral >= 8)
  {
    uint8_t position = (peripheral >> 2) - 8;
    channel->muxMask &= ~(1 << position);
    channel->muxValue |= (peripheral & 1) << position;
  }
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
static enum result gpdmaInit(void *object, const void *configPtr)
{
  const struct GpDmaConfig * const config = configPtr;
  struct GpDma *channel = object;

  assert(config && config->channel < CHANNEL_COUNT);

  channel->callback = 0;
  channel->number = (uint8_t)config->channel;
  channel->reg = calcPeripheral(channel->number);

  uint8_t peripheral = eventToPeripheral(config->event);
  channel->control = CONTROL_INT | CONTROL_SRC_WIDTH(config->width)
      | CONTROL_DST_WIDTH(config->width);
  channel->config = CONFIG_TYPE(config->type) | CONFIG_IE | CONFIG_ITC;

  /* Two bytes burst mode is unsupported */
  switch (config->type)
  {
    case GPDMA_TYPE_M2M:
      /* Set 4-byte burst size for memory transfers */
      channel->control |= CONTROL_SRC_BURST(1) | CONTROL_DST_BURST(1);
      break;
    case GPDMA_TYPE_M2P:
      channel->control |= CONTROL_SRC_BURST(1);
      if (config->burst >= DMA_BURST_4)
        channel->control |= CONTROL_DST_BURST(config->burst - 1);
      channel->config |= CONFIG_DST_PERIPH(peripheral);
      break;
    case GPDMA_TYPE_P2M:
      channel->control |= CONTROL_DST_BURST(1);
      if (config->burst >= DMA_BURST_4)
        channel->control |= CONTROL_SRC_BURST(config->burst - 1);
      channel->config |= CONFIG_SRC_PERIPH(peripheral);
      break;
  }

  /* Reset multiplexer mask and value */
  channel->muxMask = 0xFF;
  channel->muxValue = 0x00;
  /* Calculate new mask and value for event multiplexer */
  if (config->type == GPDMA_TYPE_M2P || config->type == GPDMA_TYPE_P2M)
    setEventMux(channel, peripheral);

  if (config->source.increment)
    channel->control |= CONTROL_SRC_INC;
  if (config->destination.increment)
    channel->control |= CONTROL_DST_INC;

  if (!instances++)
  {
    sysPowerEnable(PWR_GPDMA);
    LPC_GPDMA->CONFIG |= DMA_ENABLE;
    irqEnable(DMA_IRQ);
    /* TODO Add priority configuration for GPDMA block */
    /* setPriority(DMA_IRQ, config->priority); */
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void gpdmaDeinit(void *object __attribute__((unused)))
{
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
      && (reg->CONFIG & CONFIG_ENABLE);
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
enum result gpdmaStart(void *object, void *destination, const void *source,
    uint32_t size)
{
  struct GpDma *channel = object;

  if (setDescriptor(channel->number, object) != E_OK)
    return E_ERROR;

  LPC_SC->DMAREQSEL = (LPC_SC->DMAREQSEL & channel->muxMask)
      | channel->muxValue;

  LPC_GPDMACH_Type *reg = channel->reg;

  reg->SRCADDR = (uint32_t)source;
  reg->DESTADDR = (uint32_t)destination;
  reg->CONTROL = channel->control | CONTROL_SIZE(size);
  reg->CONFIG = channel->config;
  reg->LLI = 0;

  /* Clear interrupt requests of the selected channel */
  LPC_GPDMA->INTTCCLEAR |= 1 << channel->number;
  LPC_GPDMA->INTERRCLEAR |= 1 << channel->number;

  reg->CONFIG |= CONFIG_ENABLE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
void gpdmaStop(void *object)
{
  /* Disable channel */
  ((LPC_GPDMACH_Type *)((struct GpDma *)object)->reg)->CONFIG &= ~CONFIG_ENABLE;
}
/*----------------------------------------------------------------------------*/
static void *gpdmaListAllocate(void *object __attribute__((unused)),
    uint32_t size)
{
  /* Allocation should produce memory chunks aligned along 4-byte boundary */
  return malloc(sizeof(struct GpDmaListItem) * size);
}
/*----------------------------------------------------------------------------*/
static void gpdmaListAppend(void *object, void *first, uint32_t index,
    void *destination, const void *source, uint32_t size)
{
  struct GpDmaListItem *item = (struct GpDmaListItem *)first + index;

  if (index)
    (item - 1)->next = (uint32_t)item;

  item->source = (uint32_t)source;
  item->destination = (uint32_t)destination;
  item->next = 0;
  item->control = ((struct GpDma *)object)->control | CONTROL_SIZE(size);
}
/*----------------------------------------------------------------------------*/
static enum result gpdmaListStart(void *object, const void *firstPtr)
{
  const struct GpDmaListItem *first = firstPtr;
  struct GpDma *channel = object;

  if (setDescriptor(channel->number, object) != E_OK)
    return E_ERROR;

  LPC_SC->DMAREQSEL = (LPC_SC->DMAREQSEL & channel->muxMask)
      | channel->muxValue;

  LPC_GPDMACH_Type *reg = channel->reg;

  reg->SRCADDR = first->source;
  reg->DESTADDR = first->destination;
  reg->CONTROL = first->control;
  reg->CONFIG = channel->config;
  reg->LLI = first->next;

  /* Clear interrupt requests of the selected channel */
  LPC_GPDMA->INTTCCLEAR |= 1 << channel->number;
  LPC_GPDMA->INTERRCLEAR |= 1 << channel->number;

  reg->CONFIG |= CONFIG_ENABLE;

  return E_OK;
}
