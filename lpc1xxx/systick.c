/*
 * systick.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include "device_defs.h"
#include "macro.h"
#include "mutex.h"
#include "nvic.h"
#include "system.h"
#include "systick.h"
/*----------------------------------------------------------------------------*/
#define CTRL_ENABLE                     BIT(0) /* System tick timer enable */
#define CTRL_TICKINT                    BIT(1) /* Interrupt enable */
/* Clock source selection: 0 for clock divider, 1 for system clock source */
#define CTRL_CLKSOURCE                  BIT(2)
#define CTRL_COUNTFLAG                  BIT(16) /* Set when counter reaches 0 */
/*----------------------------------------------------------------------------*/
#define DEFAULT_PRIORITY 255 /* Lowest interrupt priority in Cortex-M3 */
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static enum result setDescriptor(struct SysTickTimer *);
/*----------------------------------------------------------------------------*/
static enum result systickInit(void *, const void *);
static void systickDeinit(void *);
static void systickSetCallback(void *, void (*)(void *), void *);
static void systickSetEnabled(void *, bool);
static void systickSetFrequency(void *, uint32_t);
static void systickSetOverflow(void *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct TimerClass timerTable = {
    .size = sizeof(struct SysTickTimer),
    .init = systickInit,
    .deinit = systickDeinit,

    .setCallback = systickSetCallback,
    .setEnabled = systickSetEnabled,
    .setFrequency = systickSetFrequency,
    .setOverflow = systickSetOverflow
};
/*----------------------------------------------------------------------------*/
const struct TimerClass *SysTickTimer = &timerTable;
/*----------------------------------------------------------------------------*/
static struct SysTickTimer *descriptor = 0;
static Mutex lock = MUTEX_UNLOCKED;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct SysTickTimer *device = object;

  if ((SysTick->CTRL & CTRL_COUNTFLAG) && device->callback)
    device->callback(device->callbackArgument);
}
/*----------------------------------------------------------------------------*/
enum result setDescriptor(struct SysTickTimer *device)
{
  enum result res = E_ERROR;

  mutexLock(&lock);
  if (!descriptor)
  {
    descriptor = device;
    res = E_OK;
  }
  mutexUnlock(&lock);
  return res;
}
/*----------------------------------------------------------------------------*/
void SYSTICK_ISR(void)
{
  if (descriptor)
    descriptor->handler(descriptor);
}
/*----------------------------------------------------------------------------*/
static void systickSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SysTickTimer *device = object;

  device->callback = callback;
  device->callbackArgument = argument;

  if (callback)
  {
    (void)SysTick->CTRL; /* Clear pending interrupt */
    SysTick->CTRL |= CTRL_TICKINT;
  }
  else
    SysTick->CTRL &= ~CTRL_TICKINT;
}
/*----------------------------------------------------------------------------*/
static void systickSetEnabled(void *object, bool state)
{
  if (state)
    SysTick->CTRL |= CTRL_ENABLE;
  else
    SysTick->CTRL &= ~CTRL_ENABLE;
}
/*----------------------------------------------------------------------------*/
static void systickSetFrequency(void *object, uint32_t frequency)
{
  struct SysTickTimer *device = object;

  device->frequency = frequency;

  SysTick->CTRL &= ~CTRL_ENABLE;
  /* FIXME overflow + 1? */
  SysTick->LOAD = (sysCoreClock / device->frequency) * device->overflow - 1;
  SysTick->VAL = 0;
  SysTick->CTRL |= CTRL_ENABLE;
}
/*----------------------------------------------------------------------------*/
static void systickSetOverflow(void *object, uint32_t overflow)
{
  struct SysTickTimer *device = object;

  device->overflow = overflow;

  SysTick->CTRL &= ~CTRL_ENABLE;
  /* FIXME overflow + 1? */
  SysTick->LOAD = (sysCoreClock / device->frequency) * device->overflow - 1;
  SysTick->VAL = 0;
  SysTick->CTRL |= CTRL_ENABLE;
}
/*----------------------------------------------------------------------------*/
static enum result systickInit(void *object, const void *configPtr)
{
  /* Set pointer to device configuration data */
  const struct SysTickTimerConfig * const config = configPtr;
  struct SysTickTimer *device = object;

  /* Check device configuration data */
  assert(config);

  /* Try to set peripheral descriptor */
  if (setDescriptor(device) != E_OK)
    return E_ERROR;

  /* Set hardware interrupt handler to default handler */
  device->handler = interruptHandler;

  device->frequency = config->frequency;
  device->overflow = 1; //FIXME
  SysTick->LOAD = sysCoreClock / device->frequency - 1;
  SysTick->VAL = 0;

  SysTick->CTRL |= CTRL_ENABLE | CTRL_CLKSOURCE;

  /* Enable interrupt */
  nvicEnable(SYSTICK_IRQ);
  /* Set interrupt priority, lowest by default */
  nvicSetPriority(SYSTICK_IRQ, DEFAULT_PRIORITY);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void systickDeinit(void *object)
{
  /* Disable interrupt */
  nvicDisable(SYSTICK_IRQ);
  /* Disable timer */
  SysTick->CTRL &= ~CTRL_ENABLE;
  /* Reset descriptor */
  setDescriptor(0);
}
