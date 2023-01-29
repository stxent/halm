/*
 * halm/platform/lpc/sct_pwm.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SCT_PWM_H_
#define HALM_PLATFORM_LPC_SCT_PWM_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/sct_base.h>
#include <halm/pwm.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const SctPwmUnit;

struct SctPwmUnitConfig
{
  /** Mandatory: timer frequency. */
  uint32_t frequency;
  /** Mandatory: cycle resolution. */
  uint32_t resolution;
  /** Optional: clock input. */
  enum SctInput clock;
  /** Optional: timer part. */
  enum SctPart part;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct SctPwmUnit
{
  struct SctBase base;

  /* Desired timer frequency */
  uint32_t frequency;
  /* Cycle width measured in timer ticks */
  uint32_t resolution;
  /* Match channel used for counter reset */
  uint8_t event;
};
/*----------------------------------------------------------------------------*/
extern const struct PwmClass * const SctPwm;
extern const struct PwmClass * const SctUnifiedPwm;

struct SctPwmConfig
{
  /** Mandatory: peripheral unit. */
  struct SctPwmUnit *parent;
  /** Mandatory: pin used as an output for modulated signal. */
  PinNumber pin;
};

struct SctPwm
{
  struct Pwm base;

  /* Pointer to a parent unit */
  struct SctPwmUnit *unit;
  /* Pointer to a match register */
  volatile void *value;
  /* Output channel number */
  uint8_t channel;
  /* Event number */
  uint8_t event;
};
/*----------------------------------------------------------------------------*/
extern const struct PwmClass * const SctPwmDoubleEdge;
extern const struct PwmClass * const SctUnifiedPwmDoubleEdge;

struct SctPwmDoubleEdgeConfig
{
  /** Mandatory: peripheral unit. */
  struct SctPwmUnit *parent;
  /** Mandatory: pin used as an output for modulated signal. */
  PinNumber pin;
};

struct SctPwmDoubleEdge
{
  struct Pwm base;

  /* Pointer to a parent unit */
  struct SctPwmUnit *unit;
  /* Pointer to a match register containing leading edge time */
  volatile void *leading;
  /* Pointer to a match register containing trailing edge time */
  volatile void *trailing;
  /* Output channel number */
  uint8_t channel;
  /* Event number for leading edge */
  uint8_t leadingEvent;
  /* Event number for trailing edge */
  uint8_t trailingEvent;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void *sctPwmCreate(void *, PinNumber);
void *sctPwmCreateDoubleEdge(void *, PinNumber);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SCT_PWM_H_ */
