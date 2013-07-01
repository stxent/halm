/*
 * gpio.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPIO_H_
#define GPIO_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include "device_defs.h"
/*----------------------------------------------------------------------------*/
typedef uint16_t gpioKey;
/*----------------------------------------------------------------------------*/
/* External pin id consist of port and pin numbers in 1's complement form */
/* Unused pins should be initialized with zero */
/* This method supports up to 2^7 ports and 2^8 pins on each port */
#define GPIO_TO_PIN(port, pin) ((gpioKey)(~((uint8_t)port << 8 | (uint8_t)pin)))
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
  gpioKey key;
  struct
  {
    uint8_t offset;
    uint8_t port;
  };
};
/*----------------------------------------------------------------------------*/
struct GpioDescriptor
{
  gpioKey key;
  uint8_t channel;
  uint8_t value;
};
/*----------------------------------------------------------------------------*/
struct Gpio
{
  LPC_GPIO_TypeDef *control;
  union GpioPin pin;
};
/*----------------------------------------------------------------------------*/
struct Gpio gpioInit(gpioKey, enum gpioDir);
void gpioDeinit(struct Gpio *);
/*----------------------------------------------------------------------------*/
uint8_t gpioRead(struct Gpio *);
void gpioWrite(struct Gpio *, uint8_t);
/*----------------------------------------------------------------------------*/
const struct GpioDescriptor *gpioFind(const struct GpioDescriptor *, gpioKey,
    uint8_t);
void gpioSetFunction(struct Gpio *, uint8_t);
void gpioSetPull(struct Gpio *, enum gpioPull);
void gpioSetType(struct Gpio *, enum gpioType);
gpioKey gpioGetKey(struct Gpio *);
/*----------------------------------------------------------------------------*/
#endif /* GPIO_H_ */
