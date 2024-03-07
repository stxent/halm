/*
 * spi_base.c
 * Copyright (C) 2020, 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/bdma_oneshot.h>
#include <halm/platform/stm32/clocking.h>
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
    }
#endif
};
/*----------------------------------------------------------------------------*/
const struct PinEntry spiPins[] = {
#ifdef CONFIG_PLATFORM_STM32_SPI1
    {
        .key = PIN(PORT_A, 4), /* SPI1_NSS */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_A, 5), /* SPI1_SCK */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_A, 6), /* SPI1_MISO */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_A, 7), /* SPI1_MOSI */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_A, 15), /* SPI1_NSS */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_B, 3), /* SPI1_SCK */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_B, 4), /* SPI1_MISO */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_B, 5), /* SPI1_MOSI */
        .channel = 0,
        .value = 0
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_SPI2
    {
        .key = PIN(PORT_B, 9), /* SPI2_NSS */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_B, 10), /* SPI2_SCK */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_B, 12), /* SPI2_NSS */
        .channel = 1,
        .value = 0
    }, {
        .key = PIN(PORT_B, 13), /* SPI2_SCK */
        .channel = 1,
        .value = 0
    }, {
        .key = PIN(PORT_B, 14), /* SPI2_MISO */
        .channel = 1,
        .value = 0
    }, {
        .key = PIN(PORT_B, 15), /* SPI2_MOSI */
        .channel = 1,
        .value = 0
    }, {
        .key = PIN(PORT_C, 2), /* SPI2_MISO */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(PORT_C, 3), /* SPI2_MOSI */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(PORT_D, 0), /* SPI2_NSS */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(PORT_D, 1), /* SPI2_SCK */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(PORT_D, 3), /* SPI2_MISO */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(PORT_D, 4), /* SPI2_MOSI */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(PORT_E, 12), /* SPI2_NSS */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(PORT_E, 13), /* SPI2_SCK */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(PORT_E, 14), /* SPI2_MISO */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(PORT_E, 15), /* SPI2_MOSI */
        .channel = 1,
        .value = 1
    },
#endif
    {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct SpiBase *instances[2] = {NULL};
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
uint32_t spiGetClock([[maybe_unused]] const struct SpiBase *interface)
{
  return clockFrequency(ApbClock);
}
/*----------------------------------------------------------------------------*/
void *spiMakeOneShotDma([[maybe_unused]] uint8_t channel, uint8_t stream,
    enum DmaPriority priority, enum DmaType type)
{
  const struct BdmaOneShotConfig config = {
      .event = DMA_GENERIC,
      .priority = priority,
      .type = type,
      .stream = stream
  };

  return init(BdmaOneShot, &config);
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
  sysResetEnable(entry->reset);
  sysResetDisable(entry->reset);

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
