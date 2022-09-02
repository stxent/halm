/*
 * halm/platform/lpc/lpc43xx/bod.h
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LPC_BOD_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC43XX_BOD_H_
#define HALM_PLATFORM_LPC_LPC43XX_BOD_H_
/*----------------------------------------------------------------------------*/
enum BodEventLevel
{
  /* Unavailable on flash-based parts  */
  BOD_EVENT_2V75,
  /* Unavailable on flash-based parts  */
  BOD_EVENT_2V85,

  BOD_EVENT_2V95,
  BOD_EVENT_3V05
};

enum BodResetLevel
{
  /* Unavailable on flash-based parts  */
  BOD_RESET_1V7,
  /* Unavailable on flash-based parts  */
  BOD_RESET_1V8,

  /* 1.9V for flashless parts */
  BOD_RESET_2V1,
  /* 2.0V for flashless parts  */
  BOD_RESET_2V2
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_BOD_H_ */
