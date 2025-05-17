/*
 * edma_base.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/irq.h>
#include <halm/platform/imxrt/edma_base.h>
#include <halm/platform/imxrt/edma_defs.h>
#include <halm/platform/imxrt/system.h>
#include <halm/platform/platform_defs.h>
#include <xcore/accel.h>
#include <xcore/atomic.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define GENERIC_DMA_HANDLER(a, b) \
  do \
  { \
    if (instances[a] != NULL) \
      instances[a]->handler(instances[a], E_OK); \
    if (instances[b] != NULL) \
      instances[b]->handler(instances[b], E_OK); \
  } while (0)

#define CHANNEL_COUNT 32
/*----------------------------------------------------------------------------*/
static IrqNumber calcIrqNumber(uint8_t);

static enum Result channelInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const EdmaBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = channelInit,
    .deinit = NULL /* Default destructor */
};
/*----------------------------------------------------------------------------*/
static struct EdmaBase *instances[CHANNEL_COUNT] = {NULL};
/*----------------------------------------------------------------------------*/
static IrqNumber calcIrqNumber(uint8_t number)
{
  const unsigned int offset = (number >= CHANNEL_COUNT / 2) ?
      (number - CHANNEL_COUNT / 2) : number;

  return DMA0_DMA16_IRQ + offset;
}
/*----------------------------------------------------------------------------*/
bool edmaBindInstance(struct EdmaBase *channel)
{
  assert(channel != NULL);

  void *expected = NULL;

  return compareExchangePointer(&instances[channel->number],
      &expected, channel);
}
/*----------------------------------------------------------------------------*/
void edmaUnbindInstance(struct EdmaBase *channel)
{
  assert(channel != NULL);

  const uint8_t number = channel->number;

  ((IMX_DMAMUX_Type *)channel->mux)->CHCFG[number] = 0;
  ((IMX_EDMA_Type *)channel->reg)->CEEI = CEEI_CEEI(number);

  instances[number] = NULL;
}
/*----------------------------------------------------------------------------*/
void DMA0_DMA16_ISR(void)
{
  GENERIC_DMA_HANDLER(0, 16);
}
/*----------------------------------------------------------------------------*/
void DMA1_DMA17_ISR(void)
{
  GENERIC_DMA_HANDLER(1, 17);
}
/*----------------------------------------------------------------------------*/
void DMA2_DMA18_ISR(void)
{
  GENERIC_DMA_HANDLER(2, 18);
}
/*----------------------------------------------------------------------------*/
void DMA3_DMA19_ISR(void)
{
  GENERIC_DMA_HANDLER(3, 19);
}
/*----------------------------------------------------------------------------*/
void DMA4_DMA20_ISR(void)
{
  GENERIC_DMA_HANDLER(4, 20);
}
/*----------------------------------------------------------------------------*/
void DMA5_DMA21_ISR(void)
{
  GENERIC_DMA_HANDLER(5, 21);
}
/*----------------------------------------------------------------------------*/
void DMA6_DMA22_ISR(void)
{
  GENERIC_DMA_HANDLER(6, 22);
}
/*----------------------------------------------------------------------------*/
void DMA7_DMA23_ISR(void)
{
  GENERIC_DMA_HANDLER(7, 23);
}
/*----------------------------------------------------------------------------*/
void DMA8_DMA24_ISR(void)
{
  GENERIC_DMA_HANDLER(8, 24);
}
/*----------------------------------------------------------------------------*/
void DMA9_DMA25_ISR(void)
{
  GENERIC_DMA_HANDLER(9, 25);
}
/*----------------------------------------------------------------------------*/
void DMA10_DMA26_ISR(void)
{
  GENERIC_DMA_HANDLER(10, 26);
}
/*----------------------------------------------------------------------------*/
void DMA11_DMA27_ISR(void)
{
  GENERIC_DMA_HANDLER(11, 27);
}
/*----------------------------------------------------------------------------*/
void DMA12_DMA28_ISR(void)
{
  GENERIC_DMA_HANDLER(12, 28);
}
/*----------------------------------------------------------------------------*/
void DMA13_DMA29_ISR(void)
{
  GENERIC_DMA_HANDLER(13, 29);
}
/*----------------------------------------------------------------------------*/
void DMA14_DMA30_ISR(void)
{
  GENERIC_DMA_HANDLER(14, 30);
}
/*----------------------------------------------------------------------------*/
void DMA15_DMA31_ISR(void)
{
  GENERIC_DMA_HANDLER(15, 31);
}
/*----------------------------------------------------------------------------*/
void DMA_ERROR_ISR(void)
{
  uint32_t errors = reverseBits32(IMX_EDMA->ERR);

  do
  {
    const unsigned int index = countLeadingZeros32(errors);
    struct EdmaBase * const descriptor = instances[index];

    IMX_EDMA->CERR = CERR_CERR(index);

    descriptor->handler(descriptor, E_ERROR);
    errors -= (1UL << 31) >> index;
  }
  while (errors);
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct EdmaBaseConfig * const config = configBase;
  struct EdmaBase * const channel = object;

  assert(config->channel < CHANNEL_COUNT / 2);
  assert(config->event < EDMA_EVENT_END);
  assert(config->priority <= DMA_PRIORITY_HIGH);

  const uint8_t number = config->priority == DMA_PRIORITY_LOW ?
      config->channel : (config->channel + CHANNEL_COUNT / 2);
  const IrqNumber irq = calcIrqNumber(number);

  channel->handler = NULL;
  channel->controller = 0;
  channel->event = config->event;
  channel->number = number;
  channel->mux = IMX_DMAMUX;
  channel->reg = IMX_EDMA;

  if (!sysClockStatus(CLK_DMA))
  {
    sysClockEnable(CLK_DMA);

    /* Initial configuration */
    IMX_EDMA->CR = CR_GRP1PRI | CR_EMLM | CR_ERCA;

    irqSetPriority(DMA_ERROR_IRQ, CONFIG_PLATFORM_IMXRT_EDMA_PRIORITY);
    irqEnable(DMA_ERROR_IRQ);
  }

  if (!irqStatus(irq))
  {
    irqSetPriority(irq, CONFIG_PLATFORM_IMXRT_EDMA_PRIORITY);
    irqEnable(irq);
  }

  return E_OK;
}
