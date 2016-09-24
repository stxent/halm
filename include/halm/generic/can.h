/*
 * halm/generic/can.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_GENERIC_CAN_H_
#define HALM_GENERIC_CAN_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
enum canFlags
{
  CAN_EXT_ID  = 0x01,
  CAN_RTR     = 0x02,
  CAN_FD      = 0x04,
  CAN_SELF_RX = 0x08
};
/*----------------------------------------------------------------------------*/
enum canOption
{
  /** Enable active mode. */
  IF_CAN_ACTIVE = IF_OPTION_END,
  /** Enable listener mode. This mode is set by default. */
  IF_CAN_LISTENER,
  /** Enable loopback mode. */
  IF_CAN_LOOPBACK
};
/*----------------------------------------------------------------------------*/
struct CanMessage
{
  uint32_t timestamp;
  uint32_t id;
  uint8_t flags;
  uint8_t length;
  uint8_t data[];
};

struct CanStandardMessage
{
  uint32_t timestamp;
  uint32_t id;
  uint8_t flags;
  uint8_t length;
  uint8_t data[8];
};

struct CanFdMessage
{
  uint32_t timestamp;
  uint32_t id;
  uint8_t flags;
  uint8_t length;
  uint8_t data[64];
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_CAN_H_ */
