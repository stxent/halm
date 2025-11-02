/*
 * spi_base.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/clocking.h>
#include <halm/platform/stm32/dma_list.h>
#include <halm/platform/stm32/dma_oneshot.h>
#include <halm/platform/stm32/spi_base.h>
#include <halm/platform/stm32/spi_defs.h>
#include <halm/platform/stm32/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct SpiBlockDescriptor
{
  STM_SPI_Type *reg;
  /* Reset control identifier */
  enum SysBlockReset reset;
  /* Peripheral clock branch */
  enum SysClockBranch clock;
  /* Peripheral interrupt request identifier */
  IrqNumber irq;
  /* Peripheral channel */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
static const struct SpiBlockDescriptor *findDescriptor(uint8_t);
static bool setInstance(uint8_t, struct SpiBase *);
/*----------------------------------------------------------------------------*/
static enum Result spiInit(void *, const void *);

#ifndef CONFIG_PLATFORM_STM32_SPI_NO_DEINIT
static void spiDeinit(void *);
#else
#  define spiDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const SpiBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = spiInit,
    .deinit = spiDeinit
};
/*----------------------------------------------------------------------------*/
static const struct SpiBlockDescriptor spiBlockEntries[] = {
#ifdef CONFIG_PLATFORM_STM32_SPI1
    {
        .reg = STM_SPI1,
        .clock = CLK_SPI1,
        .reset = RST_SPI1,
        .irq = SPI1_IRQ,
        .channel = SPI1
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_SPI2
    {
        .reg = STM_SPI2,
        .clock = CLK_SPI2,
        .reset = RST_SPI2,
        .irq = SPI2_IRQ,
        .channel = SPI2
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_SPI3
    {
        .reg = STM_SPI3,
        .clock = CLK_SPI3,
        .reset = RST_SPI3,
        .irq = SPI3_IRQ,
        .channel = SPI3
    }
#endif
};
/*----------------------------------------------------------------------------*/
const struct PinEntry spiPins[] = {
#ifdef CONFIG_PLATFORM_STM32_SPI1
    {
        .key = PIN(PORT_A, 4), /* SPI1_NSS */
        .channel = 0,
        .value = 5
    }, {
        .key = PIN(PORT_A, 5), /* SPI1_SCK */
        .channel = 0,
        .value = 5
    }, {
        .key = PIN(PORT_A, 6), /* SPI1_MISO */
        .channel = 0,
        .value = 5
    }, {
        .key = PIN(PORT_A, 7), /* SPI1_MOSI */
        .channel = 0,
        .value = 5
    }, {
        .key = PIN(PORT_A, 15), /* SPI1_NSS */
        .channel = 0,
        .value = 5
    }, {
        .key = PIN(PORT_B, 3), /* SPI1_SCK */
        .channel = 0,
        .value = 5
    }, {
        .key = PIN(PORT_B, 4), /* SPI1_MISO */
        .channel = 0,
        .value = 5
    }, {
        .key = PIN(PORT_B, 5), /* SPI1_MOSI */
        .channel = 0,
        .value = 5
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_SPI2
    {
        .key = PIN(PORT_B, 9), /* SPI2_NSS/I2S2_WS */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_B, 10), /* SPI2_SCK/I2S2_CK */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_B, 12), /* SPI2_NSS/I2S2_WS */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_B, 13), /* SPI2_SCK/I2S2_CK */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_B, 14), /* SPI2_MISO */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_B, 15), /* SPI2_MOSI/I2S2_SD */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_C, 2), /* SPI2_MISO */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_C, 3), /* SPI2_MOSI/I2S2_SD */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_C, 6), /* I2S2_MCK */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_I, 0), /* SPI2_NSS/I2S2_WS */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_I, 1), /* SPI2_SCK/I2S2_CK */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_I, 2), /* SPI2_MISO */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_I, 3), /* SPI2_MOSI/I2S2_SD */
        .channel = 1,
        .value = 5
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_SPI3
    {
        .key = PIN(PORT_A, 4), /* SPI3_NSS/I2S3_WS */
        .channel = 2,
        .value = 6
    }, {
        .key = PIN(PORT_A, 15), /* SPI3_NSS/I2S3_WS */
        .channel = 2,
        .value = 6
    }, {
        .key = PIN(PORT_B, 3), /* SPI3_SCK/I2S3_CK */
        .channel = 2,
        .value = 6
    }, {
        .key = PIN(PORT_B, 4), /* SPI3_MISO */
        .channel = 2,
        .value = 6
    }, {
        .key = PIN(PORT_B, 5), /* SPI3_MOSI/I2S3_SD */
        .channel = 2,
        .value = 6
    }, {
        .key = PIN(PORT_C, 7), /* I2S3_MCK */
        .channel = 2,
        .value = 6
    }, {
        .key = PIN(PORT_C, 10), /* SPI3_SCK/I2S3_CK */
        .channel = 2,
        .value = 6
    }, {
        .key = PIN(PORT_C, 11), /* SPI3_MISO */
        .channel = 2,
        .value = 6
    }, {
        .key = PIN(PORT_C, 12), /* SPI3_MOSI/I2S3_SD */
        .channel = 2,
        .value = 6
    },
