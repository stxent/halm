/*
 * halm/interrupt.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Abstract interrupt class.
 */

#ifndef HALM_INTERRUPT_H_
#define HALM_INTERRUPT_H_
/*----------------------------------------------------------------------------*/
#include <xcore/entity.h>
#include <stdbool.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct InterruptClass
{
  CLASS_HEADER

  void (*enable)(void *);
  void (*disable)(void *);

  void (*setCallback)(void *, void (*)(void *), void *);
};

struct Interrupt
{
  struct Entity base;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/**
 * Enable the interrupt generation.
 * @param interrupt Pointer to an Interrupt object.
 */
static inline void interruptEnable(void *interrupt)
{
  ((const struct InterruptClass *)CLASS(interrupt))->enable(interrupt);
}

/**
 * Disable the interrupt generation.
 * @param interrupt Pointer to an Interrupt object.
 */
static inline void interruptDisable(void *interrupt)
{
  ((const struct InterruptClass *)CLASS(interrupt))->disable(interrupt);
}

/**
 * Set a callback function for an interrupt.
 * @param interrupt Pointer to an Interrupt object.
 * @param callback Callback function.
 * @param argument Callback function argument.
 */
static inline void interruptSetCallback(void *interrupt,
    void (*callback)(void *), void *argument)
{
  ((const struct InterruptClass *)CLASS(interrupt))->setCallback(interrupt,
      callback, argument);
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_INTERRUPT_H_ */
