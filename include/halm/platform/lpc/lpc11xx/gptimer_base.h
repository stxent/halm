/*
 * halm/platform/lpc/lpc11xx/gptimer_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GPTIMER_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC11XX_GPTIMER_BASE_H_
#define HALM_PLATFORM_LPC_LPC11XX_GPTIMER_BASE_H_
/*----------------------------------------------------------------------------*/
/** Symbolic names for two different types of timers. */
enum
{
  GPTIMER_CT16B0,
  GPTIMER_CT16B1,
  GPTIMER_CT32B0,
  GPTIMER_CT32B1
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void gpTimerEnableCapture(uint8_t channel __attribute__((unused)),
    uint8_t number __attribute__((unused)))
{
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC11XX_GPTIMER_BASE_H_ */
