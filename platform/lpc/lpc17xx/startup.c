/*
 * startup.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/core/cortex/mpu.h>
#include <halm/platform/lpc/lpc17xx/system_defs.h>
#include <halm/platform/lpc/system.h>
#include <xcore/bits.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
void platformStartup(void)
{
  static const enum SysBlockPower blocksToDisable[] = {
      PWR_TIM0,
      PWR_TIM1,
      PWR_UART0,
      PWR_UART1,
      PWR_PWM1,
      PWR_I2C0,
      PWR_SPI,
      PWR_SSP1,
      PWR_I2C1,
      PWR_SSP0,
      PWR_I2C2
  };

  /*
   * PWR_RTC is left untouched. PWR_GPIO is enabled by default.
   * Other peripherals are disabled by default.
   */

  for (size_t index = 0; index < ARRAY_SIZE(blocksToDisable); ++index)
    sysPowerDisable(blocksToDisable[index]);

#ifdef CONFIG_CORE_CORTEX_MPU
  mpuEnable();
#endif
}
