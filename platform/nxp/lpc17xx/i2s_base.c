/*
 * i2s_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <memory.h>
#include <platform/nxp/i2s_base.h>
#include <platform/nxp/lpc17xx/clocking.h>
#include <platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
/* PCLK for I2S must not be higher than 74 MHz */
#define DEFAULT_DIV CLK_DIV2
/*----------------------------------------------------------------------------*/
static enum result configPins(struct I2sBase *, const struct I2sBaseConfig *);
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
        /* Unavailable on LPC175x series */
        .key = PIN(0, 4), /* I2SRX_CLK */
        .channel = 0,
        .value = 1
    }, {
        /* Unavailable on LPC175x series */
        .key = PIN(0, 5), /* I2SRX_WS */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 6), /* I2SRX_SDA */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 7), /* I2STX_CLK */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 8), /* I2STX_WS */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 9), /* I2STX_SDA */
        .channel = 0,
        .value = 1
    }, {
        /* Unavailable on LPC175x series */
        .key = PIN(0, 23), /* I2SRX_CLK */
        .channel = 0,
        .value = 2
    }, {
        /* Unavailable on LPC175x series */
        .key = PIN(0, 24), /* I2SRX_WS */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(0, 25), /* I2SRX_SDA */
        .channel = 0,
        .value = 2
    }, {
        /* Unavailable on LPC175x series */
        .key = PIN(2, 11), /* I2STX_CLK */
        .channel = 0,
        .value = 3
    }, {
        /* Unavailable on LPC175x series */
        .key = PIN(2, 12), /* I2STX_WS */
        .channel = 0,
        .value = 3
    }, {
        /* Unavailable on LPC175x series */
        .key = PIN(2, 13), /* I2STX_SDA */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(4, 28), /* RX_MCLK */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(4, 29), /* TX_MCLK */
        .channel = 0,
        .value = 1
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const I2sBase = &i2sTable;
static struct I2sBase *descriptors[1] = {0};
/*----------------------------------------------------------------------------*/
static enum result configPins(struct I2sBase *interface,
    const struct I2sBaseConfig *config)
{
  const pin_t pinArray[] = {
      config->rx.sck, config->rx.ws, config->rx.sda, config->rx.mclk,
      config->tx.sck, config->tx.ws, config->tx.sda, config->tx.mclk
  };
  const struct PinEntry *pinEntry;
  struct Pin pin;

  for (uint8_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      pinEntry = pinFind(i2sPins, pinArray[index], interface->channel);
      if (!pinEntry)
        return E_VALUE;
      pinInput((pin = pinInit(pinArray[index])));
      pinSetFunction(pin, pinEntry->value);
    }
  }

  return E_OK;
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
void I2S_ISR(void)
{
  descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
uint32_t i2sGetClock(const struct I2sBase *interface __attribute__((unused)))
{
  return clockFrequency(MainClock) / sysClockDivToValue(DEFAULT_DIV);
}
/*----------------------------------------------------------------------------*/
static enum result i2sInit(void *object, const void *configBase)
{
  const struct I2sBaseConfig * const config = configBase;
  struct I2sBase * const interface = object;
  enum result res;

  /* Try to set peripheral descriptor */
  interface->channel = config->channel;
  if ((res = setDescriptor(interface->channel, 0, interface)) != E_OK)
    return res;

  if ((res = configPins(interface, configBase)) != E_OK)
    return res;

  sysPowerEnable(PWR_I2S);
  sysClockControl(CLK_I2S, DEFAULT_DIV);

  interface->handler = 0;
  interface->irq = I2S_IRQ;
  interface->reg = LPC_I2S;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void i2sDeinit(void *object)
{
  const struct I2sBase * const interface = object;

  sysPowerDisable(PWR_I2S);
  setDescriptor(interface->channel, interface, 0);
}