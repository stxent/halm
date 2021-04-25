/*
 * halm/platform/lpc/lpc43xx/eeprom.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC43XX_EEPROM_H_
#define HALM_PLATFORM_LPC_LPC43XX_EEPROM_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/platform/lpc/lpc43xx/eeprom_base.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Eeprom;

struct EepromConfig
{
  /** Optional: interrupt priority. */
  IrqPriority priority;
};

struct Eeprom
{
  struct EepromBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Current address */
  uint32_t position;

  /* Input buffer */
  const uint32_t *buffer;
  /* Offset from the start of the memory */
  uint32_t offset;
  /* Words left in the buffer */
  size_t left;

  /* Selection between blocking mode and zero copy mode */
  bool blocking;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_EEPROM_H_ */
