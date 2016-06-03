/*
 * halm/gpio_bus.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Abstract class definition and reference implementation of
 * General Purpose Input/Output bus.
 */

#ifndef HALM_GPIO_BUS_H_
#define HALM_GPIO_BUS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/entity.h>
#include <halm/pin.h>
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
  struct Entity base;
};
/*----------------------------------------------------------------------------*/
/**
 * Read logic levels from bus lines.
 * @param bus Pointer to a GpioBus object.
 * @return Current logic levels on bus lines.
 */
static inline uint32_t gpioBusRead(void *bus)
{
  return ((const struct GpioBusClass *)CLASS(bus))->read(bus);
}
/*----------------------------------------------------------------------------*/
/**
 * Set logic levels on bus lines.
 * @param bus Pointer to a GpioBus object.
 * @param value Logic levels to be set.
 */
static inline void gpioBusWrite(void *bus, uint32_t value)
{
  ((const struct GpioBusClass *)CLASS(bus))->write(bus, value);
}
/*----------------------------------------------------------------------------*/
/** Simple GpioBus implementation based on a pin array. */
extern const struct GpioBusClass * const SimpleGpioBus;
/*----------------------------------------------------------------------------*/
struct SimpleGpioBusConfig
{
  /** Mandatory: pointer to an array of pins terminated with a zero element. */
  const pinNumber *pins;
  /** Optional: initial output value for pins configured as outputs. */
  uint32_t initial;
  /** Mandatory: direction of pins in the bus. */
  enum pinDirection direction;
  /** Optional: pull-up and pull-down configuration. */
  enum pinPull pull;
  /** Optional: slew rate control of output pins. */
  enum pinSlewRate rate;
  /** Optional: push-pull or open-drain configuration for output pins. */
  enum pinType type;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GPIO_BUS_H_ */
