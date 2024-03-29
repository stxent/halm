/*
 * i2s_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/i2s_base.h>
#include <halm/platform/lpc/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
/* Errata: PCLK for I2S must not be higher than 74 MHz */
#define DEFAULT_DIV CLK_DIV2
/*----------------------------------------------------------------------------*/
static void configPins(const struct I2SBaseConfig *);
static bool setInstance(struct I2SBase *);
/*----------------------------------------------------------------------------*/
static enum Result i2sInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_I2S_NO_DEINIT
static void i2sDeinit(void *);
#else
#  define i2sDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const I2SBase = &(const struct EntityClass){
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
static struct I2SBase *instance = NULL;
/*----------------------------------------------------------------------------*/
static void configPins(const struct I2SBaseConfig *config)
{
  const PinNumber pinArray[] = {
      config->rx.sck, config->rx.ws, config->rx.sda, config->rx.mclk,
      config->tx.sck, config->tx.ws, config->tx.sda, config->tx.mclk
  };

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      const struct PinEntry * const pinEntry = pinFind(i2sPins, pinArray[index],
          config->channel);
      assert(pinEntry != NULL);

      const struct Pin pin = pinInit(pinArray[index]);

      pinInput(pin);
      pinSetFunction(pin, pinEntry->value);
    }
  }
}
/*----------------------------------------------------------------------------*/
static bool setInstance(struct I2SBase *object)
{
  if (instance == NULL)
  {
    instance = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void I2S_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
uint32_t i2sGetClock(const struct I2SBase *)
{
  return clockFrequency(MainClock) / sysClockDivToValue(DEFAULT_DIV);
}
/*----------------------------------------------------------------------------*/
static enum Result i2sInit(void *object, const void *configBase)
{
  const struct I2SBaseConfig * const config = configBase;
  struct I2SBase * const interface = object;

  assert(config->channel == 0);
  if (!setInstance(interface))
    return E_BUSY;

  /* Configure input and output pins */
  configPins(config);

  sysPowerEnable(PWR_I2S);
  sysClockControl(CLK_I2S, DEFAULT_DIV);

  interface->handler = NULL;
  interface->channel = config->channel;
  interface->irq = I2S_IRQ;
  interface->reg = LPC_I2S;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_I2S_NO_DEINIT
static void i2sDeinit(void *)
{
  sysPowerDisable(PWR_I2S);
  instance = NULL;
}
#endif
