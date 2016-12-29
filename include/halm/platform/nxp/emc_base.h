/*
 * halm/platform/nxp/emc_base.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_EMC_BASE_H_
#define HALM_PLATFORM_NXP_EMC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/entity.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
struct EmcPinDescription
{
  pinNumber cas;
  pinNumber oe;
  pinNumber ras;
  pinNumber we;

  pinNumber bls[4];
  pinNumber ckeout[4];
  pinNumber clk[4];
  pinNumber cs[4];
  pinNumber dqmout[4];
  pinNumber dycs[4];
};
/*----------------------------------------------------------------------------*/
uint32_t emcGetClock(void);
bool emcSetDynamicMemoryDescriptor(uint8_t, const struct Entity *,
    struct Entity *);
bool emcSetStaticMemoryDescriptor(uint8_t channel, const struct Entity *,
    struct Entity *);
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_EMC_BASE_H_ */
