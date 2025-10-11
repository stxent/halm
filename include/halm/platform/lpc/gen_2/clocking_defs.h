/*
 * halm/platform/lpc/gen_2/clocking_defs.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GEN_2_CLOCKING_DEFS_H_
#define HALM_PLATFORM_LPC_GEN_2_CLOCKING_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#define INT_OSC_FREQUENCY 12000000
#define LP_OSC_FREQUENCY  10000
#define USB_PLL_FREQUENCY 48000000

#define TICK_RATE(frequency, latency) \
    (((frequency) / (latency) * 4398046ULL + (1ULL << 31)) >> 32)
/*----------------------------------------------------------------------------*/
struct ClockDescriptor
{
  volatile uint32_t sourceSelect;
  volatile uint32_t sourceUpdate;
  volatile uint32_t divider;
};
/*------------------Clock Source Update registers-----------------------------*/
#define CLKUEN_ENA                      BIT(0)
/*------------------Clock Output Source Select register-----------------------*/
#define CLKOUTCLKSEL_IRC                BIT_FIELD(0, 0)
#define CLKOUTCLKSEL_SYSOSC             BIT_FIELD(1, 0)
#define CLKOUTCLKSEL_WDT                BIT_FIELD(2, 0)
#define CLKOUTCLKSEL_MAIN_CLOCK         BIT_FIELD(3, 0)
/*------------------Main Clock Source Select register-------------------------*/
#define MAINCLKSEL_IRC                  BIT_FIELD(0, 0)
#define MAINCLKSEL_PLL_INPUT            BIT_FIELD(1, 0)
#define MAINCLKSEL_WDT                  BIT_FIELD(2, 0)
#define MAINCLKSEL_PLL_OUTPUT           BIT_FIELD(3, 0)
/*------------------PLL Clock Source Select registers-------------------------*/
#define PLLCLKSEL_IRC                   BIT_FIELD(0, 0)
#define PLLCLKSEL_SYSOSC                BIT_FIELD(1, 0)
/*------------------PLL Control registers-------------------------------------*/
#define PLLCTRL_MSEL(value)             BIT_FIELD((value), 0)
#define PLLCTRL_MSEL_MASK               BIT_FIELD(MASK(5), 0)
#define PLLCTRL_PSEL(value)             BIT_FIELD((value), 5)
#define PLLCTRL_PSEL_MASK               BIT_FIELD(MASK(2), 5)
/*------------------PLL Status registers--------------------------------------*/
#define PLLSTAT_LOCK                    BIT(0)
/*------------------System Oscillator Control register------------------------*/
#define SYSOSCCTRL_BYPASS               BIT(0)
#define SYSOSCCTRL_FREQRANGE            BIT(1) /* Set for 15 - 25 MHz range */
/*------------------USB Clock Source Select register--------------------------*/
#define USBCLKSEL_USBPLL_OUTPUT         BIT_FIELD(0, 0)
#define USBCLKSEL_MAIN_CLOCK            BIT_FIELD(1, 0)
/*------------------Watchdog Clock Source Select register---------------------*/
#define WDTCLKSEL_IRC                   BIT_FIELD(0, 0)
#define WDTCLKSEL_MAIN_CLOCK            BIT_FIELD(1, 0)
#define WDTCLKSEL_WDT                   BIT_FIELD(2, 0)
/*------------------Watchdog Oscillator Control register----------------------*/
#define WDTOSCCTRL_DIVSEL(value)        BIT_FIELD((value), 0)
#define WDTOSCCTRL_DIVSEL_MASK          BIT_FIELD(MASK(5), 0)
#define WDTOSCCTRL_DIVSEL_VALUE(reg) \
    FIELD_VALUE((reg), WDTOSCCTRL_DIVSEL_MASK, 0)

#define WDTOSCCTRL_FREQSEL(value)       BIT_FIELD((value), 5)
#define WDTOSCCTRL_FREQSEL_MASK         BIT_FIELD(MASK(4), 5)
#define WDTOSCCTRL_FREQSEL_VALUE(reg) \
    FIELD_VALUE((reg), WDTOSCCTRL_FREQSEL_MASK, 5)
/*------------------Deep Power Down Control register--------------------------*/
#define DPDCTRL_LPOSCEN                 BIT(2)
#define DPDCTRL_LPOSCDPDEN              BIT(3)
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/clocking_defs.h>
#include HEADER_PATH
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_CLOCKING_DEFS_H_ */
