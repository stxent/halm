/*
 * halm/platform/nxp/lpc43xx/eeprom.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC43XX_EEPROM_H_
#define HALM_PLATFORM_NXP_LPC43XX_EEPROM_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <xcore/interface.h>
#include <halm/irq.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Eeprom;
/*----------------------------------------------------------------------------*/
struct EepromConfig
{
  /** Optional: interrupt priority. */
  irqPriority priority;
};
/*----------------------------------------------------------------------------*/
struct Eeprom
{
  struct Interface base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Current address */
  uintptr_t position;
  /* Size of the memory */
  size_t size;

  /* Output buffer */
  const uint8_t *buffer;
  /* Bytes left in the buffer */
  size_t left;
  /* Offset from the start of the memory */
  uintptr_t offset;

  /* Selection between blocking mode and zero copy mode */
  bool blocking;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC43XX_EEPROM_H_ */
