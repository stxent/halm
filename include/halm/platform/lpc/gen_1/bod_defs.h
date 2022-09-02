/*
 * halm/platform/lpc/gen_1/bod_defs.h
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GEN_1_BOD_DEFS_H_
#define HALM_PLATFORM_LPC_GEN_1_BOD_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------BOD Control register--------------------------------------*/
#define BODCTRL_BODRSTLEV_MASK          BIT_FIELD(MASK(2), 0)
#define BODCTRL_BODRSTLEV(value)        BIT_FIELD((value), 0)
#define BODCTRL_BODRSTLEV_VALUE(reg) \
    FIELD_VALUE((reg), BODCTRL_BODRSTLEV_MASK, 0)

#define BODCTRL_BODINTVAL_MASK          BIT_FIELD(MASK(2), 2)
#define BODCTRL_BODINTVAL(value)        BIT_FIELD((value), 2)
#define BODCTRL_BODINTVAL_VALUE(reg) \
    FIELD_VALUE((reg), BODCTRL_BODINTVAL_MASK, 2)

#define BODCTRL_BODRSTENA               BIT(4)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_1_BOD_DEFS_H_ */
