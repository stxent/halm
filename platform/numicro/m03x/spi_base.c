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
static bool setInstance(struct SpiBase *);
/*----------------------------------------------------------------------------*/
static enum Result spiInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NUMICRO_SPI_NO_DEINIT
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
const struct PinEntry spiPins[] = {
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
    }, {
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
        .key = PIN(PORT_D, 14), /* SPI0_I2SMCLK */
        .channel = 0,
        .value = 5
    }, {
        .key = PIN(PORT_F, 10), /* SPI0_I2SMCLK */
        .channel = 0,
        .value = 5
    }, {
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
    }, {
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
    }, {
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
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct SpiBase *instance = NULL;
/*----------------------------------------------------------------------------*/
static bool setInstance(struct SpiBase *object)
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
void SPI0_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
uint32_t spiGetClock(const struct SpiBase *interface __attribute__((unused)))
{
  return clockFrequency(Spi0Clock);
}
/*----------------------------------------------------------------------------*/
static enum Result spiInit(void *object, const void *configBase)
{
  const struct SpiBaseConfig * const config = configBase;
  struct SpiBase * const interface = object;

  assert(config->channel == 0);
  if (!setInstance(interface))
    return E_BUSY;

  /* Configure input and output pins */
  spiConfigPins(config);

  /* Enable clock to peripheral */
  sysClockEnable(CLK_SPI0);
  /* Reset registers to default values */
  sysResetBlock(RST_SPI0);

  interface->channel = config->channel;
  interface->handler = NULL;
  interface->irq = SPI0_IRQ;
  interface->reg = NM_SPI0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_SPI_NO_DEINIT
static void spiDeinit(void *object __attribute__((unused)))
{
  sysClockDisable(CLK_SPI0);
  instance = NULL;
}
#endif
