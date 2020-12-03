/*
 * pin.c
 * Copyright (C) 2012, 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/pin.h>
#include <halm/platform/lpc/gen_2/pin_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_Type *calcPort(uint8_t);
static inline volatile uint32_t *calcControlReg(uint8_t, uint8_t);
static inline volatile uint32_t *calcMaskedReg(uint8_t, uint8_t);
static void commonPinInit(struct Pin);
static inline bool isI2CPin(struct Pin);
static inline bool isSystemPin(struct Pin);
/*----------------------------------------------------------------------------*/
static const uint8_t pinRegMap[4][12] = {
    {0x0C, 0x10, 0x1C, 0x2C, 0x30, 0x34, 0x4C, 0x50, 0x60, 0x64, 0x68, 0x74},
    {0x78, 0x7C, 0x80, 0x90, 0x94, 0xA0, 0xA4, 0xA8, 0x14, 0x38, 0x6C, 0x98},
    {0x08, 0x28, 0x5C, 0x8C, 0x40, 0x44, 0x00, 0x20, 0x24, 0x54, 0x58, 0x70},
    {0x84, 0x88, 0x9C, 0xAC, 0x3C, 0x48}
};
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_Type *calcPort(uint8_t port)
{
  assert(port < ARRAY_SIZE(LPC_GPIO));
  return &LPC_GPIO[port];
}
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcControlReg(uint8_t port, uint8_t number)
{
  return (volatile uint32_t *)((uintptr_t)LPC_IOCON + pinRegMap[port][number]);
}
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcMaskedReg(uint8_t port, uint8_t number)
{
  if (port < ARRAY_SIZE(LPC_GPIO))
    return calcPort(port)->MASKED_ACCESS + (1UL << number);
  else
    return 0;
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
static bool isI2CPin(struct Pin pin)
{
  /* PIO0_4 and PIO0_5 are I2C pins with different function set */
  return pin.port == 0 && pin.number >= 4 && pin.number <= 5;
}
/*----------------------------------------------------------------------------*/
static bool isSystemPin(struct Pin pin)
{
  /*
   * PIO0_0 is Reset pin, PIO0_10 and PIO1_3 are SWD pins,
   * PIO0_11, PIO1_0 through PIO1_2 have a reserved function.
   */
  switch (pin.port)
  {
    case 0:
      return (pin.number >= 10 && pin.number <= 11) || pin.number == 0;

    case 1:
      return pin.number <= 3;

    default:
      return false;
  }
}
/*----------------------------------------------------------------------------*/
struct Pin pinInit(PinNumber id)
{
  struct Pin pin;

  pin.port = PIN_TO_PORT(id);
  pin.number = PIN_TO_OFFSET(id);
  pin.reg = (void *)calcMaskedReg(pin.port, pin.number);

  return pin;
}
/*----------------------------------------------------------------------------*/
void pinInput(struct Pin pin)
{
  if (!pinValid(pin))
    return;

  LPC_GPIO_Type * const reg = calcPort(pin.port);

  commonPinInit(pin);
  reg->DIR &= ~(1UL << pin.number);
}
/*----------------------------------------------------------------------------*/
void pinOutput(struct Pin pin, bool value)
{
  if (!pinValid(pin))
    return;

  LPC_GPIO_Type * const reg = calcPort(pin.port);

  commonPinInit(pin);
  reg->DIR |= 1UL << pin.number;
  pinWrite(pin, value);
}
/*----------------------------------------------------------------------------*/
void pinSetFunction(struct Pin pin, uint8_t function)
{
  if (!pinValid(pin))
    return;

  volatile uint32_t * const reg = calcControlReg(pin.port, pin.number);
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
  if (!pinValid(pin) || isI2CPin(pin))
    return;

  volatile uint32_t * const reg = calcControlReg(pin.port, pin.number);
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
  if (!pinValid(pin) || !isI2CPin(pin))
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
  if (!pinValid(pin) || isI2CPin(pin))
    return;

  volatile uint32_t * const reg = calcControlReg(pin.port, pin.number);

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
