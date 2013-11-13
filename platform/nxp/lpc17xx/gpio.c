/*
 * gpio.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <gpio.h>
#include <platform/nxp/lpc17xx/gpio_defs.h>
/*----------------------------------------------------------------------------*/
#define REG_PIN_TYPE(port) (volatile uint32_t *)(&LPC_PINCON->PINMODE_OD0 + (port))
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
  struct Gpio gpio;

  gpio.pin.key = ~id;
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
  ((LPC_GPIO_Type *)gpio.reg)->DIR |= 1 << gpio.pin.offset;
  gpioWrite(gpio, value);
}
/*----------------------------------------------------------------------------*/
void gpioSetFunction(struct Gpio gpio, uint8_t function)
{
  /* Function should not be used outside platform drivers */
  switch (function)
  {
    case GPIO_DEFAULT:
      function = 0; /* Zero function is the default mode */
      break;
    case GPIO_ANALOG:
      return;
  }

  uint32_t *pointer = calcPinSelect(gpio.pin);
  *pointer = (*pointer & ~PIN_OFFSET(PIN_MASK, gpio.pin.offset))
      | PIN_OFFSET(function, gpio.pin.offset);
}
/*----------------------------------------------------------------------------*/
void gpioSetPull(struct Gpio gpio, enum gpioPull pull)
{
  uint32_t *pointer = calcPinMode(gpio.pin);
  uint32_t value = *pointer & ~PIN_OFFSET(PIN_MASK, gpio.pin.offset);

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
  *pointer = value;
}
/*----------------------------------------------------------------------------*/
void gpioSetType(struct Gpio gpio, enum gpioType type)
{
  uint32_t *pointer = calcPinType(gpio.pin);
  uint32_t value = *pointer;

  switch (type)
  {
    case GPIO_PUSHPULL:
      value &= ~(1 << gpio.pin.offset);
      break;
    case GPIO_OPENDRAIN:
      value |= 1 << gpio.pin.offset;
      break;
  }
  *pointer = value;
}
