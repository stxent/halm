/*
 * libhalm/target.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef LIBHALM_TARGET_H_
#define LIBHALM_TARGET_H_
/*----------------------------------------------------------------------------*/
#include <libxcore/target.h>
/*----------------------------------------------------------------------------*/
#if defined(LPC11XX)
#define GENERATION      gen_1
#define PLATFORM        lpc11xx
#define PLATFORM_TYPE   nxp
#elif defined(LPC11EXX)
#define GENERATION      gen_1
#define PLATFORM        lpc11exx
#define PLATFORM_TYPE   nxp
#elif defined(LPC13XX)
#define GENERATION      gen_1
#define PLATFORM        lpc13xx
#define PLATFORM_TYPE   nxp
#elif defined(LPC17XX)
#define GENERATION      gen_1
#define PLATFORM        lpc17xx
#define PLATFORM_TYPE   nxp
#elif defined(LPC43XX)
#define GENERATION      gen_1
#define PLATFORM        lpc43xx
#define PLATFORM_TYPE   nxp
#else
#error "Target architecture is undefined"
#endif
/*----------------------------------------------------------------------------*/
#endif /* LIBHALM_TARGET_H_ */
