/*
 * rtc_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <xcore/memory.h>
#include <halm/platform/nxp/gen_1/rtc_base.h>
#include <halm/platform/nxp/lpc43xx/system.h>
/*----------------------------------------------------------------------------*/
static bool setDescriptor(const struct RtcBase *, struct RtcBase *);
/*----------------------------------------------------------------------------*/
static enum Result clkInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NXP_RTC_NO_DEINIT
static void clkDeinit(void *);
#else
#define clkDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static const struct EntityClass clkTable = {
    .size = sizeof(struct RtcBase),
    .init = clkInit,
    .deinit = clkDeinit
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const RtcBase = &clkTable;
static struct RtcBase *descriptor = 0;
/*----------------------------------------------------------------------------*/
static bool setDescriptor(const struct RtcBase *state, struct RtcBase *clock)
{
  return compareExchangePointer((void **)&descriptor, state, clock);
}
/*----------------------------------------------------------------------------*/
void RTC_ISR(void)
{
  descriptor->handler(descriptor);
}
/*----------------------------------------------------------------------------*/
static enum Result clkInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct RtcBase * const clock = object;

  if (!setDescriptor(0, clock))
    return E_BUSY;

  clock->handler = 0;
  clock->irq = RTC_IRQ;
  clock->reg = LPC_RTC;

  /* CLK_M4_BUS is already enabled */

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_RTC_NO_DEINIT
static void clkDeinit(void *object)
{
  setDescriptor(object, 0);
}
#endif
