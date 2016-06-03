/*
 * i2s_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/bits.h>
#include <xcore/memory.h>
#include <halm/platform/nxp/i2s_base.h>
#include <halm/platform/nxp/lpc43xx/clocking.h>
#include <halm/platform/nxp/lpc43xx/system.h>
/*----------------------------------------------------------------------------*/
#define CHANNEL_COUNT                   8
#define CHANNEL_INDEX(channel, index)   ((channel) * CHANNEL_COUNT + (index))
#define CHANNEL_RX_SCK(channel)         ((channel) * CHANNEL_COUNT + 0)
#define CHANNEL_RX_WS(channel)          ((channel) * CHANNEL_COUNT + 1)
#define CHANNEL_RX_SDA(channel)         ((channel) * CHANNEL_COUNT + 2)
#define CHANNEL_RX_MCLK(channel)        ((channel) * CHANNEL_COUNT + 3)
#define CHANNEL_TX_SCK(channel)         ((channel) * CHANNEL_COUNT + 4)
#define CHANNEL_TX_WS(channel)          ((channel) * CHANNEL_COUNT + 5)
#define CHANNEL_TX_SDA(channel)         ((channel) * CHANNEL_COUNT + 6)
#define CHANNEL_TX_MCLK(channel)        ((channel) * CHANNEL_COUNT + 7)
/*----------------------------------------------------------------------------*/
static void configPins(struct I2sBase *, const struct I2sBaseConfig *);
static enum result setDescriptor(uint8_t, const struct I2sBase *,
    struct I2sBase *);
