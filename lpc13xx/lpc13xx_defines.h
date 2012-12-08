/*
 * lpc13xx_defines.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef LPC13XX_DEFINES_H_
#define LPC13XX_DEFINES_H_
/*----------------------------------------------------------------------------*/
#define GET_PRIORITY(priority)          ((uint32_t)(15 - (priority)))
#define BIT(offset)                     ((uint32_t)(1 << (offset)))
#define BIT_FIELD(value, offset)        ((uint32_t)(value << (offset)))
/*----------------------------------------------------------------------------*/
/*------------------SYSAHBCLKCTRL register, reset value 0x0000485F------------*/
#define SYSAHBCLKCTRL_SYS               BIT(0)
#define SYSAHBCLKCTRL_ROM               BIT(1)
#define SYSAHBCLKCTRL_RAM               BIT(2)
#define SYSAHBCLKCTRL_FLASHREG          BIT(3)
#define SYSAHBCLKCTRL_FLASHARRAY        BIT(4)
#define SYSAHBCLKCTRL_I2C               BIT(5)
#define SYSAHBCLKCTRL_GPIO              BIT(6)
#define SYSAHBCLKCTRL_CT16B0            BIT(7)
#define SYSAHBCLKCTRL_CT16B1            BIT(8)
#define SYSAHBCLKCTRL_CT32B0            BIT(9)
#define SYSAHBCLKCTRL_CT32B1            BIT(10)
#define SYSAHBCLKCTRL_SSP               BIT(11)
#define SYSAHBCLKCTRL_UART              BIT(12)
#define SYSAHBCLKCTRL_ADC               BIT(13)
#define SYSAHBCLKCTRL_USB_REG           BIT(14)
#define SYSAHBCLKCTRL_WDT               BIT(15)
#define SYSAHBCLKCTRL_IOCON             BIT(16)
/*----------------------------------------------------------------------------*/
#endif /* LPC13XX_DEFINES_H_ */
