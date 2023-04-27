/*
 * spi_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/clocking.h>
#include <halm/platform/numicro/spi_base.h>
#include <halm/platform/numicro/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct SpiBlockDescriptor
{
  NM_SPI_Type *reg;
  /* Clock branch identifier */
  enum SysClockBranch branch;
  /* Reset control identifier */
  enum SysBlockReset reset;
  /* Peripheral interrupt request identifier */
  IrqNumber irq;
};
/*----------------------------------------------------------------------------*/
static uint8_t channelToIndex(uint8_t);
static bool setInstance(uint8_t, struct SpiBase *);
/*----------------------------------------------------------------------------*/
static enum Result spiInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NUMICRO_SPI_NO_DEINIT
static void spiDeinit(void *);
#else
#define spiDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const SpiBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = spiInit,
    .deinit = spiDeinit
};
/*----------------------------------------------------------------------------*/
static const struct SpiBlockDescriptor spiBlockEntries[] = {
#ifdef CONFIG_PLATFORM_NUMICRO_SPI0
    {
        .reg = NM_SPI0,
        .branch = CLK_SPI0,
        .reset = RST_SPI0,
        .irq = SPI0_IRQ
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_SPI1
    {
        .reg = NM_SPI1,
        .branch = CLK_SPI1,
        .reset = RST_SPI1,
        .irq = SPI1_IRQ
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_SPI2
    {
        .reg = NM_SPI2,
        .branch = CLK_SPI2,
        .reset = RST_SPI2,
        .irq = SPI2_IRQ
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_SPI3
    {
        .reg = NM_SPI3,
        .branch = CLK_SPI3,
        .reset = RST_SPI3,
        .irq = SPI3_IRQ
    }
#endif
};
/*----------------------------------------------------------------------------*/
const struct PinEntry spiPins[] = {
#ifdef CONFIG_PLATFORM_NUMICRO_SPI0
    /* SPI0_CLK */
    {
        .key = PIN(PORT_A, 2), /* SPI0_CLK */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_B, 14), /* SPI0_CLK */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_D, 2), /* SPI0_CLK */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_F, 8), /* SPI0_CLK */
        .channel = 0,
        .value = 5
    },
    /* SPI0_I2SMCLK */
    {
        .key = PIN(PORT_A, 4), /* SPI0_I2SMCLK */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_B, 0), /* SPI0_I2SMCLK */
        .channel = 0,
        .value = 8
    }, {
        .key = PIN(PORT_B, 11), /* SPI0_I2SMCLK */
        .channel = 0,
        .value = 9
    }, {
        .key = PIN(PORT_C, 14), /* SPI0_I2SMCLK */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_D, 13), /* SPI0_I2SMCLK */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_D, 14), /* SPI0_I2SMCLK on M48xxE8AE/M48xxGCAE */
        .channel = 0,
        .value = 5
    }, {
        .key = PIN(PORT_F, 10), /* SPI0_I2SMCLK */
        .channel = 0,
        .value = 5
    },
    /* SPI0_MISO */
    {
        .key = PIN(PORT_A, 1), /* SPI0_MISO */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_B, 13), /* SPI0_MISO */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_D, 1), /* SPI0_MISO */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_F, 7), /* SPI0_MISO */
        .channel = 0,
        .value = 5
    },
    /* SPI0_MOSI */
    {
        .key = PIN(PORT_A, 0), /* SPI0_MOSI */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_B, 12), /* SPI0_MOSI */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_D, 0), /* SPI0_MOSI */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_F, 6), /* SPI0_MOSI */
        .channel = 0,
        .value = 5
    },
    /* SPI0_SS */
    {
        .key = PIN(PORT_A, 3), /* SPI0_SS */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_B, 15), /* SPI0_SS */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_D, 3), /* SPI0_SS */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_F, 9), /* SPI0_SS */
        .channel = 0,
        .value = 5
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_SPI1
    /* SPI1_CLK */
    {
        .key = PIN(PORT_A, 7), /* SPI1_CLK */
        .channel = 1,
        .value = 4
    }, {
        .key = PIN(PORT_B, 3), /* SPI1_CLK */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_C, 1), /* SPI1_CLK */
        .channel = 1,
        .value = 7
    }, {
        .key = PIN(PORT_D, 5), /* SPI1_CLK on M48xxIDAE */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_H, 6), /* SPI1_CLK */
        .channel = 1,
        .value = 3
    }, {
        .key = PIN(PORT_H, 8), /* SPI1_CLK */
        .channel = 1,
        .value = 6
    },
    /* SPI1_I2SMCLK */
    {
        .key = PIN(PORT_A, 5), /* SPI1_I2SMCLK */
        .channel = 1,
        .value = 4
    }, {
        .key = PIN(PORT_B, 1), /* SPI1_I2SMCLK */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_C, 4), /* SPI1_I2SMCLK */
        .channel = 1,
        .value = 7
    }, {
        .key = PIN(PORT_D, 13), /* SPI1_I2SMCLK */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_H, 3), /* SPI1_I2SMCLK on M48xxIDAE */
        .channel = 1,
        .value = 3
    }, {
        .key = PIN(PORT_H, 10), /* SPI1_I2SMCLK */
        .channel = 1,
        .value = 6
    },
    /* SPI1_MISO */
    {
        .key = PIN(PORT_B, 5), /* SPI1_MISO */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_C, 3), /* SPI1_MISO */
        .channel = 1,
        .value = 7
    }, {
        .key = PIN(PORT_C, 7), /* SPI1_MISO */
        .channel = 1,
        .value = 4
    }, {
        .key = PIN(PORT_D, 6), /* SPI1_MISO on M48xxIDAE */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_E, 1), /* SPI1_MISO */
        .channel = 1,
        .value = 6
    }, {
        .key = PIN(PORT_H, 4), /* SPI1_MISO */
        .channel = 1,
        .value = 3
    },
    /* SPI1_MOSI */
    {
        .key = PIN(PORT_B, 4), /* SPI1_MOSI */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_C, 2), /* SPI1_MOSI */
        .channel = 1,
        .value = 7
    }, {
        .key = PIN(PORT_C, 6), /* SPI1_MOSI */
        .channel = 1,
        .value = 4
    }, {
        .key = PIN(PORT_D, 4), /* SPI1_MOSI on M48xxIDAE */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_E, 0), /* SPI1_MOSI */
        .channel = 1,
        .value = 6
    }, {
        .key = PIN(PORT_H, 5), /* SPI1_MOSI */
        .channel = 1,
        .value = 3
    },
    /* SPI1_SS */
    {
        .key = PIN(PORT_A, 6), /* SPI1_SS */
        .channel = 1,
        .value = 4
    }, {
        .key = PIN(PORT_B, 2), /* SPI1_SS */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_C, 0), /* SPI1_SS */
        .channel = 1,
        .value = 7
    }, {
        .key = PIN(PORT_D, 4), /* SPI1_SS on M48xxIDAE */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_H, 7), /* SPI1_SS */
        .channel = 1,
        .value = 3
    }, {
        .key = PIN(PORT_H, 9), /* SPI1_SS */
        .channel = 1,
        .value = 6
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_SPI2
    /* SPI2_CLK */
    {
        .key = PIN(PORT_A, 10), /* SPI2_CLK */
        .channel = 2,
        .value = 4
    }, {
        .key = PIN(PORT_A, 13), /* SPI2_CLK */
        .channel = 2,
        .value = 5
    }, {
        .key = PIN(PORT_E, 8), /* SPI2_CLK */
        .channel = 2,
        .value = 5
    }, {
        .key = PIN(PORT_G, 3), /* SPI2_CLK */
        .channel = 2,
        .value = 3
    },
    /* SPI2_I2SMCLK */
    {
        .key = PIN(PORT_B, 0), /* SPI2_I2SMCLK */
        .channel = 2,
        .value = 4
    }, {
        .key = PIN(PORT_C, 13), /* SPI2_I2SMCLK */
        .channel = 2,
        .value = 4
    }, {
        .key = PIN(PORT_E, 12), /* SPI2_I2SMCLK */
        .channel = 2,
        .value = 5
    },
    /* SPI2_MISO */
    {
        .key = PIN(PORT_A, 9), /* SPI2_MISO */
        .channel = 2,
        .value = 4
    }, {
        .key = PIN(PORT_A, 14), /* SPI2_MISO */
        .channel = 2,
        .value = 5
    }, {
        .key = PIN(PORT_E, 9), /* SPI2_MISO */
        .channel = 2,
        .value = 5
    }, {
        .key = PIN(PORT_G, 4), /* SPI2_MISO */
        .channel = 2,
        .value = 3
    },
    /* SPI2_MOSI */
    {
        .key = PIN(PORT_A, 8), /* SPI2_MOSI */
        .channel = 2,
        .value = 4
    }, {
        .key = PIN(PORT_A, 15), /* SPI2_MOSI */
        .channel = 2,
        .value = 5
    }, {
        .key = PIN(PORT_E, 10), /* SPI2_MOSI */
        .channel = 2,
        .value = 5
    }, {
        .key = PIN(PORT_F, 11), /* SPI2_MOSI */
        .channel = 2,
        .value = 3
    },
    /* SPI2_SS */
    {
        .key = PIN(PORT_A, 11), /* SPI2_SS */
        .channel = 2,
        .value = 4
    }, {
        .key = PIN(PORT_A, 12), /* SPI2_SS */
        .channel = 2,
        .value = 5
    }, {
        .key = PIN(PORT_E, 11), /* SPI2_SS */
        .channel = 2,
        .value = 5
    }, {
        .key = PIN(PORT_G, 2), /* SPI2_SS */
        .channel = 2,
        .value = 3
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_SPI3
    /* SPI3_CLK */
    {
        .key = PIN(PORT_B, 11), /* SPI3_CLK */
        .channel = 3,
        .value = 11
    }, {
        .key = PIN(PORT_C, 10), /* SPI3_CLK */
        .channel = 3,
        .value = 6
    }, {
        .key = PIN(PORT_E, 4), /* SPI3_CLK */
        .channel = 3,
        .value = 5
    }, {
        .key = PIN(PORT_G, 6), /* SPI3_CLK */
        .channel = 3,
        .value = 3
    },
    /* SPI3_I2SMCLK */
    {
        .key = PIN(PORT_B, 1), /* SPI3_I2SMCLK */
        .channel = 3,
        .value = 6
    }, {
        .key = PIN(PORT_D, 14), /* SPI3_I2SMCLK */
        .channel = 3,
        .value = 3
    }, {
        .key = PIN(PORT_E, 6), /* SPI3_I2SMCLK */
        .channel = 3,
        .value = 5
    },
    /* SPI3_MISO */
    {
        .key = PIN(PORT_B, 9), /* SPI3_MISO */
        .channel = 3,
        .value = 11
    }, {
        .key = PIN(PORT_C, 12), /* SPI3_MISO */
        .channel = 3,
        .value = 6
    }, {
        .key = PIN(PORT_E, 3), /* SPI3_MISO */
        .channel = 3,
        .value = 5
    }, {
        .key = PIN(PORT_G, 7), /* SPI3_MISO */
        .channel = 3,
        .value = 3
    },
    /* SPI3_MOSI */
    {
        .key = PIN(PORT_B, 8), /* SPI3_MOSI */
        .channel = 3,
        .value = 11
    }, {
        .key = PIN(PORT_C, 11), /* SPI3_MOSI */
        .channel = 3,
        .value = 6
    }, {
        .key = PIN(PORT_E, 2), /* SPI3_MOSI */
        .channel = 3,
        .value = 5
    }, {
        .key = PIN(PORT_G, 8), /* SPI3_MOSI */
        .channel = 3,
        .value = 3
    },
    /* SPI3_SS */
    {
        .key = PIN(PORT_B, 10), /* SPI3_SS */
        .channel = 3,
        .value = 11
    }, {
        .key = PIN(PORT_C, 9), /* SPI3_SS */
        .channel = 3,
        .value = 6
    }, {
        .key = PIN(PORT_E, 5), /* SPI3_SS */
        .channel = 3,
        .value = 5
    }, {
        .key = PIN(PORT_G, 5), /* SPI3_SS */
        .channel = 3,
        .value = 3
    },
#endif
    {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct SpiBase *instances[4] = {0};
/*----------------------------------------------------------------------------*/
static uint8_t channelToIndex(uint8_t channel)
{
  uint8_t index = 0;

#ifdef CONFIG_PLATFORM_NUMICRO_SPI0
  if (channel == 0)
    return index;
  ++index;
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_SPI1
  if (channel == 1)
    return index;
  ++index;
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_SPI2
  if (channel == 2)
    return index;
  ++index;
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_SPI3
  if (channel == 3)
    return index;
  ++index;
#endif

  return UINT8_MAX;
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
#ifdef CONFIG_PLATFORM_NUMICRO_SPI0
void SPI0_ISR(void)
{
  instances[0]->handler(instances[0]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_SPI1
void SPI1_ISR(void)
{
  instances[1]->handler(instances[1]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_SPI2
void SPI2_ISR(void)
{
  instances[2]->handler(instances[2]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_SPI3
void SPI3_ISR(void)
{
  instances[3]->handler(instances[3]);
}
#endif
/*----------------------------------------------------------------------------*/
uint32_t spiGetClock(const struct SpiBase *interface)
{
  const void *clock = NULL;

  switch (interface->channel)
  {
#ifdef CONFIG_PLATFORM_NUMICRO_SPI0
    case 0:
      clock = Spi0Clock;
      break;
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_SPI1
    case 1:
      clock = Spi1Clock;
      break;
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_SPI2
    case 2:
      clock = Spi2Clock;
      break;
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_SPI3
    case 3:
      clock = Spi3Clock;
      break;
#endif
  }

  return clockFrequency(clock);
}
/*----------------------------------------------------------------------------*/
static enum Result spiInit(void *object, const void *configBase)
{
  const struct SpiBaseConfig * const config = configBase;
  const size_t index = channelToIndex(config->channel);
  struct SpiBase * const interface = object;

  assert(index != UINT8_MAX);
  if (!setInstance(config->channel, interface))
    return E_BUSY;

  /* Configure input and output pins */
  spiConfigPins(config);

  const struct SpiBlockDescriptor * const entry = &spiBlockEntries[index];

  /* Enable clock to peripheral */
  sysClockEnable(entry->branch);
  /* Reset registers to default values */
  sysResetBlock(entry->reset);

  interface->channel = config->channel;
  interface->handler = NULL;
  interface->irq = entry->irq;
  interface->reg = entry->reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_SPI_NO_DEINIT
static void spiDeinit(void *object)
{
  const struct SpiBase * const interface = object;
  const struct SpiBlockDescriptor * const entry =
      &spiBlockEntries[channelToIndex(interface->channel)];

  sysClockDisable(entry->branch);
  instances[interface->channel] = NULL;
}
#endif
