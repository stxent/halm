/*
 * halm/platform/lpc/lpc17xx/bod.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_BOD_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC17XX_BOD_H_
#define HALM_PLATFORM_LPC_LPC17XX_BOD_H_
/*----------------------------------------------------------------------------*/
enum BodEventLevel
{
  BOD_EVENT_2V2
};

enum BodResetLevel
{
  BOD_RESET_DISABLED,
  BOD_RESET_1V85
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC17XX_BOD_H_ */
