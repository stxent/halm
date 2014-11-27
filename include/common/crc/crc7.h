/*
 * crc7.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef CRC7_H_
#define CRC7_H_
/*----------------------------------------------------------------------------*/
#include <crc.h>
/*----------------------------------------------------------------------------*/
extern const struct CrcEngineClass * const Crc7;
/*----------------------------------------------------------------------------*/
struct Crc7
{
  struct CrcEngine parent;
};
/*----------------------------------------------------------------------------*/
#endif /* CRC7_H_ */
