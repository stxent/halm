/*
 * halm/platform/bouffalo/pwm_base.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_PWM_BASE_H_
#define HALM_PLATFORM_BOUFFALO_PWM_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const PwmBase;

struct PwmBaseConfig
{
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct PwmBase
{
  struct Timer base;

  void *reg;
  void (*handler)(void *);
  IrqNumber irq;

  /* Peripheral block identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Platform-specific functions */
uint8_t pwmChannelFromPin(PinNumber);
void pwmConfigPins(PinNumber, bool);
uint32_t pwmGetClock(const struct PwmBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_PWM_BASE_H_ */
