/*
 * halm/common/can.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_COMMON_CAN_H_
#define HALM_COMMON_CAN_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
enum canFlags
{
  CAN_EXTID = 0x01,
  CAN_RTR   = 0x02,
  CAN_FD    = 0x04
};
/*----------------------------------------------------------------------------*/
struct CanMessage
{
  uint32_t id;
  uint8_t flags;
  uint8_t length;
  uint8_t data[];
};

struct CanStandardMessage
{
  uint32_t id;
  uint8_t flags;
  uint8_t length;
  uint8_t data[8];
};

struct CanFdMessage
{
  uint32_t id;
  uint8_t flags;
  uint8_t length;
  uint8_t data[64];
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_COMMON_CAN_H_ */
