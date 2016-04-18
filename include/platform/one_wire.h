/*
 * platform/one_wire.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_ONE_WIRE_SSP_H_
#define HALM_PLATFORM_ONE_WIRE_SSP_H_
/*----------------------------------------------------------------------------*/
#include <interface.h>
/*----------------------------------------------------------------------------*/
enum oneWireOption
{
  /** Start the device search. */
  IF_ONE_WIRE_START_SEARCH = IF_OPTION_END,
  /**
   * Read an address of the next device. Returns @b E_EMPTY when
   * all devices have already been found or @b E_OK otherwise.
   */
  IF_ONE_WIRE_FIND_NEXT
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_ONE_WIRE_SSP_H_ */