/*----------------------------------------------------------------------------*/
static enum result i2sInit(void *, const void *);
static void i2sDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass i2sTable = {
    .size = 0, /* Abstract class */
    .init = i2sInit,
    .deinit = i2sDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinEntry i2sPins[] = {
    {
        .key = PIN(PORT_0, 0), /* I2S0_TX_WS */
        .channel = CHANNEL_TX_WS(0),
        .value = 6
    }, {
        .key = PIN(PORT_0, 0), /* I2S1_TX_WS */
        .channel = CHANNEL_TX_WS(1),
        .value = 7
    }, {
        .key = PIN(PORT_0, 1), /* I2S1_TX_SDA */
        .channel = CHANNEL_TX_SDA(1),
        .value = 7
    }, {
        .key = PIN(PORT_1, 19), /* I2S0_RX_MCLK */
        .channel = CHANNEL_RX_MCLK(0),
        .value = 6
    }, {
        .key = PIN(PORT_1, 19), /* I2S1_TX_SCK */
        .channel = CHANNEL_TX_SCK(1),
        .value = 7
    }, {
        .key = PIN(PORT_3, 0), /* I2S0_RX_SCK */
        .channel = CHANNEL_RX_SCK(0),
        .value = 0
    }, {
        .key = PIN(PORT_3, 0), /* I2S0_RX_MCLK */
        .channel = CHANNEL_RX_MCLK(0),
        .value = 1
    }, {
        .key = PIN(PORT_3, 0), /* I2S0_TX_SCK */
        .channel = CHANNEL_TX_SCK(0),
        .value = 2
    }, {
        .key = PIN(PORT_3, 0), /* I2S0_TX_MCLK */
        .channel = CHANNEL_TX_MCLK(0),
        .value = 3
    }, {
        .key = PIN(PORT_3, 1), /* I2S0_TX_WS */
        .channel = CHANNEL_TX_WS(0),
        .value = 0
    }, {
        .key = PIN(PORT_3, 1), /* I2S0_RX_WS */
        .channel = CHANNEL_RX_WS(0),
        .value = 1
    }, {
        .key = PIN(PORT_3, 2), /* I2S0_TX_SDA */
        .channel = CHANNEL_TX_SDA(0),
        .value = 0
    }, {
        .key = PIN(PORT_3, 2), /* I2S0_RX_SDA */
        .channel = CHANNEL_RX_SDA(0),
        .value = 1
    }, {
        .key = PIN(PORT_3, 3), /* I2S0_TX_MCLK */
        .channel = CHANNEL_TX_MCLK(0),
        .value = 6
    }, {
        .key = PIN(PORT_3, 3), /* I2S1_TX_SCK */
        .channel = CHANNEL_TX_SCK(1),
        .value = 7
    }, {
        .key = PIN(PORT_3, 4), /* I2S0_TX_WS */
        .channel = CHANNEL_TX_WS(0),
        .value = 5
    }, {
        .key = PIN(PORT_3, 4), /* I2S1_RX_SDA */
        .channel = CHANNEL_RX_SDA(1),
        .value = 6
    }, {
        .key = PIN(PORT_3, 5), /* I2S0_TX_SDA */
        .channel = CHANNEL_TX_SDA(0),
        .value = 5
    }, {
        .key = PIN(PORT_3, 5), /* I2S1_RX_WS */
        .channel = CHANNEL_RX_WS(1),
        .value = 6
    }, {
        .key = PIN(PORT_4, 7), /* I2S1_TX_SCK */
        .channel = CHANNEL_TX_SCK(1),
        .value = 6
    }, {
        .key = PIN(PORT_4, 7), /* I2S0_TX_SCK */
        .channel = CHANNEL_TX_SCK(0),
        .value = 7
    }, {
        .key = PIN(PORT_6, 0), /* I2S0_RX_MCLK */
        .channel = CHANNEL_RX_MCLK(0),
        .value = 1
    }, {
        .key = PIN(PORT_6, 0), /* I2S0_RX_SCK */
        .channel = CHANNEL_RX_SCK(0),
        .value = 4
    }, {
        .key = PIN(PORT_6, 1), /* I2S0_RX_WS */
        .channel = CHANNEL_RX_WS(0),
        .value = 3
    }, {
        .key = PIN(PORT_6, 2), /* I2S0_RX_SDA */
        .channel = CHANNEL_RX_SDA(0),
        .value = 3
    }, {
        .key = PIN(PORT_7, 1), /* I2S0_TX_WS */
        .channel = CHANNEL_TX_WS(0),
        .value = 2
    }, {
        .key = PIN(PORT_7, 2), /* I2S0_TX_SDA */
        .channel = CHANNEL_TX_SDA(0),
        .value = 2
    }, {
        .key = PIN(PORT_8, 8), /* I2S1_TX_MCLK */
        .channel = CHANNEL_TX_MCLK(1),
        .value = 7
    }, {
        .key = PIN(PORT_9, 1), /* I2S0_TX_WS */
        .channel = CHANNEL_TX_WS(0),
        .value = 4
    }, {
        .key = PIN(PORT_9, 2), /* I2S0_TX_SDA */
        .channel = CHANNEL_TX_SDA(0),
        .value = 4
    }, {
        .key = PIN(PORT_A, 0), /* I2S1_RX_MCLK */
        .channel = CHANNEL_RX_MCLK(1),
        .value = 5
    }, {
        .key = PIN(PORT_C, 12), /* I2S0_TX_SDA */
        .channel = CHANNEL_TX_SDA(0),
        .value = 6
    }, {
        .key = PIN(PORT_C, 13), /* I2S0_TX_WS */
        .channel = CHANNEL_TX_WS(0),
        .value = 6
    }, {
        .key = PIN(PORT_F, 0), /* I2S1_TX_MCLK */
        .channel = CHANNEL_TX_MCLK(1),
        .value = 7
    }, {
        .key = PIN(PORT_F, 4), /* I2S0_TX_MCLK */
        .channel = CHANNEL_TX_MCLK(0),
        .value = 6
    }, {
        .key = PIN(PORT_F, 4), /* I2S0_RX_SCK */
        .channel = CHANNEL_RX_SCK(0),
        .value = 7
    }, {
        .key = PIN(PORT_F, 6), /* I2S1_TX_SDA */
        .channel = CHANNEL_TX_SDA(1),
        .value = 6
    }, {
        .key = PIN(PORT_F, 7), /* I2S1_TX_WS */
        .channel = CHANNEL_TX_WS(1),
        .value = 6
    }, {
        .key = PIN(PORT_CLK, 1), /* I2S1_TX_MCLK */
        .channel = CHANNEL_TX_MCLK(1),
        .value = 7
    }, {
        .key = PIN(PORT_CLK, 2), /* I2S0_TX_MCLK */
        .channel = CHANNEL_TX_MCLK(0),
        .value = 6
    }, {
        .key = PIN(PORT_CLK, 2), /* I2S1_RX_SCK */
        .channel = CHANNEL_RX_SCK(1),
        .value = 7
    }, {
        .key = PIN(PORT_CLK, 3), /* I2S1_RX_SCK */
        .channel = CHANNEL_RX_SCK(1),
        .value = 7
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const I2sBase = &i2sTable;
static struct I2sBase *descriptors[2] = {0};
/*----------------------------------------------------------------------------*/
static void configPins(struct I2sBase *interface,
    const struct I2sBaseConfig *config)
{
  const pinNumber pinArray[] = {
      config->rx.sck, config->rx.ws, config->rx.sda, config->rx.mclk,
      config->tx.sck, config->tx.ws, config->tx.sda, config->tx.mclk
  };

  for (unsigned int index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      const struct PinEntry * const pinEntry = pinFind(i2sPins, pinArray[index],
          CHANNEL_INDEX(interface->channel, index));
      assert(pinEntry);

      const struct Pin pin = pinInit(pinArray[index]);

      pinInput(pin);
      pinSetFunction(pin, pinEntry->value);
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel,
    const struct I2sBase *state, struct I2sBase *interface)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state,
      interface) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
void I2S0_ISR(void)
{
  descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void I2S1_ISR(void)
{
  descriptors[1]->handler(descriptors[1]);
}
/*----------------------------------------------------------------------------*/
uint32_t i2sGetClock(const struct I2sBase *interface __attribute__((unused)))
{
  return clockFrequency(Apb1Clock);
}
/*----------------------------------------------------------------------------*/
static enum result i2sInit(void *object, const void *configBase)
{
  const struct I2sBaseConfig * const config = configBase;
  struct I2sBase * const interface = object;
  enum result res;

  interface->channel = config->channel;
  interface->handler = 0;

  /* Try to set peripheral descriptor */
  if ((res = setDescriptor(interface->channel, 0, interface)) != E_OK)
    return res;

  configPins(interface, configBase);

  /* Check whether other channel is disabled too */
  if (!descriptors[interface->channel ^ 1])
  {
    /* Enable clock to register interface and to peripheral */
    sysClockEnable(CLK_APB1_I2S);
    /* Reset registers to default values */
    sysResetEnable(RST_I2S);
  }

  switch (interface->channel)
  {
    case 0:
      interface->irq = I2S0_IRQ;
      interface->reg = LPC_I2S0;
      break;

    case 1:
      interface->irq = I2S1_IRQ;
      interface->reg = LPC_I2S1;
      break;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void i2sDeinit(void *object)
{
  const struct I2sBase * const interface = object;

  if (!descriptors[interface->channel ^ 1])
    sysClockDisable(CLK_APB1_I2S);

  setDescriptor(interface->channel, interface, 0);
}
