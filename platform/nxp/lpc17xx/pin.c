/*
 * pin.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <pin.h>
#include <platform/nxp/lpc17xx/pin_defs.h>
/*----------------------------------------------------------------------------*/
static inline void *calcPort(union PinData);
static inline volatile uint32_t *calcPinSelect(union PinData);
static inline volatile uint32_t *calcPinMode(union PinData);
static inline volatile uint32_t *calcPinType(union PinData);
/*----------------------------------------------------------------------------*/
static void commonPinSetup(struct Pin);
/*----------------------------------------------------------------------------*/
static inline void *calcPort(union PinData pin)
{
  return (void *)(((uint32_t)LPC_GPIO1 - (uint32_t)LPC_GPIO0) * pin.port
      + (uint32_t)LPC_GPIO0);
}
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcPinSelect(union PinData pin)
{
  return &LPC_PINCON->PINSEL0 + (pin.offset >> 4) + (pin.port << 1);
}
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcPinMode(union PinData pin)
{
  return &LPC_PINCON->PINMODE0 + (pin.offset >> 4) + (pin.port << 1);
}
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcPinType(union PinData pin)
{
  return &LPC_PINCON->PINMODE_OD0 + pin.port;
}
/*----------------------------------------------------------------------------*/
static void commonPinSetup(struct Pin pin)
{
  pinSetFunction(pin, PIN_DEFAULT);
  pinSetPull(pin, PIN_NOPULL);
  pinSetType(pin, PIN_PUSHPULL);
}
/*----------------------------------------------------------------------------*/
struct Pin pinInit(pin_t id)
{
  struct Pin pin;

  pin.data.key = ~id;
  pin.reg = calcPort(pin.data);

  return pin;
}
/*----------------------------------------------------------------------------*/
void pinInput(struct Pin pin)
{
  commonPinSetup(pin);
  ((LPC_GPIO_Type *)pin.reg)->DIR &= ~(1 << pin.data.offset);
}
/*----------------------------------------------------------------------------*/
void pinOutput(struct Pin pin, uint8_t value)
{
  commonPinSetup(pin);
  ((LPC_GPIO_Type *)pin.reg)->DIR |= 1 << pin.data.offset;
  pinWrite(pin, value);
}
/*----------------------------------------------------------------------------*/
void pinSetFunction(struct Pin pin, uint8_t function)
{
  /* Function should not be used outside platform drivers */
  switch (function)
  {
    case PIN_DEFAULT:
      function = 0; /* Zero function is the default for all pins */
      break;

    case PIN_ANALOG:
      return;
  }

  volatile uint32_t * const reg = calcPinSelect(pin.data);

  *reg = (*reg & ~PIN_OFFSET(PIN_MASK, pin.data.offset))
      | PIN_OFFSET(function, pin.data.offset);
}
/*----------------------------------------------------------------------------*/
void pinSetPull(struct Pin pin, enum pinPull pull)
{
  volatile uint32_t * const reg = calcPinMode(pin.data);
  uint32_t value = *reg & ~PIN_OFFSET(PIN_MASK, pin.data.offset);

  switch (pull)
  {
    case PIN_NOPULL:
      value |= PIN_OFFSET(PIN_MODE_INACTIVE, pin.data.offset);
      break;

    case PIN_PULLUP:
      value |= PIN_OFFSET(PIN_MODE_PULLUP, pin.data.offset);
      break;

    case PIN_PULLDOWN:
      value |= PIN_OFFSET(PIN_MODE_PULLDOWN, pin.data.offset);
      break;
  }

  *reg = value;
}
/*----------------------------------------------------------------------------*/
void pinSetType(struct Pin pin, enum pinType type)
{
  volatile uint32_t * const reg = calcPinType(pin.data);
  uint32_t value = *reg;

  switch (type)
  {
    case PIN_PUSHPULL:
      value &= ~(1 << pin.data.offset);
      break;

    case PIN_OPENDRAIN:
      value |= 1 << pin.data.offset;
      break;
  }

  *reg = value;
}
