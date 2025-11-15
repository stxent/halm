/*
 * sdma_base.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/irq.h>
#include <halm/platform/lpc/sdma_base.h>
#include <halm/platform/lpc/sdma_defs.h>
#include <halm/platform/lpc/system.h>
#include <halm/platform/platform_defs.h>
#include <xcore/accel.h>
#include <xcore/atomic.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#define CHANNEL_COUNT ARRAY_SIZE(LPC_SDMA->CHANNELS)
/*----------------------------------------------------------------------------*/
struct DmaController
{
  /* Channel descriptors currently in use */
  struct SdmaBase *instances[CHANNEL_COUNT];
  /* Descriptor memory */
  struct SdmaEntry *descriptors;
};
/*----------------------------------------------------------------------------*/
static bool dmaControllerInit(void);
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const SdmaBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = channelInit,
    .deinit = NULL /* Default destructor */
};
/*----------------------------------------------------------------------------*/
static struct DmaController controller = {
    .instances = {NULL},
    .descriptors = NULL
};
/*----------------------------------------------------------------------------*/
static bool dmaControllerInit(void)
{
  const size_t descriptorTableSize = sizeof(struct SdmaEntry) * CHANNEL_COUNT;

  controller.descriptors = memalign(512, descriptorTableSize);
  if (controller.descriptors == NULL)
    return false;
  memset(controller.descriptors, 0, descriptorTableSize);

  sysClockEnable(CLK_DMA);
  sysResetPulse(RST_DMA);

  LPC_SDMA->SRAMBASE = (uint32_t)controller.descriptors;
  LPC_SDMA->CTRL = CTRL_ENABLE;

  irqSetPriority(DMA_IRQ, CONFIG_PLATFORM_LPC_SDMA_PRIORITY);
  irqEnable(DMA_IRQ);
  return true;
}
/*----------------------------------------------------------------------------*/
uint32_t sdmaBaseCalcTransferConfig(const struct SdmaBase *descriptor,
    const struct SdmaSettings *settings)
{
  assert(settings->width <= DMA_WIDTH_WORD);
  assert(settings->source.stride <= SDMA_STRIDE_4);
  assert(settings->destination.stride <= SDMA_STRIDE_4);

  uint32_t config = XFERCFG_CFGVALID | XFERCFG_SETINTA;

  config |= XFERCFG_WIDTH(settings->width);
  config |= XFERCFG_SRCINC(settings->source.stride);
  config |= XFERCFG_DSTINC(settings->destination.stride);

  if (descriptor->mux == ITRIG_INMUX_RESERVED)
    config |= XFERCFG_SWTRIG;

  return config;
}
/*----------------------------------------------------------------------------*/
void sdmaResetInstance(uint8_t channel)
{
  controller.instances[channel] = NULL;
}
/*----------------------------------------------------------------------------*/
bool sdmaSetInstance(uint8_t channel, struct SdmaBase *object)
{
  assert(channel < CHANNEL_COUNT);

  void *expected = NULL;

  return compareExchangePointer(&controller.instances[channel],
      &expected, object);
}
/*----------------------------------------------------------------------------*/
void sdmaSetMux(struct SdmaBase *descriptor)
{
  LPC_DMA_INMUX->DMA_ITRIG_INMUX[descriptor->number] = descriptor->mux;
}
/*----------------------------------------------------------------------------*/
void DMA_ISR(void)
{
  const uint32_t errorStatus = LPC_SDMA->ERRINT;
  const uint32_t transferStatus = LPC_SDMA->INTA;
  uint32_t status = errorStatus | transferStatus;

  do
  {
    const unsigned int index = 31 - countLeadingZeros32(status);
    struct SdmaBase * const descriptor = controller.instances[index];
    const uint32_t mask = 1UL << index;
    enum Result res = E_OK;

    status &= ~mask;

    if (errorStatus & mask)
    {
      LPC_SDMA->ERRINT = mask;
      res = E_ERROR;
    }
    else
    {
      LPC_SDMA->INTA = mask;

      if (LPC_SDMA->ACTIVE & mask)
        res = E_BUSY;
    }

    descriptor->handler(descriptor, res);
  }
  while (status);
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct SdmaBaseConfig * const config = configBase;
  assert(config->priority <= CFG_CHPRIORITY_MAX);
  assert(config->request < SDMA_REQUEST_END);
  assert(config->trigger < SDMA_TRIGGER_END);
  assert(config->request == SDMA_REQUEST_NONE
      || config->trigger == SDMA_TRIGGER_NONE);

  struct SdmaBase * const channel = object;
  uint8_t number;

  if (config->request != SDMA_REQUEST_NONE)
  {
    assert(config->channel == SDMA_CHANNEL_AUTO);
    number = config->request;
  }
  else
  {
    assert(config->channel < CHANNEL_COUNT);
    number = config->channel;
  }

  if (!sysClockStatus(CLK_DMA))
  {
    if (!dmaControllerInit())
      return E_MEMORY;
  }

  channel->handler = NULL;
  channel->reg = &LPC_SDMA->CHANNELS[number];

  channel->head = controller.descriptors + number;
  channel->config = CFG_CHPRIORITY(CFG_CHPRIORITY_MAX - config->priority);
  channel->number = number;

  if (config->request != SDMA_REQUEST_NONE)
    channel->config |= CFG_PERIPHREQEN;

  if (config->trigger != SDMA_TRIGGER_NONE)
  {
    channel->config |= CFG_HWTRIGEN | CFG_TRIGBURST;
    if (config->polarity)
      channel->config |= CFG_TRIGPOL;

    channel->mux = config->trigger;
  }
  else
    channel->mux = ITRIG_INMUX_RESERVED;

  return E_OK;
}
