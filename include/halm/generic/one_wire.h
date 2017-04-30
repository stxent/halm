/*
 * halm/generic/one_wire.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_GENERIC_ONE_WIRE_H_
#define HALM_GENERIC_ONE_WIRE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
enum OneWireOption
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
#endif /* HALM_GENERIC_ONE_WIRE_H_ */
