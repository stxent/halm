/*
 * pin.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdbool.h>
#include <entity.h>
#include <pin.h>
#include <platform/nxp/lpc13xx/pin_defs.h>
#include <platform/nxp/lpc13xx/system.h>
/*----------------------------------------------------------------------------*/
struct PinHandler
{
  struct Entity parent;

  /* Initialized pins count */
  uint16_t instances;
};
/*----------------------------------------------------------------------------*/
static inline void *calcPort(union PinData);
static inline volatile uint32_t *calcControlReg(union PinData);
static void commonPinSetup(struct Pin);
/*----------------------------------------------------------------------------*/
static inline void pinHandlerAttach();
static inline void pinHandlerDetach();
static enum result pinHandlerInit(void *, const void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass handlerTable = {
    .size = sizeof(struct PinHandler),
    .init = pinHandlerInit,
    .deinit = 0
};
/*----------------------------------------------------------------------------*/
static const uint8_t pinRegMap[4][12] = {
    {0x0C, 0x10, 0x1C, 0x2C, 0x30, 0x34, 0x4C, 0x50, 0x60, 0x64, 0x68, 0x74},
    {0x78, 0x7C, 0x80, 0x90, 0x94, 0xA0, 0xA4, 0xA8, 0x14, 0x38, 0x6C, 0x98},
    {0x08, 0x28, 0x5C, 0x8C, 0x40, 0x44, 0x00, 0x20, 0x24, 0x54, 0x58, 0x70},
    {0x84, 0x88, 0x9C, 0xAC, 0x3C, 0x48}
};
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const PinHandler = &handlerTable;
static struct PinHandler *pinHandler = 0;
/*----------------------------------------------------------------------------*/
static inline void *calcPort(union PinData data)
{
  return (void *)(((uint32_t)LPC_GPIO1 - (uint32_t)LPC_GPIO0) * data.port
      + (uint32_t)LPC_GPIO0);
}
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcControlReg(union PinData data)
{
  return (volatile uint32_t *)((uint32_t)LPC_IOCON
      + pinRegMap[data.port][data.offset]);
}
/*----------------------------------------------------------------------------*/
static void commonPinSetup(struct Pin pin)
{
  /* Register new pin in the handler */
  pinHandlerAttach();

  pinSetFunction(pin, PIN_DEFAULT);
  pinSetPull(pin, PIN_NOPULL);
  pinSetType(pin, PIN_PUSHPULL);
}
/*----------------------------------------------------------------------------*/
static inline void pinHandlerAttach()
{
  /* Create handler object on first function call */
  if (!pinHandler)
    pinHandler = init(PinHandler, 0);

  if (!pinHandler->instances++)
  {
    sysClockEnable(CLK_IOCON);
    sysClockEnable(CLK_GPIO);
  }
}
/*----------------------------------------------------------------------------*/
static inline void pinHandlerDetach()
{
  /* Disable clocks when no active pins exist */
  if (!--pinHandler->instances)
  {
    sysClockDisable(CLK_GPIO);
    sysClockDisable(CLK_IOCON);
  }
}
/*----------------------------------------------------------------------------*/
static enum result pinHandlerInit(void *object,
    const void *configPtr __attribute__((unused)))
{
  struct PinHandler * const handler = object;

  handler->instances = 0;
  return E_OK;
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
  volatile uint32_t * const iocon = calcControlReg(pin.data);
  const uint32_t value = *iocon;

  switch (function)
  {
    case PIN_DEFAULT:
      /* Some pins have default function value other than zero */
      function = (pin.data.port == 1 && pin.data.offset <= 2)
          || (pin.data.port == 0 && pin.data.offset == 11) ? 1 : 0;
      break;

    case PIN_ANALOG:
      *iocon = value & ~IOCON_DIGITAL;
      return;
  }

  *iocon = (value & ~IOCON_FUNC_MASK) | IOCON_FUNC(function);
}
/*----------------------------------------------------------------------------*/
void pinSetPull(struct Pin pin, enum pinPull pull)
{
  volatile uint32_t * const iocon = calcControlReg(pin.data);
  uint32_t value = *iocon & ~IOCON_MODE_MASK;

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

  *iocon = value;
}
/*----------------------------------------------------------------------------*/
void pinSetType(struct Pin pin, enum pinType type)
{
  volatile uint32_t * const iocon = calcControlReg(pin.data);

  switch (type)
  {
    case PIN_PUSHPULL:
      *iocon &= ~IOCON_OD;
      break;

    case PIN_OPENDRAIN:
      *iocon |= IOCON_OD;
      break;
  }
}
