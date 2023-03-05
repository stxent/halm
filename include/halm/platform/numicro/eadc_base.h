/*
 * halm/platform/numicro/eadc_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_EADC_BASE_H_
#define HALM_PLATFORM_NUMICRO_EADC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/interface.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/eadc_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const EadcBase;

struct EadcBaseConfig
{
  /** Optional: resolution of the conversion results. */
  uint8_t accuracy;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: disable automatic peripheral locking. */
  bool shared;
};

struct EadcBase
{
  struct Interface base;

  void *reg;
  void (*handler)(void *);

  struct
  {
    IrqNumber p0;
    IrqNumber p1;
    IrqNumber p2;
    IrqNumber p3;
  } irq;

  /* Precalculated value of Control Register */
  uint32_t control;
  /* Unique peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Common functions */
struct AdcPin adcConfigPin(const struct EadcBase *, PinNumber);
void adcReleasePin(struct AdcPin);
uint32_t adcSetupPins(struct EadcBase *, const PinNumber *, struct AdcPin *,
    size_t);
struct Pin adcSetupTriggerPin(uint8_t, PinNumber);

/* Platform-specific functions */
struct EadcBase *adcGetInstance(uint8_t);
bool adcSetInstance(uint8_t, struct EadcBase *, struct EadcBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_EADC_BASE_H_ */
