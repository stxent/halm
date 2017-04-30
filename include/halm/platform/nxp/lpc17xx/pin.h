/*
 * halm/platform/nxp/lpc17xx/pin.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC17XX_PIN_H_
#define HALM_PLATFORM_NXP_LPC17XX_PIN_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
void *pinAddress(struct Pin);
struct Pin pinInit(PinNumber);
void pinInput(struct Pin);
void pinOutput(struct Pin, bool);
void pinToggle(struct Pin);
void pinSetFunction(struct Pin, uint8_t);
void pinSetPull(struct Pin, enum PinPull);
void pinSetType(struct Pin, enum PinType);
void pinSetSlewRate(struct Pin, enum PinSlewRate);
/*----------------------------------------------------------------------------*/
static inline bool pinRead(struct Pin pin)
{
  const uint32_t mask = 1UL << pin.data.offset;

  return (((const LPC_GPIO_Type *)pin.reg)->PIN & mask) != 0;
}
/*----------------------------------------------------------------------------*/
static inline void pinReset(struct Pin pin)
{
  ((LPC_GPIO_Type *)pin.reg)->CLR = 1UL << pin.data.offset;
}
/*----------------------------------------------------------------------------*/
static inline void pinSet(struct Pin pin)
{
  ((LPC_GPIO_Type *)pin.reg)->SET = 1UL << pin.data.offset;
}
/*----------------------------------------------------------------------------*/
static inline void pinWrite(struct Pin pin, bool value)
{
  const uint32_t mask = 1UL << pin.data.offset;

  if (value)
    ((LPC_GPIO_Type *)pin.reg)->SET = mask;
  else
    ((LPC_GPIO_Type *)pin.reg)->CLR = mask;
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC17XX_PIN_H_ */
