/*
 * halm/platform/lpc/lpc43xx/gima_defs.h
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC43XX_GIMA_DEFS_H_
#define HALM_PLATFORM_LPC_LPC43XX_GIMA_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Input multiplexer registers-------------------------------*/
/* Invert input */
#define GIMA_INV                        BIT(0)
/* Enable rising edge detection */
#define GIMA_EDGE                       BIT(1)
/* Enable synchronization */
#define GIMA_SYNCH                      BIT(2)
/* Enable single pulse generation */
#define GIMA_PULSE                      BIT(3)

/* Input selection */
#define GIMA_SELECT_MASK                BIT_FIELD(MASK(4), 4)
#define GIMA_SELECT(value)              BIT_FIELD((value), 4)
#define GIMA_SELECT_VALUE(reg)          FIELD_VALUE((reg), GIMA_SELECT_MASK, 4)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_GIMA_DEFS_H_ */
