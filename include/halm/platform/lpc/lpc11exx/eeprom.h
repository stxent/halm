/*
 * halm/platform/lpc/lpc11exx/eeprom.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC11EXX_EEPROM_H_
#define HALM_PLATFORM_LPC_LPC11EXX_EEPROM_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Eeprom;

struct Eeprom
{
  struct Interface base;

  /* Current address */
  uintptr_t position;
  /* Size of the memory */
  size_t size;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC11EXX_EEPROM_H_ */
