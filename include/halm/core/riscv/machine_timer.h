/*
 * halm/core/riscv/machine_timer.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_RISCV_MACHINE_TIMER_H_
#define HALM_CORE_RISCV_MACHINE_TIMER_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const MachineTimer;
extern const struct Timer64Class * const MachineTimer64;

struct MachineTimerConfig
{
  /** Optional: timer interrupt priority. */
  IrqPriority priority;
};

struct MachineTimer
{
  struct Timer base;

  void (*callback)(void *);
  void *callbackArgument;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_RISCV_MACHINE_TIMER_H_ */
