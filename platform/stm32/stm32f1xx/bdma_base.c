/*
 * bdma_base.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/irq.h>
#include <halm/platform/platform_defs.h>
#include <halm/platform/stm32/bdma_base.h>
#include <halm/platform/stm32/bdma_defs.h>
#include <halm/platform/stm32/system.h>
#include <xcore/atomic.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
/* DMA1 has 7 streams and DMA2 has 5 streams */
#define DMA1_STREAM_COUNT 7
#define DMA2_STREAM_COUNT 5
#define STREAM_COUNT      (DMA1_STREAM_COUNT + DMA2_STREAM_COUNT)

#define STREAM_ENCODE(controller, stream) \
    ((controller) * DMA1_STREAM_COUNT + (stream))
/*----------------------------------------------------------------------------*/
static void dma1StreamHandler(unsigned int);
static void dma2StreamHandler(unsigned int);
static enum Result streamInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const BdmaBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = streamInit,
    .deinit = NULL /* Default destructor */
};
/*----------------------------------------------------------------------------*/
static struct BdmaBase *instances[STREAM_COUNT] = {NULL};
/*----------------------------------------------------------------------------*/
const struct BdmaBase *bdmaGetInstance(uint8_t number)
{
  const unsigned int controller = number >= DMA2_STREAM1;
  const unsigned int index = controller ? number - DMA2_STREAM1 : number;

  assert(STREAM_ENCODE(controller, index) < ARRAY_SIZE(instances));
  return instances[STREAM_ENCODE(controller, index)];
}
/*----------------------------------------------------------------------------*/
void bdmaResetInstance(uint8_t number)
{
  const unsigned int controller = number >= DMA2_STREAM1;
  const unsigned int index = controller ? number - DMA2_STREAM1 : number;

  assert(STREAM_ENCODE(controller, index) < ARRAY_SIZE(instances));
  instances[STREAM_ENCODE(controller, index)] = NULL;
}
/*----------------------------------------------------------------------------*/
bool bdmaSetInstance(uint8_t number, struct BdmaBase *stream)
{
  const unsigned int controller = number >= DMA2_STREAM1;
  const unsigned int index = controller ? number - DMA2_STREAM1 : number;
  void *expected = NULL;

  assert(stream != NULL);
  assert(STREAM_ENCODE(controller, index) < ARRAY_SIZE(instances));

  if (compareExchangePointer(&instances[STREAM_ENCODE(controller, index)],
      &expected, stream))
  {
    STM_DMA_Type * const dma = controller ? STM_DMA2 : STM_DMA1;

    /* Clear pending interrupt flags */
    dma->IFCR = ISR_CHANNEL_MASK(index);

    return true;
  }

  return false;
}
/*----------------------------------------------------------------------------*/
void DMA1_CHANNEL1_ISR(void)
{
  dma1StreamHandler(0);
}
/*----------------------------------------------------------------------------*/
void DMA1_CHANNEL2_ISR(void)
{
  dma1StreamHandler(1);
}
/*----------------------------------------------------------------------------*/
void DMA1_CHANNEL3_ISR(void)
{
  dma1StreamHandler(2);
}
/*----------------------------------------------------------------------------*/
void DMA1_CHANNEL4_ISR(void)
{
  dma1StreamHandler(3);
}
/*----------------------------------------------------------------------------*/
void DMA1_CHANNEL5_ISR(void)
{
  dma1StreamHandler(4);
}
/*----------------------------------------------------------------------------*/
void DMA1_CHANNEL6_ISR(void)
{
  dma1StreamHandler(5);
}
/*----------------------------------------------------------------------------*/
void DMA1_CHANNEL7_ISR(void)
{
  dma1StreamHandler(6);
}
/*----------------------------------------------------------------------------*/
void DMA2_CHANNEL1_ISR(void)
{
  dma2StreamHandler(0);
}
/*----------------------------------------------------------------------------*/
void DMA2_CHANNEL2_ISR(void)
{
  dma2StreamHandler(1);
}
/*----------------------------------------------------------------------------*/
void DMA2_CHANNEL3_ISR(void)
{
  dma2StreamHandler(2);
}
/*----------------------------------------------------------------------------*/
void DMA2_CHANNEL4_ISR(void)
{
  if (STM_DMA2->ISR & ISR_GIF(3))
    dma2StreamHandler(3);
}
/*----------------------------------------------------------------------------*/
void DMA2_CHANNEL5_ISR(void)
{
  if (STM_DMA2->ISR & ISR_GIF(4))
    dma2StreamHandler(4);
}
/*----------------------------------------------------------------------------*/
static void dma1StreamHandler(unsigned int index)
{
  struct BdmaBase * const stream = instances[STREAM_ENCODE(0, index)];
  const uint32_t rawStatus = STM_DMA1->ISR & ISR_CHANNEL_MASK(index);
  const uint32_t status = ISR_CHANNEL_VALUE(rawStatus, index);
  enum Result res = E_OK;

  /* Clear interrupt flags */
  STM_DMA1->IFCR = rawStatus;

  if (!(status & ISR_TCIF_GENERIC))
    res = (status & ISR_TEIF_GENERIC) ? E_ERROR : E_BUSY;

  stream->handler(stream, res);
}
/*----------------------------------------------------------------------------*/
static void dma2StreamHandler(unsigned int index)
{
  struct BdmaBase * const stream = instances[STREAM_ENCODE(1, index)];
  const uint32_t rawStatus = STM_DMA2->ISR & ISR_CHANNEL_MASK(index);
  const uint32_t status = ISR_CHANNEL_VALUE(rawStatus, index);
  enum Result res = E_OK;

  /* Clear interrupt flags */
  STM_DMA2->IFCR = rawStatus;

  if (!(status & ISR_TCIF_GENERIC))
    res = (status & ISR_TEIF_GENERIC) ? E_ERROR : E_BUSY;

  stream->handler(stream, res);
}
/*----------------------------------------------------------------------------*/
static enum Result streamInit(void *object, const void *configBase)
{
  const struct BdmaBaseConfig * const config = configBase;
  struct BdmaBase * const stream = object;

  assert(config->stream < ARRAY_SIZE(instances));
  assert(config->priority <= DMA_PRIORITY_VERY_HIGH);

  const unsigned int controller = config->stream >= DMA1_STREAM_COUNT;
  const unsigned int index = controller ?
      config->stream - DMA1_STREAM_COUNT : config->stream;

  if (controller == 0)
  {
    assert(index < DMA1_STREAM_COUNT);
    stream->reg = STM_DMA1->CHANNELS + index;
    stream->irq = DMA1_CHANNEL1_IRQ + index;

    if (!sysClockStatus(CLK_DMA1))
      sysClockEnable(CLK_DMA1);
  }
  else
  {
    assert(index < DMA2_STREAM_COUNT);
    stream->reg = STM_DMA2->CHANNELS + index;
    stream->irq = DMA2_CHANNEL1_IRQ + index;

    if (!sysClockStatus(CLK_DMA2))
      sysClockEnable(CLK_DMA2);
  }

  stream->config = CCR_PL(config->priority);
  stream->handler = NULL;
  stream->number = config->stream;

  if (!irqStatus(stream->irq))
  {
    irqSetPriority(stream->irq, CONFIG_PLATFORM_STM32_BDMA_PRIORITY);
    irqEnable(stream->irq);
  }

  switch (config->type)
  {
    case DMA_TYPE_M2M:
      stream->config |= CCR_MEM2MEM;
      break;

    case DMA_TYPE_M2P:
      stream->config |= CCR_DIR;
      break;

    default:
      break;
  }

  return E_OK;
}
