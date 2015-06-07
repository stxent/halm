/*
 * platform/nxp/gen_1/rtc_defs.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_GEN_1_RTC_DEFS_H_
#define PLATFORM_NXP_GEN_1_RTC_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <bits.h>
/*------------------Interrupt Location Register-------------------------------*/
#define ILR_RTCCIF                      BIT(0) /* Counter increment interrupt */
#define ILR_RTCALF                      BIT(1) /* Alarm interrupt */
/*------------------Clock Control Register------------------------------------*/
#define CCR_CLKEN                       BIT(0) /* Clock enable */
#define CCR_CTCRST                      BIT(1) /* CTC reset */
#define CCR_CCALEN                      BIT(4) /* Calibration counter enable */
/*------------------Counter Increment Interrupt Register----------------------*/
#define CIIR_MASK                       BIT_FIELD(MASK(8), 0)
#define CIIR_IMSEC                      BIT(0)
#define CIIR_IMMIN                      BIT(1)
#define CIIR_IMHOUR                     BIT(2)
#define CIIR_IMDOM                      BIT(3)
#define CIIR_IMDOW                      BIT(4)
#define CIIR_IMDOY                      BIT(5)
#define CIIR_IMMON                      BIT(6)
#define CIIR_IMYEAR                     BIT(7)
/*------------------Alarm Mask Register---------------------------------------*/
#define AMR_MASK                        BIT_FIELD(MASK(8), 0)
#define AMR_SEC                         BIT(0)
#define AMR_MIN                         BIT(1)
#define AMR_HOUR                        BIT(2)
#define AMR_DOM                         BIT(3)
#define AMR_DOW                         BIT(4)
#define AMR_DOY                         BIT(5)
#define AMR_MON                         BIT(6)
#define AMR_YEAR                        BIT(7)
/*------------------Consolidated Time register 0------------------------------*/
#define CTIME0_SECONDS_VALUE(reg) \
    FIELD_VALUE((reg), BIT_FIELD(MASK(6), 0), 0)
#define CTIME0_MINUTES_VALUE(reg) \
    FIELD_VALUE((reg), BIT_FIELD(MASK(6), 8), 8)
#define CTIME0_HOURS_VALUE(reg) \
    FIELD_VALUE((reg), BIT_FIELD(MASK(5), 16), 16)
#define CTIME0_DOW_VALUE(reg) \
    FIELD_VALUE((reg), BIT_FIELD(MASK(3), 24), 24)
/*------------------Consolidated Time register 1------------------------------*/
#define CTIME1_DOM_VALUE(reg) \
    FIELD_VALUE((reg), BIT_FIELD(MASK(5), 0), 0)
#define CTIME1_MONTH_VALUE(reg) \
    FIELD_VALUE((reg), BIT_FIELD(MASK(4), 8), 8)
#define CTIME1_YEAR_VALUE(reg) \
    FIELD_VALUE((reg), BIT_FIELD(MASK(12), 16), 16)
/*------------------Consolidated Time register 2------------------------------*/
#define CTIME2_DOY_VALUE(reg) \
    FIELD_VALUE((reg), BIT_FIELD(MASK(9), 0), 0)
/*------------------Calibration register--------------------------------------*/
#define CALIBRATION_VAL_MASK            BIT_FIELD(MASK(17), 0)
#define CALIBRATION_VAL(value)          BIT_FIELD((value), 0)
#define CALIBRATION_VAL_VALUE(reg) \
    FIELD_VALUE((reg), CALIBRATION_VAL_MASK, 0)
/* Set to 0 for forward calibration, to 1 for backward calibration */
#define CALIBRATION_DIR                 BIT(17)
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_GEN_1_RTC_DEFS_H_ */
