/*
 * pin.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/pin.h>
#include <halm/platform/nxp/lpc11exx/pin_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static volatile uint32_t *calcControlReg(struct PinData);
static void commonPinInit(struct Pin);
static inline bool isI2CPin(struct Pin);
static inline bool isSystemPin(struct Pin);
/*----------------------------------------------------------------------------*/
static volatile uint32_t *calcControlReg(struct PinData data)
{
  switch (data.port)
  {
    case 0:
      return &LPC_IOCON->PIO0[data.offset];

    case 1:
      return &LPC_IOCON->PIO1[data.offset];

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
}
/*----------------------------------------------------------------------------*/
static bool isI2CPin(struct Pin pin)
{
  /* PIO0_4 and PIO0_5 are I2C pins with different function set */
  return pin.data.port == 0 && pin.data.offset >= 4 && pin.data.offset <= 5;
}
/*----------------------------------------------------------------------------*/
static bool isSystemPin(struct Pin pin)
{
  /* PIO0_0 is Reset pin, PIO0_10 through PIO0_15 are JTAG/SWD pins */
  return pin.data.port == 0
      && (pin.data.offset == 0
          || (pin.data.offset >= 10 && pin.data.offset <= 15));
}
/*----------------------------------------------------------------------------*/
struct Pin pinInit(PinNumber id)
{
  struct Pin pin;

  pin.data.port = PIN_TO_PORT(id);
  pin.data.offset = PIN_TO_OFFSET(id);

  pin.reg = (void *)calcControlReg(pin.data);
  assert(pin.reg);

  return pin;
}
/*----------------------------------------------------------------------------*/
void pinInput(struct Pin pin)
{
  commonPinInit(pin);
  LPC_GPIO->DIR[pin.data.port] &= ~(1UL << pin.data.offset);
}
/*----------------------------------------------------------------------------*/
void pinOutput(struct Pin pin, bool value)
{
  commonPinInit(pin);
  LPC_GPIO->DIR[pin.data.port] |= 1UL << pin.data.offset;
  pinWrite(pin, value);
}
/*----------------------------------------------------------------------------*/
void pinSetFunction(struct Pin pin, uint8_t function)
{
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
  if (isI2CPin(pin))
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
  if (!isI2CPin(pin))
    return;

  volatile uint32_t * const reg = calcControlReg(pin.data);
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
  if (isI2CPin(pin))
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
