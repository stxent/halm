/*
 * pin.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/pin.h>
#include <halm/platform/lpc/lpc17xx/pin_defs.h>
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_Type *calcPort(uint8_t);
static inline volatile uint32_t *calcPinSelect(uint8_t, uint8_t);
static inline volatile uint32_t *calcPinMode(uint8_t, uint8_t);
static inline volatile uint32_t *calcPinType(uint8_t);
/*----------------------------------------------------------------------------*/
static void commonPinInit(struct Pin);
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_Type *calcPort(uint8_t port)
{
  return &LPC_GPIO[port];
}
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcPinSelect(uint8_t port, uint8_t number)
{
  return LPC_PINCON->PINSEL + (number >> 4) + (port << 1);
}
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcPinMode(uint8_t port, uint8_t number)
{
  return LPC_PINCON->PINMODE + (number >> 4) + (port << 1);
}
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcPinType(uint8_t port)
{
  return LPC_PINCON->PINMODE_OD + port;
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
struct Pin pinInit(PinNumber id)
{
  struct Pin pin;

  pin.port = PIN_TO_PORT(id);
  pin.number = PIN_TO_OFFSET(id);
  pin.mask = 1UL << pin.number;
  pin.reg = id ? calcPort(pin.port) : 0;

  return pin;
}
/*----------------------------------------------------------------------------*/
void pinInput(struct Pin pin)
{
  commonPinInit(pin);
  ((LPC_GPIO_Type *)pin.reg)->DIR &= ~pin.mask;
}
/*----------------------------------------------------------------------------*/
void pinOutput(struct Pin pin, bool value)
{
  commonPinInit(pin);
  ((LPC_GPIO_Type *)pin.reg)->DIR |= pin.mask;
  pinWrite(pin, value);
}
/*----------------------------------------------------------------------------*/
void pinToggle(struct Pin pin)
{
  pinWrite(pin, !pinRead(pin));
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

  volatile uint32_t * const reg = calcPinSelect(pin.port, pin.number);
  uint32_t value = *reg & ~PIN_OFFSET(PIN_MASK, pin.number);

  value |= PIN_OFFSET(function, pin.number);

  *reg = value;
}
/*----------------------------------------------------------------------------*/
void pinSetPull(struct Pin pin, enum PinPull pull)
{
  volatile uint32_t * const reg = calcPinMode(pin.port, pin.number);
  uint32_t value = *reg & ~PIN_OFFSET(PIN_MASK, pin.number);

  switch (pull)
  {
    case PIN_NOPULL:
      value |= PIN_OFFSET(PIN_MODE_INACTIVE, pin.number);
      break;

    case PIN_PULLUP:
      value |= PIN_OFFSET(PIN_MODE_PULLUP, pin.number);
      break;

    case PIN_PULLDOWN:
      value |= PIN_OFFSET(PIN_MODE_PULLDOWN, pin.number);
      break;
  }

  *reg = value;
}
/*----------------------------------------------------------------------------*/
void pinSetSlewRate(struct Pin pin, enum PinSlewRate rate)
{
  /* Slew rate control is available only for I2C pins */
  if (pin.port == 0 && (pin.number >= 27 && pin.number <= 28))
  {
    const uint8_t offset = pin.number - 27;
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
void pinSetType(struct Pin pin, enum PinType type)
{
  volatile uint32_t * const reg = calcPinType(pin.port);
  uint32_t value = *reg;

  switch (type)
  {
    case PIN_PUSHPULL:
      value &= ~pin.mask;
      break;

    case PIN_OPENDRAIN:
      value |= pin.mask;
      break;
  }

  *reg = value;
}
