/*
 * pin.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <pin.h>
#include <platform/nxp/lpc17xx/pin_defs.h>
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_Type *calcPort(struct PinData);
static inline volatile uint32_t *calcPinSelect(struct PinData);
static inline volatile uint32_t *calcPinMode(struct PinData);
static inline volatile uint32_t *calcPinType(struct PinData);
/*----------------------------------------------------------------------------*/
static void commonPinInit(struct Pin);
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_Type *calcPort(struct PinData pin)
{
  return (LPC_GPIO_Type *)((uint32_t)LPC_GPIO0
      + pin.port * ((uint32_t)LPC_GPIO1 - (uint32_t)LPC_GPIO0));
}
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcPinSelect(struct PinData pin)
{
  return &LPC_PINCON->PINSEL0 + (pin.offset >> 4) + (pin.port << 1);
}
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcPinMode(struct PinData pin)
{
  return &LPC_PINCON->PINMODE0 + (pin.offset >> 4) + (pin.port << 1);
}
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcPinType(struct PinData pin)
{
  return &LPC_PINCON->PINMODE_OD0 + pin.port;
}
/*----------------------------------------------------------------------------*/
static void commonPinInit(struct Pin pin)
{
  pinSetFunction(pin, PIN_DEFAULT);
  pinSetPull(pin, PIN_NOPULL);
  pinSetType(pin, PIN_PUSHPULL);
}
/*----------------------------------------------------------------------------*/
void *pinAddress(struct Pin pin)
{
  return (void *)&((LPC_GPIO_Type *)pin.reg)->PIN;
}
/*----------------------------------------------------------------------------*/
struct Pin pinInit(pinNumber id)
{
  struct Pin pin;

  pin.data.port = PIN_TO_PORT(id);
  pin.data.offset = PIN_TO_OFFSET(id);
  pin.reg = calcPort(pin.data);

  return pin;
}
/*----------------------------------------------------------------------------*/
void pinInput(struct Pin pin)
{
  commonPinInit(pin);
  ((LPC_GPIO_Type *)pin.reg)->DIR &= ~(1UL << pin.data.offset);
}
/*----------------------------------------------------------------------------*/
void pinOutput(struct Pin pin, uint8_t value)
{
  commonPinInit(pin);
  ((LPC_GPIO_Type *)pin.reg)->DIR |= 1UL << pin.data.offset;
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
  uint32_t value = *reg & ~PIN_OFFSET(PIN_MASK, pin.data.offset);

  value |= PIN_OFFSET(function, pin.data.offset);

  *reg = value;
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
void pinSetSlewRate(struct Pin pin, enum pinSlewRate rate)
{
  /* Slew rate control is available only for I2C pins */
  if (pin.data.port == 0 && (pin.data.offset >= 27 && pin.data.offset <= 28))
  {
    const uint8_t offset = pin.data.offset - 27;
    const uint32_t mask = I2CPADCFG_DRIVE(offset);
    uint32_t value = LPC_PINCON->I2CPADCFG & ~I2CPADCFG_FILTERING(offset);

    if (rate == PIN_SLEW_FAST)
      value |= mask;
    else
      value &= ~mask;

    LPC_PINCON->I2CPADCFG = value;
  }
}
/*----------------------------------------------------------------------------*/
void pinSetType(struct Pin pin, enum pinType type)
{
  volatile uint32_t * const reg = calcPinType(pin.data);
  uint32_t value = *reg;

  switch (type)
  {
    case PIN_PUSHPULL:
      value &= ~(1UL << pin.data.offset);
      break;

    case PIN_OPENDRAIN:
      value |= 1UL << pin.data.offset;
      break;
  }

  *reg = value;
}
