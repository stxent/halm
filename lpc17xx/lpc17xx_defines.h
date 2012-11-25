/*
 * lpc17xx_defines.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef LPC17XX_DEFINES_H_
#define LPC17XX_DEFINES_H_
/*----------------------------------------------------------------------------*/
#define GET_PRIORITY(i)                 (15 - (i))
#define BIT(i)                          ((uint32_t)1 << (i))
/*----------------------------------------------------------------------------*/
/*------------------PCONP register, reset value 0x042887DE--------------------*/
#define PCONP_PCTIM0                    BIT(1)
#define PCONP_PCTIM1                    BIT(2)
#define PCONP_PCUART0                   BIT(3)
#define PCONP_PCUART1                   BIT(4)
#define PCONP_PCPWM1                    BIT(6)
#define PCONP_PCI2C0                    BIT(7)
#define PCONP_PCSPI                     BIT(8)
#define PCONP_PCRTC                     BIT(9)
#define PCONP_PCSSP1                    BIT(10)
#define PCONP_PCADC                     BIT(12)
#define PCONP_PCCAN1                    BIT(13)
#define PCONP_PCCAN2                    BIT(14)
#define PCONP_PCGPIO                    BIT(15)
#define PCONP_PCRIT                     BIT(16)
#define PCONP_PCMCPWM                   BIT(17)
#define PCONP_PCQEI                     BIT(18)
#define PCONP_PCI2C1                    BIT(19)
#define PCONP_PCSSP0                    BIT(21)
#define PCONP_PCTIM2                    BIT(22)
#define PCONP_PCTIM3                    BIT(23)
#define PCONP_PCUART2                   BIT(24)
#define PCONP_PCUART3                   BIT(25)
#define PCONP_PCI2C2                    BIT(26)
#define PCONP_PCI2S                     BIT(27)
#define PCONP_PCGPDMA                   BIT(29)
#define PCONP_PCENET                    BIT(30)
#define PCONP_PCUSB                     BIT(31)
/*----------------------------------------------------------------------------*/
/*------------------Divider values for PCLKSEL registers----------------------*/
#define PCLK_DIV1                       1
#define PCLK_DIV2                       2
#define PCLK_DIV4                       0
#define PCLK_DIV8                       3
/* Exception for CAN1, CAN2 and CAN filtering */
#define PCLK_DIV6                       3
/*------------------Offsets for PCLKSEL registers, reset values 0x00000000----*/
enum sysPeriphClock {
/* PCLKSEL0 register */
  PCLK_WDT      = 0,
  PCLK_TIMER0   = 2,
  PCLK_TIMER1   = 4,
  PCLK_UART0    = 6,
  PCLK_UART1    = 8,
  PCLK_PWM1     = 12,
  PCLK_I2C0     = 14,
  PCLK_SPI      = 16,
  PCLK_SSP1     = 20,
  PCLK_DAC      = 22,
  PCLK_ADC      = 24,
  PCLK_CAN1     = 26,
  PCLK_CAN2     = 28,
  PCLK_ACF      = 30,
  /* PCLKSEL1 */
  PCLK_QEI      = 0x20 + 0,
  PCLK_GPIOINT  = 0x20 + 2,
  PCLK_PCB      = 0x20 + 4,
  PCLK_I2C1     = 0x20 + 6,
  PCLK_SSP0     = 0x20 + 10,
  PCLK_TIMER2   = 0x20 + 12,
  PCLK_TIMER3   = 0x20 + 14,
  PCLK_UART2    = 0x20 + 16,
  PCLK_UART3    = 0x20 + 18,
  PCLK_I2C2     = 0x20 + 20,
  PCLK_I2S      = 0x20 + 22,
  PCLK_RIT      = 0x20 + 26,
  PCLK_SYSCON   = 0x20 + 28,
  PCLK_MC       = 0x20 + 30
};
/*----------------------------------------------------------------------------*/
#endif /* LPC17XX_DEFINES_H_ */
