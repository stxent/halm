/*
 * ssp_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/ssp_base.h>
#include <halm/platform/lpc/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define CHANNEL_COUNT                   4
#define CHANNEL_INDEX(channel, index)   ((channel) * CHANNEL_COUNT + (index))
#define CHANNEL_MISO(channel)           ((channel) * CHANNEL_COUNT + 0)
#define CHANNEL_MOSI(channel)           ((channel) * CHANNEL_COUNT + 1)
#define CHANNEL_SCK(channel)            ((channel) * CHANNEL_COUNT + 2)
#define CHANNEL_SSEL(channel)           ((channel) * CHANNEL_COUNT + 3)
/*----------------------------------------------------------------------------*/
struct SspBlockDescriptor
{
  LPC_SSP_Type *reg;
  /* Peripheral clock branch */
  enum SysClockBranch peripheralBranch;
  /* Clock to register interface */
  enum SysClockBranch registerBranch;
  /* Reset control identifier */
  enum SysBlockReset reset;
  /* Peripheral interrupt request identifier */
  IrqNumber irq;
};
/*----------------------------------------------------------------------------*/
static void configPins(const struct SspBaseConfig *);
static bool setInstance(uint8_t, struct SspBase *);
/*----------------------------------------------------------------------------*/
static enum Result sspInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_SSP_NO_DEINIT
static void sspDeinit(void *);
#else
#define sspDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const SspBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = sspInit,
    .deinit = sspDeinit
};
/*----------------------------------------------------------------------------*/
static const struct SspBlockDescriptor sspBlockEntries[] = {
    {
        .reg = LPC_SSP0,
        .registerBranch = CLK_M4_SSP0,
        .peripheralBranch = CLK_APB0_SSP0,
        .reset = RST_SSP0,
        .irq = SSP0_IRQ
    }, {
        .reg = LPC_SSP1,
        .registerBranch = CLK_M4_SSP1,
        .peripheralBranch = CLK_APB2_SSP1,
        .reset = RST_SSP1,
        .irq = SSP1_IRQ
    }
};
/*----------------------------------------------------------------------------*/
const struct PinEntry sspPins[] = {
    {
        .key = PIN(PORT_0, 0), /* SSP1_MISO */
        .channel = CHANNEL_MISO(1),
        .value = 1
    }, {
        .key = PIN(PORT_0, 1), /* SSP1_MOSI */
        .channel = CHANNEL_MOSI(1),
        .value = 1
    }, {
        .key = PIN(PORT_1, 0), /* SSP0_SSEL */
        .channel = CHANNEL_SSEL(0),
        .value = 5
    }, {
        .key = PIN(PORT_1, 1), /* SSP0_MISO */
        .channel = CHANNEL_MISO(0),
        .value = 5
    }, {
        .key = PIN(PORT_1, 2), /* SSP0_MOSI */
        .channel = CHANNEL_MOSI(0),
        .value = 5
    }, {
        .key = PIN(PORT_1, 3), /* SSP1_MISO */
        .channel = CHANNEL_MISO(1),
        .value = 5
    }, {
        .key = PIN(PORT_1, 4), /* SSP1_MOSI */
        .channel = CHANNEL_MOSI(1),
        .value = 5
    }, {
        .key = PIN(PORT_1, 5), /* SSP1_SSEL */
        .channel = CHANNEL_SSEL(1),
        .value = 5
    }, {
        .key = PIN(PORT_1, 19), /* SSP1_SCK */
        .channel = CHANNEL_SCK(1),
        .value = 1
    }, {
        .key = PIN(PORT_1, 20), /* SSP1_SSEL */
        .channel = CHANNEL_SSEL(1),
        .value = 1
    }, {
        .key = PIN(PORT_3, 0), /* SSP0_SCK */
        .channel = CHANNEL_SCK(0),
        .value = 5
    }, {
        .key = PIN(PORT_3, 3), /* SSP0_SCK */
        .channel = CHANNEL_SCK(0),
        .value = 2
    }, {
        .key = PIN(PORT_3, 6), /* SSP0_MISO */
        .channel = CHANNEL_MISO(0),
        .value = 5
    }, {
        .key = PIN(PORT_3, 6), /* SSP0_SSEL */
        .channel = CHANNEL_SSEL(0),
        .value = 2
    }, {
        .key = PIN(PORT_3, 7), /* SSP0_MISO */
        .channel = CHANNEL_MISO(0),
        .value = 2
    }, {
        .key = PIN(PORT_3, 7), /* SSP0_MOSI */
        .channel = CHANNEL_MOSI(0),
        .value = 5
    }, {
        .key = PIN(PORT_3, 8), /* SSP0_MOSI */
        .channel = CHANNEL_MOSI(0),
        .value = 2
    }, {
        .key = PIN(PORT_3, 8), /* SSP0_SSEL */
        .channel = CHANNEL_SSEL(0),
        .value = 5
    }, {
        .key = PIN(PORT_9, 0), /* SSP0_SSEL */
        .channel = CHANNEL_SSEL(0),
        .value = 7
    }, {
        .key = PIN(PORT_9, 1), /* SSP0_MISO */
        .channel = CHANNEL_MISO(0),
        .value = 7
    }, {
        .key = PIN(PORT_9, 2), /* SSP0_MOSI */
        .channel = CHANNEL_MOSI(0),
        .value = 7
    }, {
        .key = PIN(PORT_F, 0), /* SSP0_SCK */
        .channel = CHANNEL_SCK(0),
        .value = 0
    }, {
        .key = PIN(PORT_F, 1), /* SSP0_SSEL */
        .channel = CHANNEL_SSEL(0),
        .value = 2
    }, {
        .key = PIN(PORT_F, 2), /* SSP0_MISO */
        .channel = CHANNEL_MISO(0),
        .value = 2
    }, {
        .key = PIN(PORT_F, 3), /* SSP0_MOSI */
        .channel = CHANNEL_MOSI(0),
        .value = 2
    }, {
        .key = PIN(PORT_F, 4), /* SSP1_SCK */
        .channel = CHANNEL_SCK(1),
        .value = 0
    }, {
        .key = PIN(PORT_F, 5), /* SSP1_SSEL */
        .channel = CHANNEL_SSEL(1),
        .value = 2
    }, {
        .key = PIN(PORT_F, 6), /* SSP1_MISO */
        .channel = CHANNEL_MISO(1),
        .value = 2
    }, {
        .key = PIN(PORT_F, 7), /* SSP1_MOSI */
        .channel = CHANNEL_MOSI(1),
        .value = 2
    }, {
        .key = PIN(PORT_CLK, 0), /* SSP1_SCK */
        .channel = CHANNEL_SCK(1),
        .value = 6
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct SspBase *instances[2] = {NULL};
/*----------------------------------------------------------------------------*/
static void configPins(const struct SspBaseConfig *config)
{
  const PinNumber pinArray[] = {
      config->miso, config->mosi, config->sck, config->cs
  };

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      const struct PinEntry * const pinEntry = pinFind(sspPins, pinArray[index],
          CHANNEL_INDEX(config->channel, index));
      assert(pinEntry != NULL);

      const struct Pin pin = pinInit(pinArray[index]);

      pinInput(pin);
      pinSetFunction(pin, pinEntry->value);
    }
  }
}
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct SspBase *object)
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
void SSP0_ISR(void)
{
  /* In M0 cores SSP0 IRQ is combined with SSP1 IRQ */
  if (instances[0]->handler != NULL)
    instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void SSP1_ISR(void)
{
  /* In M0 cores SSP1 IRQ is combined with SSP0 IRQ */
  if (instances[1]->handler != NULL)
    instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
uint32_t sspGetClock(const struct SspBase *interface)
{
  return clockFrequency(interface->channel == 0 ? Ssp0Clock : Ssp1Clock);
}
/*----------------------------------------------------------------------------*/
static enum Result sspInit(void *object, const void *configBase)
{
  const struct SspBaseConfig * const config = configBase;
  struct SspBase * const interface = object;

  if (!setInstance(config->channel, interface))
    return E_BUSY;

  /*
   * Pin configuration is not similar to other device families due
   * to the differences in the pin tables. One pin may have more than two
   * alternate functions of the same peripheral channel.
   */
  configPins(config);

  const struct SspBlockDescriptor * const entry =
      &sspBlockEntries[config->channel];

  /* Enable clocks to register interface and peripheral */
  sysClockEnable(entry->peripheralBranch);
  sysClockEnable(entry->registerBranch);
  /* Reset registers to default values */
  sysResetEnable(entry->reset);

  interface->channel = config->channel;
  interface->handler = NULL;
  interface->irq = entry->irq;
  interface->reg = entry->reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_SSP_NO_DEINIT
static void sspDeinit(void *object)
{
  const struct SspBase * const interface = object;
  const struct SspBlockDescriptor * const entry =
      &sspBlockEntries[interface->channel];

  sysClockDisable(entry->registerBranch);
  sysClockDisable(entry->peripheralBranch);

  instances[interface->channel] = NULL;
}
#endif
