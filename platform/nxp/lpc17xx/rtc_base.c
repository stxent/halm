/*
 * rtc_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <memory.h>
#include <platform/nxp/gen_1/rtc_base.h>
#include <platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
static enum result clkInit(void *, const void *);
static void clkDeinit(void *);
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
static enum result setDescriptor(const struct RtcBase *state,
    struct RtcBase *clock)
{
  return compareExchangePointer((void **)(&descriptor), state, clock) ?
      E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
void RTC_ISR(void)
{
  descriptor->handler(descriptor);
}
/*----------------------------------------------------------------------------*/
static enum result clkInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct RtcBase * const clock = object;
  enum result res;

  if ((res = setDescriptor(0, clock)) != E_OK)
    return res;

  clock->handler = 0;
  clock->irq = RTC_IRQ;
  clock->reg = LPC_RTC;

  if (!sysPowerStatus(PWR_RTC))
    sysPowerEnable(PWR_RTC);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void clkDeinit(void *object)
{
  const struct RtcBase * const clock = object;

  sysPowerDisable(PWR_RTC);
  setDescriptor(clock, 0);
}
