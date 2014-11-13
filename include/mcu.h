/*
 * mcu.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef MCU_H_
#define MCU_H_
/*----------------------------------------------------------------------------*/
#if defined(LPC11XX)
#define CORE            m0
#define CORE_TYPE       cortex
#define PLATFORM        lpc11xx
#define PLATFORM_TYPE   nxp
#elif defined(LPC11EXX)
#define CORE            m0
#define CORE_TYPE       cortex
#define PLATFORM        lpc11exx
#define PLATFORM_TYPE   nxp
#elif defined(LPC13XX)
#define CORE            m3
#define CORE_TYPE       cortex
#define PLATFORM        lpc13xx
#define PLATFORM_TYPE   nxp
#elif defined(LPC17XX)
#define CORE            m3
#define CORE_TYPE       cortex
#define PLATFORM        lpc17xx
#define PLATFORM_TYPE   nxp
#elif defined(LPC43XX)
#define CORE            m4
#define CORE_TYPE       cortex
#define PLATFORM        lpc43xx
#define PLATFORM_TYPE   nxp
#endif
/*----------------------------------------------------------------------------*/
#endif /* MCU_H_ */
