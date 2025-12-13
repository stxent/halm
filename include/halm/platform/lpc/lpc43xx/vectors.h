/*
 * halm/platform/lpc/lpc43xx/vectors.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_VECTORS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC43XX_VECTORS_H_
#define HALM_PLATFORM_LPC_LPC43XX_VECTORS_H_
/*----------------------------------------------------------------------------*/
#ifdef LPC43XX
#  include "vectors_m4.h"
#elifdef LPC43XX_M0APP
#  include "vectors_m0_app.h"
#elifdef LPC43XX_M0SUB
#  include "vectors_m0_sub.h"
#else
#  error "Target core is not defined"
#endif
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_VECTORS_H_ */
