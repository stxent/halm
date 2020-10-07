/*
 * halm/pm.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Power Management interface for embedded systems.
 */

#ifndef HALM_PM_H_
#define HALM_PM_H_
/*----------------------------------------------------------------------------*/
#include <xcore/error.h>
#include <xcore/helpers.h>
#include <stdbool.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
enum PmState
{
  /** Normal operational mode. */
  PM_ACTIVE,
  /**
   * Low power mode with a slightly lower power consumption than active mode.
   * The system is able to resume normal mode almost immediately.
   */
  PM_SLEEP,
  /**
   * Lower power consumption mode. Logic states and system memory are
   * maintained. Resuming from this mode takes longer time and may require
   * additional hardware features.
   */
  PM_SUSPEND,
  /**
   * The lowest power consumption mode. The logic state of the entire system
   * with the exception of backup domain is lost. Resuming from this mode
   * requires complete reinitialization of hardware.
   */
  PM_SHUTDOWN
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/**
 * Change the system state.
 * Two processor-specific functions should be defined externally:
 * @b pmCoreChangeState for core-dependent code and
 * @b pmPlatformChangeState for platform-dependent code.
 * When the low power mode is selected, function returns after resuming from
 * that mode to the active state.
 * @param state Requested low-power state.
 */
void pmChangeState(enum PmState state);

/**
 * Register a callback function.
 * @param object Pointer to an object used as function argument.
 * @param callback Callback function.
 * @return @b E_OK on success.
 */
enum Result pmRegister(void (*)(void *, enum PmState), void *object);

/**
 * Unregister the callback function.
 * @param object Pointer to an object to be deleted from the list.
 */
void pmUnregister(const void *object);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PM_H_ */
