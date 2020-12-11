/*
 * halm/platform/lpc/gen_1/eeprom.h
 * Copyright (C) 2016, 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_EEPROM_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_1_EEPROM_H_
#define HALM_PLATFORM_LPC_GEN_1_EEPROM_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/gen_1/eeprom_base.h>
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
#endif /* HALM_PLATFORM_LPC_GEN_1_EEPROM_H_ */