#endif
    {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct SpiBase *instances[3] = {NULL};
/*----------------------------------------------------------------------------*/
static const struct SpiBlockDescriptor *findDescriptor(uint8_t channel)
{
  for (size_t index = 0; index < ARRAY_SIZE(spiBlockEntries); ++index)
  {
    if (spiBlockEntries[index].channel == channel)
      return &spiBlockEntries[index];
  }

  return NULL;
}
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct SpiBase *object)
{
  assert(channel < ARRAY_SIZE(instances));

  if (instances[channel] == NULL)
  {
    instances[channel] = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_SPI1
void SPI1_ISR(void)
{
  instances[0]->handler(instances[0]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_SPI2
void SPI2_ISR(void)
{
  instances[1]->handler(instances[1]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_SPI3
void SPI3_ISR(void)
{
  instances[2]->handler(instances[2]);
}
#endif
/*----------------------------------------------------------------------------*/
uint32_t i2sGetClock(const struct SpiBase *)
{
  return clockFrequency(AudioPll);
}
/*----------------------------------------------------------------------------*/
void *i2sGetExtension(const struct SpiBase *interface)
{
  switch (interface->channel)
  {
    case SPI2:
      return STM_I2S2EXT;

    case SPI3:
      return STM_I2S3EXT;

    default:
      return NULL;
  }
}
/*----------------------------------------------------------------------------*/
uint32_t spiGetClock(const struct SpiBase *interface)
{
  return clockFrequency(interface->channel == SPI1 ? Apb2Clock : Apb1Clock);
}
/*----------------------------------------------------------------------------*/
void *i2sMakeListDma(uint8_t channel, uint8_t stream,
    enum DmaPriority priority, enum DmaType type, size_t number)
{
  const struct DmaListConfig config = {
      .number = number,
      .event = (type == DMA_TYPE_P2M) ?
          dmaGetEventI2SRx(channel) : dmaGetEventI2STx(channel),
      .priority = priority,
      .type = type,
      .stream = stream
  };

  return init(DmaList, &config);
}
/*----------------------------------------------------------------------------*/
void *spiMakeListDma(uint8_t channel, uint8_t stream,
    enum DmaPriority priority, enum DmaType type, size_t number)
{
  const struct DmaListConfig config = {
      .number = number,
      .event = (type == DMA_TYPE_P2M) ?
          dmaGetEventSpiRx(channel) : dmaGetEventSpiTx(channel),
      .priority = priority,
      .type = type,
      .stream = stream
  };

  return init(DmaList, &config);
}
/*----------------------------------------------------------------------------*/
void *spiMakeOneShotDma(uint8_t channel, uint8_t stream,
    enum DmaPriority priority, enum DmaType type)
{
  const struct DmaOneShotConfig config = {
      .event = (type == DMA_TYPE_P2M) ?
          dmaGetEventSpiRx(channel) : dmaGetEventSpiTx(channel),
      .priority = priority,
      .type = type,
      .stream = stream
  };

  return init(DmaOneShot, &config);
}
/*----------------------------------------------------------------------------*/
static enum Result spiInit(void *object, const void *configBase)
{
  const struct SpiBaseConfig * const config = configBase;
  struct SpiBase * const interface = object;

  const struct SpiBlockDescriptor * const entry =
      findDescriptor(config->channel);

  assert(entry != NULL);
  if (!setInstance(config->channel, interface))
    return E_BUSY;

  /* Configure input and output pins */
  spiConfigPins(config);

  /* Enable clocks to register interface and peripheral */
  sysClockEnable(entry->clock);
  /* Reset registers to default values */
  sysResetPulse(entry->reset);

  interface->channel = config->channel;
  interface->handler = NULL;
  interface->irq = entry->irq;
  interface->reg = entry->reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_SPI_NO_DEINIT
static void spiDeinit(void *object)
{
  const struct SpiBase * const interface = object;
  const struct SpiBlockDescriptor * const entry =
      findDescriptor(interface->channel);

  sysClockDisable(entry->clock);
  instances[interface->channel] = NULL;
}
#endif
