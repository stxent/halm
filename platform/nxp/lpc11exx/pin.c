/*
 * pin.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <entity.h>
#include <pin.h>
#include <spinlock.h>
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
static volatile uint32_t *calcControlReg(union PinData);
static void commonPinInit(struct Pin);
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
static spinlock_t spinlock = SPIN_UNLOCKED;
/*----------------------------------------------------------------------------*/
static volatile uint32_t *calcControlReg(union PinData data)
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
  /* Register new pin in the handler */
  pinHandlerAttach();

  pinSetFunction(pin, PIN_DEFAULT);
  pinSetPull(pin, PIN_NOPULL);
  pinSetType(pin, PIN_PUSHPULL);
}
/*----------------------------------------------------------------------------*/
static inline void pinHandlerAttach()
{
  spinLock(&spinlock);

  /* Create handler object on first function call */
  if (!pinHandler)
    pinHandler = init(PinHandler, 0);
  assert(pinHandler);

  if (!pinHandler->instances++)
  {
    sysClockEnable(CLK_IOCON);
    sysClockEnable(CLK_GPIO);
  }

  spinUnlock(&spinlock);
}
/*----------------------------------------------------------------------------*/
static inline void pinHandlerDetach()
{
  spinLock(&spinlock);

  /* Disable clocks when no active pins exist */
  if (!--pinHandler->instances)
  {
    sysClockDisable(CLK_GPIO);
    sysClockDisable(CLK_IOCON);
  }

  spinUnlock(&spinlock);
}
/*----------------------------------------------------------------------------*/
static enum result pinHandlerInit(void *object,
    const void *configBase __attribute__((unused)))
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
  pin.reg = (void *)calcControlReg(pin.data);

  return pin;
}
/*----------------------------------------------------------------------------*/
void pinInput(struct Pin pin)
{
  commonPinInit(pin);
  LPC_GPIO->DIR[pin.data.port] &= ~(1 << pin.data.offset);
}
/*----------------------------------------------------------------------------*/
void pinOutput(struct Pin pin, uint8_t value)
{
  commonPinInit(pin);
  LPC_GPIO->DIR[pin.data.port] |= 1 << pin.data.offset;
  pinWrite(pin, value);
}
/*----------------------------------------------------------------------------*/
void pinSetFunction(struct Pin pin, uint8_t function)
{
  volatile uint32_t * const reg = pin.reg;
  const uint32_t value = *reg;

  switch (function)
  {
    case PIN_DEFAULT:
      /* Some pins have default function value other than zero */
      if (pin.data.port == 0 && (pin.data.offset == 0
          || (pin.data.offset >= 10 && pin.data.offset <= 15)))
      {
        function = 1;
      }
      else
        function = 0;
      break;

    case PIN_ANALOG:
      *reg = value & ~IOCON_DIGITAL;
      return;
  }

  *reg = (value & ~IOCON_FUNC_MASK) | IOCON_FUNC(function);
}
/*----------------------------------------------------------------------------*/
void pinSetPull(struct Pin pin, enum pinPull pull)
{
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
void pinSetType(struct Pin pin, enum pinType type)
{
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
