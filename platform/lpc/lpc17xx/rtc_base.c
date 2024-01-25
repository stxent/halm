/*
 * rtc_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/rtc_base.h>
#include <halm/platform/lpc/system.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(struct RtcBase *);
/*----------------------------------------------------------------------------*/
static enum Result clkInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_RTC_NO_DEINIT
static void clkDeinit(void *);
#else
#  define clkDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const RtcBase = &(const struct EntityClass){
    .size = sizeof(struct RtcBase),
    .init = clkInit,
    .deinit = clkDeinit
};
/*----------------------------------------------------------------------------*/
static struct RtcBase *instance = NULL;
/*----------------------------------------------------------------------------*/
static bool setInstance(struct RtcBase *object)
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
void RTC_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
static enum Result clkInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct RtcBase * const clock = object;

  if (setInstance(clock))
  {
    if (!sysPowerStatus(PWR_RTC))
      sysPowerEnable(PWR_RTC);

    clock->handler = NULL;
    clock->irq = RTC_IRQ;
    clock->reg = LPC_RTC;

    return E_OK;
  }
  else
    return E_BUSY;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_RTC_NO_DEINIT
static void clkDeinit(void *object __attribute__((unused)))
{
  sysPowerDisable(PWR_RTC);
  instance = NULL;
}
#endif
