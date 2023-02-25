/*
 * halm/platform/numicro/m48x/config_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_M48X_CONFIG_DEFS_H_
#define HALM_PLATFORM_NUMICRO_M48X_CONFIG_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Configuration register 0----------------------------------*/
#define CONFIG0_DFEN                    BIT(0)
#define CONFIG0_LOCK                    BIT(1)

#define CONFIG0_CWDTEN_MASK             BIT_FIELD(MASK(2), 3)
#define CONFIG0_CWDTEN(value)           BIT_FIELD((value), 3)
#define CONFIG0_CWDTEN_VALUE(reg) \
    FIELD_VALUE((reg), CONFIG0_CWDTEN_MASK, 3)

#define CONFIG0_MBS                     BIT(5)

#define CONFIG0_CBS_MASK                BIT_FIELD(MASK(2), 6)
#define CONFIG0_CBS(value)              BIT_FIELD((value), 6)
#define CONFIG0_CBS_VALUE(reg) \
    FIELD_VALUE((reg), CONFIG0_CBS_MASK, 6)

#define CONFIG0_CIOINI                  BIT(10)
#define CONFIG0_ICELOCK                 BIT(11)
#define CONFIG0_CBODEN                  BIT(19)
#define CONFIG0_CBORST                  BIT(20)

#define CONFIG0_CBOV_MASK               BIT_FIELD(MASK(3), 21)
#define CONFIG0_CBOV(value)             BIT_FIELD((value), 21)
#define CONFIG0_CBOV_VALUE(reg) \
    FIELD_VALUE((reg), CONFIG0_CBOV_MASK, 21)

#define CONFIG0_CFGXT1                  BIT(27)
#define CONFIG0_CWDTPDEN                BIT(30)
#define CONFIG0_CWDTEN2                 BIT(31)
/*------------------Configuration register 2----------------------------------*/
#define CONFIG2_ALOCK_MASK              BIT_FIELD(MASK(8), 0)
#define CONFIG2_ALOCK(value)            BIT_FIELD((value), 0)
#define CONFIG2_ALOCK_VALUE(reg) \
    FIELD_VALUE((reg), CONFIG2_ALOCK_MASK, 0)

#define CONFIG2_SBLOCK_MASK             BIT_FIELD(MASK(8), 8)
#define CONFIG2_SBLOCK(value)           BIT_FIELD((value), 8)
#define CONFIG2_SBLOCK_VALUE(reg) \
    FIELD_VALUE((reg), CONFIG2_SBLOCK_MASK, 8)
/*------------------Configuration register 3----------------------------------*/
#define CONFIG3_UART1PSL_MASK           BIT_FIELD(MASK(2), 0)
#define CONFIG3_UART1PSL(value)         BIT_FIELD((value), 0)
#define CONFIG3_UART1PSL_VALUE(reg) \
    FIELD_VALUE((reg), CONFIG3_UART1PSL_MASK, 0)

#define CONFIG3_SPIMPSL_MASK            BIT_FIELD(MASK(2), 4)
#define CONFIG3_SPIMPSL(value)          BIT_FIELD((value), 4)
#define CONFIG3_SPIMPSL_VALUE(reg) \
    FIELD_VALUE((reg), CONFIG3_SPIMPSL_MASK, 4)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M48X_CONFIG_DEFS_H_ */
