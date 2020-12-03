/*
 * ssp_base.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/lpc13uxx/clocking.h>
#include <halm/platform/lpc/lpc13uxx/system.h>
#include <halm/platform/lpc/lpc13uxx/system_defs.h>
#include <halm/platform/lpc/ssp_base.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
/* SSP clock divisor is the number from 1 to 255 or 0 to disable */
#define DEFAULT_DIV       1
#define DEFAULT_DIV_VALUE 1
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
static struct SspBase *instances[2] = {0};
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
  return (clockFrequency(MainClock) * LPC_SYSCON->SYSAHBCLKDIV)
      / DEFAULT_DIV_VALUE;
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
      sysClockEnable(CLK_SSP0);
      LPC_SYSCON->SSP0CLKDIV = DEFAULT_DIV;
      LPC_SYSCON->PRESETCTRL |= PRESETCTRL_SSP0;
      interface->reg = LPC_SSP0;
      interface->irq = SSP0_IRQ;
      break;

    case 1:
      sysClockEnable(CLK_SSP1);
      LPC_SYSCON->SSP1CLKDIV = DEFAULT_DIV;
      LPC_SYSCON->PRESETCTRL |= PRESETCTRL_SSP1;
      interface->reg = LPC_SSP1;
      interface->irq = SSP1_IRQ;
      break;
  }

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

  instances[interface->channel] = 0;
}
#endif
