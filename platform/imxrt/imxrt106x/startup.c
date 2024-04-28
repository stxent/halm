/*
 * startup.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/core/cortex/cache.h>
#include <halm/core/cortex/fpu.h>
#include <halm/core/cortex/mpu.h>
#include <halm/core/cortex/nvic.h>
#include <halm/platform/imxrt/system.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
extern unsigned long _stext;
/*----------------------------------------------------------------------------*/
void platformStartup(void)
{
  /* CLK_XBAR1, CLK_XBAR2, CLK_SEMC are disabled by default */
  static const enum SysClockBranch CLOCKS_TO_DISABLE[] = {
      CLK_MQS,
      CLK_DCP,
      CLK_LPUART3,
      CLK_CAN1,
      CLK_CAN1_SERIAL,
      CLK_CAN2,
      CLK_CAN2_SERIAL,
      CLK_GPT2,
      CLK_GPT2_SERIAL,
      CLK_LPUART2,
      CLK_GPIO2,
      CLK_LPSPI1,
      CLK_LPSPI2,
      CLK_LPSPI3,
      CLK_LPSPI4,
      CLK_ADC2,
      CLK_ENET1,
      CLK_PIT,
      CLK_AOI2,
      CLK_ADC1,
      CLK_GPT1,
      CLK_GPT1_SERIAL,
      CLK_LPUART4,
      CLK_GPIO1,
      CLK_GPIO5,
      CLK_CSI,
      CLK_LPI2C1,
      CLK_LPI2C2,
      CLK_LPI2C3,
      CLK_XBAR3,
      CLK_GPIO3,
      CLK_LCD,
      CLK_PXP,
      CLK_FLEXIO2,
      CLK_LPUART5,
      CLK_LPUART6,
      CLK_AOI1,
      CLK_LCDIF_PIX,
      CLK_GPIO4,
      CLK_EWM,
      CLK_ACMP1,
      CLK_ACMP2,
      CLK_ACMP3,
      CLK_ACMP4,
      CLK_TSC,
      CLK_PWM1,
      CLK_PWM2,
      CLK_PWM3,
      CLK_PWM4,
      CLK_ENC1,
      CLK_ENC2,
      CLK_ENC3,
      CLK_ENC4,
      CLK_ROM,
      CLK_FLEXIO1,
      CLK_DMA,
      CLK_KPP,
      CLK_SPDIF,
      CLK_SAI1,
      CLK_SAI2,
      CLK_SAI3,
      CLK_LPUART1,
      CLK_LPUART7,
      CLK_USBOH3,
      CLK_USBHC1,
      CLK_USBHC2,
      CLK_TRNG,
      CLK_LPUART8,
      CLK_TIMER4,
      CLK_LPI2C4,
      CLK_TIMER1,
      CLK_TIMER2,
      CLK_TIMER3,
      CLK_ENET2,
      CLK_CAN3,
      CLK_CAN3_SERIAL,
      CLK_FLEXIO3
  };
  static const enum SysClockBranch CLOCKS_TO_ENABLE[] = {
      CLK_IOMUXC_SNVS_GPR,
      CLK_IOMUXC_SNVS,
      CLK_IOMUXC,
      CLK_IOMUXC_GPR
  };

  for (size_t index = 0; index < ARRAY_SIZE(CLOCKS_TO_DISABLE); ++index)
    sysClockDisable(CLOCKS_TO_DISABLE[index]);
  for (size_t index = 0; index < ARRAY_SIZE(CLOCKS_TO_ENABLE); ++index)
    sysClockEnable(CLOCKS_TO_ENABLE[index]);

#ifdef CONFIG_CORE_CORTEX_FPU
  fpuEnable();
#endif

#ifdef CONFIG_CORE_CORTEX_MPU
  mpuEnable();
#endif

  iCacheEnable();
  dCacheEnable();

  nvicSetVectorTableOffset((uint32_t)&_stext);
}
