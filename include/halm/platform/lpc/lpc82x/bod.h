/*
 * halm/platform/lpc/lpc82x/bod.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_BOD_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC82X_BOD_H_
#define HALM_PLATFORM_LPC_LPC82X_BOD_H_
/*----------------------------------------------------------------------------*/
enum BodEventLevel
{
  BOD_EVENT_2V25  = 2,
  BOD_EVENT_2V54  = 3,
  BOD_EVENT_2V85  = 4,

  BOD_EVENT_MIN   = BOD_EVENT_2V25,
  BOD_EVENT_MAX   = BOD_EVENT_2V85
};

enum BodResetLevel
{
  BOD_RESET_DISABLED  = 0,

  BOD_RESET_2V05      = 2,
  BOD_RESET_2V34      = 3,
  BOD_RESET_2V63      = 4,

  BOD_RESET_MIN       = BOD_RESET_2V05,
  BOD_RESET_MAX       = BOD_RESET_2V63
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC82X_BOD_H_ */
