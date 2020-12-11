/*
 * dma_base.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/irq.h>
#include <halm/platform/platform_defs.h>
#include <halm/platform/stm32/stm32f1xx/dma_base.h>
#include <halm/platform/stm32/stm32f1xx/dma_defs.h>
#include <halm/platform/stm32/system.h>
#include <xcore/memory.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
/* DMA1 has 7 streams and DMA2 has 5 streams */
#define DMA1_STREAM_COUNT 7
#define DMA2_STREAM_COUNT 5
#define STREAM_COUNT      (DMA1_STREAM_COUNT + DMA2_STREAM_COUNT)

#define STREAM_ENCODE(controller, stream) \
    ((controller) * DMA1_STREAM_COUNT + (stream))
/*----------------------------------------------------------------------------*/
static void dma1StreamHandler(uint8_t);
static void dma2StreamHandler(uint8_t);
static enum Result streamInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const DmaBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = streamInit,
    .deinit = 0 /* Default destructor */
};
/*----------------------------------------------------------------------------*/
static struct DmaBase *instances[STREAM_COUNT] = {0};
/*----------------------------------------------------------------------------*/
const struct DmaBase *dmaGetInstance(uint8_t stream)
{
  assert(stream < ARRAY_SIZE(instances));
  return instances[stream];
}
/*----------------------------------------------------------------------------*/
void dmaResetInstance(uint8_t stream)
{
  assert(stream < ARRAY_SIZE(instances));
  instances[stream] = 0;
}
/*----------------------------------------------------------------------------*/
bool dmaSetInstance(uint8_t stream, struct DmaBase *object)
{
  assert(object);
  assert(stream < ARRAY_SIZE(instances));

  void *expected = 0;
  return compareExchangePointer(&instances[stream], &expected, object);
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
static void dma1StreamHandler(uint8_t number)
{
  struct DmaBase * const stream = instances[STREAM_ENCODE(0, number)];
  const uint32_t rawStatus = STM_DMA1->ISR & ISR_CHANNEL_MASK(number);
  const uint32_t status = ISR_CHANNEL_VALUE(rawStatus, number);
  enum Result res = E_OK;

  /* Clear interrupt flags */
  STM_DMA1->IFCR = rawStatus;

  if (!(status & ISR_TCIF_GENERIC))
    res = (status & ISR_TEIF_GENERIC) ? E_ERROR : E_BUSY;

  stream->handler(stream, res);
}
/*----------------------------------------------------------------------------*/
static void dma2StreamHandler(uint8_t number)
{
  struct DmaBase * const stream = instances[STREAM_ENCODE(1, number)];
  const uint32_t rawStatus = STM_DMA2->ISR & ISR_CHANNEL_MASK(number);
  const uint32_t status = ISR_CHANNEL_VALUE(rawStatus, number);
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
  const struct DmaBaseConfig * const config = configBase;
  struct DmaBase * const stream = object;

  assert(config->stream < ARRAY_SIZE(instances));
  assert(config->priority < 4);

  const unsigned int controller = config->stream >= DMA1_STREAM_COUNT;
  const unsigned int number = controller ?
      config->stream - DMA1_STREAM_COUNT : config->stream;

  if (controller == 0)
  {
    assert(number < 7);
    stream->reg = STM_DMA1->CHANNELS + number;
    stream->irq = DMA1_CHANNEL1_IRQ + number;

    if (!sysClockStatus(CLK_DMA1))
      sysClockEnable(CLK_DMA1);
  }
  else
  {
    assert(number < 5);
    stream->reg = STM_DMA2->CHANNELS + number;
    stream->irq = DMA2_CHANNEL1_IRQ + number;

    if (!sysClockStatus(CLK_DMA2))
      sysClockEnable(CLK_DMA2);
  }

  stream->config = CCR_PL(config->priority);
  stream->handler = 0;
  stream->number = config->stream;

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
