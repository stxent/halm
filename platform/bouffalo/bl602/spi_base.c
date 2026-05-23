/*
 * spi_base.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/bouffalo/bl602/glb_defs.h>
#include <halm/platform/bouffalo/clocking.h>
#include <halm/platform/bouffalo/spi_base.h>
#include <halm/platform/bouffalo/spi_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(struct SpiBase *);
/*----------------------------------------------------------------------------*/
static enum Result spiInit(void *, const void *);

#ifndef CONFIG_PLATFORM_BOUFFALO_SPI_NO_DEINIT
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
[[gnu::interrupt]] void SPI_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
void spiConfigPins(const struct SpiBaseConfig *config)
{
  const PinNumber pinArray[] = {
      config->miso, config->mosi, config->cs, config->sck
  };

  if (config->master)
  {
    BL_GLB->GLB_PARM |= GLB_PARM_REG_SPI_0_MASTER_MODE
        | GLB_PARM_REG_SPI_0_SWAP;
  }
  else
  {
    BL_GLB->GLB_PARM &= ~(GLB_PARM_REG_SPI_0_MASTER_MODE
        | GLB_PARM_REG_SPI_0_SWAP);
  }

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      const struct Pin pin = pinInit(pinArray[index]);

      if ((config->master && index) || (!config->master && !index))
        pinOutput(pin, false);
      if ((config->master && !index) || (!config->master && index))
        pinInput(pin);

      pinSetFunction(pin, SPI_FUNCTION);
    }
  }
}
/*----------------------------------------------------------------------------*/
uint32_t spiGetClock(const struct SpiBase *)
{
  return clockFrequency(SpiClock);
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

  interface->channel = 0;
  interface->handler = NULL;
  interface->irq = SPI_IRQ;
  interface->reg = BL_SPI;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_BOUFFALO_SPI_NO_DEINIT
static void spiDeinit(void *)
{
  instance = NULL;
}
#endif
