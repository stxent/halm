/*
 * platform/nxp/gppwm_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_GPPWM_DEFS_H_
#define PLATFORM_NXP_GPPWM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <platform/nxp/gptimer_defs.h>
/*------------------Timer Control Register------------------------------------*/
#define TCR_PWM_ENABLE                  BIT(3)
/*------------------PWM Control Register--------------------------------------*/
/* Available for channels from 2 to 6 */
#define PCR_DOUBLE_EDGE(channel)        BIT((channel))
/* Available for channels from 1 to 6 */
#define PCR_OUTPUT_ENABLED(channel)     BIT((channel) + 8)
/*------------------PWM Latch Enable Register---------------------------------*/
#define LER_ENABLE_LATCH(channel)       BIT((channel))
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_GPPWM_DEFS_H_ */
