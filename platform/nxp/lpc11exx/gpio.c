/*
 * gpio.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdbool.h>
#include <entity.h>
#include <gpio.h>
#include <platform/nxp/lpc11exx/gpio_defs.h>
#include <platform/nxp/lpc11exx/system.h>
/*----------------------------------------------------------------------------*/
struct GpioHandler
{
  struct Entity parent;

  /* Initialized pins count */
  uint8_t instances;
};
/*----------------------------------------------------------------------------*/
static void *calcControlReg(union GpioPin);
static void commonGpioSetup(struct Gpio);
/*----------------------------------------------------------------------------*/
static inline void gpioHandlerAttach();
static inline void gpioHandlerDetach();
static enum result gpioHandlerInit(void *, const void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass handlerTable = {
    .size = sizeof(struct GpioHandler),
    .init = gpioHandlerInit,
    .deinit = 0
};
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const GpioHandler = &handlerTable;
static struct GpioHandler *gpioHandler = 0;
/*----------------------------------------------------------------------------*/
static void *calcControlReg(union GpioPin pin)
{
  volatile uint32_t *iocon = 0;

  switch (pin.port)
  {
    case 0:
      iocon = &LPC_IOCON->PIO0[pin.offset];
      break;

    case 1:
      iocon = &LPC_IOCON->PIO1[pin.offset];
      break;
  }

  return (void *)iocon;
}
/*----------------------------------------------------------------------------*/
static void commonGpioSetup(struct Gpio gpio)
{
  /* Register new pin in the handler */
  gpioHandlerAttach();

  gpioSetFunction(gpio, GPIO_DEFAULT);
  gpioSetPull(gpio, GPIO_NOPULL);
  gpioSetType(gpio, GPIO_PUSHPULL);
}
/*----------------------------------------------------------------------------*/
static inline void gpioHandlerAttach()
{
  /* Create handler object on first function call */
  if (!gpioHandler)
    gpioHandler = init(GpioHandler, 0);

  if (!gpioHandler->instances++)
  {
    sysClockEnable(CLK_IOCON);
    sysClockEnable(CLK_GPIO);
  }
}
/*----------------------------------------------------------------------------*/
static inline void gpioHandlerDetach()
{
  /* Disable clocks when no active pins exist */
  if (!--gpioHandler->instances)
  {
    sysClockDisable(CLK_GPIO);
    sysClockDisable(CLK_IOCON);
  }
}
/*----------------------------------------------------------------------------*/
static enum result gpioHandlerInit(void *object,
    const void *configPtr __attribute__((unused)))
{
  struct GpioHandler * const handler = object;

  handler->instances = 0;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
struct Gpio gpioInit(gpio_t id)
{
  struct Gpio gpio;

  gpio.pin.key = ~id;
  gpio.reg = calcControlReg(gpio.pin);

  return gpio;
}
/*----------------------------------------------------------------------------*/
void gpioInput(struct Gpio gpio)
{
  commonGpioSetup(gpio);
  LPC_GPIO->DIR[gpio.pin.port] &= ~(1 << gpio.pin.offset);
}
/*----------------------------------------------------------------------------*/
void gpioOutput(struct Gpio gpio, uint8_t value)
{
  commonGpioSetup(gpio);
  LPC_GPIO->DIR[gpio.pin.port] |= 1 << gpio.pin.offset;
  gpioWrite(gpio, value);
}
/*----------------------------------------------------------------------------*/
void gpioSetFunction(struct Gpio gpio, uint8_t function)
{
  volatile uint32_t * const iocon = gpio.reg;
  const uint32_t value = *iocon;

  switch (function)
  {
    case GPIO_DEFAULT:
      /* Some pins have default function value other than zero */
      function = (gpio.pin.port == 1 && gpio.pin.offset <= 2)
          || (gpio.pin.port == 0 && gpio.pin.offset == 11) ? 1 : 0;
      break;

    case GPIO_ANALOG:
      *iocon = value & ~IOCON_MODE_DIGITAL;
      return;
  }

  *iocon = (value & ~IOCON_FUNC_MASK) | IOCON_FUNC(function);
}
/*----------------------------------------------------------------------------*/
void gpioSetPull(struct Gpio gpio, enum gpioPull pull)
{
  volatile uint32_t * const iocon = gpio.reg;
  uint32_t value = *iocon & ~IOCON_MODE_MASK;

  switch (pull)
  {
    case GPIO_NOPULL:
      value |= IOCON_MODE_INACTIVE;
      break;

    case GPIO_PULLUP:
      value |= IOCON_MODE_PULLUP;
      break;

    case GPIO_PULLDOWN:
      value |= IOCON_MODE_PULLDOWN;
      break;
  }

  *iocon = value;
}
/*----------------------------------------------------------------------------*/
void gpioSetType(struct Gpio gpio, enum gpioType type)
{
  volatile uint32_t * const iocon = gpio.reg;

  switch (type)
  {
    case GPIO_PUSHPULL:
      *iocon &= ~IOCON_OD;
      break;

    case GPIO_OPENDRAIN:
      *iocon |= IOCON_OD;
      break;
  }
}
