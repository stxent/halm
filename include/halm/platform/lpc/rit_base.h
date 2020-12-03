/*
 * halm/platform/lpc/rit_base.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_RIT_BASE_H_
#define HALM_PLATFORM_LPC_RIT_BASE_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
void ritBaseInit(void);
void ritBaseDeinit(void);
uint32_t ritGetClock(void);
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_RIT_BASE_H_ */
