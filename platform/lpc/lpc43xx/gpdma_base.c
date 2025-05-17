/*
 * gpdma_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/irq.h>
#include <halm/platform/lpc/gpdma_base.h>
#include <halm/platform/lpc/gpdma_defs.h>
#include <halm/platform/lpc/system.h>
#include <halm/platform/platform_defs.h>
#include <xcore/accel.h>
#include <xcore/atomic.h>
#include <assert.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#define CHANNEL_COUNT ARRAY_SIZE(LPC_GPDMA->CHANNELS)
#define EVENT_COUNT   16
#define EVENT_SOURCES 4
/*----------------------------------------------------------------------------*/
struct DmaController
{
  /* Channel descriptors currently in use */
  struct GpDmaBase *instances[CHANNEL_COUNT];
  /* Peripheral connection statuses */
  uint8_t connections[EVENT_COUNT];
};
/*----------------------------------------------------------------------------*/
static unsigned int dmaControllerAllocate(struct GpDmaBase *, enum GpDmaEvent);
static void dmaControllerFree(struct GpDmaBase *);
static void dmaControllerInit(void);
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *, const void *);
static void channelDeinit(void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const GpDmaBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = channelInit,
    .deinit = channelDeinit
};
/*----------------------------------------------------------------------------*/
static const enum GpDmaEvent dmaEventMap[EVENT_COUNT][EVENT_SOURCES] = {
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
static struct DmaController controller = {
    .instances = {NULL},
    .connections = {0}
};
/*----------------------------------------------------------------------------*/
uint32_t gpDmaBaseCalcControl(const struct GpDmaBase *channel,
    const struct GpDmaSettings *settings)
{
  assert(settings->source.burst <= DMA_BURST_256);
  assert(settings->source.width <= DMA_WIDTH_WORD);
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

  control |= CONTROL_SRC_BURST(settings->source.burst);
  control |= CONTROL_SRC_WIDTH(settings->source.width);
  control |= CONTROL_DST_BURST(settings->destination.burst);
  control |= CONTROL_DST_WIDTH(settings->destination.width);

  if (settings->source.increment)
    control |= CONTROL_SRC_INC;
  if (settings->destination.increment)
    control |= CONTROL_DST_INC;

  return control;
}
/*----------------------------------------------------------------------------*/
uint32_t gpDmaBaseCalcMasterAffinity(const struct GpDmaBase *channel,
    enum GpDmaMaster dstMaster, enum GpDmaMaster srcMaster)
{
  const uint32_t type = CONFIG_TYPE_VALUE(channel->config);
  uint32_t control = 0;

  if (srcMaster == GPDMA_MASTER_DEFAULT)
  {
    if (type == GPDMA_TYPE_P2M)
      control |= CONTROL_SRC_MASTER(1);
  }
  else if (srcMaster == GPDMA_MASTER_1)
  {
    control |= CONTROL_SRC_MASTER(1);
  }

  if (dstMaster == GPDMA_MASTER_DEFAULT)
  {
    if (type == GPDMA_TYPE_M2P)
      control |= CONTROL_DST_MASTER(1);
  }
  else if (dstMaster == GPDMA_MASTER_1)
  {
    control |= CONTROL_DST_MASTER(1);
  }

  return control;
}
/*----------------------------------------------------------------------------*/
void gpDmaResetInstance(uint8_t channel)
{
  controller.instances[channel] = NULL;
}
/*----------------------------------------------------------------------------*/
bool gpDmaSetInstance(uint8_t channel, struct GpDmaBase *object)
{
  assert(channel < CHANNEL_COUNT);

  void *expected = NULL;

  return compareExchangePointer(&controller.instances[channel],
      &expected, object);
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
  const uint32_t terminalStatus = LPC_GPDMA->INTTCSTAT;
  const uint32_t errorStatus = LPC_GPDMA->INTERRSTAT;

  LPC_GPDMA->INTTCCLEAR = terminalStatus;
  LPC_GPDMA->INTERRCLEAR = errorStatus;

  uint32_t intStatus = errorStatus | terminalStatus;

  do
  {
    const unsigned int index = 31 - countLeadingZeros32(intStatus);
    struct GpDmaBase * const descriptor = controller.instances[index];
    const uint32_t mask = 1UL << index;
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
    intStatus -= mask;
  }
  while (intStatus);
}
/*----------------------------------------------------------------------------*/
static unsigned int dmaControllerAllocate(struct GpDmaBase *channel,
    enum GpDmaEvent event)
{
  assert(event < GPDMA_MEMORY);

  size_t entryIndex = 0, entryOffset = 0;
  unsigned int minValue = 0;
  bool found = false;

  for (size_t index = 0; index < EVENT_COUNT; ++index)
  {
    size_t entry = 0;
    bool allowed = false;

    do
    {
      if (dmaEventMap[index][entry] == event)
      {
        allowed = true;
        break;
      }
    }
    while (++entry < EVENT_SOURCES);

    if (allowed && (!found || controller.connections[index] < minValue))
    {
      found = true;
      entryIndex = index;
      entryOffset = entry;
      minValue = controller.connections[index];
    }
  }

  assert(found);

  ++controller.connections[entryIndex];
  channel->mux.mask &= ~(0x03 << (entryIndex << 1));
  channel->mux.value = entryOffset << (entryIndex << 1);

  return entryIndex;
}
/*----------------------------------------------------------------------------*/
static void dmaControllerFree(struct GpDmaBase *channel)
{
  uint32_t mask = ~channel->mux.mask;
  size_t index = 0;

  /* DMA Handler is already initialized */
  while (mask)
  {
    if (mask & 0x03)
      --controller.connections[index];

    ++index;
    mask >>= 2;
  }
}
/*----------------------------------------------------------------------------*/
static void dmaControllerInit(void)
{
  sysClockEnable(CLK_M4_GPDMA);
  sysResetEnable(RST_GPDMA);

  LPC_GPDMA->CONFIG |= DMA_ENABLE;

#ifndef CONFIG_PLATFORM_LPC_GPDMA_SYNC
  /* Disable synchronization logic to improve response time */
  LPC_GPDMA->SYNC = SYNC_MASK;
#endif

  irqSetPriority(GPDMA_IRQ, CONFIG_PLATFORM_LPC_GPDMA_PRIORITY);
  irqEnable(GPDMA_IRQ);
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct GpDmaBaseConfig * const config = configBase;
  assert(config->channel < CHANNEL_COUNT);

  struct GpDmaBase * const channel = object;

  channel->config = CONFIG_TYPE(config->type) | CONFIG_IE | CONFIG_ITC;
  channel->handler = NULL;
  channel->number = config->channel;
  channel->reg = &LPC_GPDMA->CHANNELS[channel->number];

  /* Reset multiplexer mask and value */
  channel->mux.mask = 0xFFFFFFFFUL;
  channel->mux.value = 0;

  if (!sysClockStatus(CLK_M4_GPDMA))
    dmaControllerInit();

  if (config->type != GPDMA_TYPE_M2M)
  {
    const unsigned int peripheral = dmaControllerAllocate(channel,
        config->event);

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

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  dmaControllerFree(object);
}
