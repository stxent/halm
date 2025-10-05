/*
 * wdt_base.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/lpc82x/system_defs.h>
#include <halm/platform/lpc/system.h>
#include <halm/platform/lpc/wdt_base.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(struct WdtBase *);

static enum Result wdtInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const WdtBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = wdtInit,
    .deinit = NULL /* Default destructor */
};
/*----------------------------------------------------------------------------*/
static struct WdtBase *instance = NULL;
/*----------------------------------------------------------------------------*/
static bool setInstance(struct WdtBase *object)
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
void WWDT_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
uint32_t wdtGetClock(const struct WdtBase *)
{
  return clockFrequency(WdtOsc);
}
/*----------------------------------------------------------------------------*/
static enum Result wdtInit(void *object, const void *)
{
  struct WdtBase * const timer = object;

  if (setInstance(timer))
  {
    sysClockEnable(CLK_WWDT);

    timer->handler = NULL;
    timer->irq = WWDT_IRQ;

#ifdef CONFIG_PM
    /* Watchdog interrupt will wake the controller from low-power modes */
    LPC_SYSCON->STARTERP1 |= BIT(STARTERP1_WWDT);
#endif

    return E_OK;
  }
  else
    return E_BUSY;
}
