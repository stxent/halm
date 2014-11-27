/*
 * crc16.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef CRC16_H_
#define CRC16_H_
/*----------------------------------------------------------------------------*/
#include <crc.h>
/*----------------------------------------------------------------------------*/
extern const struct CrcEngineClass * const Crc16;
/*----------------------------------------------------------------------------*/
struct Crc16
{
  struct CrcEngine parent;
};
/*----------------------------------------------------------------------------*/
#endif /* CRC16_H_ */
