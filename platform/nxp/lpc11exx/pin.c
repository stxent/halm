/*
 * pin.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <entity.h>
#include <pin.h>
#include <platform/nxp/lpc11exx/pin_defs.h>
#include <platform/nxp/lpc11exx/system.h>
/*----------------------------------------------------------------------------*/
struct PinHandler
{
  struct Entity parent;

  /* Initialized pins count */
  uint16_t instances;
};
/*----------------------------------------------------------------------------*/
static void *calcControlReg(union PinData);
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
static const struct EntityClass * const PinHandler = &handlerTable;
static struct PinHandler *pinHandler = 0;
/*----------------------------------------------------------------------------*/
static void *calcControlReg(union PinData data)
{
  volatile uint32_t *iocon = 0;

  switch (data.port)
  {
    case 0:
      iocon = &LPC_IOCON->PIO0[data.offset];
      break;

    case 1:
      iocon = &LPC_IOCON->PIO1[data.offset];
      break;
  }

  return (void *)iocon;
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
  pin.reg = calcControlReg(pin.data);

  return pin;
}
/*----------------------------------------------------------------------------*/
void pinInput(struct Pin pin)
{
  commonPinSetup(pin);
  LPC_GPIO->DIR[pin.data.port] &= ~(1 << pin.data.offset);
}
/*----------------------------------------------------------------------------*/
void pinOutput(struct Pin pin, uint8_t value)
{
  commonPinSetup(pin);
  LPC_GPIO->DIR[pin.data.port] |= 1 << pin.data.offset;
  pinWrite(pin, value);
}
/*----------------------------------------------------------------------------*/
void pinSetFunction(struct Pin pin, uint8_t function)
{
  volatile uint32_t * const iocon = pin.reg;
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
  volatile uint32_t * const iocon = pin.reg;
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
  volatile uint32_t * const iocon = pin.reg;

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
