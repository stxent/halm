/*
 * machine_timer.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/core/riscv/machine_timer.h>
#include <halm/platform/bouffalo/clocking.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void setCurrentValue(uint64_t);
static bool setInstance(struct MachineTimer *);
static void setOverflow(uint64_t);
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);
static void tmrEnable(void *);
static void tmrDisable(void *);
static void tmrSetCallback(void *, void (*)(void *), void *);
static uint32_t tmrGetFrequency(const void *);
static uint32_t tmrGetOverflow(const void *);
static void tmrSetOverflow(void *, uint32_t);
static uint32_t tmrGetValue(const void *);
static void tmrSetValue(void *, uint32_t);
static uint64_t tmrGetValue64(const void *);
static void tmrSetValue64(void *, uint64_t);

#ifndef CONFIG_CORE_RISCV_MACHINE_TIMER_NO_DEINIT
static void tmrDeinit(void *);
#else
#  define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct TimerClass * const MachineTimer = &(const struct TimerClass){
    .size = sizeof(struct MachineTimer),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .enable = tmrEnable,
    .disable = tmrDisable,
    .setAutostop = NULL,
    .setCallback = tmrSetCallback,
    .getFrequency = tmrGetFrequency,
    .setFrequency = NULL,
    .getOverflow = tmrGetOverflow,
    .setOverflow = tmrSetOverflow,
    .getValue = tmrGetValue,
    .setValue = tmrSetValue
};

const struct Timer64Class * const MachineTimer64 = &(const struct Timer64Class){
    .base = {
        .size = sizeof(struct MachineTimer),
        .init = tmrInit,
        .deinit = tmrDeinit,

        .enable = tmrEnable,
        .disable = tmrDisable,
        .setAutostop = NULL,
        .setCallback = tmrSetCallback,
        .getFrequency = tmrGetFrequency,
        .setFrequency = NULL,
        .getOverflow = tmrGetOverflow,
        .setOverflow = tmrSetOverflow,
        .getValue = tmrGetValue,
        .setValue = tmrSetValue
    },

    .getValue64 = tmrGetValue64,
    .setValue64 = tmrSetValue64
};
/*----------------------------------------------------------------------------*/
static struct MachineTimer *instance = NULL;
/*----------------------------------------------------------------------------*/
static void setCurrentValue(uint64_t value)
{
  volatile uint32_t * const mtime = (volatile uint32_t *)&BL_CLIC->MTIME;

  mtime[0] = 0; /* Prevent carry */
  mtime[1] = value >> 32;
  mtime[0] = value;
}
/*----------------------------------------------------------------------------*/
static bool setInstance(struct MachineTimer *object)
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
static void setOverflow(uint64_t value)
{
  volatile uint32_t * const mtimecmp = (volatile uint32_t *)&BL_CLIC->MTIMECMP;

  mtimecmp[1] = UINT32_MAX; /* Prevent interrupt generation */
  mtimecmp[0] = value;
  mtimecmp[1] = value >> 32;
}
/*----------------------------------------------------------------------------*/
[[gnu::interrupt]] void CLIC_MTIP_ISR(void)
{
  setCurrentValue(0);

  if (instance != NULL && instance->callback != NULL)
    instance->callback(instance->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct MachineTimerConfig * const config = configBase;
  struct MachineTimer * const timer = object;

  if (setInstance(timer))
  {
    timer->callback = NULL;

    setOverflow(UINT64_MAX);
    setCurrentValue(0);

    if (config != NULL)
      irqSetPriority(CLIC_MTIP_IRQ, config->priority);

    return E_OK;
  }
  else
    return E_BUSY;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_CORE_RISCV_MACHINE_TIMER_NO_DEINIT
static void tmrDeinit(void *)
{
  irqDisable(CLIC_MTIP_IRQ);
  instance = NULL;
}
#endif
/*----------------------------------------------------------------------------*/
static void tmrEnable(void *)
{
  setCurrentValue(0);
  irqEnable(CLIC_MTIP_IRQ);
}
/*----------------------------------------------------------------------------*/
static void tmrDisable(void *)
{
  irqDisable(CLIC_MTIP_IRQ);
}
/*----------------------------------------------------------------------------*/
static void tmrSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct MachineTimer * const timer = object;

  timer->callback = callback;
  timer->callbackArgument = argument;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetFrequency(const void *)
{
  return clockFrequency(MainClock) / 16;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetOverflow(const void *)
{
  return (uint32_t)BL_CLIC->MTIMECMP + 1;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *, uint32_t overflow)
{
  setOverflow(overflow - 1);
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrGetValue(const void *)
{
  return *(const volatile uint32_t *)&BL_CLIC->MTIME;
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue(void *, uint32_t value)
{
  setCurrentValue(value);
}
/*----------------------------------------------------------------------------*/
static uint64_t tmrGetValue64(const void *)
{
  const volatile uint32_t * const mtime =
      (const volatile uint32_t *)&BL_CLIC->MTIME;
  uint32_t high0, high1, low;

  do
  {
    high0 = *(mtime + 1);
    low = *mtime;
    high1 = *(mtime + 1);
  }
  while (high1 != high0);

  return ((uint64_t)high0 << 32) | low;
}
/*----------------------------------------------------------------------------*/
static void tmrSetValue64(void *, uint64_t value)
{
  setCurrentValue(value);
}
