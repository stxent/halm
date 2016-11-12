/*
 * ssp_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/bits.h>
#include <xcore/memory.h>
#include <halm/platform/nxp/lpc43xx/clocking.h>
#include <halm/platform/nxp/lpc43xx/system.h>
#include <halm/platform/nxp/ssp_base.h>
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
  /* Peripheral interrupt request identifier */
  irqNumber irq;
  /* Reset control identifier */
  enum sysBlockReset reset;
  /* Peripheral clock branch */
  enum sysClockBranch periperalBranch;
  /* Clock to register interface */
  enum sysClockBranch registerBranch;
};
/*----------------------------------------------------------------------------*/
static void configPins(struct SspBase *, const struct SspBaseConfig *);
static bool setDescriptor(uint8_t, const struct SspBase *, struct SspBase *);
/*----------------------------------------------------------------------------*/
static enum result sspInit(void *, const void *);
static void sspDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass sspTable = {
    .size = 0, /* Abstract class */
    .init = sspInit,
    .deinit = sspDeinit
};
/*----------------------------------------------------------------------------*/
static const struct SspBlockDescriptor sspBlockEntries[] = {
    {
        .reg = LPC_SSP0,
        .irq = SSP0_IRQ,
        .registerBranch = CLK_M4_SSP0,
        .periperalBranch = CLK_APB0_SSP0,
        .reset = RST_SSP0
    },
    {
        .reg = LPC_SSP1,
        .irq = SSP1_IRQ,
        .registerBranch = CLK_M4_SSP1,
        .periperalBranch = CLK_APB2_SSP1,
        .reset = RST_SSP1
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
const struct EntityClass * const SspBase = &sspTable;
static struct SspBase *descriptors[2] = {0};
/*----------------------------------------------------------------------------*/
static void configPins(struct SspBase *interface,
    const struct SspBaseConfig *config)
{
  const pinNumber pinArray[] = {
      config->miso, config->mosi, config->sck, config->cs
  };

  for (unsigned int index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      const struct PinEntry * const pinEntry = pinFind(sspPins, pinArray[index],
          CHANNEL_INDEX(interface->channel, index));
      assert(pinEntry);

      const struct Pin pin = pinInit(pinArray[index]);

      pinInput(pin);
      pinSetFunction(pin, pinEntry->value);
    }
  }
}
/*----------------------------------------------------------------------------*/
static bool setDescriptor(uint8_t channel, const struct SspBase *state,
    struct SspBase *interface)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state,
      interface);
}
/*----------------------------------------------------------------------------*/
void SSP0_ISR(void)
{
  descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void SSP1_ISR(void)
{
  descriptors[1]->handler(descriptors[1]);
}
/*----------------------------------------------------------------------------*/
uint32_t sspGetClock(const struct SspBase *interface)
{
  const void *clock = 0;

  switch (interface->channel)
  {
    case 0:
      clock = Ssp0Clock;
      break;

    case 1:
      clock = Ssp1Clock;
      break;

    default:
      return 0;
  }

  return clockFrequency(clock);
}
/*----------------------------------------------------------------------------*/
static enum result sspInit(void *object, const void *configBase)
{
  const struct SspBaseConfig * const config = configBase;
  struct SspBase * const interface = object;

  interface->channel = config->channel;
  interface->handler = 0;

  /* Try to set peripheral descriptor */
  if (!setDescriptor(interface->channel, 0, interface))
    return E_BUSY;

  /*
   * Pin configuration is not similar to other device families due
   * to the differences in the pin tables. One pin may have more than two
   * alternate functions of the same peripheral channel.
   */
  configPins(interface, config);

  const struct SspBlockDescriptor * const entry =
      &sspBlockEntries[interface->channel];

  /* Enable clocks to register interface and peripheral */
  sysClockEnable(entry->periperalBranch);
  sysClockEnable(entry->registerBranch);
  /* Reset registers to default values */
  sysResetEnable(entry->reset);

  interface->irq = entry->irq;
  interface->reg = entry->reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sspDeinit(void *object)
{
  const struct SspBase * const interface = object;
  const struct SspBlockDescriptor * const entry =
      &sspBlockEntries[interface->channel];

  sysClockDisable(entry->registerBranch);
  sysClockDisable(entry->periperalBranch);
  setDescriptor(interface->channel, interface, 0);
}
