/*
 * halm/platform/lpc/lpc17xx/pin.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_PIN_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC17XX_PIN_H_
#define HALM_PLATFORM_LPC_LPC17XX_PIN_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
#include <stdbool.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
enum
{
  PORT_0,
  PORT_1,
  PORT_2,
  PORT_3,
  PORT_4
};
/*----------------------------------------------------------------------------*/
struct Pin
{
  void *reg;
  uint32_t mask;
  uint8_t number;
  uint8_t port;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void *pinAddress(struct Pin);
struct Pin pinInit(PinNumber);
void pinInput(struct Pin);
void pinOutput(struct Pin, bool);
void pinToggle(struct Pin);
void pinSetFunction(struct Pin, uint8_t);
void pinSetPull(struct Pin, enum PinPull);
void pinSetType(struct Pin, enum PinType);
void pinSetSlewRate(struct Pin, enum PinSlewRate);

END_DECLS
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline bool pinRead(struct Pin pin)
{
  return (((const LPC_GPIO_Type *)pin.reg)->PIN & pin.mask) != 0;
}

static inline void pinReset(struct Pin pin)
{
  ((LPC_GPIO_Type *)pin.reg)->CLR = pin.mask;
}

static inline void pinSet(struct Pin pin)
{
  ((LPC_GPIO_Type *)pin.reg)->SET = pin.mask;
}

static inline struct Pin pinStub(void)
{
  return (struct Pin){NULL, 0, 0, 0};
}

static inline bool pinValid(struct Pin pin)
{
  return pin.reg != NULL;
}

static inline void pinWrite(struct Pin pin, bool value)
{
  if (value)
    ((LPC_GPIO_Type *)pin.reg)->SET = pin.mask;
  else
    ((LPC_GPIO_Type *)pin.reg)->CLR = pin.mask;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC17XX_PIN_H_ */
