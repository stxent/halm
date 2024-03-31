/*
 * dma_base.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/irq.h>
#include <halm/platform/platform_defs.h>
#include <halm/platform/stm32/dma_base.h>
#include <halm/platform/stm32/dma_defs.h>
#include <halm/platform/stm32/system.h>
#include <xcore/atomic.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define EVENT_COUNT   8
#define EVENT_SOURCES 8
#define STREAM_COUNT  8
#define STREAM_ENCODE(controller, stream) \
    ((controller) * STREAM_COUNT + (stream))
/*----------------------------------------------------------------------------*/
static uint8_t findChannelByEvent(unsigned int, unsigned int, enum DmaEvent);
static inline void genericStreamHandler(unsigned int, unsigned int,
    volatile uint32_t *, volatile uint32_t *);
static enum Result streamInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const DmaBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = streamInit,
    .deinit = NULL /* Default destructor */
};
/*----------------------------------------------------------------------------*/
static const enum DmaEvent eventMap1[EVENT_COUNT][EVENT_SOURCES] = {
    {
        DMA_SPI3_RX,            DMA_I2C1_RX,
        DMA_TIM4_CH1,           DMA_I2S3EXT_RX,
        DMA_UART5_RX,           DMA_EVENT_END,
        DMA_TIM5_CH3_UP,        DMA_EVENT_END
    }, {
        DMA_EVENT_END,          DMA_EVENT_END,
        DMA_EVENT_END,          DMA_TIM2_CH3_UP,
        DMA_USART3_RX,          DMA_EVENT_END,
        DMA_TIM5_CH4_TRIG,      DMA_TIM6_UP
    }, {
        DMA_SPI3_RX,            DMA_TIM7_UP,
        DMA_I2S2EXT_RX,         DMA_I2C3_RX,
        DMA_UART4_RX,           DMA_TIM3_CH4_UP,
        DMA_TIM5_CH1,           DMA_I2C2_RX
    }, {
        DMA_SPI2_RX,            DMA_EVENT_END,
        DMA_TIM4_CH2,           DMA_I2S2EXT_RX,
        DMA_USART3_TX,          DMA_EVENT_END,
        DMA_TIM5_CH4_TRIG,      DMA_I2C2_RX
    }, {
        DMA_SPI2_TX,            DMA_TIM7_UP,
        DMA_I2S2EXT_TX,         DMA_I2C3_TX,
        DMA_UART4_TX,           DMA_TIM3_CH1_TRIG,
        DMA_TIM5_CH2,           DMA_USART3_TX
    }, {
        DMA_SPI3_TX,            DMA_I2C1_RX,
        DMA_I2S3EXT_TX,         DMA_TIM2_CH1,
        DMA_USART2_RX,          DMA_TIM3_CH2,
        DMA_EVENT_END,          DMA_DAC1
    }, {
        DMA_EVENT_END,          DMA_I2C1_TX,
        DMA_TIM4_UP,            DMA_TIM2_CH2_CH4,
        DMA_USART2_TX,          DMA_EVENT_END,
        DMA_TIM5_UP,            DMA_DAC2
    }, {
        DMA_SPI3_TX,            DMA_I2C1_TX,
        DMA_TIM4_CH3,           DMA_TIM2_CH4_UP,
        DMA_UART5_TX,           DMA_TIM3_CH3,
        DMA_EVENT_END,          DMA_I2C2_TX
    }
};

