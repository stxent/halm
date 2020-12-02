/*
 * halm/platform/nxp/gen_1/eeprom.h
 * Copyright (C) 2016, 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_GEN_1_EEPROM_H_
#define HALM_PLATFORM_NXP_GEN_1_EEPROM_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/nxp/gen_1/eeprom_base.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Eeprom;

struct Eeprom
{
  struct EepromBase base;

  /* Current address */
  uint32_t position;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_GEN_1_EEPROM_H_ */
