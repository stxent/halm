/*
 * halm/platform/lpc/lpc11exx/bod.h
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_BOD_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC11EXX_BOD_H_
#define HALM_PLATFORM_LPC_LPC11EXX_BOD_H_
/*----------------------------------------------------------------------------*/
enum BodEventLevel
{
  BOD_EVENT_2V22  = 2,
  BOD_EVENT_2V52  = 3,
  BOD_EVENT_2V80  = 4,

  BOD_EVENT_MIN   = BOD_EVENT_2V22,
  BOD_EVENT_MAX   = BOD_EVENT_2V80
};

enum BodResetLevel
{
  BOD_RESET_DISABLED  = 0,

  BOD_RESET_1V46      = 1,
  BOD_RESET_2V06      = 2,
  BOD_RESET_2V35      = 3,
  BOD_RESET_2V63      = 4,

  BOD_RESET_MIN       = BOD_RESET_1V46,
  BOD_RESET_MAX       = BOD_RESET_2V63
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC11EXX_BOD_H_ */
