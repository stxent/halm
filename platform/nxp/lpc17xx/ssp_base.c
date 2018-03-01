/*
 * ssp_base.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/lpc17xx/clocking.h>
#include <halm/platform/nxp/lpc17xx/system.h>
#include <halm/platform/nxp/ssp_base.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/*----------------------------------------------------------------------------*/
static void resetInstance(uint8_t);
static bool setInstance(uint8_t, struct SspBase *);
/*----------------------------------------------------------------------------*/
static enum Result sspInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NXP_SSP_NO_DEINIT
static void sspDeinit(void *);
#else
#define sspDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static const struct EntityClass sspTable = {
    .size = 0, /* Abstract class */
    .init = sspInit,
    .deinit = sspDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinEntry sspPins[] = {
    {
        .key = PIN(0, 6), /* SSP1_SSEL */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(0, 7), /* SSP1_SCK */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(0, 8), /* SSP1_MISO */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(0, 9), /* SSP1_MOSI */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(0, 15), /* SSP0_SCK */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(0, 16), /* SSP0_SSEL */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(0, 17), /* SSP0_MISO */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(0, 18), /* SSP0_MOSI */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(1, 20), /* SSP0_SCK */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(1, 21), /* SSP0_SSEL */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(1, 23), /* SSP0_MISO */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(1, 24), /* SSP0_MOSI */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(1, 31), /* SSP1_SCK */
        .channel = 1,
        .value = 2
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const SspBase = &sspTable;
static struct SspBase *instances[2] = {0};
/*----------------------------------------------------------------------------*/
static void resetInstance(uint8_t channel)
{
  instances[channel] = 0;
}
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct SspBase *object)
{
  assert(channel < ARRAY_SIZE(instances));

  if (!instances[channel])
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
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void SSP1_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
uint32_t sspGetClock(const struct SspBase *interface __attribute__((unused)))
{
  return clockFrequency(MainClock) / sysClockDivToValue(DEFAULT_DIV);
}
/*----------------------------------------------------------------------------*/
static enum Result sspInit(void *object, const void *configBase)
{
  const struct SspBaseConfig * const config = configBase;
  struct SspBase * const interface = object;

  interface->channel = config->channel;
  interface->handler = 0;

  if (!setInstance(interface->channel, interface))
    return E_BUSY;

  /* Configure input and output pins */
  sspConfigPins(interface, config);

  switch (interface->channel)
  {
    case 0:
      sysPowerEnable(PWR_SSP0);
      sysClockControl(CLK_SSP0, DEFAULT_DIV);
      interface->reg = LPC_SSP0;
      interface->irq = SSP0_IRQ;
      break;

    case 1:
      sysPowerEnable(PWR_SSP1);
      sysClockControl(CLK_SSP1, DEFAULT_DIV);
      interface->reg = LPC_SSP1;
      interface->irq = SSP1_IRQ;
      break;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_SSP_NO_DEINIT
static void sspDeinit(void *object)
{
  const struct SspBase * const interface = object;

  switch (interface->channel)
  {
    case 0:
      sysPowerDisable(PWR_SSP0);
      break;

    case 1:
      sysPowerDisable(PWR_SSP1);
      break;
  }
  resetInstance(interface->channel);
}
#endif
