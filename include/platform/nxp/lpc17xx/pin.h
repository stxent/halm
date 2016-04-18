/*
 * platform/nxp/lpc17xx/pin.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC17XX_PIN_H_
#define HALM_PLATFORM_NXP_LPC17XX_PIN_H_
/*----------------------------------------------------------------------------*/
#include <platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
void *pinAddress(struct Pin);
struct Pin pinInit(pinNumber);
void pinInput(struct Pin);
void pinOutput(struct Pin, uint8_t);
void pinSetFunction(struct Pin, uint8_t);
void pinSetPull(struct Pin, enum pinPull);
void pinSetType(struct Pin, enum pinType);
void pinSetSlewRate(struct Pin, enum pinSlewRate);
/*----------------------------------------------------------------------------*/
static inline uint8_t pinRead(struct Pin pin)
{
  return (((const LPC_GPIO_Type *)pin.reg)->PIN & (1 << pin.data.offset)) != 0;
}
/*----------------------------------------------------------------------------*/
static inline void pinReset(struct Pin pin)
{
  ((LPC_GPIO_Type *)pin.reg)->CLR = 1 << pin.data.offset;
}
/*----------------------------------------------------------------------------*/
static inline void pinSet(struct Pin pin)
{
  ((LPC_GPIO_Type *)pin.reg)->SET = 1 << pin.data.offset;
}
/*----------------------------------------------------------------------------*/
static inline void pinWrite(struct Pin pin, uint8_t value)
{
  if (value)
    ((LPC_GPIO_Type *)pin.reg)->SET = 1 << pin.data.offset;
  else
    ((LPC_GPIO_Type *)pin.reg)->CLR = 1 << pin.data.offset;
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC17XX_PIN_H_ */
