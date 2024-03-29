/*
 * rtc_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/lpc43xx/event_router.h>
#include <halm/platform/lpc/rtc_base.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
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
static void interruptHandler(void *object)
{
  ((struct RtcBase *)object)->handler(object);
}
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
static enum Result clkInit(void *object, const void *)
{
  struct RtcBase * const clock = object;

  if (setInstance(clock))
  {
    /* CLK_M4_BUS is already enabled */

    clock->handler = NULL;
    clock->irq = IRQ_RESERVED;
    clock->reg = LPC_RTC;

    return erRegister(interruptHandler, clock, ER_RTC);
  }
  else
    return E_BUSY;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_RTC_NO_DEINIT
static void clkDeinit(void *object)
{
  erUnregister(object);
  instance = NULL;
}
#endif
