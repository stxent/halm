/*
 * halm/core/cortex/armv6m/scb_defs.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_CORTEX_SCB_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_CORE_CORTEX_ARMV6M_SCB_DEFS_H_
#define HALM_CORE_CORTEX_ARMV6M_SCB_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
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
#define AIRCR_VECTCLRACTIVE             BIT(1)
#define AIRCR_SYSRESETREQ               BIT(2)

#define AIRCR_VECTKEY_MASK              BIT_FIELD(MASK(16), 16)
#define AIRCR_VECTKEY(value)            BIT_FIELD((value), 16)
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_ARMV6M_SCB_DEFS_H_ */
