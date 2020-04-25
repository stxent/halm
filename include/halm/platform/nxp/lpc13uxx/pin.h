/*
 * halm/platform/nxp/lpc13uxx/pin.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC13UXX_PIN_H_
#define HALM_PLATFORM_NXP_LPC13UXX_PIN_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
enum
{
  PORT_0,
  PORT_1
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

struct Pin pinInit(PinNumber);
void pinInput(struct Pin);
void pinOutput(struct Pin, bool);
void pinSetFunction(struct Pin, uint8_t);
void pinSetPull(struct Pin, enum PinPull);
void pinSetSlewRate(struct Pin, enum PinSlewRate);
void pinSetType(struct Pin, enum PinType);

END_DECLS
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline bool pinRead(struct Pin pin)
{
  return (bool)LPC_GPIO->B[(pin.data.port << 5) + pin.data.offset];
}

static inline void pinReset(struct Pin pin)
{
  LPC_GPIO->B[(pin.data.port << 5) + pin.data.offset] = 0;
}

static inline void pinSet(struct Pin pin)
{
  LPC_GPIO->B[(pin.data.port << 5) + pin.data.offset] = 1;
}

static inline void pinToggle(struct Pin pin)
{
  LPC_GPIO->NOT[pin.data.port] = 1UL << pin.data.offset;
}

static inline void pinWrite(struct Pin pin, bool value)
{
  /* Only 0 and 1 are allowed */
  LPC_GPIO->B[(pin.data.port << 5) + pin.data.offset] = value;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC13UXX_PIN_H_ */
