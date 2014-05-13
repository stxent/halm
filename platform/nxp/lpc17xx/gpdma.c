/*
 * gpdma.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <irq.h>
#include <memory.h>
#include <platform/nxp/gpdma.h>
#include <platform/nxp/gpdma_defs.h>
#include <platform/nxp/gpdma_list.h>
#include <platform/nxp/platform_defs.h>
#include <platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
struct DmaHandler
{
  struct Entity parent;

  /* Channel descriptors currently in use */
  struct GpDma *descriptors[8];
  /* Initialized channels count */
  uint8_t instances;
};
/*----------------------------------------------------------------------------*/
struct DescriptorListItem
{
  uint32_t source;
  uint32_t destination;
  uint32_t next;
  uint32_t control;
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
struct DescriptorList
{
  /* Pointer to a channel object */
  void *owner;
  /* List size */
  uint16_t size;
  /* Enable circular buffer */
  bool circular;
};
/*----------------------------------------------------------------------------*/
static inline void *calcPeripheral(uint8_t);
static uint8_t eventToPeripheral(enum gpDmaEvent);
static enum result setDescriptor(uint8_t, struct GpDma *);
static void setEventMux(struct GpDma *, enum gpDmaEvent);
/*----------------------------------------------------------------------------*/
static inline void dmaHandlerAttach();
static inline void dmaHandlerDetach();
static enum result dmaHandlerInit(void *, const void *);
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *, const void *);
static void channelDeinit(void *);
static bool channelActive(void *);
static void channelCallback(void *, void (*)(void *), void *);
static enum result channelStart(void *, void *, const void *, uint32_t);
static void channelStop(void *);

