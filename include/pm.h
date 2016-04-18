/*
 * pm.h
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
#include <stdbool.h>
#include <stdint.h>
#include <error.h>
/*----------------------------------------------------------------------------*/
enum pmState
{
  /** Normal operational mode. */
  PM_ACTIVE,
  /**
   * Lower power consumption mode. The system is able to resume normal
   * mode almost immediately.
   */
  PM_SLEEP,
  /**
   * The lowest power consumption mode. Resuming from this mode takes longer
   * time and may require additional hardware features.
   */
  PM_SUSPEND
};
/*----------------------------------------------------------------------------*/
typedef enum result (*PmCallback)(void *, enum pmState);
/*----------------------------------------------------------------------------*/
/**
 * Change the system state.
 * Two processor-specific functions should be defined externally:
 * @b pmCoreChangeState for core-dependent code and
 * @b pmPlatformChangeState for platform-dependent code.
 * When the low power mode is selected, function returns after resuming from
 * that mode to the active state.
 * @param state Next system state.
 * @return @b E_OK on success or error otherwise.
 */
enum result pmChangeState(enum pmState state);
/*----------------------------------------------------------------------------*/
/**
 * Register a callback function.
 * @param object Pointer to an object used as function argument.
 * @param callback Callback function.
 * @return @b E_OK on success.
 */
enum result pmRegister(void *object, PmCallback callback);
/*----------------------------------------------------------------------------*/
/**
 * Unregister the callback function.
 * @param object Pointer to an object to be deleted from the list.
 */
void pmUnregister(const void *object);
/*----------------------------------------------------------------------------*/
#endif /* HALM_PM_H_ */
