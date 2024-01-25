/*
 * rtc.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gen_1/rtc_defs.h>
#include <halm/platform/lpc/rtc.h>
#include <halm/platform/platform_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static void setTime(struct Rtc *, time64_t);
/*----------------------------------------------------------------------------*/
static enum Result clkInit(void *, const void *);
static enum Result clkSetAlarm(void *, time64_t);
static void clkSetCallback(void *, void (*)(void *), void *);
static enum Result clkSetTime(void *, time64_t);
static void clkStop(void *);
static time64_t clkTime(void *);

#ifndef CONFIG_PLATFORM_LPC_RTC_NO_DEINIT
static void clkDeinit(void *);
#else
#  define clkDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct RtClockClass * const Rtc = &(const struct RtClockClass){
    .size = sizeof(struct Rtc),
    .init = clkInit,
    .deinit = clkDeinit,

    .setAlarm = clkSetAlarm,
    .setCallback = clkSetCallback,
    .setTime = clkSetTime,
    .stop = clkStop,
    .time = clkTime
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Rtc * const clock = object;
  LPC_RTC_Type * const reg = clock->base.reg;
  bool event = false;

  if (reg->ILR & ILR_RTCALF)
  {
    /* Disable further interrupts */
    reg->AMR = AMR_MASK;

    event = true;
  }

  /* Clear pending interrupts */
  reg->ILR = ILR_RTCCIF | ILR_RTCALF;

  if (event && clock->callback)
    clock->callback(clock->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void setTime(struct Rtc *clock, time64_t timestamp)
{
  LPC_RTC_Type * const reg = clock->base.reg;

  if ((reg->CCR & CCR_CLKEN) != 0)
  {
    const uint32_t mask = CCR_CTCRST | CCR_CCALEN;

    /* Stop and reset counters */
    reg->CCR = mask;
    while ((reg->CCR & mask) != mask);
  }

  struct RtDateTime dateTime;
  rtMakeTime(&dateTime, timestamp);

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
  reg->CCR = CCR_CLKEN;
}
/*----------------------------------------------------------------------------*/
static enum Result clkInit(void *object, const void *configBase)
{
  const struct RtcConfig * const config = configBase;
  assert(config != NULL);

  struct Rtc * const clock = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = RtcBase->init(clock, NULL)) != E_OK)
    return res;

  clock->base.handler = interruptHandler;
  clock->callback = NULL;

  LPC_RTC_Type * const reg = clock->base.reg;

  /* Disable interrupts */
  reg->CIIR = 0;
  reg->ILR = ILR_RTCCIF | ILR_RTCALF;

  if (config->timestamp)
  {
    reg->AMR = AMR_MASK;
    reg->CALIBRATION = 0;

    /* Reinitialize current time */
    setTime(clock, config->timestamp);
  }
  else if (!(reg->CCR & CCR_CLKEN))
  {
    reg->AMR = AMR_MASK;
    reg->CALIBRATION = 0;

    /* Set default time */
    setTime(clock, 0);
  }

  if (clock->base.irq != IRQ_RESERVED)
  {
    irqSetPriority(clock->base.irq, config->priority);
    irqEnable(clock->base.irq);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_RTC_NO_DEINIT
static void clkDeinit(void *object __attribute__((unused)))
{
  struct Rtc * const clock = object;
  LPC_RTC_Type * const reg = clock->base.reg;

  /* Disable interrupts */
  reg->AMR = AMR_MASK;
  /* Reset counters */
  reg->CCR = CCR_CTCRST | CCR_CCALEN;

  if (clock->base.irq != IRQ_RESERVED)
    irqDisable(clock->base.irq);

  RtcBase->deinit(clock);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result clkSetAlarm(void *object, time64_t alarmTime)
{
  struct Rtc * const clock = object;
  LPC_RTC_Type * const reg = clock->base.reg;
  struct RtDateTime dateTime;

  rtMakeTime(&dateTime, alarmTime);

  /* Wait until write to the alarm mask register is pending */
  while (reg->AMR != AMR_MASK);

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

  /* Enable alarm interrupt */
  reg->AMR = AMR_DOW | AMR_DOY;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void clkSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Rtc * const clock = object;

  clock->callbackArgument = argument;
  clock->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result clkSetTime(void *object, time64_t timestamp)
{
  setTime(object, timestamp);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void clkStop(void *object)
{
  const struct Rtc * const clock = object;
  LPC_RTC_Type * const reg = clock->base.reg;

  if ((reg->CCR & CCR_CLKEN) != 0)
  {
    /* Stop and reset counters */
    reg->CCR = CCR_CTCRST | CCR_CCALEN;
  }
}
/*----------------------------------------------------------------------------*/
static time64_t clkTime(void *object)
{
  const struct Rtc * const clock = object;
  const LPC_RTC_Type * const reg = clock->base.reg;

  if ((reg->CCR & CCR_CLKEN) != 0)
  {
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
    time64_t timestamp = 0;

    if (rtMakeEpochTime(&timestamp, &dateTime) == E_OK)
      return timestamp;
  }

  return 0;
}
/*----------------------------------------------------------------------------*/
int32_t rtcCalcCalibration(time64_t globaltime, time64_t localtime,
    time64_t prevSync, int32_t prevCalib)
{
  if (prevSync != 0)
  {
    time64_t uncorrected = localtime;

    if (prevCalib != 0)
      uncorrected -= (localtime - prevSync) / prevCalib;

    const int32_t limit = (int32_t)CALIBRATION_VAL_MASK;
    const int32_t delta = (int32_t)(globaltime - uncorrected);
    int32_t calib = delta != 0 ? (globaltime - prevSync) / delta : 0;

    if (calib > limit)
      calib = limit;
    else if (calib < -limit)
      calib = -limit;

    return calib;
  }
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
void rtcSetCalibration(void *clock, int32_t offset)
{
  LPC_RTC_Type * const reg = ((struct Rtc *)clock)->base.reg;
  uint32_t calibration;

  if (offset < 0)
  {
    offset = -offset;
    calibration = CALIBRATION_DIR;
  }
  else
    calibration = 0;

  offset = MIN(offset, (int32_t)CALIBRATION_VAL_MASK);
  calibration |= CALIBRATION_VAL(offset);

  reg->CALIBRATION = calibration;
}
