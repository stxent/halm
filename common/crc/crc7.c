/*
 * crc7.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <crc/crc7.h>
/*----------------------------------------------------------------------------*/
static enum result engineInit(void *, const void *);
static void engineDeinit(void *);
static uint32_t engineUpdate(void *, uint32_t, const uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct CrcEngineClass engineTable = {
    .size = sizeof(struct Crc7),
    .init = engineInit,
    .deinit = engineDeinit,

    .update = engineUpdate
};
/*----------------------------------------------------------------------------*/
const struct CrcEngineClass * const Crc7 = &engineTable;
/*----------------------------------------------------------------------------*/
static enum result engineInit(void *object __attribute__((unused)),
    const void *configBase __attribute__((unused)))
{
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void engineDeinit(void *object __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static uint32_t engineUpdate(void *object __attribute__((unused)),
    uint32_t previous, const uint8_t *buffer, uint32_t length)
{
  uint8_t crc = (uint8_t)previous;

  while (length--)
  {
    uint8_t value = *buffer++;

    for (uint8_t index = 0; index < 8; ++index)
    {
      crc <<= 1;

      if ((value & 0x80) ^ (crc & 0x80))
        crc ^= 0x09;

      value <<= 1;
    }
  }

  return (uint32_t)crc;
}
