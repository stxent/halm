/*
 * halm/platform/numicro/m03x/clocking_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_M03X_CLOCKING_DEFS_H_
#define HALM_PLATFORM_NUMICRO_M03X_CLOCKING_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Clock Divider Number registers----------------------------*/
#define BRANCH_FIELD(index, offset, size) \
    ((size) | ((offset) << 5) | ((index) << 10))
#define EXTRACT_BRANCH_INDEX(value)     ((value) >> 10)
#define EXTRACT_BRANCH_OFFSET(value)    (((value) >> 5) & 0x1F)
#define EXTRACT_BRANCH_SIZE(value)      ((value) & 0x1F)

enum ClockBranch
{
  BRANCH_HCLK     = BRANCH_FIELD(0, 0, 3),
  BRANCH_STCLK    = BRANCH_FIELD(0, 3, 3),
  BRANCH_USBD     = BRANCH_FIELD(0, 8, 1),
  BRANCH_WDT      = BRANCH_FIELD(1, 0, 2),
  BRANCH_WWDT     = BRANCH_FIELD(1, 2, 2),
  BRANCH_CLKO     = BRANCH_FIELD(1, 4, 3),
  BRANCH_TMR0     = BRANCH_FIELD(1, 8, 3),
  BRANCH_TMR1     = BRANCH_FIELD(1, 12, 3),
  BRANCH_TMR2     = BRANCH_FIELD(1, 16, 3),
  BRANCH_TMR3     = BRANCH_FIELD(1, 20, 3),
  BRANCH_UART0    = BRANCH_FIELD(1, 24, 3),
  BRANCH_UART1    = BRANCH_FIELD(1, 28, 3),
  BRANCH_PWM0     = BRANCH_FIELD(2, 0, 1),
  BRANCH_PWM1     = BRANCH_FIELD(2, 1, 1),
  BRANCH_QSPI0    = BRANCH_FIELD(2, 2, 2),
  BRANCH_SPI0     = BRANCH_FIELD(2, 4, 2),
  BRANCH_BPWM0    = BRANCH_FIELD(2, 8, 1),
  BRANCH_BPWM1    = BRANCH_FIELD(2, 9, 1),
  BRANCH_ADC      = BRANCH_FIELD(2, 20, 2),
  BRANCH_UART6    = BRANCH_FIELD(3, 8, 3),
  BRANCH_UART7    = BRANCH_FIELD(3, 12, 3),
  BRANCH_UART4    = BRANCH_FIELD(3, 16, 3),
  BRANCH_UART5    = BRANCH_FIELD(3, 20, 3),
  BRANCH_UART2    = BRANCH_FIELD(3, 24, 3),
  BRANCH_UART3    = BRANCH_FIELD(3, 28, 3)
} __attribute__((packed));

#define DIVIDER_FIELD(index, offset, size) \
    ((size) | ((offset) << 5) | ((index) << 10))
#define EXTRACT_DIVIDER_INDEX(value)    ((value) >> 10)
#define EXTRACT_DIVIDER_OFFSET(value)   (((value) >> 5) & 0x1F)
#define EXTRACT_DIVIDER_SIZE(value)     ((value) & 0x1F)

enum ClockBranchGroup
{
  BRANCH_GROUP_ADC_SPI,
  BRANCH_GROUP_PWM,
  BRANCH_GROUP_TMR,
  BRANCH_GROUP_UART,

  BRANCH_GROUP_CLKO,
  BRANCH_GROUP_HCLK,
  BRANCH_GROUP_STCLK,
  BRANCH_GROUP_USB,
  BRANCH_GROUP_WDT,
  BRANCH_GROUP_WWDT,

  GROUP_COUNT
} __attribute__((packed));

enum BranchSource
{
  BRANCH_SOURCE_HXT             = 0,
  BRANCH_SOURCE_PWM_PLL         = 0,
  BRANCH_SOURCE_USB_HIRC        = 0,

