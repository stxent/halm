/*
 * ssp_base.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/lpc13uxx/system_defs.h>
#include <halm/platform/lpc/ssp_base.h>
#include <halm/platform/lpc/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t, struct SspBase *);
/*----------------------------------------------------------------------------*/
static enum Result sspInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_SSP_NO_DEINIT
static void sspDeinit(void *);
#else
#define sspDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const SspBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = sspInit,
    .deinit = sspDeinit
};
/*----------------------------------------------------------------------------*/
/* SSP1 peripheral available only on LPC1313 */
const struct PinEntry sspPins[] = {
    {
        .key = PIN(0, 2), /* SSEL0 */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(1, 19), /* SSEL1 */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(1, 23), /* SSEL1 */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(0, 6), /* SCK0 */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(0, 10), /* SCK0 */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(1, 15), /* SCK1 */
        .channel = 1,
        .value = 3
    }, {
        .key = PIN(1, 20), /* SCK1 */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(1, 29), /* SCK0 */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 8), /* MISO0 */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 22), /* MISO1 */
        .channel = 1,
        .value = 3
    }, {
        .key = PIN(1, 21), /* MISO1 */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(0, 9), /* MOSI0 */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 21), /* MOSI1 */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(1, 22), /* MOSI1 */
        .channel = 1,
        .value = 2
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct SspBase *instances[2] = {NULL};
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct SspBase *object)
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
uint32_t sspGetClock(const struct SspBase *interface)
{
  const uint32_t frequency = clockFrequency(MainClock);
  const uint32_t divider = interface->channel == 0 ?
      LPC_SYSCON->SSP0CLKDIV : LPC_SYSCON->SSP1CLKDIV;

  return frequency * LPC_SYSCON->SYSAHBCLKDIV / divider;
}
/*----------------------------------------------------------------------------*/
static enum Result sspInit(void *object, const void *configBase)
{
  const struct SspBaseConfig * const config = configBase;
  struct SspBase * const interface = object;

  if (!setInstance(config->channel, interface))
    return E_BUSY;

  /* Configure input and output pins */
  sspConfigPins(config);

  switch (config->channel)
  {
    case 0:
      sysClockEnable(CLK_SSP0);
      LPC_SYSCON->SSP0CLKDIV = LPC_SYSCON->SYSAHBCLKDIV;
      LPC_SYSCON->PRESETCTRL |= PRESETCTRL_SSP0;
      interface->irq = SSP0_IRQ;
      interface->reg = LPC_SSP0;
      break;

    case 1:
      sysClockEnable(CLK_SSP1);
      LPC_SYSCON->SSP1CLKDIV = LPC_SYSCON->SYSAHBCLKDIV;
      LPC_SYSCON->PRESETCTRL |= PRESETCTRL_SSP1;
      interface->irq = SSP1_IRQ;
      interface->reg = LPC_SSP1;
      break;
  }

  interface->channel = config->channel;
  interface->handler = NULL;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_SSP_NO_DEINIT
static void sspDeinit(void *object)
{
  const struct SspBase * const interface = object;

  /* Put peripheral in reset and disable clock */
  switch (interface->channel)
  {
    case 0:
      LPC_SYSCON->PRESETCTRL &= ~PRESETCTRL_SSP0;
      LPC_SYSCON->SSP0CLKDIV = 0;
      sysClockDisable(CLK_SSP0);
      break;

    case 1:
      LPC_SYSCON->PRESETCTRL &= ~PRESETCTRL_SSP1;
      LPC_SYSCON->SSP1CLKDIV = 0;
      sysClockDisable(CLK_SSP1);
      break;
  }

  instances[interface->channel] = NULL;
}
#endif
