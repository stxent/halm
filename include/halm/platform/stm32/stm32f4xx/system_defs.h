/*
 * halm/platform/stm32/stm32f4xx/system_defs.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_STM32F4XX_SYSTEM_DEFS_H_
#define HALM_PLATFORM_STM32_STM32F4XX_SYSTEM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Flash Access Control Register-----------------------------*/
#define FLASH_ACR_LATENCY(value)        BIT_FIELD((value), 0)
#define FLASH_ACR_LATENCY_MASK          BIT_FIELD(MASK(3), 0)
#define FLASH_ACR_LATENCY_VALUE(reg) \
    FIELD_VALUE((reg), FLASH_ACR_LATENCY_MASK, 0)

#define FLASH_ACR_PRFTEN                BIT(8)
#define FLASH_ACR_ICEN                  BIT(9)
#define FLASH_ACR_DCEN                  BIT(10)
#define FLASH_ACR_ICRST                 BIT(11)
#define FLASH_ACR_DCRST                 BIT(12)
/*------------------Power Control Register------------------------------------*/
/* Low-power deep sleep */
#define PWR_CR_LPDS                     BIT(0)
/* Power down deep sleep */
#define PWR_CR_PDDS                     BIT(1)
/* Clear wake-up flag */
#define PWR_CR_CWUF                     BIT(2)
/* Clear standby flag */
#define PWR_CR_CSBF                     BIT(3)
/* Enable power voltage detector */
#define PWR_CR_PVDE                     BIT(4)

#define PWR_CR_PLS(value)               BIT_FIELD((value), 5)
#define PWR_CR_PLS_MASK                 BIT_FIELD(MASK(3), 0)
#define PWR_CR_PLS_VALUE(reg)           FIELD_VALUE((reg), PWR_CR_PLS_MASK, 5)

/* Disable backup domain write protection */
#define PWR_CR_DBP                      BIT(8)
/* Flash power down in Stop Mode */
#define PWR_CR_FPDS                     BIT(9)
/* Regulator voltage scaling output selection */
#define PWR_CR_VOS                      BIT(14)
/*------------------Power Control/Status Register-----------------------------*/
/* Wakeup flag */
#define PWR_CSR_WUF                     BIT(0)
/* Standby flag */
#define PWR_CSR_SBF                     BIT(1)
/* PVD output */
#define PWR_CSR_PVDO                    BIT(2)
/* Backup regulator ready */
#define PWR_CSR_BRR                     BIT(3)
/* Enable WKUP pin */
#define PWR_CSR_EWUP                    BIT(8)
/* Backup regulator enable */
#define PWR_CSR_BRE                     BIT(9)
/* VOS ready */
#define PWR_CSR_VOSRDY                  BIT(14)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F4XX_SYSTEM_DEFS_H_ */