  BRANCH_SOURCE_LXT             = 1,
  BRANCH_SOURCE_ADC_SPI_PLL     = 1,
  BRANCH_SOURCE_PWM_PCLK        = 1,
  BRANCH_SOURCE_UART_PLL        = 1,
  BRANCH_SOURCE_USB_PLL         = 1,
  
  BRANCH_SOURCE_HCLK            = 2,
  BRANCH_SOURCE_ADC_SPI_PCLK    = 2,
  BRANCH_SOURCE_HCLK_PLL        = 2,
  BRANCH_SOURCE_STCLK_HXT_DIV2  = 2,
  BRANCH_SOURCE_TMR_PCLK        = 2,
  BRANCH_SOURCE_UART_LXT        = 2,

  BRANCH_SOURCE_HIRC            = 3,
  BRANCH_SOURCE_HCLK_LIRC       = 3,
  BRANCH_SOURCE_STCLK_HCLK_DIV2 = 3,
  BRANCH_SOURCE_TMR_TM          = 3,
  BRANCH_SOURCE_WDT_LIRC        = 3,
  BRANCH_SOURCE_WWDT_LIRC       = 3,

  BRANCH_SOURCE_PCLK            = 4,
  BRANCH_SOURCE_CLKO_LIRC       = 4,

  BRANCH_SOURCE_LIRC            = 5,
  BRANCH_SOURCE_CLKO_HIRC       = 5,

  BRANCH_SOURCE_CLKO_PLL        = 6,

  BRANCH_SOURCE_HCLK_HIRC       = 7,
  BRANCH_SOURCE_STCLK_HIRC_DIV2 = 7,
  BRANCH_SOURCE_TMR_HIRC        = 7,

  SOURCE_RESERVED               = 8,
  SOURCE_COUNT                  = 8
} __attribute__((packed));

enum ClockDivider
{
  DIVIDER_HCLK  = DIVIDER_FIELD(0, 0, 4),
  DIVIDER_USB   = DIVIDER_FIELD(0, 4, 4),
  DIVIDER_UART0 = DIVIDER_FIELD(0, 8, 4),
  DIVIDER_UART1 = DIVIDER_FIELD(0, 12, 4),
  DIVIDER_ADC   = DIVIDER_FIELD(0, 16, 8),

  DIVIDER_UART2 = DIVIDER_FIELD(4, 0, 4),
  DIVIDER_UART3 = DIVIDER_FIELD(4, 4, 4),
  DIVIDER_UART4 = DIVIDER_FIELD(4, 8, 4),
  DIVIDER_UART5 = DIVIDER_FIELD(4, 12, 4),
  DIVIDER_UART6 = DIVIDER_FIELD(4, 16, 4),
  DIVIDER_UART7 = DIVIDER_FIELD(4, 20, 4),

  DIVIDER_APB0  = DIVIDER_FIELD(5, 0, 3),
  DIVIDER_APB1  = DIVIDER_FIELD(5, 4, 3)
} __attribute__((packed));
/*------------------System Power-down Control register------------------------*/
enum
{
  HXTGAIN_4MHZ        = 0,
  HXTGAIN_4MHZ_8MHZ   = 1,
  HXTGAIN_8MHZ_12MHZ  = 2,
  HXTGAIN_12MHZ_16MHZ = 3,
  HXTGAIN_16MHZ_24MHZ = 4,
  HXTGAIN_24MHZ_32MHZ = 7
};

enum
{
  LXTGAIN_LOW   = 0,
  LXTGAIN_HIGH  = 2
};

#define PWRCTL_HXTEN                    BIT(0)
#define PWRCTL_LXTEN                    BIT(1)
#define PWRCTL_HIRCEN                   BIT(2)
#define PWRCTL_LIRCEN                   BIT(3)
#define PWRCTL_PDWKDLY                  BIT(4)
#define PWRCTL_PDWKIEN                  BIT(5)
#define PWRCTL_PDWKIF                   BIT(6)
#define PWRCTL_PDEN                     BIT(7)

