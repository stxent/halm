/*
 * halm/platform/numicro/m03x/system.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * System configuration functions for Nuvoton M031/M032 chips.
 */

#ifndef HALM_PLATFORM_NUMICRO_SYSTEM_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_M03X_SYSTEM_H_
#define HALM_PLATFORM_NUMICRO_M03X_SYSTEM_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
#include <xcore/helpers.h>
#include <stdbool.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
/* Reset control for core and peripherals */
enum SysBlockReset
{
  RST_CHIP    = 0,
  RST_CPU     = 1,
  RST_PDMA    = 2,
  RST_EBI     = 3,
  RST_HDIV    = 4,
  RST_CRC     = 7,

  RST_GPIO    = 0x20 + 1,
  RST_TMR0    = 0x20 + 2,
  RST_TMR1    = 0x20 + 3,
  RST_TMR2    = 0x20 + 4,
  RST_TMR3    = 0x20 + 5,
  RST_ACMP01  = 0x20 + 7,
  RST_I2C0    = 0x20 + 8,
  RST_I2C1    = 0x20 + 9,
  RST_QSPI0   = 0x20 + 12,
  RST_SPI0    = 0x20 + 13,
  RST_UART0   = 0x20 + 16,
  RST_UART1   = 0x20 + 17,
  RST_UART2   = 0x20 + 18,
  RST_UART3   = 0x20 + 19,
  RST_UART4   = 0x20 + 20,
  RST_UART5   = 0x20 + 21,
  RST_UART6   = 0x20 + 22,
  RST_UART7   = 0x20 + 23,
  RST_USBD    = 0x20 + 27,
  RST_ADC     = 0x20 + 28,

  RST_USCI0   = 0x40 + 8,
  RST_USCI1   = 0x40 + 9,
  RST_PWM0    = 0x40 + 16,
  RST_PWM1    = 0x40 + 17,
  RST_BPWM0   = 0x40 + 18,
  RST_BPWM1   = 0x40 + 19
} __attribute__((packed));

/* Enable or disable clock branches */
enum SysClockBranch
{
  CLK_PDMA    = 1,
  CLK_ISP     = 2,
  CLK_EBI     = 3,
  CLK_HDIV    = 4,
  CLK_CRC     = 7,

  CLK_WDT     = 0x20 + 0,
  CLK_RTC     = 0x20 + 1,
  CLK_TMR0    = 0x20 + 2,
  CLK_TMR1    = 0x20 + 3,
  CLK_TMR2    = 0x20 + 4,
  CLK_TMR3    = 0x20 + 5,
  CLK_CLKO    = 0x20 + 6,
  CLK_ACMP01  = 0x20 + 7,
  CLK_I2C0    = 0x20 + 8,
  CLK_I2C1    = 0x20 + 9,
  CLK_QSPI0   = 0x20 + 12,
  CLK_SPI0    = 0x20 + 13,
  CLK_UART0   = 0x20 + 16,
  CLK_UART1   = 0x20 + 17,
  CLK_UART2   = 0x20 + 18,
  CLK_UART3   = 0x20 + 19,
  CLK_UART4   = 0x20 + 20,
  CLK_UART5   = 0x20 + 21,
  CLK_UART6   = 0x20 + 22,
  CLK_UART7   = 0x20 + 23,
  CLK_USBD    = 0x20 + 27,
  CLK_ADC     = 0x20 + 28,

  CLK_USCI0   = 0x40 + 8,
  CLK_USCI1   = 0x40 + 9,
  CLK_PWM0    = 0x40 + 16,
  CLK_PWM1    = 0x40 + 17,
  CLK_BPWM0   = 0x40 + 18,
  CLK_BPWM1   = 0x40 + 19
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void sysClockDisable(enum SysClockBranch);
void sysClockEnable(enum SysClockBranch);
bool sysClockStatus(enum SysClockBranch);
size_t sysGetSizeAPROM(void);
void sysFlashLatencyReset(void);
unsigned int sysFlashLatencyUpdate(uint32_t);
void sysResetBlock(enum SysBlockReset);

void sysLockReg(void);
void sysUnlockReg(void);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M03X_SYSTEM_H_ */
