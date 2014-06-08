/*
 * gpio_bus.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Abstract class definition and reference implementation of
 * General Purpose Input/Output bus.
 */

#ifndef GPIO_BUS_H_
#define GPIO_BUS_H_
/*----------------------------------------------------------------------------*/
#include <entity.h>
#include <gpio.h>
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct GpioBusClass
{
  CLASS_HEADER

  uint32_t (*read)(void *);
  void (*write)(void *, uint32_t);
};
/*----------------------------------------------------------------------------*/
struct GpioBus
{
  struct Entity parent;
};
/*----------------------------------------------------------------------------*/
/**
 * Read logical levels from bus lines.
 * @param bus Pointer to a GpioBus object.
 * @return Current logical levels on bus lines.
 */
static inline uint32_t gpioBusRead(void *bus)
{
  return ((struct GpioBusClass *)CLASS(bus))->read(bus);
}
/*----------------------------------------------------------------------------*/
/**
 * Set logical values on bus lines.
 * @param bus Pointer to a GpioBus object.
 * @param value Signal values to be set.
 */
static inline void gpioBusWrite(void *bus, uint32_t value)
{
  ((struct GpioBusClass *)CLASS(bus))->write(bus, value);
}
/*----------------------------------------------------------------------------*/
/** Reference GpioBus implementation based on the pin array. */
extern const struct GpioBusClass *GpioBus;
/*----------------------------------------------------------------------------*/
struct GpioBusConfig
{
  /** Mandatory: pointer to an array of pins terminated with zero element. */
  const gpio_t *pins;
  /** Optional: initial output value for pins configured as outputs. */
  uint32_t initial;
  /** Mandatory: direction of pins in the bus. */
  enum gpioDirection direction;
  /** Optional: pull-up and pull-down configuration. */
  enum gpioPull pull;
  /** Optional: slew rate control of output pins. */
  enum gpioSlewRate rate;
  /** Optional: push-pull or open-drain configuration for output pins. */
  enum gpioType type;
};
/*----------------------------------------------------------------------------*/
#endif /* GPIO_BUS_H_ */
