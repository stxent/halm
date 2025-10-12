/*
 * wkt_base.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/lpc82x/system_defs.h>
#include <halm/platform/lpc/system.h>
#include <halm/platform/lpc/wkt_base.h>
#include <halm/platform/lpc/wkt_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(struct WktBase *);
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_WKT_NO_DEINIT
static void tmrDeinit(void *);
#else
#  define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const WktBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = tmrInit,
    .deinit = tmrDeinit
};
/*----------------------------------------------------------------------------*/
static struct WktBase *instance = NULL;
/*----------------------------------------------------------------------------*/
static bool setInstance(struct WktBase *object)
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
void WKT_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
uint32_t wktGetClock(const struct WktBase *)
{
  if (LPC_WKT->CTRL & CTRL_SEL_EXTCLK)
    return 0;

  if (LPC_WKT->CTRL & CTRL_CLKSEL)
    return clockFrequency(LowPowerOsc);
  else
    return clockFrequency(InternalOsc);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct WktBaseConfig * const config = configBase;
  struct WktBase * const timer = object;

  if (!setInstance(timer))
    return E_BUSY;

  sysClockEnable(CLK_WKT);
  sysResetPulse(RST_WKT);

  timer->handler = NULL;
  timer->reg = LPC_WKT;
  timer->irq = WKT_IRQ;

  switch (config->source)
  {
    case WKT_CLOCK_IRC:
      LPC_WKT->CTRL &= ~(CTRL_CLKSEL | CTRL_ALARMFLAG | CTRL_SEL_EXTCLK);
      break;

    case WKT_CLOCK_LOW_POWER:
      LPC_WKT->CTRL = (LPC_WKT->CTRL & ~(CTRL_ALARMFLAG | CTRL_SEL_EXTCLK))
          | CTRL_CLKSEL;
      break;

    case WKT_CLOCK_EXTERNAL_PIN:
    {
      assert(config->pin == PIN(0, 28));

      struct Pin pin = pinInit(config->pin);
      pinInput(pin);

      LPC_PMU->DPDCTRL |= DPDCTRL_WAKEUPCLKHYS | DPDCTRL_WAKECLKPAD_ENABLE;
      LPC_WKT->CTRL = (LPC_WKT->CTRL & ~(CTRL_CLKSEL | CTRL_ALARMFLAG))
          | CTRL_SEL_EXTCLK;
      break;
    }
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_WKT_NO_DEINIT
static void tmrDeinit(void *)
{
  /* Disable external clock pad */
  if (LPC_WKT->CTRL & CTRL_SEL_EXTCLK)
    LPC_PMU->DPDCTRL &= ~(DPDCTRL_WAKEUPCLKHYS | DPDCTRL_WAKECLKPAD_ENABLE);

  instance = NULL;
}
#endif
