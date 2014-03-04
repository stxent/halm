/*
 * platform/nxp/lpc17xx/gpio.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPIO_H_
#define GPIO_H_
/*----------------------------------------------------------------------------*/
#include "../platform_defs.h"
/*----------------------------------------------------------------------------*/
struct Gpio gpioInit(gpio_t);
void gpioInput(struct Gpio);
void gpioOutput(struct Gpio, uint8_t);
void gpioSetFunction(struct Gpio, uint8_t);
void gpioSetPull(struct Gpio, enum gpioPull);
void gpioSetType(struct Gpio, enum gpioType);
/*----------------------------------------------------------------------------*/
static inline uint8_t gpioRead(struct Gpio gpio)
{
  return (((LPC_GPIO_Type *)gpio.reg)->PIN & (1 << gpio.pin.offset)) != 0;
}
/*----------------------------------------------------------------------------*/
static inline void gpioReset(struct Gpio gpio)
{
  ((LPC_GPIO_Type *)gpio.reg)->CLR = 1 << gpio.pin.offset;
}
/*----------------------------------------------------------------------------*/
static inline void gpioSet(struct Gpio gpio)
{
  ((LPC_GPIO_Type *)gpio.reg)->SET = 1 << gpio.pin.offset;
}
/*----------------------------------------------------------------------------*/
static inline void gpioWrite(struct Gpio gpio, uint8_t value)
{
  if (value)
    ((LPC_GPIO_Type *)gpio.reg)->SET = 1 << gpio.pin.offset;
  else
    ((LPC_GPIO_Type *)gpio.reg)->CLR = 1 << gpio.pin.offset;
}
/*----------------------------------------------------------------------------*/
static inline void gpioSetSlewRate(struct Gpio gpio __attribute__((unused)),
    enum gpioSlewRate rate __attribute__((unused)))
{
  /* Slew rate control is not supported on these devices */
}
/*----------------------------------------------------------------------------*/
#endif /* GPIO_H_ */
