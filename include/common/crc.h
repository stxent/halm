/*
 * crc.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef CRC_H_
#define CRC_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <entity.h>
/*----------------------------------------------------------------------------*/
struct CrcEngineClass
{
  CLASS_HEADER

  uint32_t (*update)(void *, uint32_t, const uint8_t *, uint32_t);
};
/*----------------------------------------------------------------------------*/
struct CrcEngine
{
  struct Entity parent;
};
/*----------------------------------------------------------------------------*/
static inline uint32_t crcUpdate(void *engine, uint32_t previous,
    const uint8_t *buffer, uint32_t length)
{
  return ((const struct CrcEngineClass *)CLASS(engine))->update(engine,
      previous, buffer, length);
}
/*----------------------------------------------------------------------------*/
#endif /* CRC_H_ */
