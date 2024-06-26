/*
 * halm/core/cortex/armv7m/scb_defs.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_CORTEX_SCB_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_CORE_CORTEX_ARMV7M_SCB_DEFS_H_
#define HALM_CORE_CORTEX_ARMV7M_SCB_DEFS_H_
/*----------------------------------------------------------------------------*/
#include "../armv6m/scb_defs.h"
/*------------------Interrupt Control and State Register----------------------*/
#define ICSR_VECTACTIVE_MASK            BIT_FIELD(MASK(9), 0)
#define ICSR_VECTACTIVE(value)          BIT_FIELD((value), 0)
#define ICSR_VECTACTIVE_VALUE(reg) \
    FIELD_VALUE((reg), ICSR_VECTACTIVE_MASK, 0)

#define ICSR_RETTOBASE                  BIT(11)

#define ICSR_VECTPENDING_MASK           BIT_FIELD(MASK(9), 12)
#define ICSR_VECTPENDING(value)         BIT_FIELD((value), 12)
#define ICSR_VECTPENDING_VALUE(reg) \
    FIELD_VALUE((reg), ICSR_VECTPENDING_MASK, 12)

#define ICSR_ISRPENDING                 BIT(22)
#define ICSR_ISRPREEMPT                 BIT(23)
#define ICSR_PENDSTCLR                  BIT(25)
#define ICSR_PENDSTSET                  BIT(26)
#define ICSR_PENDSVCLR                  BIT(27)
#define ICSR_PENDSVSET                  BIT(28)
#define ICSR_NMIPENDSET                 BIT(31)
/*------------------Application Interrupt and Reset Control Register----------*/
#define AIRCR_VECTRESET                 BIT(0)

#define AIRCR_PRIGROUP_MASK             BIT_FIELD(MASK(3), 8)
#define AIRCR_PRIGROUP(value)           BIT_FIELD((value), 8)
#define AIRCR_PRIGROUP_VALUE(reg) \
    FIELD_VALUE((reg), AIRCR_PRIGROUP_MASK, 8)

#define AIRCR_ENDIANNESS                BIT(15)
/*------------------Configuration and Control Register------------------------*/
#define CCR_NONBASETHRDENA              BIT(0)
#define CCR_USERSETMPEND                BIT(1)
#define CCR_UNALIGN_TRP                 BIT(3)
#define CCR_DIV_0_TRP                   BIT(4)
#define CCR_BFHFNMIGN                   BIT(8)
#define CCR_STKALIGN                    BIT(9)
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_ARMV7M_SCB_DEFS_H_ */
