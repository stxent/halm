/*
 * halm/platform/lpc/lpc43xx/event_router.h
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC43XX_EVENT_ROUTER_H_
#define HALM_PLATFORM_LPC_LPC43XX_EVENT_ROUTER_H_
/*----------------------------------------------------------------------------*/
#include <xcore/error.h>
#include <xcore/helpers.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
/* Router Event sources */
enum [[gnu::packed]] RouterEvent
{
  ER_WAKEUP0    = 1 << 0,
  ER_WAKEUP1    = 1 << 1,
  ER_WAKEUP2    = 1 << 2,
  ER_WAKEUP3    = 1 << 3,
  ER_ATIMER     = 1 << 4,
  ER_RTC        = 1 << 5,
  ER_BOD        = 1 << 6,
  ER_WWDT       = 1 << 7,
  ER_ETH        = 1 << 8,
  ER_USB0       = 1 << 9,
  ER_USB1       = 1 << 10,
  ER_SDMMC      = 1 << 11,
  ER_CAN        = 1 << 12,
  ER_TIM2       = 1 << 13,
  ER_TIM6       = 1 << 14,
  ER_QEI        = 1 << 15,
  ER_TIM14      = 1 << 16,
  ER_RESET      = 1 << 19,
  ER_BODRESET   = 1 << 20,
  ER_DPDRESET   = 1 << 21
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/**
 * Register a callback function.
 * @param callback Callback function.
 * @param object Pointer to an object used as a function argument.
 * @param events Events enabled for this callback function.
 * @return @b E_OK on success.
 */
enum Result erRegister(void (*callback)(void *), void *object, uint32_t events);

/**
 * Unregister the callback function.
 * @param object Pointer to an object to be deleted from the list.
 */
void erUnregister(const void *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_EVENT_ROUTER_H_ */
