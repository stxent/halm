/*
 * halm/platform/nxp/gen_1/eeprom_base.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_GEN_1_EEPROM_BASE_H_
#define HALM_PLATFORM_NXP_GEN_1_EEPROM_BASE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/entity.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const EepromBase;

struct EepromBase
{
  struct Entity base;

  /* Total memory size */
  uint32_t size;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_GEN_1_EEPROM_BASE_H_ */
