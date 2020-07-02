/*
 * halm/platform/generic/signal_handler.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_GENERIC_SIGNAL_HANDLER_H_
#define HALM_PLATFORM_GENERIC_SIGNAL_HANDLER_H_
/*----------------------------------------------------------------------------*/
#include <halm/interrupt.h>
/*----------------------------------------------------------------------------*/
extern const struct InterruptClass * const SignalHandler;

struct SignalHandlerConfig
{
  /** Mandatory: signal number. */
  int signum;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_GENERIC_SIGNAL_HANDLER_H_ */
