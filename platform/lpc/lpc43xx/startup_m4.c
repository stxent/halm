/*
 * startup_m4.c
 * Copyright (C) 2015, 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#ifdef CONFIG_CORE_CORTEX_FPU
#include <halm/core/cortex/fpu.h>
#endif

#include <halm/platform/lpc/lpc43xx/system_defs.h>
#include <halm/platform/lpc/system.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
void platformStartup(void)
{
  static const enum SysClockBranch clocksToDisable[] = {
      CLK_APB3_I2C1,
      CLK_APB3_DAC,
      CLK_APB3_ADC0,
      CLK_APB3_ADC1,
      CLK_APB3_CAN0,
      CLK_APB1_MCPWM,
      CLK_APB1_I2C0,
      CLK_APB1_I2S,
      CLK_APB1_CAN1,
      CLK_M4_SPIFI,
      CLK_M4_LCD,
      CLK_M4_ETHERNET,
      CLK_M4_USB0,
      CLK_M4_EMC,
      CLK_M4_SDIO,
      CLK_M4_GPDMA,
      CLK_M4_SCT,
      CLK_M4_USB1,
      CLK_M4_EMCDIV,
      CLK_M4_M0APP,
      CLK_M4_ADCHS,
      CLK_M4_EEPROM,
      CLK_M4_WWDT,
      CLK_M4_USART0,
      CLK_M4_UART1,
      CLK_M4_SSP0,
      CLK_M4_TIMER0,
      CLK_M4_TIMER1,
      CLK_M4_RIT,
      CLK_M4_USART2,
      CLK_M4_USART3,
      CLK_M4_TIMER2,
      CLK_M4_TIMER3,
      CLK_M4_SSP1,
      CLK_M4_QEI,
      CLK_APB2_USART3,
      CLK_APB2_USART2,
      CLK_APB0_UART1,
      CLK_APB0_USART0,
      CLK_APB2_SSP1,
      CLK_APB0_SSP0,
      CLK_SPIFI,
      CLK_SPI,
      CLK_ADCHS,
      CLK_AUDIO,
      CLK_SDIO,
      CLK_PERIPH_SGPIO
  };

  /*
   * CLK_M4_BUS, CLK_M4_GPIO, CLK_M4_M4CORE, CLK_M4_FLASHA, CLK_M4_FLASHB,
   * CLK_M4_SCU, CLK_M4_CREG are enabled by default.
   * CLK_USB0, CLK_USB1, CLK_PERIPH_BUS, CLK_PERIPH_CORE are left untouched.
   */

  for (size_t index = 0; index < ARRAY_SIZE(clocksToDisable); ++index)
    sysClockDisable(clocksToDisable[index]);

#ifdef CONFIG_CORE_CORTEX_FPU
  fpuEnable();
#endif

  /* Reset SCU and GPIO blocks */
  sysResetEnable(RST_SCU);
  sysResetEnable(RST_GPIO);
}
