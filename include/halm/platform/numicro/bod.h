/*
 * halm/platform/numicro/bod.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_BOD_H_
#define HALM_PLATFORM_NUMICRO_BOD_H_
/*----------------------------------------------------------------------------*/
#include <halm/interrupt.h>
#include <halm/irq.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/bod.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct InterruptClass * const Bod;

struct BodConfig
{
  /**
   * Optional: event type, may be left unused when BOD interrupt is not enabled.
   * @n Available options:
   *   - @b INPUT_RISING.
   *   - @b INPUT_FALLING.
   *   - @b INPUT_TOGGLE.
   */
  enum InputEvent event;
  /** Mandatory: brown-out detector threshold voltage. */
  enum BodLevel level;
  /** Optional: output de-glitch time. */
  enum BodTimeout timeout;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Optional: enable reset function. */
  bool reset;
  /** Optional: enable low power mode. */
  bool slow;
};

struct Bod
{
  struct Interrupt base;

  void (*callback)(void *);
  void *callbackArgument;

  enum InputEvent event;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_BOD_H_ */
