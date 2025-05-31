/*
 * halm/platform/numicro/m48x/system.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * System configuration functions for Nuvoton M48x chips.
 */

#ifndef HALM_PLATFORM_NUMICRO_SYSTEM_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_M48X_SYSTEM_H_
#define HALM_PLATFORM_NUMICRO_M48X_SYSTEM_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
#include <xcore/helpers.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
/* Reset control for core and peripherals */
enum [[gnu::packed]] SysBlockReset
{
  RST_CHIP    = 0,
  RST_CPU     = 1,
  RST_PDMA    = 2,
  RST_EBI     = 3,
  RST_EMAC    = 5,
  RST_SDH0    = 6,
  RST_CRC     = 7,
  RST_CCAP    = 8,
  RST_HSUSBD  = 10,
  RST_CRPT    = 12,
  RST_SPIM    = 14,
  RST_HSUSBH  = 16,
  RST_SDH1    = 17,

  RST_GPIO    = 0x20 + 1,
  RST_TMR0    = 0x20 + 2,
  RST_TMR1    = 0x20 + 3,
  RST_TMR2    = 0x20 + 4,
  RST_TMR3    = 0x20 + 5,
  RST_ACMP01  = 0x20 + 7,
  RST_I2C0    = 0x20 + 8,
  RST_I2C1    = 0x20 + 9,
  RST_I2C2    = 0x20 + 10,
  RST_QSPI0   = 0x20 + 12,
  RST_SPI0    = 0x20 + 13,
  RST_SPI1    = 0x20 + 14,
  RST_SPI2    = 0x20 + 15,
  RST_UART0   = 0x20 + 16,
  RST_UART1   = 0x20 + 17,
  RST_UART2   = 0x20 + 18,
  RST_UART3   = 0x20 + 19,
  RST_UART4   = 0x20 + 20,
  RST_UART5   = 0x20 + 21,
  RST_UART6   = 0x20 + 22,
  RST_UART7   = 0x20 + 23,
  RST_CAN0    = 0x20 + 24,
  RST_CAN1    = 0x20 + 25,
  RST_OTG     = 0x20 + 26,
  RST_USBD    = 0x20 + 27,
  RST_EADC0   = 0x20 + 28,
  RST_I2S0    = 0x20 + 29,
  RST_HSOTG   = 0x20 + 30,
  RST_TRNG    = 0x20 + 31,

  RST_SC0     = 0x40 + 0,
  RST_SC1     = 0x40 + 1,
  RST_SC2     = 0x40 + 2,
  RST_QSPI1   = 0x40 + 4,
  RST_SPI3    = 0x40 + 6,
  RST_USCI0   = 0x40 + 8,
  RST_USCI1   = 0x40 + 9,
  RST_DAC     = 0x40 + 12,
  RST_EPWM0   = 0x40 + 16,
  RST_EPWM1   = 0x40 + 17,
  RST_BPWM0   = 0x40 + 18,
  RST_BPWM1   = 0x40 + 19,
  RST_QEI0    = 0x40 + 22,
  RST_QEI1    = 0x40 + 23,
  RST_ECAP0   = 0x40 + 26,
  RST_ECAP1   = 0x40 + 27,
  RST_CAN2    = 0x40 + 28,
  RST_OPA     = 0x40 + 30,
  RST_EADC1   = 0x40 + 31
};

/* Enable or disable clock branches */
enum [[gnu::packed]] SysClockBranch
{
  CLK_PDMA    = 1,
  CLK_ISP     = 2,
  CLK_EBI     = 3,
  CLK_EMAC    = 5,
  CLK_SDH0    = 6,
  CLK_CRC     = 7,
  CLK_CCAP    = 8,
  CLK_CCAPSEN = 9,
  CLK_HSUSBD  = 10,
  CLK_CRPT    = 12,
  CLK_SPIM    = 14,
  CLK_FMCIDLE = 15,
  CLK_HSUSBH  = 16,
  CLK_SDH1    = 17,

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
  CLK_I2C2    = 0x20 + 10,
  CLK_QSPI0   = 0x20 + 12,
  CLK_SPI0    = 0x20 + 13,
  CLK_SPI1    = 0x20 + 14,
  CLK_SPI2    = 0x20 + 15,
  CLK_UART0   = 0x20 + 16,
  CLK_UART1   = 0x20 + 17,
  CLK_UART2   = 0x20 + 18,
  CLK_UART3   = 0x20 + 19,
  CLK_UART4   = 0x20 + 20,
  CLK_UART5   = 0x20 + 21,
  CLK_UART6   = 0x20 + 22,
  CLK_UART7   = 0x20 + 23,
  CLK_CAN0    = 0x20 + 24,
  CLK_CAN1    = 0x20 + 25,
  CLK_OTG     = 0x20 + 26,
  CLK_USBD    = 0x20 + 27,
  CLK_EADC0   = 0x20 + 28,
  CLK_I2S0    = 0x20 + 29,
  CLK_HSOTG   = 0x20 + 30,

  CLK_SC0     = 0x40 + 0,
  CLK_SC1     = 0x40 + 1,
  CLK_SC2     = 0x40 + 2,
  CLK_QSPI1   = 0x40 + 4,
  CLK_SPI3    = 0x40 + 6,
  CLK_USCI0   = 0x40 + 8,
  CLK_USCI1   = 0x40 + 9,
  CLK_DAC     = 0x40 + 12,
  CLK_EPWM0   = 0x40 + 16,
  CLK_EPWM1   = 0x40 + 17,
  CLK_BPWM0   = 0x40 + 18,
  CLK_BPWM1   = 0x40 + 19,
  CLK_QEI0    = 0x40 + 22,
  CLK_QEI1    = 0x40 + 23,
  CLK_TRNG    = 0x40 + 25,
  CLK_ECAP0   = 0x40 + 26,
  CLK_ECAP1   = 0x40 + 27,
  CLK_CAN2    = 0x40 + 28,
  CLK_OPA     = 0x40 + 30,
  CLK_EADC1   = 0x40 + 31
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void sysClockDisable(enum SysClockBranch);
void sysClockEnable(enum SysClockBranch);
bool sysClockStatus(enum SysClockBranch);
void sysFlashLatencyReset(void);
void sysFlashLatencyUpdate(uint32_t);
size_t sysGetSizeAPROM(void);
void sysPowerLevelReset(void);
void sysPowerLevelUpdate(uint32_t);
void sysResetBlock(enum SysBlockReset);

void sysLockReg(void);
void sysUnlockReg(void);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M48X_SYSTEM_H_ */
