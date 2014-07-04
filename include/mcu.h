/*
 * mcu.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef MCU_H_
#define MCU_H_
/*----------------------------------------------------------------------------*/
#if defined(LPC11XX)
#define CORE_TYPE cortex
#define CORE m0
#define PLATFORM_TYPE nxp
#define PLATFORM lpc11xx
#elif defined(LPC11EXX)
#define CORE_TYPE cortex
#define CORE m0
#define PLATFORM_TYPE nxp
#define PLATFORM lpc11exx
#elif defined(LPC13XX)
#define CORE_TYPE cortex
#define CORE m3
#define PLATFORM_TYPE nxp
#define PLATFORM lpc13xx
#elif defined(LPC17XX)
#define CORE_TYPE cortex
#define CORE m3
#define PLATFORM_TYPE nxp
#define PLATFORM lpc17xx
#elif defined(LPC43XX)
#define CORE_TYPE cortex
#define CORE m4
#define PLATFORM_TYPE nxp
#define PLATFORM lpc43xx
#endif
/*----------------------------------------------------------------------------*/
#endif /* MCU_H_ */
