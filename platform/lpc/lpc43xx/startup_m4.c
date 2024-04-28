/*
 * startup_m4.c
 * Copyright (C) 2015, 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/core/cortex/fpu.h>
#include <halm/core/cortex/mpu.h>
#include <halm/platform/lpc/lpc43xx/system_defs.h>
#include <halm/platform/lpc/system.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
void platformStartup(void)
{
  static const enum SysClockBranch CLOCKS_TO_DISABLE[] = {
      CLK_APB3_I2C1,
      CLK_APB3_DAC,
      CLK_APB3_ADC0,
      CLK_APB3_ADC1,
      CLK_APB3_CAN0,
      CLK_APB1_MCPWM,
      CLK_APB1_I2C0,
      CLK_APB1_I2S,
      CLK_APB1_CAN1,
      CLK_M4_LCD,
      CLK_M4_ETHERNET,
      CLK_M4_USB0,
      CLK_M4_SDIO,
      CLK_M4_GPDMA,
      CLK_M4_SCT,
      CLK_M4_USB1,
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
      CLK_SPI,
      CLK_ADCHS,
      CLK_AUDIO,
      CLK_SDIO,
      CLK_PERIPH_SGPIO
  };

  /*
   * CLK_M4_BUS, CLK_M4_GPIO, CLK_M4_M4CORE, CLK_M4_FLASHA, CLK_M4_FLASHB,
   * CLK_M4_SCU, CLK_M4_CREG are enabled by default.
   *
   * CLK_M4_M0APP, CLK_USB0, CLK_USB1, CLK_PERIPH_BUS, CLK_PERIPH_CORE
   * are left untouched.
   *
   * Clock branches which can be used to boot from memory-mapped regions,
   * are also left untouched:
   *   - CLK_M4_EMC and CLK_M4_EMCDIV are used by EMC boot.
   *   - CLK_M4_SPIFI and CLK_SPIFI are used by SPIFI boot sequence.
   */

  for (size_t index = 0; index < ARRAY_SIZE(CLOCKS_TO_DISABLE); ++index)
    sysClockDisable(CLOCKS_TO_DISABLE[index]);

#ifdef CONFIG_CORE_CORTEX_FPU
  fpuEnable();
#endif

#ifdef CONFIG_CORE_CORTEX_MPU
  mpuEnable();
#endif
}
