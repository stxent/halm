/*
 * spim_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/clocking.h>
#include <halm/platform/numicro/spim_base.h>
#include <halm/platform/numicro/system.h>
#include <xcore/bits.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void configPins(struct SpimBase *, const struct SpimBaseConfig *);
static bool setInstance(struct SpimBase *);
/*----------------------------------------------------------------------------*/
static enum Result spimInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NUMICRO_SPIM_NO_DEINIT
static void spimDeinit(void *);
#else
#  define spimDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const SpimBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = spimInit,
    .deinit = spimDeinit
};
/*----------------------------------------------------------------------------*/
static const struct PinEntry spimPins[] = {
    {
        .key = PIN(PORT_A, 0), /* SPIM_MOSI */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(PORT_A, 1), /* SPIM_MISO */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(PORT_A, 2), /* SPIM_CLK */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(PORT_A, 3), /* SPIM_SS */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(PORT_A, 4), /* SPIM_D3 */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(PORT_A, 5), /* SPIM_D2 */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(PORT_C, 0), /* SPIM_MOSI */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_C, 1), /* SPIM_MISO */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_C, 2), /* SPIM_CLK */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_C, 3), /* SPIM_SS */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_C, 4), /* SPIM_D3 */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_C, 5), /* SPIM_D2 */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_E, 2), /* SPIM_MOSI */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_E, 3), /* SPIM_MISO */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_E, 4), /* SPIM_CLK */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_E, 5), /* SPIM_SS */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_E, 6), /* SPIM_D3 */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_E, 7), /* SPIM_D2 */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_G, 9), /* SPIM_D2 */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_G, 10), /* SPIM_D3 */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_G, 11), /* SPIM_SS */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_G, 12), /* SPIM_CLK */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_G, 13), /* SPIM_MISO */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_G, 14), /* SPIM_MOSI */
        .channel = 0,
        .value = 4
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct SpimBase *instance = NULL;
/*----------------------------------------------------------------------------*/
static void configPins(struct SpimBase *interface,
    const struct SpimBaseConfig *config)
{
  const PinNumber pinArray[] = {
      config->cs,
      config->io0,
      config->io1,
      config->io2,
      config->io3,
      config->sck
  };
  bool wide = true;

  assert(config->cs && config->sck && config->io0 && config->io1);
  assert((!config->io2 && !config->io3) || (config->io2 && config->io3));

  if (config->io2 && config->io3)
    wide = true;

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (!pinArray[index])
      continue;

    const struct PinEntry * const pinEntry = pinFind(spimPins,
        pinArray[index], 0);
    assert(pinEntry);

    const struct Pin pin = pinInit(pinArray[index]);

    pinInput(pin);
    pinSetFunction(pin, pinEntry->value);
  }

  interface->wide = wide;
}
/*----------------------------------------------------------------------------*/
static bool setInstance(struct SpimBase *object)
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
void SPIM_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
uint32_t spimGetClock(const struct SpimBase *)
{
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
void *spimGetMemoryAddress(const struct SpimBase *)
{
  return (void *)NM_SPIM_BASE;
}
/*----------------------------------------------------------------------------*/
static enum Result spimInit(void *object, const void *configBase)
{
  const struct SpimBaseConfig * const config = configBase;
  struct SpimBase * const interface = object;

  if (!setInstance(interface))
    return E_BUSY;

  /* Configure input and output pins */
  configPins(interface, config);

  /* Enable clock to peripheral */
  sysClockEnable(CLK_SPIM);
  /* Reset registers to default values */
  sysResetBlock(RST_SPIM);

  interface->handler = NULL;
  interface->irq = SPIM_IRQ;
  interface->reg = NM_SPIM;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_SPIM_NO_DEINIT
static void spimDeinit(void *)
{
  sysClockDisable(CLK_SPIM);
  instance = NULL;
}
#endif
