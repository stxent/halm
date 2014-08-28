/*
 * platform/nxp/lpc11exx/pin.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PIN_H_
#define PIN_H_
/*----------------------------------------------------------------------------*/
#include <platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
struct Pin pinInit(pin_t);
void pinInput(struct Pin);
void pinOutput(struct Pin, uint8_t);
void pinSetFunction(struct Pin, uint8_t);
void pinSetPull(struct Pin, enum pinPull);
void pinSetType(struct Pin, enum pinType);
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
static inline void pinSetSlewRate(struct Pin pin __attribute__((unused)),
    enum pinSlewRate rate __attribute__((unused)))
{
  /* Slew rate control is not supported on these devices */
}
/*----------------------------------------------------------------------------*/
#endif /* PIN_H_ */
