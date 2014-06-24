/*
 * gpdma_base.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <irq.h>
#include <memory.h>
#include <platform/nxp/gpdma_base.h>
#include <platform/nxp/gpdma_defs.h>
#include <platform/nxp/platform_defs.h>
#include <platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
struct DmaHandler
{
  struct Entity parent;

  /* Channel descriptors currently in use */
  struct GpDmaBase *descriptors[8];
  /* Initialized channels count */
  uint16_t instances;
};
/*----------------------------------------------------------------------------*/
static inline void *calcPeripheral(uint8_t);
static uint8_t eventToPeripheral(enum gpDmaEvent);
static void updateEventMux(struct GpDmaBase *, enum gpDmaEvent);
/*----------------------------------------------------------------------------*/
static inline enum result dmaHandlerAttach();
static inline void dmaHandlerDetach();
static enum result dmaHandlerInit(void *, const void *);
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *, const void *);
static void channelDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass handlerTable = {
    .size = sizeof(struct DmaHandler),
    .init = dmaHandlerInit,
    .deinit = 0
};
/*----------------------------------------------------------------------------*/
static const struct EntityClass channelTable = {
    .size = sizeof(struct GpDmaBase),
    .init = channelInit,
    .deinit = channelDeinit
};
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const DmaHandler = &handlerTable;
const struct EntityClass * const GpDmaBase = &channelTable;
static struct DmaHandler *dmaHandler = 0;
/*----------------------------------------------------------------------------*/
static inline void *calcPeripheral(uint8_t channel)
{
  return (void *)((uint32_t)LPC_GPDMACH0 + ((uint32_t)LPC_GPDMACH1
      - (uint32_t)LPC_GPDMACH0) * channel);
}
/*----------------------------------------------------------------------------*/
static inline enum result dmaHandlerAttach()
{
  if (!dmaHandler)
  {
    if (!(dmaHandler = init(DmaHandler, 0)))
      return E_ERROR;
  }

  if (!dmaHandler->instances++)
  {
    sysPowerEnable(PWR_GPDMA);
    LPC_GPDMA->CONFIG |= DMA_ENABLE;
    irqEnable(DMA_IRQ);
  }

  return E_OK;
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
  struct DmaHandler * const handler = object;

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
static void updateEventMux(struct GpDmaBase *channel, enum gpDmaEvent event)
{
  if (event >= GPDMA_MAT0_0 && event <= GPDMA_MAT3_1)
  {
    const uint32_t position = event - GPDMA_MAT0_0;

    channel->mux.mask &= ~(1 << position);
    channel->mux.value |= 1 << position;
  }
}
/*----------------------------------------------------------------------------*/
const struct GpDmaBase *gpDmaGetDescriptor(uint8_t channel)
{
  assert(channel < GPDMA_CHANNEL_COUNT);

  return dmaHandler->descriptors[channel];
}
/*----------------------------------------------------------------------------*/
enum result gpDmaSetDescriptor(uint8_t channel, struct GpDmaBase *descriptor)
{
  assert(channel < GPDMA_CHANNEL_COUNT);

  return compareExchangePointer((void **)(dmaHandler->descriptors + channel),
      0, descriptor) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
void gpDmaSetupMux(struct GpDmaBase *descriptor)
{
  LPC_SC->DMAREQSEL = (LPC_SC->DMAREQSEL & descriptor->mux.mask)
      | descriptor->mux.value;
}
/*----------------------------------------------------------------------------*/
void DMA_ISR(void)
{
  const uint8_t errorStatus = LPC_GPDMA->INTERRSTAT;
  const uint8_t terminalStatus = LPC_GPDMA->INTTCSTAT;
  const uint8_t intStatus = errorStatus | terminalStatus;

  LPC_GPDMA->INTERRCLEAR = errorStatus;
  LPC_GPDMA->INTTCCLEAR = terminalStatus;

  for (uint8_t index = 0; index < GPDMA_CHANNEL_COUNT; ++index)
  {
    const uint8_t mask = 1 << index;

    if (intStatus & mask)
    {
      struct GpDmaBase * const descriptor = dmaHandler->descriptors[index];
      LPC_GPDMACH_Type * const reg = descriptor->reg;
      enum result res = E_OK;

      if (!(reg->CONFIG & CONFIG_ENABLE))
      {
        /* Clear descriptor when channel is stopped or transfer is completed */
        dmaHandler->descriptors[index] = 0;
      }
      else
        res = E_BUSY;

      if (errorStatus & mask)
        res = E_ERROR;

      if (descriptor->handler)
        descriptor->handler(descriptor, res);
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *object, const void *configPtr)
{
  const struct GpDmaBaseConfig * const config = configPtr;
  struct GpDmaBase * const channel = object;
  enum result res;

  assert(config->channel < GPDMA_CHANNEL_COUNT);

  channel->config = 0;
  channel->control = 0;
  channel->handler = 0;
  channel->number = (uint8_t)config->channel;
  channel->reg = calcPeripheral(channel->number);

  /* Reset multiplexer mask and value */
  channel->mux.mask = 0xFF;
  channel->mux.value = 0;

  if (config->type != GPDMA_TYPE_M2M)
  {
    const uint8_t peripheral = eventToPeripheral(config->event);

    switch (config->type)
    {
      case GPDMA_TYPE_M2P:
        channel->config |= CONFIG_DST_PERIPH(peripheral);
        break;

      case GPDMA_TYPE_P2M:
        channel->config |= CONFIG_SRC_PERIPH(peripheral);
        break;

      default:
        break;
    }

    /* Calculate new mask and value for event multiplexer */
    updateEventMux(channel, config->event);
  }

  /* Register new descriptor in the handler */
  if ((res = dmaHandlerAttach()) != E_OK)
    return res;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object __attribute__((unused)))
{
  dmaHandlerDetach();
}
