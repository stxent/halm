/*
 * halm/platform/lpc/lpc13xx/gptimer_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GPTIMER_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC13XX_GPTIMER_BASE_H_
#define HALM_PLATFORM_LPC_LPC13XX_GPTIMER_BASE_H_
/*----------------------------------------------------------------------------*/
/** Symbolic names for two different types of timers. */
enum
{
  GPTIMER_CT16B0,
  GPTIMER_CT16B1,
  GPTIMER_CT32B0,
  GPTIMER_CT32B1
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void gpTimerEnableCapture(uint8_t, uint8_t)
{
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC13XX_GPTIMER_BASE_H_ */
