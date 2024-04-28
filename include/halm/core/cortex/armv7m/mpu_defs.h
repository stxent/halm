/*
 * halm/core/cortex/armv7m/mpu_defs.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_CORTEX_ARMV7M_MPU_DEFS_H_
#define HALM_CORE_CORTEX_ARMV7M_MPU_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Type register---------------------------------------------*/
#define TYPE_SEPARATE                   BIT(0)

#define TYPE_DREGION_MASK               BIT_FIELD(MASK(8), 8)
#define TYPE_DREGION_VALUE(reg) \
    FIELD_VALUE((reg), TYPE_DREGION_MASK, 8)

#define TYPE_IREGION_MASK               BIT_FIELD(MASK(8), 16)
#define TYPE_IREGION_VALUE(reg) \
    FIELD_VALUE((reg), TYPE_IREGION_MASK, 16)
/*------------------Control register------------------------------------------*/
#define CTRL_ENABLE                     BIT(0)
#define CTRL_HFNMIENA                   BIT(1)
#define CTRL_PRIVDEFENA                 BIT(2)
/*------------------Region Base Address Register------------------------------*/
#define RBAR_REGION_MASK                BIT_FIELD(MASK(4), 0)
#define RBAR_REGION(value)              BIT_FIELD((value), 0)
#define RBAR_REGION_VALUE(reg)          FIELD_VALUE((reg), RBAR_REGION_MASK, 0)

#define RBAR_VALID                      BIT(4)

#define RBAR_ADDR_MASK                  BIT_FIELD(MASK(27), 5)
#define RBAR_ADDR(value)                ((value) & RBAR_ADDR_MASK)
/*------------------Region Attribute and Size Register------------------------*/
enum
{
  AP_NO_ACCESS    = 0,
  AP_RW_NA        = 1,
  AP_RW_RO        = 2,
  AP_FULL_ACCESS  = 3,
  AP_RO_NA        = 5,
  AP_READ_ONLY    = 6
};

#define RASR_ENABLE                     BIT(0)

/* Protection Region Size field */
#define RASR_SIZE_MASK                  BIT_FIELD(MASK(5), 1)
#define RASR_SIZE(value)                BIT_FIELD((value), 1)
#define RASR_SIZE_VALUE(reg)            FIELD_VALUE((reg), RASR_SIZE_MASK, 1)

/* Sub-Region Disable field */
#define RASR_SRD_MASK                   BIT_FIELD(MASK(8), 8)
#define RASR_SRD(value)                 BIT_FIELD((value), 8)
#define RASR_SRD_VALUE(reg)             FIELD_VALUE((reg), RASR_SRD_MASK, 8)

/* Bufferable bit */
#define RASR_B                          BIT(16)
/* Cacheable bit */
#define RASR_C                          BIT(17)
/* Shareable bit */
#define RASR_S                          BIT(18)

/* Type Extension field */
#define RASR_TEX_MASK                   BIT_FIELD(MASK(3), 19)
#define RASR_TEX(value)                 BIT_FIELD((value), 19)
#define RASR_TEX_VALUE(reg)             FIELD_VALUE((reg), RASR_TEX_MASK, 19)

/* Data Access Permission field */
#define RASR_AP_MASK                    BIT_FIELD(MASK(3), 24)
#define RASR_AP(value)                  BIT_FIELD((value), 24)
#define RASR_AP_VALUE(reg)              FIELD_VALUE((reg), RASR_AP_MASK, 24)

/* Instruction Access Disable bit */
#define RASR_XN                         BIT(28)
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_ARMV7M_MPU_DEFS_H_ */
