/*
 * halm/platform/bouffalo/bl602/pin.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_PIN_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_BOUFFALO_BL602_PIN_H_
#define HALM_PLATFORM_BOUFFALO_BL602_PIN_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/bouffalo/bl602/pin_defs.h>
#include <halm/platform/platform_defs.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
enum
{
  PORT_DEFAULT
};
/*----------------------------------------------------------------------------*/
struct Pin
{
  uint8_t number;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

struct Pin pinInit(PinNumber);
void pinInput(struct Pin);
void pinOutput(struct Pin, bool);
void pinSetFunction(struct Pin, uint8_t);
void pinSetPull(struct Pin, enum PinPull);
void pinSetSlewRate(struct Pin, enum PinSlewRate);

END_DECLS
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline bool pinRead(struct Pin pin)
{
  return (BL_GLB->GPIO_CFGCTL30 & (1UL << pin.number)) != 0;
}

static inline void pinReset(struct Pin pin)
{
  BL_GLB->GPIO_CFGCTL32 &= ~(1UL << pin.number);
}

static inline void pinSet(struct Pin pin)
{
  BL_GLB->GPIO_CFGCTL32 |= 1UL << pin.number;
}

static inline struct Pin pinStub(void)
{
  return (struct Pin){0xFF};
}

static inline void pinToggle(struct Pin pin)
{
  const uint32_t mask = 1UL << pin.number;

  if (BL_GLB->GPIO_CFGCTL32 & mask)
    BL_GLB->GPIO_CFGCTL32 &= ~mask;
  else
    BL_GLB->GPIO_CFGCTL32 |= mask;
}

static inline bool pinValid(struct Pin pin)
{
  return pin.number != 0xFF;
}

static inline void pinWrite(struct Pin pin, bool value)
{
  const uint32_t mask = 1UL << pin.number;

  if (value)
    BL_GLB->GPIO_CFGCTL32 |= mask;
  else
    BL_GLB->GPIO_CFGCTL32 &= ~mask;
}

static inline void pinSetType(struct Pin, enum PinType)
{
  /* Pin type control is not supported on these devices */
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_BL602_PIN_H_ */
