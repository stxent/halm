/*
 * halm/platform/lpc/fast_gpio_bus.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_FAST_GPIO_BUS_H_
#define HALM_PLATFORM_LPC_FAST_GPIO_BUS_H_
/*----------------------------------------------------------------------------*/
#include <halm/gpio_bus.h>
/*----------------------------------------------------------------------------*/
extern const struct GpioBusClass * const FastGpioBus;

struct FastGpioBusConfig
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

struct FastGpioBus
{
  struct GpioBus base;

  /* Access mask */
  uint32_t mask;

  /* First pin of the bus */
  struct Pin first;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void fastGpioBusConfigPins(struct FastGpioBus *,
    const struct FastGpioBusConfig *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_FAST_GPIO_BUS_H_ */
