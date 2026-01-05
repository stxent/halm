/*
 * halm/platform/bouffalo/bl602/l1c_defs.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_BL602_L1C_DEFS_H_
#define HALM_PLATFORM_BOUFFALO_BL602_L1C_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------L1C feature configuration---------------------------------*/
#define L1C_CONFIG_CACHEABLE            BIT(0)
#define L1C_CONFIG_CNT_EN               BIT(1)
#define L1C_CONFIG_INVALID_EN           BIT(2)
#define L1C_CONFIG_INVALID_DONE         BIT(3)

#define L1C_CONFIG_WAYDIS(value)        BIT_FIELD((value), 8)
#define L1C_CONFIG_WAYDIS_MASK          BIT_FIELD(MASK(4), 8)
#define L1C_CONFIG_WAYDIS_VALUE(reg) \
    FIELD_VALUE((reg), L1C_CONFIG_WAYDIS_MASK, 8)

#define L1C_CONFIG_IROM_2T_ACCESS       BIT(12)
#define L1C_CONFIG_BYPASS               BIT(14)
#define L1C_CONFIG_BMX_ERR_EN           BIT(15)

#define L1C_CONFIG_BMX_ARB_MODE(value)  BIT_FIELD((value), 16)
#define L1C_CONFIG_BMX_ARB_MODE_MASK    BIT_FIELD(MASK(2), 16)
#define L1C_CONFIG_BMX_ARB_MODE_VALUE(reg) \
    FIELD_VALUE((reg), L1C_CONFIG_BMX_ARB_MODE_MASK, 16)

#define L1C_CONFIG_TIMEOUT_EN(value)    BIT_FIELD((value), 20)
#define L1C_CONFIG_TIMEOUT_EN_MASK      BIT_FIELD(MASK(4), 20)
#define L1C_CONFIG_TIMEOUT_EN_VALUE(reg) \
    FIELD_VALUE((reg), L1C_CONFIG_TIMEOUT_EN_MASK, 20)

#define L1C_CONFIG_BMX_BUSY_OPTION_DIS  BIT(24)
#define L1C_CONFIG_EARLY_RESP_DIS       BIT(25)
#define L1C_CONFIG_WRAP_DIS             BIT(26)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_BL602_L1C_DEFS_H_ */
