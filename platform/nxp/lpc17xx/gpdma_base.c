/*
 * gpdma_base.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <memory.h>
#include <irq.h>
#include <platform/nxp/gpdma_base.h>
#include <platform/nxp/gpdma_defs.h>
#include <platform/nxp/lpc17xx/system.h>
#include <platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
#define GPDMA_CHANNEL_COUNT ARRAY_SIZE(LPC_GPDMA->CHANNELS)
/*----------------------------------------------------------------------------*/
struct DmaHandler
{
  struct Entity base;

  /* Channel descriptors currently in use */
  struct GpDmaBase *descriptors[GPDMA_CHANNEL_COUNT];
  /* Initialized descriptors count */
  unsigned short instances;
};
/*----------------------------------------------------------------------------*/
static unsigned int eventToPeripheral(enum gpDmaEvent);
static void updateEventMux(struct GpDmaBase *, enum gpDmaEvent);
/*----------------------------------------------------------------------------*/
static void dmaHandlerAttach(void);
static void dmaHandlerDetach(void);
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
    .size = 0, /* Abstract class */
    .init = channelInit,
    .deinit = channelDeinit
};
/*----------------------------------------------------------------------------*/
static const uint8_t eventTranslationMap[] = {
    [GPDMA_SSP0_RX]   = 1,
    [GPDMA_SSP1_RX]   = 3,
    [GPDMA_SSP0_TX]   = 0,
    [GPDMA_SSP1_TX]   = 2,
    [GPDMA_I2S0_REQ1] = 5,
    [GPDMA_I2S0_REQ2] = 6,
    [GPDMA_UART0_RX]  = 9,
    [GPDMA_UART1_RX]  = 11,
    [GPDMA_UART2_RX]  = 13,
    [GPDMA_UART3_RX]  = 15,
    [GPDMA_UART0_TX]  = 8,
    [GPDMA_UART1_TX]  = 10,
    [GPDMA_UART2_TX]  = 12,
    [GPDMA_UART3_TX]  = 14,
    [GPDMA_MAT0_0]    = 8,
    [GPDMA_MAT0_1]    = 9,
    [GPDMA_MAT1_0]    = 10,
    [GPDMA_MAT1_1]    = 11,
    [GPDMA_MAT2_0]    = 12,
    [GPDMA_MAT2_1]    = 13,
    [GPDMA_MAT3_0]    = 14,
    [GPDMA_MAT3_1]    = 15,
    [GPDMA_ADC0]      = 4,
    [GPDMA_DAC]       = 7
};
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const DmaHandler = &handlerTable;
const struct EntityClass * const GpDmaBase = &channelTable;
static struct DmaHandler *dmaHandler = 0;
/*----------------------------------------------------------------------------*/
static unsigned int eventToPeripheral(enum gpDmaEvent event)
{
  assert(event < GPDMA_MEMORY);

  return eventTranslationMap[event];
}
/*----------------------------------------------------------------------------*/
static void updateEventMux(struct GpDmaBase *channel, enum gpDmaEvent event)
{
  if (event >= GPDMA_MAT0_0 && event <= GPDMA_MAT3_1)
  {
    const unsigned int position = event - GPDMA_MAT0_0;
    const uint8_t mask = 1 << position;

    channel->mux.mask &= ~mask;
    channel->mux.value |= mask;
  }
}
/*----------------------------------------------------------------------------*/
void gpDmaClearDescriptor(uint8_t channel)
{
  assert(channel < GPDMA_CHANNEL_COUNT);

  dmaHandler->descriptors[channel] = 0;
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
  assert(descriptor);
  assert(channel < GPDMA_CHANNEL_COUNT);

  return compareExchangePointer((void **)(dmaHandler->descriptors + channel),
      0, descriptor) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
void gpDmaSetMux(struct GpDmaBase *descriptor)
{
  LPC_SC->DMAREQSEL = (LPC_SC->DMAREQSEL & descriptor->mux.mask)
      | descriptor->mux.value;
}
/*----------------------------------------------------------------------------*/
void GPDMA_ISR(void)
{
  uint32_t errorStatus = LPC_GPDMA->INTERRSTAT;
  uint32_t terminalStatus = LPC_GPDMA->INTTCSTAT;

  LPC_GPDMA->INTERRCLEAR = errorStatus;
  LPC_GPDMA->INTTCCLEAR = terminalStatus;

  errorStatus = reverseBits32(errorStatus);
  terminalStatus = reverseBits32(terminalStatus);

  uint32_t intStatus = errorStatus | terminalStatus;

  do
  {
    const unsigned int index = countLeadingZeros32(intStatus);
    const uint32_t mask = BIT(31) >> index;

    struct GpDmaBase * const descriptor = dmaHandler->descriptors[index];
    LPC_GPDMA_CHANNEL_Type * const reg = descriptor->reg;
    enum result res;

    if (errorStatus & mask)
      res = E_ERROR;
    else if (reg->CONFIG & CONFIG_ENABLE)
      res = E_BUSY;
    else
      res = E_OK;

    intStatus -= mask;

    descriptor->handler(descriptor, res);
  }
  while (intStatus);
}
/*----------------------------------------------------------------------------*/
static void dmaHandlerAttach(void)
{
  const irqState state = irqSave();

  if (!dmaHandler)
    dmaHandler = init(DmaHandler, 0);

  if (!dmaHandler->instances++)
  {
    sysPowerEnable(PWR_GPDMA);
    LPC_GPDMA->CONFIG |= DMA_ENABLE;
    irqEnable(GPDMA_IRQ);
  }

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static void dmaHandlerDetach(void)
{
  const irqState state = irqSave();

  /* Disable peripheral when no active descriptors exist */
  if (!--dmaHandler->instances)
  {
    irqDisable(GPDMA_IRQ);
    LPC_GPDMA->CONFIG &= ~DMA_ENABLE;
    sysPowerDisable(PWR_GPDMA);
  }

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static enum result dmaHandlerInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct DmaHandler * const handler = object;

  for (unsigned int index = 0; index < GPDMA_CHANNEL_COUNT; ++index)
    handler->descriptors[index] = 0;

  handler->instances = 0;

#ifndef CONFIG_PLATFORM_NXP_GPDMA_SYNC
  /* Disable synchronization logic to improve response time */
  LPC_GPDMA->SYNC = SYNC_MASK;
#endif

  irqSetPriority(GPDMA_IRQ, CONFIG_PLATFORM_NXP_GPDMA_PRIORITY);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *object, const void *configBase)
{
  const struct GpDmaBaseConfig * const config = configBase;
  struct GpDmaBase * const channel = object;

  assert(config->channel < GPDMA_CHANNEL_COUNT);

  channel->config = 0;
  channel->control = 0;
  channel->handler = 0;
  channel->number = config->channel;
  channel->reg = &LPC_GPDMA->CHANNELS[channel->number];

  /* Reset multiplexer mask and value */
  channel->mux.mask = 0xFF;
  channel->mux.value = 0;

  if (config->type != GPDMA_TYPE_M2M)
  {
    const unsigned int peripheral = eventToPeripheral(config->event);

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

  /* Register new descriptor */
  dmaHandlerAttach();

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object __attribute__((unused)))
{
  dmaHandlerDetach();
}
