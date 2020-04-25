/*
 * gpdma_base.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/memory.h>
#include <halm/irq.h>
#include <halm/platform/nxp/gpdma_base.h>
#include <halm/platform/nxp/gpdma_defs.h>
#include <halm/platform/nxp/lpc17xx/system.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
#define CHANNEL_COUNT ARRAY_SIZE(LPC_GPDMA->CHANNELS)
/*----------------------------------------------------------------------------*/
static struct GpDmaMuxConfig calcEventMux(enum GpDmaType, enum GpDmaEvent);
static void dmaPeripheralInit(void);
static unsigned int eventToPeripheral(enum GpDmaEvent);
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const GpDmaBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = channelInit,
    .deinit = 0 /* Default destructor */
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
static struct GpDmaBase *instances[CHANNEL_COUNT] = {0};
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
static void dmaPeripheralInit(void)
{
  sysPowerEnable(PWR_GPDMA);

  LPC_GPDMA->CONFIG |= DMA_ENABLE;

#ifndef CONFIG_PLATFORM_NXP_GPDMA_SYNC
  /*
   * Synchronization logic is enabled by default. It must be used when
   * the peripheral generating the DMA request runs on a different clock
   * to the GPDMA. For peripherals running on the same clock as the GPDMA,
   * disabling the synchronization logic improves the response time.
   */
  LPC_GPDMA->SYNC = SYNC_MASK;
#endif

  irqSetPriority(GPDMA_IRQ, CONFIG_PLATFORM_NXP_GPDMA_PRIORITY);
  irqEnable(GPDMA_IRQ);
}
/*----------------------------------------------------------------------------*/
static unsigned int eventToPeripheral(enum GpDmaEvent event)
{
  assert(event < GPDMA_MEMORY);
  return eventTranslationMap[event];
}
/*----------------------------------------------------------------------------*/
uint32_t gpDmaBaseCalcControl(const struct GpDmaBase *channel
    __attribute__((unused)), const struct GpDmaSettings *settings)
{
  assert(settings->source.burst != DMA_BURST_2);
  assert(settings->source.burst <= DMA_BURST_256);
  assert(settings->source.width <= DMA_WIDTH_WORD);
  assert(settings->destination.burst != DMA_BURST_2);
  assert(settings->destination.burst <= DMA_BURST_256);
  assert(settings->destination.width <= DMA_WIDTH_WORD);

  uint32_t control = 0;

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
void gpDmaResetInstance(uint8_t channel)
{
  instances[channel] = 0;
}
/*----------------------------------------------------------------------------*/
bool gpDmaSetInstance(uint8_t channel, struct GpDmaBase *object)
{
  assert(channel < CHANNEL_COUNT);

  void *expected = 0;
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
  uint32_t errorStatus = LPC_GPDMA->INTERRSTAT;

  LPC_GPDMA->INTTCCLEAR = terminalStatus;
  LPC_GPDMA->INTERRCLEAR = errorStatus;
  errorStatus = reverseBits32(errorStatus);

  uint32_t intStatus = errorStatus | reverseBits32(terminalStatus);

  do
  {
    const unsigned int index = countLeadingZeros32(intStatus);
    struct GpDmaBase * const descriptor = instances[index];
    const uint32_t mask = (1UL << 31) >> index;
    enum Result res = E_OK;

    intStatus -= mask;

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
static enum Result channelInit(void *object, const void *configBase)
{
  const struct GpDmaBaseConfig * const config = configBase;
  struct GpDmaBase * const channel = object;

  assert(config->channel < CHANNEL_COUNT);

  channel->config = CONFIG_TYPE(config->type) | CONFIG_IE | CONFIG_ITC;
  channel->handler = 0;
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
    dmaPeripheralInit();

  return E_OK;
}
