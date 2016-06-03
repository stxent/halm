/*
 * pin.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/pin.h>
#include <halm/platform/nxp/lpc13xx/pin_defs.h>
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_Type *calcPort(struct PinData);
static inline volatile uint32_t *calcControlReg(struct PinData);
static void commonPinInit(struct Pin);
static bool isCommonPin(struct Pin);
/*----------------------------------------------------------------------------*/
static const uint8_t pinRegMap[4][12] = {
    {0x0C, 0x10, 0x1C, 0x2C, 0x30, 0x34, 0x4C, 0x50, 0x60, 0x64, 0x68, 0x74},
    {0x78, 0x7C, 0x80, 0x90, 0x94, 0xA0, 0xA4, 0xA8, 0x14, 0x38, 0x6C, 0x98},
    {0x08, 0x28, 0x5C, 0x8C, 0x40, 0x44, 0x00, 0x20, 0x24, 0x54, 0x58, 0x70},
    {0x84, 0x88, 0x9C, 0xAC, 0x3C, 0x48}
};
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_Type *calcPort(struct PinData data)
{
  return (LPC_GPIO_Type *)((uint32_t)LPC_GPIO0
      + data.port * ((uint32_t)LPC_GPIO1 - (uint32_t)LPC_GPIO0));
}
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcControlReg(struct PinData data)
{
  return (volatile uint32_t *)((uint32_t)LPC_IOCON
      + pinRegMap[data.port][data.offset]);
}
/*----------------------------------------------------------------------------*/
static void commonPinInit(struct Pin pin)
{
  pinSetFunction(pin, PIN_DEFAULT);
  pinSetPull(pin, PIN_NOPULL);
  pinSetSlewRate(pin, PIN_SLEW_NORMAL);
  pinSetType(pin, PIN_PUSHPULL);
}
/*----------------------------------------------------------------------------*/
static bool isCommonPin(struct Pin pin)
{
  /* PIO0_4 and PIO0_5 are I2C pins with different function set */
  return pin.data.port != 0 || (pin.data.offset < 4 || pin.data.offset > 5);
}
/*----------------------------------------------------------------------------*/
struct Pin pinInit(pinNumber id)
{
  struct Pin pin;

  pin.data.port = PIN_TO_PORT(id);
  pin.data.offset = PIN_TO_OFFSET(id);
  pin.reg = pin.data.port != PORT_USB ? calcPort(pin.data) : 0;

  return pin;
}
/*----------------------------------------------------------------------------*/
void pinInput(struct Pin pin)
{
  if (!pin.reg)
    return;

  commonPinInit(pin);
  ((LPC_GPIO_Type *)pin.reg)->DIR &= ~(1 << pin.data.offset);
}
/*----------------------------------------------------------------------------*/
void pinOutput(struct Pin pin, uint8_t value)
{
  if (!pin.reg)
    return;

  commonPinInit(pin);
  ((LPC_GPIO_Type *)pin.reg)->DIR |= 1 << pin.data.offset;
  pinWrite(pin, value);
}
/*----------------------------------------------------------------------------*/
void pinSetFunction(struct Pin pin, uint8_t function)
{
  if (!pin.reg)
    return;

  volatile uint32_t * const reg = calcControlReg(pin.data);
  const uint32_t value = *reg;

  switch (function)
  {
    case PIN_DEFAULT:
    {
      /* Some pins have default function value other than zero */
      bool alternate;

      alternate = pin.data.port == 0 && (pin.data.offset == 0
          || (pin.data.offset >= 10 && pin.data.offset <= 11));
      alternate = alternate || (pin.data.port == 1 && pin.data.offset <= 3);
      function = alternate ? 1 : 0;
      break;
    }

    case PIN_ANALOG:
      *reg = value & ~IOCON_DIGITAL;
      return;
  }

  *reg = (value & ~IOCON_FUNC_MASK) | IOCON_FUNC(function);
}
/*----------------------------------------------------------------------------*/
void pinSetPull(struct Pin pin, enum pinPull pull)
{
  if (!pin.reg || !isCommonPin(pin))
    return;

  volatile uint32_t * const reg = calcControlReg(pin.data);
  uint32_t value = *reg & ~IOCON_MODE_MASK;

  switch (pull)
  {
    case PIN_NOPULL:
      value |= IOCON_MODE_INACTIVE;
      break;

    case PIN_PULLUP:
      value |= IOCON_MODE_PULLUP;
      break;

    case PIN_PULLDOWN:
      value |= IOCON_MODE_PULLDOWN;
      break;
  }

  *reg = value;
}
/*----------------------------------------------------------------------------*/
void pinSetSlewRate(struct Pin pin, enum pinSlewRate rate)
{
  /* Slew rate control is available only for I2C pins */
  if (!pin.reg || isCommonPin(pin))
    return;

  volatile uint32_t * const reg = calcControlReg(pin.data);

  *reg = (*reg & ~IOCON_I2C_MASK)
      | (rate == PIN_SLEW_FAST ? IOCON_I2C_PLUS : 0);
}
/*----------------------------------------------------------------------------*/
void pinSetType(struct Pin pin, enum pinType type)
{
  if (!pin.reg || !isCommonPin(pin))
    return;

  volatile uint32_t * const reg = calcControlReg(pin.data);

  switch (type)
  {
    case PIN_PUSHPULL:
      *reg &= ~IOCON_OD;
      break;

    case PIN_OPENDRAIN:
      *reg |= IOCON_OD;
      break;
  }
}
