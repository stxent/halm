/*
 * halm/platform/lpc/wkt.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_WKT_H_
#define HALM_PLATFORM_LPC_WKT_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/wkt_base.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const Wkt;

struct WktConfig
{
  /** Optional: input pin. */
  PinNumber pin;
  /** Mandatory: clock source. */
  enum WktClockSource source;
  /** Optional: timer interrupt priority. */
  IrqPriority priority;
};

struct Wkt
{
  struct WktBase base;

  /* User interrupt handler */
  void (*callback)(void *);
  /* Argument passed to the user interrupt handler */
  void *callbackArgument;

  /* Timer overflow value */
  uint32_t overflow;
  /* Timer wake-up occurred */
  bool fired;
  /* Restart the timer automatically */
  bool restart;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

bool wktWakeupOccurred(const struct Wkt *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_WKT_H_ */
