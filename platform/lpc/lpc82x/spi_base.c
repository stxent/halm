/*
 * spi_base.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/spi_base.h>
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
static void configPins(const struct SpiBaseConfig *);
static bool setInstance(uint8_t, struct SpiBase *);
/*----------------------------------------------------------------------------*/
static enum Result spiInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_SPI_NO_DEINIT
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
static struct SpiBase *instances[2] = {NULL};
/*----------------------------------------------------------------------------*/
static void configPins(const struct SpiBaseConfig *config)
{
  const PinNumber pinArray[] = {
      config->sck, config->mosi, config->miso, config->cs
  };

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      const struct Pin pin = pinInit(pinArray[index]);
      const enum PinMuxIndex mux = PINMUX_SPI0_SCK + index
          + PINMUX_SPI_STRIDE * config->channel;

      pinInput(pin);
      pinSetMux(pin, mux);
    }
  }
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
void SPI0_ISR(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void SPI1_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
uint32_t spiGetClock(const struct SpiBase *)
{
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
static enum Result spiInit(void *object, const void *configBase)
{
  const struct SpiBaseConfig * const config = configBase;
  struct SpiBase * const interface = object;

  if (!setInstance(config->channel, interface))
    return E_BUSY;

  switch (config->channel)
  {
    case 0:
      interface->reg = LPC_SPI0;
      break;

    case 1:
      interface->reg = LPC_SPI1;
      break;

    default:
      return E_ERROR;
  }

  /* Configure input and output pins */
  configPins(config);

  sysClockEnable(CLK_SPI0 + config->channel);
  sysResetPulse(RST_SPI0 + config->channel);

  interface->channel = config->channel;
  interface->handler = NULL;
  interface->irq = SPI0_IRQ + config->channel;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_SPI_NO_DEINIT
static void spiDeinit(void *object)
{
  const struct SpiBase * const interface = object;

  sysClockDisable(CLK_SPI0 + interface->channel);
  instances[interface->channel] = NULL;
}
#endif
