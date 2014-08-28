/*
 * platform/nxp/lpc43xx/pin.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_LPC43XX_PIN_H_
#define PLATFORM_NXP_LPC43XX_PIN_H_
/*----------------------------------------------------------------------------*/
#include <platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
struct Pin pinInit(pin_t);
void pinInput(struct Pin);
void pinOutput(struct Pin, uint8_t);
void pinSetFunction(struct Pin, uint8_t);
void pinSetPull(struct Pin, enum pinPull);
void pinSetSlewRate(struct Pin, enum pinSlewRate);
/*----------------------------------------------------------------------------*/
static inline uint8_t pinRead(struct Pin pin)
{
  return LPC_GPIO->B[(pin.data.port << 5) + pin.data.offset];
}
/*----------------------------------------------------------------------------*/
static inline void pinReset(struct Pin pin)
{
  LPC_GPIO->B[(pin.data.port << 5) + pin.data.offset] = 0;
}
/*----------------------------------------------------------------------------*/
static inline void pinSet(struct Pin pin)
{
  LPC_GPIO->B[(pin.data.port << 5) + pin.data.offset] = 1;
}
/*----------------------------------------------------------------------------*/
static inline void pinWrite(struct Pin pin, uint8_t value)
{
  /* Only 0 and 1 are allowed */
  LPC_GPIO->B[(pin.data.port << 5) + pin.data.offset] = value;
}
/*----------------------------------------------------------------------------*/
static inline void pinSetType(struct Pin pin __attribute__((unused)),
    enum pinType type __attribute__((unused)))
{
  /* Pin type control is not supported on these devices */
}
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC43XX_PIN_H_ */
