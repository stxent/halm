/*
 * dma_base.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/irq.h>
#include <halm/platform/bouffalo/bl602/clocking_defs.h>
#include <halm/platform/bouffalo/dma_base.h>
#include <halm/platform/bouffalo/dma_defs.h>
#include <halm/platform/platform_defs.h>
#include <xcore/accel.h>
#include <xcore/atomic.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define CHANNEL_COUNT ARRAY_SIZE(BL_DMA->CHANNELS)
/*----------------------------------------------------------------------------*/
static void dmaControllerInit(void);
static unsigned int eventToPeripheral(enum DmaEvent);
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const DmaBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = channelInit,
    .deinit = NULL /* Default destructor */
};
/*----------------------------------------------------------------------------*/
static const uint8_t dmaEventMap[] = {
    [DMA_UART0_RX]  = 0,
    [DMA_UART0_TX]  = 1,
    [DMA_UART1_RX]  = 2,
    [DMA_UART1_TX]  = 3,
    [DMA_I2C_RX]    = 6,
    [DMA_I2C_TX]    = 7,
    [SPI_RX]        = 10,
    [SPI_TX]        = 11,
    [DMA_ADC]       = 22,
    [DMA_DAC]       = 23
};
/*----------------------------------------------------------------------------*/
static struct DmaBase *instances[CHANNEL_COUNT] = {NULL};
/*----------------------------------------------------------------------------*/
static void dmaControllerInit(void)
{
  BL_GLB->CLK_CFG2 |= CLK_CFG2_DMA_CLK_EN_MASK;
  BL_DMA->CONFIG |= CONFIG_SDMAEN;

#ifndef CONFIG_PLATFORM_BOUFFALO_DMA_SYNC
  /*
   * Synchronization logic is enabled by default. It must be used when
   * the peripheral generating the DMA request runs on a different clock
   * to the GPDMA. For peripherals running on the same clock as the GPDMA,
   * disabling the synchronization logic improves the response time.
   */
  BL_DMA->SYNC = SYNC_MASK;
#endif

  irqSetPriority(DMA_ALL_IRQ, CONFIG_PLATFORM_BOUFFALO_DMA_PRIORITY);
  irqEnable(DMA_ALL_IRQ);
}
/*----------------------------------------------------------------------------*/
static unsigned int eventToPeripheral(enum DmaEvent event)
{
  assert(event < DMA_MEMORY);
  return dmaEventMap[event];
}
/*----------------------------------------------------------------------------*/
uint32_t dmaBaseCalcControl(const struct DmaBase *,
    const struct DmaSettings *settings)
{
  assert(settings->source.burst <= DMA_BURST_256);
  assert(settings->source.width <= DMA_WIDTH_WORD);
  assert(settings->destination.burst <= DMA_BURST_256);
  assert(settings->destination.width <= DMA_WIDTH_WORD);

  uint32_t control = 0;

  control |= CONTROL_SBS(settings->source.burst);
  control |= CONTROL_STW(settings->source.width);
  control |= CONTROL_DBS(settings->destination.burst);
  control |= CONTROL_DTW(settings->destination.width);

  if (settings->source.increment)
    control |= CONTROL_SI;
  if (settings->destination.increment)
    control |= CONTROL_DI;

  return control;
}
/*----------------------------------------------------------------------------*/
void dmaResetInstance(uint8_t channel)
{
  instances[channel] = NULL;
}
/*----------------------------------------------------------------------------*/
bool dmaSetInstance(uint8_t channel, struct DmaBase *object)
{
  assert(channel < CHANNEL_COUNT);

  if (instances[channel] != NULL)
    return false;
  instances[channel] = object;
  return true;
//  void *expected = NULL;
//  return compareExchangePointer(&instances[channel], &expected, object);
}
/*----------------------------------------------------------------------------*/
[[gnu::interrupt]] void DMA_ALL_ISR(void)
{
  const uint32_t terminalStatus = BL_DMA->INTTCSTATUS;
  const uint32_t errorStatus = BL_DMA->INTERRSTATUS;

  BL_DMA->INTTCCLEAR = terminalStatus;
  BL_DMA->INTERRCLEAR = errorStatus;

  uint32_t intStatus = errorStatus | terminalStatus;

  do
  {
    const unsigned int index = 31 - countLeadingZeros32(intStatus);
    struct DmaBase * const descriptor = instances[index];
    const uint32_t mask = 1UL << index;
    enum Result res = E_OK;

    intStatus -= mask;

    if (errorStatus & mask)
    {
      res = E_ERROR;
    }
    else
    {
      const BL_DMA_CHANNEL_Type * const reg = descriptor->reg;

      if (reg->CONFIG & CONFIG_CHEN)
        res = E_BUSY;
    }

    descriptor->handler(descriptor, res);
  }
  while (intStatus);
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct DmaBaseConfig * const config = configBase;
  assert(config->channel < CHANNEL_COUNT);

  struct DmaBase * const channel = object;

  channel->config = CONFIG_FLOWCTRL(config->type);
  channel->handler = NULL;
  channel->number = config->channel;
  channel->reg = &BL_DMA->CHANNELS[channel->number];

  if (config->type != DMA_TYPE_M2M)
  {
    const unsigned int peripheral = eventToPeripheral(config->event);

    switch (config->type)
    {
      case DMA_TYPE_M2P:
        channel->config |= CONFIG_DSTPH(peripheral);
        break;

      case DMA_TYPE_P2M:
        channel->config |= CONFIG_SRCPH(peripheral);
        break;

      default:
        break;
    }
  }

  if (!irqStatus(DMA_ALL_IRQ))
    dmaControllerInit();

  return E_OK;
}
