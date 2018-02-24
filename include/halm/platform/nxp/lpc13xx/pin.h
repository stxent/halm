/*
 * halm/platform/nxp/lpc13xx/pin.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC13XX_PIN_H_
#define HALM_PLATFORM_NXP_LPC13XX_PIN_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
enum
{
  PORT_0,
  PORT_1,
  PORT_2,
  PORT_3,
  PORT_USB
};

enum
{
  PIN_USB_DM,
  PIN_USB_DP
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
  return *(const volatile uint32_t *)pin.reg != 0;
}

static inline void pinReset(struct Pin pin)
{
  *(volatile uint32_t *)pin.reg = 0x000;
}

static inline void pinSet(struct Pin pin)
{
  *(volatile uint32_t *)pin.reg = 0xFFF;
}

static inline void pinToggle(struct Pin pin)
{
  *(volatile uint32_t *)pin.reg = ~(*(volatile uint32_t *)pin.reg);
}

static inline void pinWrite(struct Pin pin, bool value)
{
  *(volatile uint32_t *)pin.reg = 0xFFF * value;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC13XX_PIN_H_ */
