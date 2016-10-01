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
#include <stdbool.h>
#include <stdint.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct InterruptClass
{
  CLASS_HEADER

  void (*callback)(void *, void (*)(void *), void *);
  void (*setEnabled)(void *, bool);
};
/*----------------------------------------------------------------------------*/
struct Interrupt
{
  struct Entity base;
};
/*----------------------------------------------------------------------------*/
/**
 * Set the interrupt callback.
 * @param interrupt Pointer to an Interrupt object.
 * @param callback Callback function.
 * @param argument Callback function argument.
 */
static inline void intCallback(void *interrupt, void (*callback)(void *),
    void *argument)
{
  ((const struct InterruptClass *)CLASS(interrupt))->callback(interrupt,
      callback, argument);
}
/*----------------------------------------------------------------------------*/
/**
 * Enable or disable the interrupt generation.
 * @param interrupt Pointer to an Interrupt object.
 * @param state Interrupt state: @b true to enable or @b false to disable.
 */
static inline void intSetEnabled(void *interrupt, bool state)
{
  ((const struct InterruptClass *)CLASS(interrupt))->setEnabled(interrupt,
      state);
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_INTERRUPT_H_ */
