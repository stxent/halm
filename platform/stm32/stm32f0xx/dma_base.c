/*
 * dma_base.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/irq.h>
#include <halm/platform/platform_defs.h>
#include <halm/platform/stm32/dma_base.h>
#include <halm/platform/stm32/dma_defs.h>
#include <halm/platform/stm32/system.h>
#include <xcore/memory.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
/*
 * DMA1 has 5 streams on STM32F03x, STM32F04x and STM32F05x devices.
 * DMA1 has 7 streams on STM32F07x and STM32F09x devices.
 * DMA2 has 5 streams on STM32F09x devices.
 */

#ifdef CONFIG_PLATFORM_STM32_DMA1
#define DMA1_STREAM_COUNT 7
#else
#define DMA1_STREAM_COUNT 0
#endif

#ifdef CONFIG_PLATFORM_STM32_DMA2
#define DMA2_STREAM_COUNT 5
#else
#define DMA2_STREAM_COUNT 0
#endif

#define STREAM_COUNT      (DMA1_STREAM_COUNT + DMA2_STREAM_COUNT)

#define STREAM_ENCODE(controller, stream) \
    ((controller) * DMA1_STREAM_COUNT + (stream))
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_DMA1
static void dma1StreamHandler(uint8_t);
#endif

#ifdef CONFIG_PLATFORM_STM32_DMA2
static void dma2StreamHandler(uint8_t);
#endif

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
#ifdef CONFIG_PLATFORM_STM32_DMA1
void DMA1_CHANNEL1_ISR(void)
{
  dma1StreamHandler(0);
}
#endif
/*----------------------------------------------------------------------------*/
#if defined(CONFIG_PLATFORM_STM32_DMA1) || defined(CONFIG_PLATFORM_STM32_DMA2)
void DMA1_CHANNEL2_3_DMA2_CHANNEL1_2_ISR(void)
{
#ifdef CONFIG_PLATFORM_STM32_DMA1
  const uint32_t isr1 = STM_DMA1->ISR;

  if (isr1 & ISR_GIF(1))
    dma1StreamHandler(1);
  if (isr1 & ISR_GIF(2))
    dma1StreamHandler(2);
#endif

#ifdef CONFIG_PLATFORM_STM32_DMA2
  const uint32_t isr2 = STM_DMA2->ISR;

  if (isr2 & ISR_GIF(0))
    dma2StreamHandler(0);
  if (isr2 & ISR_GIF(1))
    dma2StreamHandler(1);
#endif
}
#endif
/*----------------------------------------------------------------------------*/
#if defined(CONFIG_PLATFORM_STM32_DMA1) || defined(CONFIG_PLATFORM_STM32_DMA2)
void DMA1_CHANNEL4_7_DMA2_CHANNEL3_5_ISR(void)
{
#ifdef CONFIG_PLATFORM_STM32_DMA1
  const uint32_t isr1 = STM_DMA1->ISR;

  if (isr1 & ISR_GIF(3))
    dma1StreamHandler(3);
  if (isr1 & ISR_GIF(4))
    dma1StreamHandler(4);
  if (isr1 & ISR_GIF(5))
    dma1StreamHandler(5);
  if (isr1 & ISR_GIF(6))
    dma1StreamHandler(6);
#endif

#ifdef CONFIG_PLATFORM_STM32_DMA2
  const uint32_t isr2 = STM_DMA2->ISR;

  if (isr2 & ISR_GIF(2))
    dma2StreamHandler(2);
  if (isr2 & ISR_GIF(3))
    dma2StreamHandler(3);
  if (isr2 & ISR_GIF(4))
    dma2StreamHandler(4);
#endif
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_DMA1
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
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_DMA2
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
#endif
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

  switch (controller)
  {
#ifdef CONFIG_PLATFORM_STM32_DMA1
    case 0:
    {
      assert(number < DMA1_STREAM_COUNT);
      stream->reg = STM_DMA1->CHANNELS + number;

      if (number >= 3)
        stream->irq = DMA1_CHANNEL4_7_DMA2_CHANNEL3_5_IRQ;
      else if (number >= 1)
        stream->irq = DMA1_CHANNEL2_3_DMA2_CHANNEL1_2_IRQ;
      else
        stream->irq = DMA1_CHANNEL1_IRQ;

      if (!sysClockStatus(CLK_DMA1))
        sysClockEnable(CLK_DMA1);
      break;
    }
#endif

#ifdef CONFIG_PLATFORM_STM32_DMA2
    case 1:
    {
      assert(number < DMA2_STREAM_COUNT);
      stream->reg = STM_DMA2->CHANNELS + number;

      if (number >= 2)
        stream->irq = DMA1_CHANNEL4_7_DMA2_CHANNEL3_5_IRQ;
      else
        stream->irq = DMA1_CHANNEL2_3_DMA2_CHANNEL1_2_IRQ;

      if (!sysClockStatus(CLK_DMA2))
        sysClockEnable(CLK_DMA2);
      break;
    }
#endif

    default:
      return E_VALUE;
  }

  stream->config = CCR_PL(config->priority);
  stream->handler = 0;
  stream->number = config->stream;

  if (!irqStatus(stream->irq))
  {
    irqSetPriority(stream->irq, CONFIG_PLATFORM_STM32_DMA_PRIORITY);
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
