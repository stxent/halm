/*
 * qspi_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/clocking.h>
#include <halm/platform/numicro/qspi_base.h>
#include <halm/platform/numicro/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t, struct QspiBase *);
/*----------------------------------------------------------------------------*/
static enum Result qspiInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NUMICRO_QSPI_NO_DEINIT
static void qspiDeinit(void *);
#else
#define qspiDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const QspiBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = qspiInit,
    .deinit = qspiDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinEntry qspiPins[] = {
#ifdef CONFIG_PLATFORM_NUMICRO_QSPI0
    /* QSPI0_CLK */
    {
        .key = PIN(PORT_A, 2), /* QSPI0_CLK */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_C, 2), /* QSPI0_CLK */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_C, 14), /* QSPI0_CLK */
        .channel = 0,
        .value = 6
    }, {
        .key = PIN(PORT_F, 2), /* QSPI0_CLK */
        .channel = 0,
        .value = 5
    }, {
        .key = PIN(PORT_H, 8), /* QSPI0_CLK */
        .channel = 0,
        .value = 3
    },
    /* QSPI0_MISO0 */
    {
        .key = PIN(PORT_A, 1), /* QSPI0_MISO0 */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_C, 1), /* QSPI0_MISO0 */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_E, 1), /* QSPI0_MISO0 */
        .channel = 0,
        .value = 3
    },
    /* QSPI0_MISO1 */
    {
        .key = PIN(PORT_A, 5), /* QSPI0_MISO1 */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_C, 5), /* QSPI0_MISO1 */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_H, 10), /* QSPI0_MISO1 */
        .channel = 0,
        .value = 3
    },
    /* QSPI0_MOSI0 */
    {
        .key = PIN(PORT_A, 0), /* QSPI0_MOSI0 */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_C, 0), /* QSPI0_MOSI0 */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_E, 0), /* QSPI0_MOSI0 */
        .channel = 0,
        .value = 3
    },
    /* QSPI0_MOSI1 */
    {
        .key = PIN(PORT_A, 4), /* QSPI0_MOSI1 */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_C, 4), /* QSPI0_MOSI1 */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_H, 11), /* QSPI0_MOSI1 */
        .channel = 0,
        .value = 3
    },
    /* QSPI0_SS */
    {
        .key = PIN(PORT_A, 3), /* QSPI0_SS */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_C, 3), /* QSPI0_SS */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_H, 9), /* QSPI0_SS */
        .channel = 0,
        .value = 3
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_QSPI1
    /* QSPI1_CLK */
    {
        .key = PIN(PORT_C, 4), /* QSPI1_CLK */
        .channel = 1,
        .value = 14
    }, {
        .key = PIN(PORT_G, 12), /* QSPI1_CLK */
        .channel = 1,
        .value = 5
    },
    /* QSPI1_MISO0 */
    {
        .key = PIN(PORT_C, 3), /* QSPI1_MISO0 */
        .channel = 1,
        .value = 14
    }, {
        .key = PIN(PORT_G, 13), /* QSPI1_MISO0 */
        .channel = 1,
        .value = 5
    },
    /* QSPI1_MISO1 */
    {
        .key = PIN(PORT_A, 7), /* QSPI1_MISO1 */
        .channel = 1,
        .value = 9
    }, {
        .key = PIN(PORT_G, 9), /* QSPI1_MISO1 */
        .channel = 1,
        .value = 5
    },
    /* QSPI1_MOSI0 */
    {
        .key = PIN(PORT_C, 2), /* QSPI1_MOSI0 */
        .channel = 1,
        .value = 14
    }, {
        .key = PIN(PORT_G, 14), /* QSPI1_MOSI0 */
        .channel = 1,
        .value = 5
    },
    /* QSPI1_MOSI1 */
    {
        .key = PIN(PORT_A, 6), /* QSPI1_MOSI1 */
        .channel = 1,
        .value = 9
    }, {
        .key = PIN(PORT_G, 10), /* QSPI1_MOSI1 */
        .channel = 1,
        .value = 5
    },
    /* QSPI1_SS */
    {
        .key = PIN(PORT_C, 5), /* QSPI1_SS */
        .channel = 1,
        .value = 14
    }, {
        .key = PIN(PORT_G, 11), /* QSPI1_SS */
        .channel = 1,
        .value = 5
    },
#endif
    {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct QspiBase *instances[2] = {NULL};
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct QspiBase *object)
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
#ifdef CONFIG_PLATFORM_NUMICRO_QSPI0
void QSPI0_ISR(void)
{
  instances[0]->handler(instances[0]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_QSPI1
void QSPI1_ISR(void)
{
  instances[1]->handler(instances[1]);
}
#endif
/*----------------------------------------------------------------------------*/
uint32_t qspiGetClock(const struct QspiBase *interface)
{
  return clockFrequency(interface->channel ? Qspi1Clock : Qspi0Clock);
}
/*----------------------------------------------------------------------------*/
static enum Result qspiInit(void *object, const void *configBase)
{
  const struct QspiBaseConfig * const config = configBase;
  struct QspiBase * const interface = object;
  enum SysClockBranch branch;
  enum SysBlockReset reset;

  switch (config->channel)
  {
#ifdef CONFIG_PLATFORM_NUMICRO_QSPI0
    case 0:
      branch = CLK_QSPI0;
      reset = RST_QSPI0;

      interface->irq = QSPI0_IRQ;
      interface->reg = NM_QSPI0;
      break;
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_QSPI1
    case 1:
      branch = CLK_QSPI1;
      reset = RST_QSPI1;

      interface->irq = QSPI1_IRQ;
      interface->reg = NM_QSPI1;
      break;
#endif

    default:
      return E_VALUE;
  }

  if (!setInstance(config->channel, interface))
    return E_BUSY;

  /* Configure input and output pins */
  qspiConfigPins(config);

  /* Enable clock to peripheral */
  sysClockEnable(branch);
  /* Reset registers to default values */
  sysResetBlock(reset);

  interface->channel = config->channel;
  interface->handler = NULL;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_QSPI_NO_DEINIT
static void qspiDeinit(void *object)
{
  const struct QspiBase * const interface = object;

  sysClockDisable(interface->channel ? CLK_QSPI1 : CLK_QSPI0);
  instances[interface->channel] = NULL;
}
#endif
