/*
 * halm/platform/lpc/gen_2/pin.h
 * Copyright (C) 2013, 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LPC_GEN_2_PIN_H_
#define HALM_PLATFORM_LPC_GEN_2_PIN_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
struct Pin
{
  void *reg;
  uint8_t number;
  uint8_t port;
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

static inline bool pinValid(struct Pin pin)
{
  return pin.reg != 0;
}

static inline void pinWrite(struct Pin pin, bool value)
{
  *(volatile uint32_t *)pin.reg = value ? 0xFFF : 0x000;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_PIN_H_ */
