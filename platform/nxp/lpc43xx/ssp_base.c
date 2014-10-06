/*
 * ssp_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <memory.h>
#include <platform/nxp/ssp_base.h>
#include <platform/nxp/lpc43xx/clocking.h>
#include <platform/nxp/lpc43xx/system.h>
/*----------------------------------------------------------------------------*/
#define CHANNEL_RANGE                   4
#define CHANNEL_INDEX(channel, index)   ((channel) * CHANNEL_RANGE + (index))
#define CHANNEL_MISO(channel)           ((channel) * CHANNEL_RANGE + 0)
#define CHANNEL_MOSI(channel)           ((channel) * CHANNEL_RANGE + 1)
#define CHANNEL_SCK(channel)            ((channel) * CHANNEL_RANGE + 2)
#define CHANNEL_SSEL(channel)           ((channel) * CHANNEL_RANGE + 3)
/*----------------------------------------------------------------------------*/
struct SspBlockDescriptor
{
  LPC_SSP_Type *reg;
  /* Peripheral interrupt request identifier */
  irq_t irq;
  /* Reset control identifier */
  enum sysDeviceReset reset;
  /* Peripheral clock branch */
  enum sysClockBranch periperalBranch;
  /* Clock to register interface */
  enum sysClockBranch registerBranch;
};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t, const struct SspBase *,
    struct SspBase *);
static enum result setupPins(struct SspBase *, const struct SspBaseConfig *);
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
static const struct SspBlockDescriptor sspBlockEntries[2] = {
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
static enum result setDescriptor(uint8_t channel,
    const struct SspBase *state, struct SspBase *interface)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state,
      interface) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
static enum result setupPins(struct SspBase *interface,
    const struct SspBaseConfig *config)
{
  const pin_t pinArray[4] = {
      config->miso, config->mosi, config->sck, config->cs
  };
  const struct PinEntry *pinEntry;
  struct Pin pin;

  for (uint8_t index = 0; index < 4; ++index)
  {
    if (pinArray[index])
    {
      pinEntry = pinFind(sspPins, pinArray[index],
          CHANNEL_INDEX(interface->channel, index));
      if (!pinEntry)
        return E_VALUE;
      pinInput((pin = pinInit(pinArray[index])));
      pinSetFunction(pin, pinEntry->value);
    }
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
void SSP0_ISR(void)
{
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void SSP1_ISR(void)
{
  if (descriptors[1])
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
  enum result res;

  /* Try to set peripheral descriptor */
  interface->channel = config->channel;
  if ((res = setDescriptor(interface->channel, 0, interface)) != E_OK)
    return res;

  /*
   * Pin setup is not similar to other device families due to differences in
   * the pin tables. One pin may have more than two alternate functions of the
   * same peripheral channel.
   */
  if ((res = setupPins(interface, config)) != E_OK)
    return res;

  const struct SspBlockDescriptor *entry = &sspBlockEntries[interface->channel];

  /* Enable clocks to register interface and peripheral */
  sysClockEnable(entry->periperalBranch);
  sysClockEnable(entry->registerBranch);
  /* Reset registers to default values */
  sysResetEnable(entry->reset);

  interface->handler = 0;
  interface->irq = entry->irq;
  interface->reg = entry->reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sspDeinit(void *object)
{
  const struct SspBase * const interface = object;
  const struct SspBlockDescriptor *entry = &sspBlockEntries[interface->channel];

  sysClockDisable(entry->registerBranch);
  sysClockDisable(entry->periperalBranch);
  setDescriptor(interface->channel, interface, 0);
}