static const enum DmaEvent eventMap2[EVENT_COUNT][EVENT_SOURCES] = {
    {
        DMA_ADC1,               DMA_EVENT_END,
        DMA_ADC3,               DMA_SPI1_RX,
        DMA_EVENT_END,          DMA_EVENT_END,
        DMA_TIM1_TRIG,          DMA_EVENT_END
    }, {
        DMA_EVENT_END,          DMA_DCMI,
        DMA_ADC3,               DMA_EVENT_END,
        DMA_EVENT_END,          DMA_USART6_RX,
        DMA_TIM1_CH1,           DMA_TIM8_UP
    }, {
        DMA_TIM8_CH1_CH2_CH3,   DMA_ADC2,
        DMA_EVENT_END,          DMA_SPI1_RX,
        DMA_USART1_RX,          DMA_USART6_RX,
        DMA_TIM1_CH2,           DMA_TIM8_CH1
    }, {
        DMA_EVENT_END,          DMA_ADC2,
        DMA_EVENT_END,          DMA_SPI1_TX,
        DMA_SDIO,               DMA_EVENT_END,
        DMA_TIM1_CH1,           DMA_TIM8_CH2
    }, {
        DMA_ADC1,               DMA_EVENT_END,
        DMA_EVENT_END,          DMA_EVENT_END,
        DMA_EVENT_END,          DMA_EVENT_END,
        DMA_TIM1_CH4_TRIG_COM,  DMA_TIM8_CH3
    }, {
        DMA_EVENT_END,          DMA_EVENT_END,
        DMA_CRYP_OUT,           DMA_SPI1_TX,
        DMA_USART1_RX,          DMA_EVENT_END,
        DMA_TIM1_UP,            DMA_EVENT_END
    }, {
        DMA_TIM1_CH1_CH2_CH3,   DMA_EVENT_END,
        DMA_CRYP_IN,            DMA_EVENT_END,
        DMA_SDIO,               DMA_USART6_TX,
        DMA_TIM1_CH3,           DMA_EVENT_END
    }, {
        DMA_EVENT_END,          DMA_DCMI,
        DMA_HASH_IN,            DMA_EVENT_END,
        DMA_USART1_TX,          DMA_USART6_TX,
        DMA_EVENT_END,          DMA_TIM8_CH4_TRIG_COM
    }
};

static struct DmaBase *instances[STREAM_COUNT * 2] = {NULL};
/*----------------------------------------------------------------------------*/
void dmaResetInstance(uint8_t number)
{
  assert(number < ARRAY_SIZE(instances));
  instances[number] = NULL;
}
/*----------------------------------------------------------------------------*/
bool dmaSetInstance(uint8_t number, struct DmaBase *stream)
{
  assert(stream != NULL);
  assert(number < ARRAY_SIZE(instances));

  void *expected = NULL;

  if (compareExchangePointer(&instances[number], &expected, stream))
  {
    STM_DMA_Type * const dma = number < STREAM_COUNT ? STM_DMA1 : STM_DMA2;
    volatile uint32_t * const ifcr = &dma->IFCR[(number & 4) ? 1 : 0];

    /* Clear pending interrupt flags */
    *ifcr = ISR_CHANNEL_MASK(ISR_CHANNEL_OFFSET(number));

    return true;
  }

  return false;
}
/*----------------------------------------------------------------------------*/
void DMA1_STREAM0_ISR(void)
{
  genericStreamHandler(0, 0, &STM_DMA1->LISR, &STM_DMA1->LIFCR);
}
/*----------------------------------------------------------------------------*/
void DMA1_STREAM1_ISR(void)
{
  genericStreamHandler(0, 1, &STM_DMA1->LISR, &STM_DMA1->LIFCR);
}
/*----------------------------------------------------------------------------*/
void DMA1_STREAM2_ISR(void)
{
  genericStreamHandler(0, 2, &STM_DMA1->LISR, &STM_DMA1->LIFCR);
}
/*----------------------------------------------------------------------------*/
void DMA1_STREAM3_ISR(void)
{
  genericStreamHandler(0, 3, &STM_DMA1->LISR, &STM_DMA1->LIFCR);
}
/*----------------------------------------------------------------------------*/
void DMA1_STREAM4_ISR(void)
{
  genericStreamHandler(0, 4, &STM_DMA1->HISR, &STM_DMA1->HIFCR);
}
/*----------------------------------------------------------------------------*/
void DMA1_STREAM5_ISR(void)
{
  genericStreamHandler(0, 5, &STM_DMA1->HISR, &STM_DMA1->HIFCR);
}
/*----------------------------------------------------------------------------*/
void DMA1_STREAM6_ISR(void)
{
  genericStreamHandler(0, 6, &STM_DMA1->HISR, &STM_DMA1->HIFCR);
}
/*----------------------------------------------------------------------------*/
void DMA1_STREAM7_ISR(void)
{
  genericStreamHandler(0, 7, &STM_DMA1->HISR, &STM_DMA1->HIFCR);
}
/*----------------------------------------------------------------------------*/
void DMA2_STREAM0_ISR(void)
{
  genericStreamHandler(1, 0, &STM_DMA2->LISR, &STM_DMA2->LIFCR);
}
/*----------------------------------------------------------------------------*/
void DMA2_STREAM1_ISR(void)
{
  genericStreamHandler(1, 1, &STM_DMA2->LISR, &STM_DMA2->LIFCR);
}
/*----------------------------------------------------------------------------*/
void DMA2_STREAM2_ISR(void)
{
  genericStreamHandler(1, 2, &STM_DMA2->LISR, &STM_DMA2->LIFCR);
}
/*----------------------------------------------------------------------------*/
void DMA2_STREAM3_ISR(void)
{
  genericStreamHandler(1, 3, &STM_DMA2->LISR, &STM_DMA2->LIFCR);
}
/*----------------------------------------------------------------------------*/
void DMA2_STREAM4_ISR(void)
{
  genericStreamHandler(1, 4, &STM_DMA2->HISR, &STM_DMA2->HIFCR);
}
/*----------------------------------------------------------------------------*/
void DMA2_STREAM5_ISR(void)
{
  genericStreamHandler(1, 5, &STM_DMA2->HISR, &STM_DMA2->HIFCR);
}
/*----------------------------------------------------------------------------*/
void DMA2_STREAM6_ISR(void)
{
  genericStreamHandler(1, 6, &STM_DMA2->HISR, &STM_DMA2->HIFCR);
}
/*----------------------------------------------------------------------------*/
void DMA2_STREAM7_ISR(void)
{
  genericStreamHandler(1, 7, &STM_DMA2->HISR, &STM_DMA2->HIFCR);
}
/*----------------------------------------------------------------------------*/
static uint8_t findChannelByEvent(unsigned int controller, unsigned int index,
    enum DmaEvent event)
{
  if (event == DMA_MEMORY)
    return 0;

