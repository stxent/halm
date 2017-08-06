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
  PinNumber cas;
  PinNumber oe;
  PinNumber ras;
  PinNumber we;

  PinNumber bls[4];
  PinNumber ckeout[4];
  PinNumber clk[4];
  PinNumber cs[4];
  PinNumber dqmout[4];
  PinNumber dycs[4];
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

uint32_t emcGetClock(void);
void *emcGetDynamicMemoryAddress(uint8_t);
void *emcGetStaticMemoryAddress(uint8_t);
bool emcSetDynamicMemoryDescriptor(uint8_t, const struct Entity *,
    struct Entity *);
bool emcSetStaticMemoryDescriptor(uint8_t, const struct Entity *,
    struct Entity *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_EMC_BASE_H_ */
