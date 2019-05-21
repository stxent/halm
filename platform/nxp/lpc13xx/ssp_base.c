/*
 * ssp_base.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/lpc13xx/clocking.h>
#include <halm/platform/nxp/lpc13xx/system.h>
#include <halm/platform/nxp/lpc13xx/system_defs.h>
#include <halm/platform/nxp/ssp_base.h>
/*----------------------------------------------------------------------------*/
/* SSP clock divisor is the number from 1 to 255 or 0 to disable */
#define DEFAULT_DIV       1
#define DEFAULT_DIV_VALUE 1
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t, struct SspBase *);
/*----------------------------------------------------------------------------*/
static enum Result sspInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NXP_SSP_NO_DEINIT
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
        .key = PIN(0, 2), /* SSP0_SSEL */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 6), /* SSP0_SCK */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(0, 8), /* SSP0_MISO */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 9), /* SSP0_MOSI */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 10), /* SSP0_SCK */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(2, 0), /* SSP1_SSEL */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(2, 1), /* SSP1_SCK */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(2, 2), /* SSP1_MISO */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(2, 3), /* SSP1_MOSI */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(2, 11), /* SSP0_SCK */
        .channel = 0,
        .value = 1
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

      /* Set SCK0 pin location register */
      switch (config->sck)
      {
        case PIN(0, 10):
          LPC_IOCON->SCK_LOC = 0;
          break;

        case PIN(2, 11):
          LPC_IOCON->SCK_LOC = 1;
          break;

        case PIN(0, 6):
          LPC_IOCON->SCK_LOC = 2;
          break;
      }
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
#ifndef CONFIG_PLATFORM_NXP_SSP_NO_DEINIT
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