static void *channelAllocate(void *, uint32_t, bool);
static enum result channelExecute(void *, const void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass handlerTable = {
    .size = sizeof(struct DmaHandler),
    .init = dmaHandlerInit,
    .deinit = 0
};
/*----------------------------------------------------------------------------*/
static const struct DmaClass channelTable = {
    .size = sizeof(struct GpDma),
    .init = channelInit,
    .deinit = channelDeinit,

    .active = channelActive,
    .callback = channelCallback,
    .start = channelStart,
    .stop = channelStop,

    .allocate = channelAllocate,
    .execute = channelExecute
};
/*----------------------------------------------------------------------------*/
static const struct EntityClass *DmaHandler = &handlerTable;
const struct DmaClass *GpDma = &channelTable;
static struct DmaHandler *dmaHandler = 0;
/*----------------------------------------------------------------------------*/
static inline void *calcPeripheral(uint8_t channel)
{
  return (void *)((uint32_t)LPC_GPDMACH0 + ((uint32_t)LPC_GPDMACH1
      - (uint32_t)LPC_GPDMACH0) * channel);
}
/*----------------------------------------------------------------------------*/
static inline void dmaHandlerAttach()
{
  if (!dmaHandler)
    dmaHandler = init(DmaHandler, 0);

  if (!dmaHandler->instances++)
  {
    sysPowerEnable(PWR_GPDMA);
    LPC_GPDMA->CONFIG |= DMA_ENABLE;
    irqEnable(DMA_IRQ);
  }
}
/*----------------------------------------------------------------------------*/
static inline void dmaHandlerDetach()
{
  /* Disable peripheral when no active descriptors exist */
  if (!--dmaHandler->instances)
  {
    irqDisable(DMA_IRQ);
    LPC_GPDMA->CONFIG &= ~DMA_ENABLE;
    sysPowerDisable(PWR_GPDMA);
  }
}
/*----------------------------------------------------------------------------*/
static enum result dmaHandlerInit(void *object,
    const void *configPtr __attribute__((unused)))
{
  struct DmaHandler *handler = object;

  /* TODO Add priority configuration for GPDMA interrupt */
  for (uint8_t index = 0; index < GPDMA_CHANNEL_COUNT; ++index)
    handler->descriptors[index] = 0;

  handler->instances = 0;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint8_t eventToPeripheral(enum gpDmaEvent event)
{
  assert(event < GPDMA_MEMORY);

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
    default:
      return 8 + (event - GPDMA_MAT0_0); /* One of the timer match events */
  }
}
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel, struct GpDma *controller)
{
  return compareExchangePointer((void **)(dmaHandler->descriptors + channel),
      0, controller) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
static void setEventMux(struct GpDma *channel, enum gpDmaEvent event)
{
  if (event >= GPDMA_MAT0_0 && event <= GPDMA_MAT3_1)
  {
    uint32_t position = event - GPDMA_MAT0_0;

    channel->muxMask &= ~(1 << position);
    channel->muxValue |= 1 << position;
  }
}
/*----------------------------------------------------------------------------*/
void DMA_ISR(void)
{
  const uint8_t errorStat = LPC_GPDMA->INTERRSTAT;
  const uint8_t terminalStat = LPC_GPDMA->INTTCSTAT;
  uint8_t mask = 0x01;

  LPC_GPDMA->INTERRCLEAR = errorStat;
  LPC_GPDMA->INTTCCLEAR = terminalStat;

  for (uint8_t index = 0; index < GPDMA_CHANNEL_COUNT; ++index)
  {
    struct GpDma *descriptor = dmaHandler->descriptors[index];

    if (descriptor && (terminalStat | errorStat) & mask)
    {
      if (!(((LPC_GPDMACH_Type *)descriptor->reg)->CONFIG & CONFIG_ENABLE))
      {
        /* Clear descriptor when channel is disabled or transfer is completed */
        dmaHandler->descriptors[index] = 0;
      }
      /* TODO Add DMA error handling in GPDMA interrupt */
      if (descriptor->callback)
        descriptor->callback(descriptor->callbackArgument);
    }

    mask <<= 1;
  }
}
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *object, const void *configPtr)
{
  const struct GpDmaConfig * const config = configPtr;
  struct GpDma *channel = object;

  assert(config->channel < GPDMA_CHANNEL_COUNT);

  channel->callback = 0;
  channel->number = (uint8_t)config->channel;
  channel->reg = calcPeripheral(channel->number);

  channel->control = CONTROL_INT | CONTROL_SRC_WIDTH(config->width)
      | CONTROL_DST_WIDTH(config->width);
  channel->config = CONFIG_TYPE(config->type) | CONFIG_IE | CONFIG_ITC;

  /* Set four-byte burst size by default */
  uint8_t dstBurst = DMA_BURST_4, srcBurst = DMA_BURST_4;

  if (config->type != GPDMA_TYPE_M2M)
  {
    const uint8_t peripheral = eventToPeripheral(config->event);

    switch (config->type)
    {
      case GPDMA_TYPE_M2P:
        dstBurst = config->burst;
        channel->config |= CONFIG_DST_PERIPH(peripheral);
        break;

      case GPDMA_TYPE_P2M:
        srcBurst = config->burst;
        channel->config |= CONFIG_SRC_PERIPH(peripheral);
        break;

      default:
        break;
    }
  }
  /* Two-byte burst requests are unsupported */
  if (srcBurst >= DMA_BURST_4)
    --srcBurst;
  if (dstBurst >= DMA_BURST_4)
    --dstBurst;
  channel->control |= CONTROL_SRC_BURST(srcBurst) | CONTROL_DST_BURST(dstBurst);

  /* Reset multiplexer mask and value */
  channel->muxMask = 0xFFFFFFFFUL;
  channel->muxValue = 0;
  /* Calculate new mask and value for event multiplexer */
  if (config->type == GPDMA_TYPE_M2P || config->type == GPDMA_TYPE_P2M)
    setEventMux(channel, config->event);

  if (config->source.increment)
    channel->control |= CONTROL_SRC_INC;
  if (config->destination.increment)
    channel->control |= CONTROL_DST_INC;

  /* Register new descriptor in the handler */
  dmaHandlerAttach();

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object __attribute__((unused)))
{
  dmaHandlerDetach();
}
/*----------------------------------------------------------------------------*/
static bool channelActive(void *object)
{
  struct GpDma *channel = object;
  LPC_GPDMACH_Type *reg = channel->reg;

  return dmaHandler->descriptors[channel->number] == channel
      && (reg->CONFIG & CONFIG_ENABLE);
}
/*----------------------------------------------------------------------------*/
static void channelCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpDma *channel = object;

  channel->callback = callback;
  channel->callbackArgument = argument;
}
/*----------------------------------------------------------------------------*/
enum result channelStart(void *object, void *destination, const void *source,
    uint32_t size)
{
  struct GpDma *channel = object;

  if (size > GPDMA_MAX_TRANSFER)
    return E_VALUE;

  if (setDescriptor(channel->number, object) != E_OK)
    return E_BUSY;

  LPC_SC->DMAREQSEL = (LPC_SC->DMAREQSEL & channel->muxMask)
      | channel->muxValue;

  const uint32_t request = 1 << channel->number;
  LPC_GPDMACH_Type *reg = channel->reg;

  reg->SRCADDR = (uint32_t)source;
  reg->DESTADDR = (uint32_t)destination;
  reg->CONTROL = channel->control | CONTROL_SIZE(size);
  reg->CONFIG = channel->config;
  reg->LLI = 0;

  /* Clear interrupt requests for current channel */
  LPC_GPDMA->INTTCCLEAR |= request;
  LPC_GPDMA->INTERRCLEAR |= request;

  /* Start the transfer */
  reg->CONFIG |= CONFIG_ENABLE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
void channelStop(void *object)
{
  ((LPC_GPDMACH_Type *)((struct GpDma *)object)->reg)->CONFIG &= ~CONFIG_ENABLE;
}
/*----------------------------------------------------------------------------*/
static void *channelAllocate(void *object, uint32_t size, bool circular)
{
  const struct GpDmaListConfig config = {
      .parent = object,
      .size = size,
      .circular = circular
  };

  return init(GpDmaList, &config);
}
/*----------------------------------------------------------------------------*/
static enum result channelExecute(void *object, const void *list)
{
  const struct GpDmaList *container = list;
  struct GpDma *channel = object;

  if (!container->size)
    return E_VALUE;

  if (setDescriptor(channel->number, object) != E_OK)
    return E_BUSY;

  LPC_SC->DMAREQSEL = (LPC_SC->DMAREQSEL & channel->muxMask)
      | channel->muxValue;

  const struct GpDmaListItem *first = container->first;
  const uint32_t request = 1 << channel->number;
  LPC_GPDMACH_Type *reg = channel->reg;

  reg->SRCADDR = first->source;
  reg->DESTADDR = first->destination;
  reg->CONTROL = first->control;
  reg->LLI = first->next;
  reg->CONFIG = channel->config;

  LPC_GPDMA->INTTCCLEAR |= request;
  LPC_GPDMA->INTERRCLEAR |= request;

  /* Start the transfer */
  reg->CONFIG |= CONFIG_ENABLE;

  return E_OK;
}
