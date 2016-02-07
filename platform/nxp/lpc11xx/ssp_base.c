/*
 * ssp_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <memory.h>
#include <platform/nxp/lpc11xx/clocking.h>
#include <platform/nxp/lpc11xx/system.h>
#include <platform/nxp/lpc11xx/system_defs.h>
#include <platform/nxp/ssp_base.h>
/*----------------------------------------------------------------------------*/
/* SSP clock divisor is the number from 1 to 255 or 0 to disable */
#define DEFAULT_DIV       1
#define DEFAULT_DIV_VALUE 1
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t, const struct SspBase *,
    struct SspBase *);
/*----------------------------------------------------------------------------*/
static enum result sspInit(void *, const void *);
static void sspDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass sspTable = {
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
const struct EntityClass * const SspBase = &sspTable;
static struct SspBase *descriptors[2] = {0};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel,
    const struct SspBase *state, struct SspBase *interface)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state,
      interface) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
void SSP0_ISR(void)
{
  descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void SSP1_ISR(void)
{
  descriptors[1]->handler(descriptors[1]);
}
/*----------------------------------------------------------------------------*/
uint32_t sspGetClock(const struct SspBase *interface __attribute__((unused)))
{
  return (clockFrequency(MainClock) * LPC_SYSCON->SYSAHBCLKDIV)
      / DEFAULT_DIV_VALUE;
}
/*----------------------------------------------------------------------------*/
static enum result sspInit(void *object, const void *configBase)
{
  const struct SspBaseConfig * const config = configBase;
  struct SspBase * const interface = object;
  enum result res;

  interface->channel = config->channel;
  interface->handler = 0;

  /* Try to set peripheral descriptor */
  if ((res = setDescriptor(interface->channel, 0, interface)) != E_OK)
    return res;

  /* Configure input and output pins */
  sspConfigPins(interface, config);

  /*
   * SSP1 configuration differs for latest silicon revisions and is not
   * completely reentrant. Device should be reset to configure peripheral
   * with different pins.
   */
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

  return E_OK;
}
/*----------------------------------------------------------------------------*/
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
  setDescriptor(interface->channel, interface, 0);
}
