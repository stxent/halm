/*
 * halm/core/cortex/armv7m/nvic_defs.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_CORTEX_ARMV7M_NVIC_DEFS_H_
#define HALM_CORE_CORTEX_ARMV7M_NVIC_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Application Interrupt and Reset Control Register----------*/
#define AIRCR_VECTRESET           BIT(0)
#define AIRCR_VECTCLRACTIVE       BIT(1)
#define AIRCR_SYSRESETREQ         BIT(2)

#define AIRCR_VECTKEY_MASK        BIT_FIELD(MASK(16), 16)
#define AIRCR_VECTKEY(value)      BIT_FIELD((value), 16)

#define AIRCR_PRIGROUP_MASK       BIT_FIELD(MASK(3), 8)
#define AIRCR_PRIGROUP(value)     BIT_FIELD((value), 8)
#define AIRCR_PRIGROUP_VALUE(reg) FIELD_VALUE((reg), AIRCR_PRIGROUP_MASK, 8)
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_ARMV7M_NVIC_DEFS_H_ */
