/*
 * platform/nxp/dac_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef DAC_DEFS_H_
#define DAC_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <bits.h>
/*------------------Converter Register----------------------------------------*/
#define CR_OUTPUT(value)                BIT_FIELD((value), 6)
#define CR_OUTPUT_MASK                  BIT_FIELD(MASK(10), 6)
#define CR_OUTPUT_VALUE(reg)            FIELD_VALUE((reg), CR_OUTPUT_MASK, 6)
/* Set to enable shorter settling time or reset to longer settling time */
#define CR_BIAS                         BIT(16)
/*------------------Control register------------------------------------------*/
#define CTRL_INT_DMA_REQ                BIT(0)
#define CTRL_DBLBUF_ENA                 BIT(1)
#define CTRL_CNT_ENA                    BIT(2)
#define CTRL_DMA_ENA                    BIT(3)
/*----------------------------------------------------------------------------*/
#endif /* DAC_DEFS_H_ */
