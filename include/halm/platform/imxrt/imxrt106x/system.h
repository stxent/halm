/*
 * halm/platform/imxrt/imxrt106x/system.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * System configuration functions for i.MX RT106x chips.
 */

#ifndef HALM_PLATFORM_IMXRT_SYSTEM_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_IMXRT_IMXRT106X_SYSTEM_H_
#define HALM_PLATFORM_IMXRT_IMXRT106X_SYSTEM_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
/* Enable or disable clock branches */
enum [[gnu::packed]] SysClockBranch
{
  /* CCGR0 */
  CLK_AIPS_TZ1        = 0,
  CLK_AIPS_TZ2        = 1,
  CLK_MQS             = 2,
  CLK_SIM_M_MAIN      = 4,
  CLK_DCP             = 5,
  CLK_LPUART3         = 6,
  CLK_CAN1            = 7,
  CLK_CAN1_SERIAL     = 8,
  CLK_CAN2            = 9,
  CLK_CAN2_SERIAL     = 10,
  CLK_TRACE           = 11,
  CLK_GPT2            = 12,
  CLK_GPT2_SERIAL     = 13,
  CLK_LPUART2         = 14,
  CLK_GPIO2           = 15,

  /* CCGR1 */
  CLK_LPSPI1          = 0x10 + 0,
  CLK_LPSPI2          = 0x10 + 1,
  CLK_LPSPI3          = 0x10 + 2,
  CLK_LPSPI4          = 0x10 + 3,
  CLK_ADC2            = 0x10 + 4,
  CLK_ENET1           = 0x10 + 5,
  CLK_PIT             = 0x10 + 6,
  CLK_AOI2            = 0x10 + 7,
  CLK_ADC1            = 0x10 + 8,
  CLK_SEMC_EXSC       = 0x10 + 9,
  CLK_GPT1            = 0x10 + 10,
  CLK_GPT1_SERIAL     = 0x10 + 11,
  CLK_LPUART4         = 0x10 + 12,
  CLK_GPIO1           = 0x10 + 13,
  CLK_CSU             = 0x10 + 14,
  CLK_GPIO5           = 0x10 + 15,

  /* CCGR2 */
  CLK_OCRAM_EXSC      = 0x20 + 0,
  CLK_CSI             = 0x20 + 1,
  CLK_IOMUXC_SNVS     = 0x20 + 2,
  CLK_LPI2C1          = 0x20 + 3,
  CLK_LPI2C2          = 0x20 + 4,
  CLK_LPI2C3          = 0x20 + 5,
  CLK_OCOTP_CTRL      = 0x20 + 6,
  CLK_XBAR3           = 0x20 + 7,
  CLK_IPMUX1          = 0x20 + 8,
  CLK_IPMUX2          = 0x20 + 9,
  CLK_IPMUX3          = 0x20 + 10,
  CLK_XBAR1           = 0x20 + 11,
  CLK_XBAR2           = 0x20 + 12,
  CLK_GPIO3           = 0x20 + 13,
  CLK_LCD             = 0x20 + 14,
  CLK_PXP             = 0x20 + 15,

  /* CCGR3 */
  CLK_FLEXIO2         = 0x30 + 0,
  CLK_LPUART5         = 0x30 + 1,
  CLK_SEMC            = 0x30 + 2,
  CLK_LPUART6         = 0x30 + 3,
  CLK_AOI1            = 0x30 + 4,
  CLK_LCDIF_PIX       = 0x30 + 5,
  CLK_GPIO4           = 0x30 + 6,
  CLK_EWM             = 0x30 + 7,
  CLK_WDOG1           = 0x30 + 8,
  CLK_FLEXRAM         = 0x30 + 9,
  CLK_ACMP1           = 0x30 + 10,
  CLK_ACMP2           = 0x30 + 11,
  CLK_ACMP3           = 0x30 + 12,
  CLK_ACMP4           = 0x30 + 13,
  CLK_OCRAM           = 0x30 + 14,
  CLK_IOMUXC_SNVS_GPR = 0x30 + 15,

  /* CCGR4 */
  CLK_SIM_M7_MAIN     = 0x40 + 0,
  CLK_IOMUXC          = 0x40 + 1,
  CLK_IOMUXC_GPR      = 0x40 + 2,
  CLK_BEE             = 0x40 + 3,
  CLK_SIM_M7          = 0x40 + 4,
  CLK_TSC             = 0x40 + 5,
  CLK_SIM_M           = 0x40 + 6,
  CLK_SIM_EMS         = 0x40 + 7,
  CLK_PWM1            = 0x40 + 8,
  CLK_PWM2            = 0x40 + 9,
  CLK_PWM3            = 0x40 + 10,
  CLK_PWM4            = 0x40 + 11,
  CLK_ENC1            = 0x40 + 12,
  CLK_ENC2            = 0x40 + 13,
  CLK_ENC3            = 0x40 + 14,
  CLK_ENC4            = 0x40 + 15,

  /* CCGR5 */
  CLK_ROM             = 0x50 + 0,
  CLK_FLEXIO1         = 0x50 + 1,
  CLK_WDOG3           = 0x50 + 2,
  CLK_DMA             = 0x50 + 3,
  CLK_KPP             = 0x50 + 4,
  CLK_WDOG2           = 0x50 + 5,
  CLK_AIPS_TZ4        = 0x50 + 6,
  CLK_SPDIF           = 0x50 + 7,
  CLK_SIM_MAIN        = 0x50 + 8,
  CLK_SAI1            = 0x50 + 9,
  CLK_SAI2            = 0x50 + 10,
  CLK_SAI3            = 0x50 + 11,
  CLK_LPUART1         = 0x50 + 12,
  CLK_LPUART7         = 0x50 + 13,
  CLK_SNVS_HP         = 0x50 + 14,
  CLK_SNVS_LP         = 0x50 + 15,

  /* CCGR6 */
  CLK_USBOH3          = 0x60 + 0,
  CLK_USBHC1          = 0x60 + 1,
  CLK_USBHC2          = 0x60 + 2,
  CLK_DCDC            = 0x60 + 3,
  CLK_IPMUX4          = 0x60 + 4,
  CLK_FLEXSPI1        = 0x60 + 5,
  CLK_TRNG            = 0x60 + 6,
  CLK_LPUART8         = 0x60 + 7,
  CLK_TIMER4          = 0x60 + 8,
  CLK_AIPS_TZ3        = 0x60 + 9,
  CLK_SIM_AXBS_P      = 0x60 + 10,
  CLK_ANADIG          = 0x60 + 11,
  CLK_LPI2C4          = 0x60 + 12,
  CLK_TIMER1          = 0x60 + 13,
  CLK_TIMER2          = 0x60 + 14,
  CLK_TIMER3          = 0x60 + 15,

  /* CCGR7 */
  CLK_ENET2           = 0x70 + 0,
  CLK_FLEXSPI2        = 0x70 + 1,
  CLK_AXBS_L          = 0x70 + 2,
  CLK_CAN3            = 0x70 + 3,
  CLK_CAN3_SERIAL     = 0x70 + 4,
  CLK_AIPS_LITE       = 0x70 + 5,
  CLK_FLEXIO3         = 0x70 + 6
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void sysClockDisable(enum SysClockBranch);
void sysClockEnable(enum SysClockBranch);
bool sysClockStatus(enum SysClockBranch);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_IMXRT_IMXRT106X_SYSTEM_H_ */
