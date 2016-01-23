/*
 * gpdma_base.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <irq.h>
#include <memory.h>
#include <spinlock.h>
#include <platform/platform_defs.h>
#include <platform/nxp/gpdma_base.h>
#include <platform/nxp/gpdma_defs.h>
#include <platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
struct DmaHandler
{
  struct Entity base;

  /* Channel descriptors currently in use */
  struct GpDmaBase *descriptors[GPDMA_CHANNEL_COUNT];
  /* Initialized descriptors count */
  uint16_t instances;
};
/*----------------------------------------------------------------------------*/
static inline LPC_GPDMACH_Type *calcPeripheral(uint8_t);
static uint8_t eventToPeripheral(enum gpDmaEvent);
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
static spinlock_t spinlock = SPIN_UNLOCKED;
/*----------------------------------------------------------------------------*/
static inline LPC_GPDMACH_Type *calcPeripheral(uint8_t channel)
{
  return (LPC_GPDMACH_Type *)((uint32_t)LPC_GPDMACH0 + ((uint32_t)LPC_GPDMACH1
      - (uint32_t)LPC_GPDMACH0) * channel);
}
/*----------------------------------------------------------------------------*/
static uint8_t eventToPeripheral(enum gpDmaEvent event)
{
  assert(event < GPDMA_MEMORY);

  return eventTranslationMap[event];
}
/*----------------------------------------------------------------------------*/
static void updateEventMux(struct GpDmaBase *channel, enum gpDmaEvent event)
{
  if (event >= GPDMA_MAT0_0 && event <= GPDMA_MAT3_1)
  {
    const uint32_t position = event - GPDMA_MAT0_0;
    const uint8_t mask = 1 << position;

    channel->mux.mask &= ~mask;
    channel->mux.value |= mask;
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
  assert(descriptor);
  assert(channel < GPDMA_CHANNEL_COUNT);

  /* Descriptor reset is performed in interrupt handler */
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
  const uint32_t errorStatus = LPC_GPDMA->INTERRSTAT;
  const uint32_t terminalStatus = LPC_GPDMA->INTTCSTAT;
  uint32_t intStatus = errorStatus | terminalStatus;

  LPC_GPDMA->INTERRCLEAR = errorStatus;
  LPC_GPDMA->INTTCCLEAR = terminalStatus;

  while (intStatus)
  {
    const uint32_t index = 31 - countLeadingZeros32(intStatus);
    const uint32_t mask = 1 << index;

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

    descriptor->handler(descriptor, res);

    intStatus -= mask;
  }
}
/*----------------------------------------------------------------------------*/
static void dmaHandlerAttach(void)
{
  spinLock(&spinlock);

  if (!dmaHandler)
    dmaHandler = init(DmaHandler, 0);

  if (!dmaHandler->instances++)
  {
    sysPowerEnable(PWR_GPDMA);
    LPC_GPDMA->CONFIG |= DMA_ENABLE;
    irqEnable(GPDMA_IRQ);
  }

  spinUnlock(&spinlock);
}
/*----------------------------------------------------------------------------*/
static void dmaHandlerDetach(void)
{
  spinLock(&spinlock);

  /* Disable peripheral when no active descriptors exist */
  if (!--dmaHandler->instances)
  {
    irqDisable(GPDMA_IRQ);
    LPC_GPDMA->CONFIG &= ~DMA_ENABLE;
    sysPowerDisable(PWR_GPDMA);
  }

  spinUnlock(&spinlock);
}
/*----------------------------------------------------------------------------*/
static enum result dmaHandlerInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct DmaHandler * const handler = object;

  for (uint8_t index = 0; index < GPDMA_CHANNEL_COUNT; ++index)
    handler->descriptors[index] = 0;

  handler->instances = 0;

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

  /* Register new descriptor */
  dmaHandlerAttach();

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object __attribute__((unused)))
{
  dmaHandlerDetach();
}
