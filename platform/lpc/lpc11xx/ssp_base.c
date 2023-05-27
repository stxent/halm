/*
 * ssp_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/lpc11xx/system_defs.h>
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
const struct PinEntry sspPins[] = {
    {
        .key = PIN(0, 2), /* SSEL0 */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 6), /* SCK0 */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(0, 8), /* MISO0 */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 9), /* MOSI0 */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 10), /* SCK0 */
        .channel = 0,
        .value = 2
    }, {
        /* Available on LPC1100XL only */
        .key = PIN(1, 9), /* MOSI1 */
        .channel = 1,
        .value = 2
    }, {
        /* Available on LPC1100XL only */
        .key = PIN(1, 10), /* MISO1 */
        .channel = 1,
        .value = 3
    }, {
        .key = PIN(2, 0), /* SSEL1 */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(2, 1), /* SCK1 */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(2, 2), /* MISO1 */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(2, 3), /* MOSI1 */
        .channel = 1,
        .value = 2
    }, {
        /* Available on LPC1100XL only */
        .key = PIN(2, 4), /* SSEL1 */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(2, 11), /* SCK0 */
        .channel = 0,
        .value = 1
    }, {
        /* Available on LPC1100XL only */
        .key = PIN(3, 2), /* SCK1 */
        .channel = 1,
        .value = 3
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

  /*
   * SSP1 configuration differs for latest silicon revisions and is not
   * completely reentrant. Device should be reset to configure the peripheral
   * with other pins.
   */
  switch (config->channel)
  {
    case 0:
      sysClockEnable(CLK_SSP0);
      LPC_SYSCON->SSP0CLKDIV = LPC_SYSCON->SYSAHBCLKDIV;
      LPC_SYSCON->PRESETCTRL |= PRESETCTRL_SSP0;
      interface->irq = SSP0_IRQ;
      interface->reg = LPC_SSP0;

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
      LPC_SYSCON->SSP1CLKDIV = LPC_SYSCON->SYSAHBCLKDIV;
      LPC_SYSCON->PRESETCTRL |= PRESETCTRL_SSP1;
      interface->irq = SSP1_IRQ;
      interface->reg = LPC_SSP1;

      /* Configure pin locations for LPC1100XL */
      if (config->sck == PIN(3, 2))
        LPC_IOCON->SCK1_LOC = 1;

      if (config->cs == PIN(2, 4))
        LPC_IOCON->SSEL1_LOC = 1;

      if (config->miso == PIN(1, 10))
        LPC_IOCON->MISO1_LOC = 1;

      if (config->mosi == PIN(1, 9))
        LPC_IOCON->MOSI1_LOC = 1;

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
