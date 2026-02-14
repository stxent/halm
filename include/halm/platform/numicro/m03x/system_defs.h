/*
 * halm/platform/numicro/m03x/system_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_SYSTEM_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_M03X_SYSTEM_DEFS_H_
#define HALM_PLATFORM_NUMICRO_M03X_SYSTEM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Multiple Function Control registers-----------------------*/
#define MFP_FUNCTION_MASK(pin)          BIT_FIELD(MASK(4), (pin) * 4)
#define MFP_FUNCTION(value, pin)        BIT_FIELD((value), (pin) * 4)
/*------------------Brown-out Detector Control register-----------------------*/
#define BODCTL_BODEN                    BIT(0)
#define BODCTL_BODRSTEN                 BIT(3)
#define BODCTL_BODIF                    BIT(4)
#define BODCTL_BODLPM                   BIT(5)
#define BODCTL_BODOUT                   BIT(6)
#define BODCTL_LVREN                    BIT(7)

#define BODCTL_BODDGSEL_MASK            BIT_FIELD(MASK(3), 8)
#define BODCTL_BODDGSEL(value)          BIT_FIELD((value), 8)
#define BODCTL_BODDGSEL_VALUE(reg) \
    FIELD_VALUE((reg), BODCTL_BODDGSEL_MASK, 8)

#define BODCTL_LVRDGSEL_MASK            BIT_FIELD(MASK(3), 12)
#define BODCTL_LVRDGSEL(value)          BIT_FIELD((value), 12)
#define BODCTL_LVRDGSEL_VALUE(reg) \
    FIELD_VALUE((reg), BODCTL_LVRDGSEL_MASK, 12)

#define BODCTL_BODVL_MASK               BIT_FIELD(MASK(3), 16)
#define BODCTL_BODVL(value)             BIT_FIELD((value), 16)
#define BODCTL_BODVL_VALUE(reg) \
    FIELD_VALUE((reg), BODCTL_BODVL_MASK, 16)

#define BODCTL_LVRVL                    BIT(20)
/*------------------Register Lock Control register----------------------------*/
#define REGLCTL_MAGIC_NUMBER_0          0x59
#define REGLCTL_MAGIC_NUMBER_1          0x16
#define REGLCTL_MAGIC_NUMBER_2          0x88
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M03X_SYSTEM_DEFS_H_ */
