/*
 * pin.c
 * Copyright (C) 2014, 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/pin.h>
#include <halm/platform/nxp/gen_3/pin_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static volatile uint32_t *calcControlReg(uint8_t, uint8_t);
static void commonPinInit(struct Pin);
static inline bool isI2CPin(struct Pin);
static inline bool isSystemPin(struct Pin);
/*----------------------------------------------------------------------------*/
static volatile uint32_t *calcControlReg(uint8_t port, uint8_t number)
{
  switch (port)
  {
    case 0:
      return &LPC_IOCON->PIO0[number];

    case 1:
      return &LPC_IOCON->PIO1[number];

    default:
      return 0;
  }
}
/*----------------------------------------------------------------------------*/
static void commonPinInit(struct Pin pin)
{
  pinSetFunction(pin, PIN_DEFAULT);
  pinSetPull(pin, PIN_NOPULL);
  pinSetType(pin, PIN_PUSHPULL);
  pinSetSlewRate(pin, PIN_SLEW_NORMAL);
}
/*----------------------------------------------------------------------------*/
static bool isI2CPin(struct Pin pin)
{
  /* PIO0_4 and PIO0_5 are I2C pins with different function set */
  return pin.port == 0 && pin.number >= 4 && pin.number <= 5;
}
/*----------------------------------------------------------------------------*/
static bool isSystemPin(struct Pin pin)
{
  /* PIO0_0 is Reset pin, PIO0_10 through PIO0_15 are JTAG/SWD pins */
  return pin.port == 0
      && (pin.number == 0 || (pin.number >= 10 && pin.number <= 15));
}
/*----------------------------------------------------------------------------*/
struct Pin pinInit(PinNumber id)
{
  struct Pin pin;

  pin.port = PIN_TO_PORT(id);
  pin.number = PIN_TO_OFFSET(id);
  pin.index = (pin.port << 5) + pin.number;

  pin.reg = (void *)calcControlReg(pin.port, pin.number);
  assert(pin.reg);

  return pin;
}
/*----------------------------------------------------------------------------*/
void pinInput(struct Pin pin)
{
  if (!pin.reg)
    return;

  commonPinInit(pin);
  LPC_GPIO->DIR[pin.port] &= ~(1UL << pin.number);
}
/*----------------------------------------------------------------------------*/
void pinOutput(struct Pin pin, bool value)
{
  if (!pin.reg)
    return;

  commonPinInit(pin);
  LPC_GPIO->DIR[pin.port] |= 1UL << pin.number;
  pinWrite(pin, value);
}
/*----------------------------------------------------------------------------*/
void pinSetFunction(struct Pin pin, uint8_t function)
{
  if (!pin.reg)
    return;

  volatile uint32_t * const reg = pin.reg;
  uint32_t value = *reg;

  switch (function)
  {
    case PIN_ANALOG:
      *reg = value & ~IOCON_DIGITAL;
      return;

    case PIN_DEFAULT:
      value &= ~IOCON_FUNC_MASK;

      /* Default function for Reset and JTAG/SWD pins is 1 */
      if (isSystemPin(pin))
        value |= IOCON_FUNC(1);

      /* Mode configuration for I2C pins */
      if (isI2CPin(pin))
        value = (value & ~IOCON_I2C_MASK) | IOCON_I2C_IO;
      break;

    default:
      value = (value & ~IOCON_FUNC_MASK) | IOCON_FUNC(function);
      break;
  }

  *reg = value;
}
/*----------------------------------------------------------------------------*/
void pinSetPull(struct Pin pin, enum PinPull pull)
{
  if (!pin.reg || isI2CPin(pin))
    return;

  volatile uint32_t * const reg = pin.reg;
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
void pinSetSlewRate(struct Pin pin, enum PinSlewRate rate)
{
  /* Slew rate control is available only for I2C pins */
  if (!pin.reg || !isI2CPin(pin))
    return;

  volatile uint32_t * const reg = calcControlReg(pin.port, pin.number);
  uint32_t value = *reg & ~IOCON_I2C_MASK;

  if (IOCON_FUNC_VALUE(value) == 1)
    value |= (rate == PIN_SLEW_FAST) ? IOCON_I2C_PLUS : IOCON_I2C_STANDARD;
  else
    value |= IOCON_I2C_IO;

  *reg = value;
}
/*----------------------------------------------------------------------------*/
void pinSetType(struct Pin pin, enum PinType type)
{
  if (!pin.reg || isI2CPin(pin))
    return;

  volatile uint32_t * const reg = pin.reg;

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
