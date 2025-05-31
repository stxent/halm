/*
 * halm/platform/numicro/m03x/pin.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_PIN_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_M03X_PIN_H_
#define HALM_PLATFORM_NUMICRO_M03X_PIN_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
enum
{
  PORT_A,
  PORT_B,
  PORT_C,
  PORT_D,
  PORT_E,
  PORT_F,
  PORT_G,
  PORT_H,
  PORT_USB
};

enum
{
  PIN_USB_DM,
  PIN_USB_DP,
  PIN_USB_VBUS
};
/*----------------------------------------------------------------------------*/
struct Pin
{
  void *reg;
  uint8_t number;
  uint8_t port;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void *pinAddress(struct Pin);
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
  *(volatile uint32_t *)pin.reg = 0;
}

static inline void pinSet(struct Pin pin)
{
  *(volatile uint32_t *)pin.reg = 1;
}

static inline struct Pin pinStub(void)
{
  return (struct Pin){NULL, 0, 0};
}

static inline void pinToggle(struct Pin pin)
{
  volatile uint32_t * const reg = (volatile uint32_t *)pin.reg;
  *reg = *reg ? 0 : 1;
}

static inline bool pinValid(struct Pin pin)
{
  return pin.reg != NULL;
}

static inline void pinWrite(struct Pin pin, bool value)
{
  *(volatile uint32_t *)pin.reg = value ? 1 : 0;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M03X_PIN_H_ */
