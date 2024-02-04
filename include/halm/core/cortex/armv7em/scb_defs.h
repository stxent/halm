/*
 * halm/core/cortex/armv7em/scb_defs.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_CORTEX_SCB_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_CORE_CORTEX_ARMV7EM_SCB_DEFS_H_
#define HALM_CORE_CORTEX_ARMV7EM_SCB_DEFS_H_
/*----------------------------------------------------------------------------*/
#include "../armv7m/scb_defs.h"
/*------------------Configuration and Control Register------------------------*/
#define CCR_DC                          BIT(16)
#define CCR_IC                          BIT(17)
#define CCR_BP                          BIT(18)
/*------------------Cache Size ID Register------------------------------------*/
#define CCSIDR_LINESIZE_MASK            BIT_FIELD(MASK(3), 0)
#define CCSIDR_LINESIZE_VALUE(reg) \
    FIELD_VALUE((reg), CCSIDR_LINESIZE_MASK, 0)

#define CCSIDR_ASSOCIATIVITY_MASK       BIT_FIELD(MASK(10), 3)
#define CCSIDR_ASSOCIATIVITY_VALUE(reg) \
    FIELD_VALUE((reg), CCSIDR_ASSOCIATIVITY_MASK, 3)

#define CCSIDR_NUMSETS_MASK             BIT_FIELD(MASK(15), 13)
#define CCSIDR_NUMSETS_VALUE(reg) \
    FIELD_VALUE((reg), CCSIDR_NUMSETS_MASK, 13)

#define CCSIDR_WA                       BIT(28)
#define CCSIDR_RA                       BIT(29)
#define CCSIDR_WB                       BIT(30)
#define CCSIDR_WT                       BIT(31)
/*------------------Cache Size Selection Register-----------------------------*/
#define CSSELR_LEVEL_MASK               BIT_FIELD(MASK(3), 1)
#define CSSELR_LEVEL(value)             BIT_FIELD((value), 1)
#define CSSELR_LEVEL_VALUE(reg)         FIELD_VALUE((reg), CSSELR_LEVEL_MASK, 1)

#define CSSELR_IND                      BIT(0)
/*------------------L1 Cache Control Register---------------------------------*/
#define CACR_SIWT                       BIT(0)
#define CACR_ECCDIS                     BIT(1)
#define CACR_FORCEWT                    BIT(2)
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_ARMV7EM_SCB_DEFS_H_ */
