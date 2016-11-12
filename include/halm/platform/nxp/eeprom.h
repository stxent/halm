/*
 * halm/platform/nxp/eeprom.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_EEPROM_H_
#define HALM_PLATFORM_NXP_EEPROM_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Eeprom;
/*----------------------------------------------------------------------------*/
struct Eeprom
{
  struct Interface base;

  /* Current address */
  uint32_t position;
  /* Size of the memory */
  uint32_t size;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_EEPROM_H_ */
