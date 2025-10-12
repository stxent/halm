/*
 * halm/platform/lpc/wkt_base.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_WKT_BASE_H_
#define HALM_PLATFORM_LPC_WKT_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const WktBase;

enum [[gnu::packed]] WktClockSource
{
  WKT_CLOCK_IRC,
  WKT_CLOCK_LOW_POWER,
  WKT_CLOCK_EXTERNAL_PIN
};

struct WktBaseConfig
{
  /** Optional: input pin. */
  PinNumber pin;
  /** Mandatory: clock source. */
  enum WktClockSource source;
};

struct WktBase
{
  struct Timer base;

  void *reg;
  void (*handler)(void *);
  IrqNumber irq;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Platform-specific functions */
uint32_t wktGetClock(const struct WktBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_WKT_BASE_H_ */
