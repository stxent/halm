/*
 * halm/platform/lpc/mrt_base.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_MRT_BASE_H_
#define HALM_PLATFORM_LPC_MRT_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const MrtBase;

struct MrtBaseConfig
{
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct MrtBase
{
  struct Timer base;

  void *reg;
  void (*handler)(void *);

  /* Base timer number */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Platform-specific functions */
uint32_t mrtGetClock(const struct MrtBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_MRT_BASE_H_ */