#define PWRCTL_HXTGAIN(value)           BIT_FIELD((value), 20)
#define PWRCTL_HXTGAIN_MASK             BIT_FIELD(MASK(3), 20)
#define PWRCTL_HXTGAIN_VALUE(reg) \
    FIELD_VALUE((reg), PWRCTL_HXTGAIN_MASK, 20)

#define PWRCTL_LXTSELXT                 BIT(24)

#define PWRCTL_LXTGAIN(value)           BIT_FIELD((value), 25)
#define PWRCTL_LXTGAIN_MASK             BIT_FIELD(MASK(2), 25)
#define PWRCTL_LXTGAIN_VALUE(reg) \
    FIELD_VALUE((reg), PWRCTL_LXTGAIN_MASK, 25)
/*------------------PLL Control register--------------------------------------*/
enum
{
  OUTDIV_DIV1 = 0,
  OUTDIV_DIV2 = 1,
  OUTDIV_DIV4 = 3
};

#define PLLCTL_FBDIV(value)             BIT_FIELD((value), 0)
#define PLLCTL_FBDIV_MASK               BIT_FIELD(MASK(9), 0)
#define PLLCTL_FBDIV_VALUE(reg)         FIELD_VALUE((reg), PLLCTL_FBDIV_MASK, 0)

#define PLLCTL_INDIV(value)             BIT_FIELD((value), 9)
#define PLLCTL_INDIV_MASK               BIT_FIELD(MASK(5), 9)
#define PLLCTL_INDIV_VALUE(reg)         FIELD_VALUE((reg), PLLCTL_INDIV_MASK, 9)

#define PLLCTL_OUTDIV(value)            BIT_FIELD((value), 14)
#define PLLCTL_OUTDIV_MASK              BIT_FIELD(MASK(2), 14)
#define PLLCTL_OUTDIV_VALUE(reg) \
    FIELD_VALUE((reg), PLLCTL_OUTDIV_MASK, 14)

#define PLLCTL_PD                       BIT(16)
#define PLLCTL_BP                       BIT(17)
#define PLLCTL_OE                       BIT(18)
#define PLLCTL_PLLSRC                   BIT(19)
#define PLLCTL_STBSEL                   BIT(23)
/*------------------Clock Status Monitor register-----------------------------*/
#define STATUS_HXTSTB                   BIT(0)
#define STATUS_LXTSTB                   BIT(1)
#define STATUS_PLLSTB                   BIT(2)
#define STATUS_LIRCSTB                  BIT(3)
#define STATUS_HIRCSTB                  BIT(4)
#define STATUS_CLKSFAIL                 BIT(7)
/*------------------Clock Output Control register-----------------------------*/
#define CLKOCTL_FREQSEL(value)          BIT_FIELD((value), 0)
#define CLKOCTL_FREQSEL_MASK            BIT_FIELD(MASK(4), 0)
#define CLKOCTL_FREQSEL_VALUE(reg) \
    FIELD_VALUE((reg), CLKOCTL_FREQSEL_MASK, 0)

#define CLKOCTL_CLKOEN                  BIT(4)
#define CLKOCTL_DIV1EN                  BIT(5)
#define CLKOCTL_CLK1HZEN                BIT(6)
/*------------------Clock Fail Detector Control register----------------------*/
#define CLKDCTL_HXTFDEN                 BIT(4)
#define CLKDCTL_HXTFIEN                 BIT(5)
#define CLKDCTL_LXTFDEN                 BIT(12)
#define CLKDCTL_LXTFIEN                 BIT(13)
#define CLKDCTL_HXTFQDEN                BIT(16)
#define CLKDCTL_HXTFQIEN                BIT(17)
/*------------------Clock Fail Detector Status register-----------------------*/
#define CLKDSTS_HXTFIF                  BIT(0)
#define CLKDSTS_LXTFIF                  BIT(1)
#define CLKDSTS_HXTFQIF                 BIT(8)
/*------------------HXT Filter Select Control register------------------------*/
#define HXTFSEL_HXTFSEL                 BIT(0)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M03X_CLOCKING_DEFS_H_ */
