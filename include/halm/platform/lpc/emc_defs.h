/*
 * halm/platform/lpc/emc_defs.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_EMC_DEFS_H_
#define HALM_PLATFORM_LPC_EMC_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#define EMC_PIN_CHANNEL_DEFAULT         0
#define EMC_PIN_CHANNEL_FEEDBACK        1
/*------------------Control register------------------------------------------*/
#define CONTROL_E                       BIT(0) /* EMC Enable */
#define CONTROL_M                       BIT(1) /* Address mirror */
#define CONTROL_L                       BIT(2) /* Low-power mode */
/*------------------Status register-------------------------------------------*/
#define STATUS_B                        BIT(0) /* Busy indicator */
#define STATUS_S                        BIT(1) /* Write buffer status */
#define STATUS_SA                       BIT(2) /* Self-refresh acknowledge */
/*------------------Configuration register------------------------------------*/
#define CONFIG_EM                       BIT(0) /* Endian mode */
/*------------------Dynamic Memory Control register---------------------------*/
enum
{
  SDRAM_INITIALIZATION_NORMAL = 0,
  SDRAM_INITIALIZATION_MODE   = 1,
  SDRAM_INITIALIZATION_PALL   = 2,
  SDRAM_INITIALIZATION_NOP    = 3
};

/* Dynamic memory clock enable */
#define DYNAMICCONTROL_CE               BIT(0)
/* Dynamic memory clock control */
#define DYNAMICCONTROL_CS               BIT(1)
/* Self-refresh request */
#define DYNAMICCONTROL_SR               BIT(2)
/* Memory clock control */
#define DYNAMICCONTROL_MMC              BIT(5)

#define DYNAMICCONTROL_I(value)         BIT_FIELD((value), 7)
#define DYNAMICCONTROL_I_MASK           BIT_FIELD(MASK(2), 7)
#define DYNAMICCONTROL_I_VALUE(reg) \
    FIELD_VALUE((reg), DYNAMICCONTROL_I_MASK, 7)
/*------------------Dynamic Memory Read Configuration registers---------------*/
enum
{
  DATA_STRATEGY_UNUSED      = 0,
  DATA_STRATEGY_0_5_CLOCKS  = 1,
  DATA_STRATEGY_1_5_CLOCKS  = 2,
  DATA_STRATEGY_2_5_CLOCKS  = 3
};

#define DYNAMICREADCONFIG_RD(value)     BIT_FIELD((value), 0)
#define DYNAMICREADCONFIG_RD_MASK       BIT_FIELD(MASK(2), 0)
#define DYNAMICREADCONFIG_RD_VALUE(reg) \
    FIELD_VALUE((reg), DYNAMICREADCONFIG_RD_MASK, 0)
/*------------------Dynamic memory Configuration registers--------------------*/
enum
{
  MD_SDRAM = 0
};

#define DYNAMICCONFIG_MD(value)         BIT_FIELD((value), 3)
#define DYNAMICCONFIG_MD_MASK           BIT_FIELD(MASK(2), 3)
#define DYNAMICCONFIG_MD_VALUE(reg) \
    FIELD_VALUE((reg), DYNAMICCONFIG_MD_MASK, 3)

#define DYNAMICCONFIG_AM0(value)        BIT_FIELD((value), 7)
#define DYNAMICCONFIG_AM0_MASK          BIT_FIELD(MASK(6), 7)
#define DYNAMICCONFIG_AM0_VALUE(reg) \
    FIELD_VALUE((reg), DYNAMICCONFIG_AM0_MASK, 7)

#define DYNAMICCONFIG_AM1               BIT(14)
#define DYNAMICCONFIG_B                 BIT(19)
#define DYNAMICCONFIG_P                 BIT(20)
/*------------------Dynamic Memory RAS and CAS Delay registers----------------*/
enum
{
  LATENCY_1_CYCLE   = 1,
  LATENCY_2_CYCLES  = 2,
  LATENCY_3_CYCLES  = 3
};

#define DYNAMICRASCAS_RAS(value)        BIT_FIELD((value), 0)
#define DYNAMICRASCAS_RAS_MASK          BIT_FIELD(MASK(2), 0)
#define DYNAMICRASCAS_RAS_VALUE(reg) \
    FIELD_VALUE((reg), DYNAMICRASCAS_RAS_MASK, 0)

#define DYNAMICRASCAS_CAS(value)        BIT_FIELD((value), 8)
#define DYNAMICRASCAS_CAS_MASK          BIT_FIELD(MASK(2), 8)
#define DYNAMICRASCAS_CAS_VALUE(reg) \
    FIELD_VALUE((reg), DYNAMICRASCAS_CAS_MASK, 8)
