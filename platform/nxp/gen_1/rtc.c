/*
 * rtc.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/gen_1/rtc_defs.h>
#include <halm/platform/nxp/rtc.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum Result clkInit(void *, const void *);
static void clkDeinit(void *);
static enum Result clkCallback(void *, void (*)(void *), void *);
static enum Result clkSetAlarm(void *, time64_t);
static enum Result clkSetTime(void *, time64_t);
static time64_t clkTime(void *);
/*----------------------------------------------------------------------------*/
static const struct RtClockClass clkTable = {
    .size = sizeof(struct Rtc),
    .init = clkInit,
    .deinit = clkDeinit,

    .callback = clkCallback,
    .setAlarm = clkSetAlarm,
    .setTime = clkSetTime,
    .time = clkTime
};
/*----------------------------------------------------------------------------*/
const struct RtClockClass * const Rtc = &clkTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Rtc * const clock = object;
  LPC_RTC_Type * const reg = clock->base.reg;
  bool event = false;

  if (reg->ILR & ILR_RTCALF)
  {
    /* Disable future interrupts */
    reg->AMR = AMR_MASK;

    event = true;
  }

  /* Clear pending interrupts */
  reg->ILR = reg->ILR;

  if (event && clock->callback)
    clock->callback(clock->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result clkInit(void *object, const void *configBase)
{
  const struct RtcConfig * const config = configBase;
  assert(config);

  struct Rtc * const clock = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = RtcBase->init(object, 0)) != E_OK)
    return res;

  clock->base.handler = interruptHandler;
  clock->callback = 0;

  LPC_RTC_Type * const reg = clock->base.reg;

  if (config->timestamp)
  {
    /* Reinitialize clock */
    if ((res = clkSetTime(clock, config->timestamp)) != E_OK)
      return res;
  }

  /* Disable interrupts */
  reg->CALIBRATION = 0;
  reg->ILR = ILR_RTCCIF | ILR_RTCALF;
  reg->AMR = AMR_MASK;

  reg->CIIR = 0;
  while ((reg->CIIR & CIIR_MASK) != 0);

  irqSetPriority(clock->base.irq, config->priority);
  irqEnable(clock->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void clkDeinit(void *object __attribute__((unused)))
{
  struct Rtc * const clock = object;
  LPC_RTC_Type * const reg = clock->base.reg;

  /* Stop time counters */
  reg->CCR &= ~CCR_CLKEN;

  irqDisable(clock->base.irq);
  RtcBase->deinit(clock);
}
/*----------------------------------------------------------------------------*/
static enum Result clkCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Rtc * const clock = object;

  clock->callbackArgument = argument;
  clock->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result clkSetAlarm(void *object, time64_t alarmTime)
{
  struct Rtc * const clock = object;
  LPC_RTC_Type * const reg = clock->base.reg;
  struct RtDateTime dateTime;

  rtMakeTime(&dateTime, alarmTime);

  /* Initialize alarm registers */
  reg->ALSEC = dateTime.second;
  reg->ALMIN = dateTime.minute;
  reg->ALHOUR = dateTime.hour;
  reg->ALDOM = dateTime.day;
  reg->ALMON = dateTime.month;
  reg->ALYEAR = dateTime.year;

  /* Unused fields */
  reg->ALDOW = 0;
  reg->ALDOY = 0;

  /* Enable interrupt */
  reg->AMR = AMR_DOW | AMR_DOY;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result clkSetTime(void *object, time64_t currentTime)
{
  struct Rtc * const clock = object;
  LPC_RTC_Type * const reg = clock->base.reg;
  struct RtDateTime dateTime;

  rtMakeTime(&dateTime, currentTime);

  /* Stop and reset counters */
  reg->CCR = CCR_CTCRST | CCR_CCALEN;
  while ((reg->CCR & (CCR_CTCRST | CCR_CCALEN)) != (CCR_CTCRST | CCR_CCALEN));

  reg->SEC = dateTime.second; /* Seconds in the range of 0 to 59 */
  reg->MIN = dateTime.minute; /* Minutes in the range of 0 to 59 */
  reg->HOUR = dateTime.hour; /* Hours in the range of 0 to 23 */
  reg->DOM = dateTime.day; /* Days in the range of 1 to 31 */
  reg->MONTH = dateTime.month; /* Month value in the range of 1 to 12 */
  reg->YEAR = dateTime.year; /* Year value in the range of 0 to 4095 */

  /* Unused fields */
  reg->DOW = 0; /* Day of week in the range of 0 to 6 */
  reg->DOY = 0; /* Day of year in the range of 0 to 366 */

  /* Enable clock */
  reg->CCR = CCR_CLKEN | CCR_CCALEN;
  while ((reg->CCR & (CCR_CLKEN | CCR_CCALEN)) != (CCR_CLKEN | CCR_CCALEN));

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static time64_t clkTime(void *object)
{
  const struct Rtc * const clock = object;
  const LPC_RTC_Type * const reg = clock->base.reg;
  uint32_t ctime0, ctime1;

  do
  {
    ctime0 = reg->CTIME0;
    ctime1 = reg->CTIME1;
  }
  while (ctime0 != reg->CTIME0);

  const struct RtDateTime dateTime = {
      .second = CTIME0_SECONDS_VALUE(ctime0),
      .minute = CTIME0_MINUTES_VALUE(ctime0),
      .hour = CTIME0_HOURS_VALUE(ctime0),
      .day = CTIME1_DOM_VALUE(ctime1),
      .month = CTIME1_MONTH_VALUE(ctime1),
      .year = CTIME1_YEAR_VALUE(ctime1)
  };
  time64_t timestamp;

  if (rtMakeEpochTime(&timestamp, &dateTime) == E_OK)
    return timestamp;
  else
    return 0;
}
