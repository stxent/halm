/*
 * platform/nxp/lpc13xx/pin.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_LPC13XX_PIN_H_
#define PLATFORM_NXP_LPC13XX_PIN_H_
/*----------------------------------------------------------------------------*/
#include <platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
enum
{
  PORT_0 = 0,
  PORT_1 = 1,
  PORT_2 = 2,
  PORT_3 = 3,
  PORT_USB
};
/*----------------------------------------------------------------------------*/
enum
{
  PIN_USB_DM,
  PIN_USB_DP
};
/*----------------------------------------------------------------------------*/
struct Pin pinInit(pin_t);
void pinInput(struct Pin);
void pinOutput(struct Pin, uint8_t);
void pinSetFunction(struct Pin, uint8_t);
void pinSetPull(struct Pin, enum pinPull);
void pinSetSlewRate(struct Pin, enum pinSlewRate);
void pinSetType(struct Pin, enum pinType);
/*----------------------------------------------------------------------------*/
static inline uint8_t pinRead(struct Pin pin)
{
  return (((LPC_GPIO_Type *)pin.reg)->DATA & (1 << pin.data.offset)) != 0;
}
/*----------------------------------------------------------------------------*/
static inline void pinReset(struct Pin pin)
{
  ((LPC_GPIO_Type *)pin.reg)->MASKED_ACCESS[1 << pin.data.offset] = 0x000;
}
/*----------------------------------------------------------------------------*/
static inline void pinSet(struct Pin pin)
{
  ((LPC_GPIO_Type *)pin.reg)->MASKED_ACCESS[1 << pin.data.offset] = 0xFFF;
}
/*----------------------------------------------------------------------------*/
static inline void pinWrite(struct Pin pin, uint8_t value)
{
  ((LPC_GPIO_Type *)pin.reg)->MASKED_ACCESS[1 << pin.data.offset] =
      value ? 0xFFF : 0x000;
}
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC13XX_PIN_H_ */
