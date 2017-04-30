/*
 * gpdma_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <string.h>
#include <xcore/memory.h>
#include <halm/irq.h>
#include <halm/platform/nxp/gpdma_base.h>
#include <halm/platform/nxp/gpdma_defs.h>
#include <halm/platform/nxp/lpc43xx/system.h>
#include <halm/platform/platform_defs.h>
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
  /* Peripheral connection statuses */
  uint8_t connections[16];
};
/*----------------------------------------------------------------------------*/
static unsigned int dmaHandlerAllocate(struct GpDmaBase *, enum GpDmaEvent);
static void dmaHandlerAttach(void);
static void dmaHandlerDetach(void);
static void dmaHandlerFree(struct GpDmaBase *);
static void dmaHandlerInstantiate(void);
static enum Result dmaHandlerInit(void *, const void *);
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *, const void *);
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
static const enum GpDmaEvent eventMap[16][4] = {
    {GPDMA_SPIFI,   GPDMA_SCT_OUT2,   GPDMA_SGPIO14,    GPDMA_MAT3_1},
    {GPDMA_MAT0_0,  GPDMA_UART0_TX,   GPDMA_EVENT_END,  GPDMA_EVENT_END},
    {GPDMA_MAT0_1,  GPDMA_UART0_RX,   GPDMA_EVENT_END,  GPDMA_EVENT_END},
    {GPDMA_MAT1_0,  GPDMA_UART1_TX,   GPDMA_I2S1_REQ1,  GPDMA_SSP1_TX},
    {GPDMA_MAT1_1,  GPDMA_UART1_RX,   GPDMA_I2S1_REQ2,  GPDMA_SSP1_RX},
    {GPDMA_MAT2_0,  GPDMA_UART2_TX,   GPDMA_SSP1_TX,    GPDMA_SGPIO15},
    {GPDMA_MAT2_1,  GPDMA_UART2_RX,   GPDMA_SSP1_RX,    GPDMA_SGPIO14},
    {GPDMA_MAT3_0,  GPDMA_UART3_TX,   GPDMA_SCT_REQ0,   GPDMA_ADCHS_WRITE},
    {GPDMA_MAT3_1,  GPDMA_UART3_RX,   GPDMA_SCT_REQ1,   GPDMA_ADCHS_READ},
    {GPDMA_SSP0_RX, GPDMA_I2S0_REQ1,  GPDMA_SCT_REQ1,   GPDMA_EVENT_END},
    {GPDMA_SSP0_TX, GPDMA_I2S0_REQ2,  GPDMA_SCT_REQ0,   GPDMA_EVENT_END},
    {GPDMA_SSP1_RX, GPDMA_SGPIO14,    GPDMA_UART0_TX,   GPDMA_EVENT_END},
    {GPDMA_SSP1_TX, GPDMA_SGPIO15,    GPDMA_UART0_RX,   GPDMA_EVENT_END},
    {GPDMA_ADC0,    GPDMA_EVENT_END,  GPDMA_SSP1_RX,    GPDMA_UART3_RX},
    {GPDMA_ADC1,    GPDMA_EVENT_END,  GPDMA_SSP1_TX,    GPDMA_UART3_TX},
    {GPDMA_DAC,     GPDMA_SCT_OUT3,   GPDMA_SGPIO15,    GPDMA_MAT3_0}
};
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const DmaHandler = &handlerTable;
const struct EntityClass * const GpDmaBase = &channelTable;
static struct DmaHandler *dmaHandler = 0;
/*----------------------------------------------------------------------------*/
uint32_t gpDmaBaseCalcControl(const struct GpDmaBase *channel,
    const struct GpDmaSettings *settings)
{
  assert(settings->source.burst != DMA_BURST_2);
  assert(settings->source.burst <= DMA_BURST_256);
  assert(settings->source.width <= DMA_WIDTH_WORD);
  assert(settings->destination.burst != DMA_BURST_2);
  assert(settings->destination.burst <= DMA_BURST_256);
  assert(settings->destination.width <= DMA_WIDTH_WORD);

  const uint32_t type = CONFIG_TYPE_VALUE(channel->config);
  uint32_t control = 0;

  switch (type)
  {
    case GPDMA_TYPE_M2P:
      control |= CONTROL_DST_MASTER(1);
      break;

    case GPDMA_TYPE_P2M:
      control |= CONTROL_SRC_MASTER(1);
      break;

    default:
      break;
  }

  control |= CONTROL_SRC_WIDTH(settings->source.width);
  control |= CONTROL_DST_WIDTH(settings->destination.width);

  /* Set four-byte burst size by default */
  uint8_t dstBurst = settings->destination.burst;
  uint8_t srcBurst = settings->source.burst;

  /* Two-byte burst requests are unsupported */
  if (srcBurst >= DMA_BURST_4)
    --srcBurst;
  if (dstBurst >= DMA_BURST_4)
    --dstBurst;

  control |= CONTROL_SRC_BURST(srcBurst) | CONTROL_DST_BURST(dstBurst);

  if (settings->source.increment)
    control |= CONTROL_SRC_INC;
  if (settings->destination.increment)
    control |= CONTROL_DST_INC;

  return control;
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
enum Result gpDmaSetDescriptor(uint8_t channel, struct GpDmaBase *descriptor)
{
  assert(descriptor);
  assert(channel < GPDMA_CHANNEL_COUNT);

  return compareExchangePointer((void **)(dmaHandler->descriptors + channel),
      0, descriptor) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
void gpDmaSetMux(struct GpDmaBase *descriptor)
{
  LPC_CREG->DMAMUX =
      (LPC_CREG->DMAMUX & descriptor->mux.mask) | descriptor->mux.value;
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

  struct GpDmaBase ** const descriptorArray = dmaHandler->descriptors;
  uint32_t intStatus = errorStatus | terminalStatus;

  do
  {
    const unsigned int index = countLeadingZeros32(intStatus);
    struct GpDmaBase * const descriptor = descriptorArray[index];
    const uint32_t mask = (1UL << 31) >> index;

    intStatus -= mask;

    enum Result res = E_OK;

    if (errorStatus & mask)
    {
      res = E_ERROR;
    }
    else
    {
      const LPC_GPDMA_CHANNEL_Type * const reg = descriptor->reg;

      if (reg->CONFIG & CONFIG_ENABLE)
        res = E_BUSY;
    }

    descriptor->handler(descriptor, res);
  }
  while (intStatus);
}
/*----------------------------------------------------------------------------*/
static unsigned int dmaHandlerAllocate(struct GpDmaBase *channel,
    enum GpDmaEvent event)
{
  size_t entryIndex = 0, entryOffset = 0;
  unsigned int minValue = 0;
  bool found = false;

  assert(event < GPDMA_MEMORY);

  const IrqState state = irqSave();
  dmaHandlerInstantiate();

  for (size_t index = 0; index < 16; ++index)
  {
    size_t entry;
    bool allowed = false;

    for (entry = 0; entry < 4; ++entry)
    {
      if (eventMap[index][entry] == event)
      {
        allowed = true;
        break;
      }
    }

    if (allowed && (!found || minValue < dmaHandler->connections[index]))
    {
      found = true;
      entryIndex = index;
      entryOffset = entry;
      minValue = dmaHandler->connections[index];
    }
  }

  assert(found);

  ++dmaHandler->connections[entryIndex];
  channel->mux.mask &= ~(0x03 << (entryIndex << 1));
  channel->mux.value = entryOffset << (entryIndex << 1);

  irqRestore(state);
  return entryIndex;
}
/*----------------------------------------------------------------------------*/
static void dmaHandlerAttach(void)
{
  const IrqState state = irqSave();
  dmaHandlerInstantiate();

  if (!dmaHandler->instances++)
  {
    sysClockEnable(CLK_M4_GPDMA);
    sysResetEnable(RST_GPDMA);
    LPC_GPDMA->CONFIG |= DMA_ENABLE;
    irqEnable(GPDMA_IRQ);
  }

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static void dmaHandlerDetach(void)
{
  const IrqState state = irqSave();

  /* Disable peripheral when no active descriptors exist */
  if (!--dmaHandler->instances)
  {
    irqDisable(GPDMA_IRQ);
    LPC_GPDMA->CONFIG &= ~DMA_ENABLE;
    sysClockDisable(CLK_M4_GPDMA);
  }

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static void dmaHandlerFree(struct GpDmaBase *channel)
{
  uint32_t mask = ~channel->mux.mask;
  unsigned int index = 0;

  /* DMA Handler is already initialized */
  while (mask)
  {
    if (mask & 0x03)
      --dmaHandler->connections[index];

    ++index;
    mask >>= 2;
  }
}
/*----------------------------------------------------------------------------*/
static void dmaHandlerInstantiate(void)
{
  const IrqState state = irqSave();

  if (!dmaHandler)
    dmaHandler = init(DmaHandler, 0);

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static enum Result dmaHandlerInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct DmaHandler * const handler = object;

  for (unsigned int index = 0; index < GPDMA_CHANNEL_COUNT; ++index)
    handler->descriptors[index] = 0;

  memset(handler->connections, 0, sizeof(handler->connections));
  handler->instances = 0;

#ifndef CONFIG_PLATFORM_NXP_GPDMA_SYNC
  /* Disable synchronization logic to improve response time */
  LPC_GPDMA->SYNC = SYNC_MASK;
#endif

  irqSetPriority(GPDMA_IRQ, CONFIG_PLATFORM_NXP_GPDMA_PRIORITY);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct GpDmaBaseConfig * const config = configBase;
  struct GpDmaBase * const channel = object;

  assert(config->channel < GPDMA_CHANNEL_COUNT);

  channel->config = CONFIG_TYPE(config->type) | CONFIG_IE | CONFIG_ITC;
  channel->handler = 0;
  channel->number = config->channel;
  channel->reg = &LPC_GPDMA->CHANNELS[channel->number];

  /* Reset multiplexer mask and value */
  channel->mux.mask = 0xFFFFFFFF;
  channel->mux.value = 0;

  if (config->type != GPDMA_TYPE_M2M)
  {
    const unsigned int peripheral = dmaHandlerAllocate(channel, config->event);

    /* Only AHB master 1 can access a peripheral */
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
  }

  /* Register new descriptor */
  dmaHandlerAttach();

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  dmaHandlerFree(object);
  dmaHandlerDetach();
}
