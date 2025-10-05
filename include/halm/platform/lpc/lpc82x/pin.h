/*
 * halm/platform/lpc/lpc82x/pin.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_PIN_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC82X_PIN_H_
#define HALM_PLATFORM_LPC_LPC82X_PIN_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/lpc82x/pin_mux.h>
#include <halm/platform/platform_defs.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
enum
{
  PORT_0
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

struct Pin pinInit(PinNumber);
void pinInput(struct Pin);
void pinOutput(struct Pin, bool);
void pinSetFunction(struct Pin, uint8_t);
void pinSetMux(struct Pin, enum PinMuxIndex);
void pinSetPull(struct Pin, enum PinPull);
void pinSetSlewRate(struct Pin, enum PinSlewRate);
void pinSetType(struct Pin, enum PinType);

END_DECLS
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline bool pinRead(struct Pin pin)
{
  return (bool)LPC_GPIO->B[pin.number];
}

static inline void pinReset(struct Pin pin)
{
  LPC_GPIO->B[pin.number] = 0;
}

static inline void pinSet(struct Pin pin)
{
  LPC_GPIO->B[pin.number] = 1;
}

static inline struct Pin pinStub(void)
{
  return (struct Pin){NULL, 0, 0};
}

static inline void pinToggle(struct Pin pin)
{
  LPC_GPIO->NOT[0] = 1UL << pin.number;
}

static inline bool pinValid(struct Pin pin)
{
  return pin.reg != NULL;
}

static inline void pinWrite(struct Pin pin, bool value)
{
  /* Only 0 and 1 are allowed */
  LPC_GPIO->B[pin.number] = value;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC82X_PIN_H_ */
