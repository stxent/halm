/*
 * halm/platform/nxp/lpc11xx/pin.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC11XX_PIN_H_
#define HALM_PLATFORM_NXP_LPC11XX_PIN_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
struct Pin pinInit(pinNumber);
void pinInput(struct Pin);
void pinOutput(struct Pin, bool);
void pinSetFunction(struct Pin, uint8_t);
void pinSetPull(struct Pin, enum pinPull);
void pinSetSlewRate(struct Pin, enum pinSlewRate);
void pinSetType(struct Pin, enum pinType);
/*----------------------------------------------------------------------------*/
static inline bool pinRead(struct Pin pin)
{
  const uint32_t mask = 1UL << pin.data.offset;

  return (((const LPC_GPIO_Type *)pin.reg)->DATA & mask) != 0;
}
/*----------------------------------------------------------------------------*/
static inline void pinReset(struct Pin pin)
{
  const uint32_t mask = 1UL << pin.data.offset;

  ((LPC_GPIO_Type *)pin.reg)->MASKED_ACCESS[mask] = 0x000;
}
/*----------------------------------------------------------------------------*/
static inline void pinSet(struct Pin pin)
{
  const uint32_t mask = 1UL << pin.data.offset;

  ((LPC_GPIO_Type *)pin.reg)->MASKED_ACCESS[mask] = 0xFFF;
}
/*----------------------------------------------------------------------------*/
static inline void pinWrite(struct Pin pin, bool value)
{
  const uint32_t mask = 1UL << pin.data.offset;

  ((LPC_GPIO_Type *)pin.reg)->MASKED_ACCESS[mask] = value ? 0xFFF : 0x000;
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC11XX_PIN_H_ */
