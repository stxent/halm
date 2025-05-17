/*
 * gpdma_base.c
 * Copyright (C) 2012 xent
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
/*----------------------------------------------------------------------------*/
#define CHANNEL_COUNT ARRAY_SIZE(LPC_GPDMA->CHANNELS)
/*----------------------------------------------------------------------------*/
static struct GpDmaMuxConfig calcEventMux(enum GpDmaType, enum GpDmaEvent);
static void dmaControllerInit(void);
static unsigned int eventToPeripheral(enum GpDmaEvent);
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const GpDmaBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = channelInit,
    .deinit = NULL /* Default destructor */
};
/*----------------------------------------------------------------------------*/
static const uint8_t dmaEventMap[] = {
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
static struct GpDmaBase *instances[CHANNEL_COUNT] = {NULL};
/*----------------------------------------------------------------------------*/
static struct GpDmaMuxConfig calcEventMux(enum GpDmaType type,
    enum GpDmaEvent event)
{
  if (type != GPDMA_TYPE_M2M)
  {
    if (event >= GPDMA_UART0_RX && event <= GPDMA_UART3_TX)
    {
      const unsigned int position = event - GPDMA_UART0_RX;
      const uint8_t mask = 1UL << position;

      return (struct GpDmaMuxConfig){
          .mask = ~mask,
          .value = 0
      };
    }
    else if (event >= GPDMA_MAT0_0 && event <= GPDMA_MAT3_1)
    {
      const unsigned int position = event - GPDMA_MAT0_0;
      const uint8_t mask = 1UL << position;

      return (struct GpDmaMuxConfig){
          .mask = ~mask,
          .value = mask
      };
    }
  }

  return (struct GpDmaMuxConfig){
      .mask = 0xFF,
      .value = 0
  };
}
/*----------------------------------------------------------------------------*/
static void dmaControllerInit(void)
{
  sysPowerEnable(PWR_GPDMA);

  LPC_GPDMA->CONFIG |= DMA_ENABLE;

#ifndef CONFIG_PLATFORM_LPC_GPDMA_SYNC
  /*
   * Synchronization logic is enabled by default. It must be used when
   * the peripheral generating the DMA request runs on a different clock
   * to the GPDMA. For peripherals running on the same clock as the GPDMA,
   * disabling the synchronization logic improves the response time.
   */
  LPC_GPDMA->SYNC = SYNC_MASK;
#endif

  irqSetPriority(GPDMA_IRQ, CONFIG_PLATFORM_LPC_GPDMA_PRIORITY);
  irqEnable(GPDMA_IRQ);
}
/*----------------------------------------------------------------------------*/
static unsigned int eventToPeripheral(enum GpDmaEvent event)
{
  assert(event < GPDMA_MEMORY);
  return dmaEventMap[event];
}
/*----------------------------------------------------------------------------*/
uint32_t gpDmaBaseCalcControl(const struct GpDmaBase *,
    const struct GpDmaSettings *settings)
{
  assert(settings->source.burst <= DMA_BURST_256);
  assert(settings->source.width <= DMA_WIDTH_WORD);
  assert(settings->destination.burst <= DMA_BURST_256);
  assert(settings->destination.width <= DMA_WIDTH_WORD);

  uint32_t control = 0;

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
uint32_t gpDmaBaseCalcMasterAffinity(const struct GpDmaBase *, enum GpDmaMaster,
    enum GpDmaMaster)
{
  return 0;
}
/*----------------------------------------------------------------------------*/
void gpDmaResetInstance(uint8_t channel)
{
  instances[channel] = NULL;
}
/*----------------------------------------------------------------------------*/
bool gpDmaSetInstance(uint8_t channel, struct GpDmaBase *object)
{
  assert(channel < CHANNEL_COUNT);

  void *expected = NULL;
  return compareExchangePointer(&instances[channel], &expected, object);
}
/*----------------------------------------------------------------------------*/
void gpDmaSetMux(struct GpDmaBase *descriptor)
{
  LPC_SC->DMAREQSEL =
      (LPC_SC->DMAREQSEL & descriptor->mux.mask) | descriptor->mux.value;
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
    struct GpDmaBase * const descriptor = instances[index];
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
static enum Result channelInit(void *object, const void *configBase)
{
  const struct GpDmaBaseConfig * const config = configBase;
  assert(config->channel < CHANNEL_COUNT);

  struct GpDmaBase * const channel = object;

  channel->config = CONFIG_TYPE(config->type) | CONFIG_IE | CONFIG_ITC;
  channel->handler = NULL;
  channel->number = config->channel;
  channel->reg = &LPC_GPDMA->CHANNELS[channel->number];

  /* Calculate multiplexer mask and value */
  channel->mux = calcEventMux(config->type, config->event);

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
  }

  if (!sysPowerStatus(PWR_GPDMA))
    dmaControllerInit();

  return E_OK;
}