/*------------------Static Memory Configuration registers---------------------*/
enum
{
  MW_8_BIT  = 0,
  MW_16_BIT = 1,
  MW_32_BIT = 2
};

#define STATICCONFIG_MW(value)          BIT_FIELD((value), 0)
#define STATICCONFIG_MW_MASK            BIT_FIELD(MASK(2), 0)
#define STATICCONFIG_MW_VALUE(reg) \
    FIELD_VALUE((reg), STATICCONFIG_MW_MASK, 0)

#define STATICCONFIG_PM                 BIT(3) /* Page mode */
#define STATICCONFIG_PC                 BIT(6) /* Chip select polarity */
#define STATICCONFIG_PB                 BIT(7) /* Byte lane state */
#define STATICCONFIG_EW                 BIT(8) /* Extended wait */
#define STATICCONFIG_B                  BIT(19) /* Buffer enable */
#define STATICCONFIG_P                  BIT(20) /* Write protect */
/*------------------Timing registers------------------------------------------*/
#define DYNAMICRP_MAX                   15 /* N + 1 cycles */
#define DYNAMICRAS_MAX                  15 /* N + 1 cycles */
#define DYNAMICSREX_MAX                 15 /* N + 1 cycles */
#define DYNAMICAPR_MAX                  15 /* N + 1 cycles */
#define DYNAMICDAL_MAX                  15 /* N cycles */
#define DYNAMICWR_MAX                   15 /* N + 1 cycles */
#define DYNAMICRC_MAX                   31 /* N + 1 cycles */
#define DYNAMICRFC_MAX                  31 /* N + 1 cycles */
#define DYNAMICXSR_MAX                  31 /* N + 1 cycles */
#define DYNAMICRRD_MAX                  15 /* N + 1 cycles */
#define DYNAMICMRD_MAX                  15 /* N + 1 cycles */
#define DYNAMICREFRESH_MAX              2047 /* N cycles */

#define STATICEXTENDEDWAIT_MAX          1023 /* N + 1 cycles */
#define STATICWAITWEN_MAX               15 /* N + 1 cycles */
#define STATICWAITOEN_MAX               15 /* N + 1 cycles */
#define STATICWAITRD_MAX                31 /* N + 1 cycles */
#define STATICWAITPAGE_MAX              31 /* N + 1 cycles */
#define STATICWAITWR_MAX                31 /* N + 2 cycles */
#define STATICWAITTURN_MAX              15 /* N + 1 cycles */
/*------------------SDRAM Mode register---------------------------------------*/
enum
{
  MODE_BURST_1    = 0,
  MODE_BURST_2    = 1,
  MODE_BURST_4    = 2,
  MODE_BURST_8    = 3,
  MODE_BURST_PAGE = 7
};

enum
{
  MODE_LATENCY_1  = 1,
  MODE_LATENCY_2  = 2,
  MODE_LATENCY_3  = 3
};

enum
{
  MODE_OPERATION_STANDARD = 0
};

#define SDRAM_MODE_BURST_LENGTH(value)  BIT_FIELD((value), 0)
#define SDRAM_MODE_BURST_LENGTH_MASK    BIT_FIELD(MASK(3), 0)
#define SDRAM_MODE_BURST_LENGTH_VALUE(reg) \
    FIELD_VALUE((reg), SDRAM_MODE_BURST_LENGTH_MASK, 0)

#define SDRAM_MODE_TYPE_SEQUENTIAL      0
#define SDRAM_MODE_TYPE_INTERLEAVED     1

#define SDRAM_MODE_LATENCY(value)       BIT_FIELD((value), 4)
#define SDRAM_MODE_LATENCY_MASK         BIT_FIELD(MASK(3), 4)
#define SDRAM_MODE_LATENCY_VALUE(reg) \
    FIELD_VALUE((reg), SDRAM_MODE_LATENCY_MASK, 4)

#define SDRAM_MODE_OPERATION(value)     BIT_FIELD((value), 7)
#define SDRAM_MODE_OPERATION_MASK       BIT_FIELD(MASK(2), 7)
#define SDRAM_MODE_OPERATION_VALUE(reg) \
    FIELD_VALUE((reg), SDRAM_MODE_OPERATION_MASK, 7)

#define SDRAM_MODE_WRITE_DEFAULT        0
#define SDRAM_MODE_WRITE_SINGLE         1
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_EMC_DEFS_H_ */
