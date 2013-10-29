/*
 * gpio.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <gpio.h>
#include <macro.h>
#include <platform/nxp/lpc17xx/gpio_defs.h>
#include <platform/nxp/lpc17xx/power.h>
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_Type *calcPort(union GpioPin);
static inline uint32_t *calcPinSelect(union GpioPin);
static inline uint32_t *calcPinMode(union GpioPin);
static inline uint32_t *calcPinType(union GpioPin);
/*----------------------------------------------------------------------------*/
static void commonGpioSetup(struct Gpio);
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_Type *calcPort(union GpioPin p)
{
  return (LPC_GPIO_Type *)((uint32_t)LPC_GPIO0
      + ((uint32_t)LPC_GPIO1 - (uint32_t)LPC_GPIO0) * p.port);
}
/*----------------------------------------------------------------------------*/
static inline uint32_t *calcPinSelect(union GpioPin p)
{
  return (uint32_t *)(&LPC_PINCON->PINSEL0 + (p.offset >> 4) + (p.port << 1));
}
/*----------------------------------------------------------------------------*/
static inline uint32_t *calcPinMode(union GpioPin p)
{
  return (uint32_t *)(&LPC_PINCON->PINMODE0 + (p.offset >> 4) + (p.port << 1));
}
/*----------------------------------------------------------------------------*/
static inline uint32_t *calcPinType(union GpioPin p)
{
  return (uint32_t *)(&LPC_PINCON->PINMODE_OD0 + p.port);
}
/*----------------------------------------------------------------------------*/
static void commonGpioSetup(struct Gpio gpio)
{
  /* Set GPIO mode */
  gpioSetFunction(gpio, GPIO_DEFAULT);
  /* Neither pull-up nor pull-down */
  gpioSetPull(gpio, GPIO_NOPULL);
  /* Push-pull output type */
  gpioSetType(gpio, GPIO_PUSHPULL);
}
/*----------------------------------------------------------------------------*/
struct Gpio gpioInit(gpio_t id)
{
  struct Gpio gpio = {
      .reg = 0,
      .pin = {
          .key = ~0
      }
  };
  union GpioPin converted = {
      .key = ~id /* Invert unique pin id */
  };

  /* TODO Add more precise pin checking */
  assert(id && (uint8_t)converted.port <= 4 && (uint8_t)converted.offset <= 31);

  gpio.pin = converted;
  gpio.reg = calcPort(gpio.pin);

  return gpio;
}
/*----------------------------------------------------------------------------*/
void gpioInput(struct Gpio gpio)
{
  commonGpioSetup(gpio);
  ((LPC_GPIO_Type *)gpio.reg)->DIR &= ~(1 << gpio.pin.offset);
}
/*----------------------------------------------------------------------------*/
void gpioOutput(struct Gpio gpio, uint8_t value)
{
  commonGpioSetup(gpio);
  gpioWrite(gpio, value);
  ((LPC_GPIO_Type *)gpio.reg)->DIR |= 1 << gpio.pin.offset;
}
/*----------------------------------------------------------------------------*/
void gpioSetFunction(struct Gpio gpio, uint8_t function)
{
  /* Function should not be used outside platform drivers */
  uint32_t *pinptr = calcPinSelect(gpio.pin);

  switch (function)
  {
    case GPIO_DEFAULT:
      function = 0; /* Zero function is the default mode */
      break;
    case GPIO_ANALOG:
      return;
  }

  *pinptr = (*pinptr & ~PIN_OFFSET(PIN_MASK, gpio.pin.offset))
      | PIN_OFFSET(function, gpio.pin.offset);
}
/*----------------------------------------------------------------------------*/
void gpioSetPull(struct Gpio gpio, enum gpioPull pull)
{
  uint32_t *pinptr = calcPinMode(gpio.pin);
  uint32_t value = *pinptr & ~PIN_OFFSET(PIN_MASK, gpio.pin.offset);

  switch (pull)
  {
    case GPIO_NOPULL:
      value |= PIN_OFFSET(PIN_MODE_INACTIVE, gpio.pin.offset);
      break;
    case GPIO_PULLUP:
      value |= PIN_OFFSET(PIN_MODE_PULLUP, gpio.pin.offset);
      break;
    case GPIO_PULLDOWN:
      value |= PIN_OFFSET(PIN_MODE_PULLDOWN, gpio.pin.offset);
      break;
  }
  *pinptr = value;
}
/*----------------------------------------------------------------------------*/
void gpioSetType(struct Gpio gpio, enum gpioType type)
{
  uint32_t *pinptr = calcPinType(gpio.pin);

  switch (type)
  {
    case GPIO_PUSHPULL:
      *pinptr &= ~(1 << gpio.pin.offset);
      break;
    case GPIO_OPENDRAIN:
      *pinptr |= 1 << gpio.pin.offset;
      break;
  }
}
