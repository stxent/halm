/*
 * platform/nxp/lpc11xx/gpio.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPIO_H_
#define GPIO_H_
/*----------------------------------------------------------------------------*/
#include "../device_defs.h"
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
  return (((LPC_GPIO_TypeDef *)p->reg)->DATA & (1 << p->pin.offset)) != 0;
}
/*----------------------------------------------------------------------------*/
static inline void gpioReset(struct Gpio *p)
{
  *(uint32_t *)(((LPC_GPIO_TypeDef *)p->reg)->MASKED_ACCESS
        + (1 << p->pin.offset)) = 0x000;
}
/*----------------------------------------------------------------------------*/
static inline void gpioSet(struct Gpio *p)
{
  *(uint32_t *)(((LPC_GPIO_TypeDef *)p->reg)->MASKED_ACCESS
        + (1 << p->pin.offset)) = 0xFFF;
}
/*----------------------------------------------------------------------------*/
static inline void gpioWrite(struct Gpio *p, uint8_t value)
{
  *(uint32_t *)(((LPC_GPIO_TypeDef *)p->reg)->MASKED_ACCESS
      + (1 << p->pin.offset)) = value ? 0xFFF : 0x000;
}
/*----------------------------------------------------------------------------*/
#endif /* GPIO_H_ */
