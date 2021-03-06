/*
 * halm/gpio_bus.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * Abstract class definition and reference implementation of
 * General Purpose Input/Output bus.
 */

#ifndef HALM_GPIO_BUS_H_
#define HALM_GPIO_BUS_H_
/*----------------------------------------------------------------------------*/
#include <halm/pin.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct GpioBusClass
{
  CLASS_HEADER

  uint32_t (*read)(void *);
  void (*write)(void *, uint32_t);
};

struct GpioBus
{
  struct Entity base;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/**
 * Read logic levels from bus lines.
 * @param bus Pointer to a GpioBus object.
 * @return Current logic levels on bus lines.
 */
static inline uint32_t gpioBusRead(void *bus)
{
  return ((const struct GpioBusClass *)CLASS(bus))->read(bus);
}

/**
 * Set logic levels on bus lines.
 * @param bus Pointer to a GpioBus object.
 * @param value Logic levels to be set.
 */
static inline void gpioBusWrite(void *bus, uint32_t value)
{
  ((const struct GpioBusClass *)CLASS(bus))->write(bus, value);
}

END_DECLS
/*----------------------------------------------------------------------------*/
/** Simple GpioBus implementation based on a pin array. */
extern const struct GpioBusClass * const SimpleGpioBus;

struct SimpleGpioBusConfig
{
  /** Mandatory: pointer to an array of pins terminated with a zero element. */
  const PinNumber *pins;
  /** Optional: initial output value for pins configured as outputs. */
  uint32_t initial;
  /** Mandatory: direction of pins in the bus. */
  enum PinDirection direction;
  /** Optional: pull-up and pull-down configuration. */
  enum PinPull pull;
  /** Optional: slew rate control of output pins. */
  enum PinSlewRate rate;
  /** Optional: push-pull or open-drain configuration for output pins. */
  enum PinType type;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GPIO_BUS_H_ */
