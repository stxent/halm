/*
 * gpio.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

//TODO Move gpioFind to common gpio.c
#ifndef GPIO_TOP_H_
#define GPIO_TOP_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <mcu.h>
/*----------------------------------------------------------------------------*/
typedef uint16_t gpio_t;
/*----------------------------------------------------------------------------*/
/* External pin id consist of port and pin numbers in 1's complement form */
/* Unused pins should be initialized with zero */
/* This method supports up to 2^7 ports and 2^8 pins on each port */
#define GPIO_TO_PIN(port, pin) ((gpio_t)(~((uint8_t)port << 8 | (uint8_t)pin)))
/*----------------------------------------------------------------------------*/
enum gpioDir
{
  GPIO_INPUT = 0,
  GPIO_OUTPUT
};
/*----------------------------------------------------------------------------*/
enum gpioPull
{
  GPIO_NOPULL = 0,
  GPIO_PULLUP,
  GPIO_PULLDOWN
};
/*----------------------------------------------------------------------------*/
enum gpioType
{
  GPIO_PUSHPULL = 0,
  GPIO_OPENDRAIN
};
/*----------------------------------------------------------------------------*/
union GpioPin
{
  gpio_t key;
  struct
  {
    uint8_t offset;
    uint8_t port;
  };
};
/*----------------------------------------------------------------------------*/
struct GpioDescriptor
{
  gpio_t key;
  uint8_t channel;
  uint8_t value;
};
/*----------------------------------------------------------------------------*/
struct Gpio
{
  void *reg;
  union GpioPin pin;
};
/*----------------------------------------------------------------------------*/
static inline gpio_t gpioGetKey(struct Gpio *p)
{
  /* Returns zero when pin not initialized */
  return ~p->pin.key;
}
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/gpio.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
#endif /* GPIO_TOP_H_ */
