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
struct Gpio gpioInit(gpio_t, enum gpioDir);
void gpioDeinit(struct Gpio *);
/*----------------------------------------------------------------------------*/
const struct GpioDescriptor *gpioFind(const struct GpioDescriptor *, gpio_t,
    uint8_t);
void gpioSetFunction(struct Gpio *, uint8_t);
void gpioSetPull(struct Gpio *, enum gpioPull);
void gpioSetType(struct Gpio *, enum gpioType);
/*----------------------------------------------------------------------------*/
static inline uint8_t gpioRead(struct Gpio *p)
{
  return (((LPC_GPIO_Type *)p->reg)->PIN & (1 << p->pin.offset)) != 0;
}
/*----------------------------------------------------------------------------*/
static inline void gpioReset(struct Gpio *p)
{
  ((LPC_GPIO_Type *)p->reg)->CLR = 1 << p->pin.offset;
}
/*----------------------------------------------------------------------------*/
static inline void gpioSet(struct Gpio *p)
{
  ((LPC_GPIO_Type *)p->reg)->SET = 1 << p->pin.offset;
}
/*----------------------------------------------------------------------------*/
static inline void gpioWrite(struct Gpio *p, uint8_t value)
{
  if (value)
    ((LPC_GPIO_Type *)p->reg)->SET = 1 << p->pin.offset;
  else
    ((LPC_GPIO_Type *)p->reg)->CLR = 1 << p->pin.offset;
}
/*----------------------------------------------------------------------------*/
#endif /* GPIO_H_ */
