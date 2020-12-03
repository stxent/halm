/*
 * halm/platform/lpc/gen_1/dac_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GEN_1_DAC_DEFS_H_
#define HALM_PLATFORM_LPC_GEN_1_DAC_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#define DAC_RESOLUTION                  10 /* Input data width in bits */
/*------------------Converter Register----------------------------------------*/
#define CR_OUTPUT(value)                BIT_FIELD((value), 6)
#define CR_OUTPUT_MASK                  BIT_FIELD(MASK(DAC_RESOLUTION), 6)
#define CR_OUTPUT_VALUE(reg)            FIELD_VALUE((reg), CR_OUTPUT_MASK, 6)
#define CR_BIAS                         BIT(16) /* Reduces output current */
/*------------------Control register------------------------------------------*/
#define CTRL_INT_DMA_REQ                BIT(0)
#define CTRL_DBLBUF_ENA                 BIT(1)
#define CTRL_CNT_ENA                    BIT(2)
#define CTRL_DMA_ENA                    BIT(3)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_1_DAC_DEFS_H_ */
