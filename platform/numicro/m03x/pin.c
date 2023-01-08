/*
 * pin.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/pin.h>
#include <halm/platform/numicro/m03x/pin_defs.h>
#include <halm/platform/numicro/m03x/system_defs.h>
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcPinFunc(uint8_t, uint8_t);
static inline NM_GPIO_Type *calcPort(uint8_t);
/*----------------------------------------------------------------------------*/
static void commonPinInit(struct Pin);
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcPinFunc(uint8_t port, uint8_t number)
{
  return number >= 8 ? &NM_SYS->GP[port].MFPH : &NM_SYS->GP[port].MFPL;
}
/*----------------------------------------------------------------------------*/
static inline NM_GPIO_Type *calcPort(uint8_t port)
{
  return NM_GPIOA + port;
}
/*----------------------------------------------------------------------------*/
static void commonPinInit(struct Pin pin)
{
  pinSetFunction(pin, PIN_DEFAULT);
  pinSetPull(pin, PIN_NOPULL);
}
/*----------------------------------------------------------------------------*/
struct Pin pinInit(PinNumber id)
{
  struct Pin pin = {
      .number = PIN_TO_OFFSET(id),
      .port = PIN_TO_PORT(id)
  };

  if (id && pin.port < PORT_USB)
    pin.reg = (void *)&NM_GPIO_PDIO->GPIO[pin.port].PDIO[pin.number];
  else
    pin.reg = 0;

  return pin;
}
/*----------------------------------------------------------------------------*/
void pinInput(struct Pin pin)
{
  NM_GPIO_Type * const reg = calcPort(pin.port);
  uint32_t mode = reg->MODE;

  commonPinInit(pin);

  mode &= ~MODE_CONTROL_MASK(pin.number);
  mode |= MODE_CONTROL(pin.number, MODE_INPUT);
  reg->MODE = mode;
}
/*----------------------------------------------------------------------------*/
void pinOutput(struct Pin pin, bool value)
{
  NM_GPIO_Type * const reg = calcPort(pin.port);
  uint32_t mode = reg->MODE;

  commonPinInit(pin);
  pinWrite(pin, value);

  mode &= ~MODE_CONTROL_MASK(pin.number);
  mode |= MODE_CONTROL(pin.number, MODE_PUSH_PULL);
  reg->MODE = mode;
}
/*----------------------------------------------------------------------------*/
void pinSetFunction(struct Pin pin, uint8_t function)
{
  /* Function should not be used outside platform drivers */
  switch (function)
  {
    case PIN_DEFAULT:
      function = 0; /* Function 0 is the default for all GPIO pins */
      break;

    case PIN_ANALOG:
      // TODO Disable digital input path
      function = 1; /* Function 1 is the default for all analog pins */
      return;
  }

  volatile uint32_t * const reg = calcPinFunc(pin.port, pin.number);
  const uint32_t offset = pin.number & 0x7;
  uint32_t value = *reg;

  value &= ~MFP_FUNCTION_MASK(offset);
  value |= MFP_FUNCTION(function, offset);
  *reg = value;
}
/*----------------------------------------------------------------------------*/
void pinSetPull(struct Pin pin __attribute__((unused)),
    enum PinPull pull __attribute__((unused)))
{
}
/*----------------------------------------------------------------------------*/
void pinSetSlewRate(struct Pin pin __attribute__((unused)),
    enum PinSlewRate rate __attribute__((unused)))
{
}
/*----------------------------------------------------------------------------*/
void pinSetType(struct Pin pin, enum PinType type)
{
  NM_GPIO_Type * const reg = calcPort(pin.port);
  uint32_t value = reg->MODE & ~MODE_CONTROL_MASK(pin.number);

  switch (type)
  {
    case PIN_PUSHPULL:
      value |= MODE_CONTROL(MODE_PUSH_PULL, pin.number);
      break;

    case PIN_OPENDRAIN:
      value |= MODE_CONTROL(MODE_OPEN_DRAIN, pin.number);
      break;
  }

  reg->MODE = value;
}