  const enum DmaEvent * const row = controller ?
      eventMap2[index] : eventMap1[index];
  uint8_t channel = UINT8_MAX;

  for (size_t position = 0; position < EVENT_SOURCES; ++position)
  {
    if (row[position] == event)
    {
      channel = position;
      break;
    }
  }

  assert(channel != UINT8_MAX);
  return channel;
}
/*----------------------------------------------------------------------------*/
static inline void genericStreamHandler(unsigned int controller,
    unsigned int index, volatile uint32_t *isr, volatile uint32_t *ifcr)
{
  struct DmaBase * const stream = instances[STREAM_ENCODE(controller, index)];
  const uint32_t offset = ISR_CHANNEL_OFFSET(index);
  const uint32_t raw = *isr & ISR_CHANNEL_MASK(offset);
  const uint32_t status = ISR_CHANNEL_VALUE(raw, offset);
  enum Result res = E_OK;

  /* Clear interrupt flags */
  *ifcr = raw;

  if (!(status & ISR_TCIF_GENERIC))
  {
    if (status & (ISR_FEIF_GENERIC | ISR_DMEIF_GENERIC | ISR_TEIF_GENERIC))
      res = E_ERROR;
    else
      res = E_BUSY;
  }

  stream->handler(stream, res);
}
/*----------------------------------------------------------------------------*/
static enum Result streamInit(void *object, const void *configBase)
{
  const struct DmaBaseConfig * const config = configBase;
  struct DmaBase * const stream = object;

  assert(config->stream < ARRAY_SIZE(instances));
  assert(config->priority <= DMA_PRIORITY_VERY_HIGH);
  assert(config->type <= DMA_TYPE_M2M);

  const unsigned int controller = config->stream >= STREAM_COUNT;
  const unsigned int index = controller ?
      config->stream - STREAM_COUNT : config->stream;
  const uint8_t channel = findChannelByEvent(controller, index, config->event);

  if (controller == 0)
  {
    /* DMA1 controller is not able to perform memory-to-memory transfers */
    assert(config->type != DMA_TYPE_M2M);

    stream->reg = STM_DMA1->STREAMS + index;

    if (index < 7)
      stream->irq = DMA1_STREAM0_IRQ + index;
    else
      stream->irq = DMA1_STREAM7_IRQ;

    if (!sysClockStatus(CLK_DMA1))
      sysClockEnable(CLK_DMA1);
  }
  else
  {
    stream->reg = STM_DMA2->STREAMS + index;

    if (index < 5)
      stream->irq = DMA2_STREAM0_IRQ + index;
    else
      stream->irq = DMA2_STREAM5_IRQ + (index - 5);

    if (!sysClockStatus(CLK_DMA2))
      sysClockEnable(CLK_DMA2);
  }

  stream->config = SCR_DIR(config->type) | SCR_PL(config->priority)
      | SCR_CHSEL(channel);
  if (config->event == DMA_SDIO)
    stream->config |= SCR_PFCTRL;

  stream->handler = NULL;
  stream->number = config->stream;

  if (!irqStatus(stream->irq))
  {
    irqSetPriority(stream->irq, CONFIG_PLATFORM_STM32_DMA_PRIORITY);
    irqEnable(stream->irq);
  }

  return E_OK;
}
