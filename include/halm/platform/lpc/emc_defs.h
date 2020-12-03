/*
 * halm/platform/lpc/emc_defs.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_EMC_DEFS_H_
#define HALM_PLATFORM_LPC_EMC_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
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
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_EMC_DEFS_H_ */
