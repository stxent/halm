/*
 * halm/platform/numicro/m48x/clocking_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_M48X_CLOCKING_DEFS_H_
#define HALM_PLATFORM_NUMICRO_M48X_CLOCKING_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Clock Divider Number registers----------------------------*/
#define SOURCE_COUNT                    8
#define SOURCE_RESERVED                 8

#define BRANCH_FIELD(index, offset, size) \
    ((size) | ((offset) << 5) | ((index) << 10))
#define EXTRACT_BRANCH_INDEX(value)     ((value) >> 10)
#define EXTRACT_BRANCH_OFFSET(value)    (((value) >> 5) & 0x1F)
#define EXTRACT_BRANCH_SIZE(value)      ((value) & 0x1F)

enum [[gnu::packed]] ClockBranch
{
  BRANCH_HCLK     = BRANCH_FIELD(0, 0, 3),
  BRANCH_STCLK    = BRANCH_FIELD(0, 3, 3),
  BRANCH_USBD     = BRANCH_FIELD(0, 8, 1),
  BRANCH_CCAP     = BRANCH_FIELD(0, 16, 2),
  BRANCH_SDH0     = BRANCH_FIELD(0, 20, 2),
  BRANCH_SDH1     = BRANCH_FIELD(0, 22, 2),
  BRANCH_WDT      = BRANCH_FIELD(1, 0, 2),
  BRANCH_TMR0     = BRANCH_FIELD(1, 8, 3),
  BRANCH_TMR1     = BRANCH_FIELD(1, 12, 3),
  BRANCH_TMR2     = BRANCH_FIELD(1, 16, 3),
  BRANCH_TMR3     = BRANCH_FIELD(1, 20, 3),
  BRANCH_UART0    = BRANCH_FIELD(1, 24, 2),
  BRANCH_UART1    = BRANCH_FIELD(1, 26, 2),
  BRANCH_CLKO     = BRANCH_FIELD(1, 28, 2),
  BRANCH_WWDT     = BRANCH_FIELD(1, 30, 2),
  BRANCH_EPWM0    = BRANCH_FIELD(2, 0, 1),
  BRANCH_EPWM1    = BRANCH_FIELD(2, 1, 1),
  BRANCH_QSPI0    = BRANCH_FIELD(2, 2, 2),
  BRANCH_SPI0     = BRANCH_FIELD(2, 4, 2),
  BRANCH_SPI1     = BRANCH_FIELD(2, 6, 2),
  BRANCH_BPWM0    = BRANCH_FIELD(2, 8, 1),
  BRANCH_BPWM1    = BRANCH_FIELD(2, 9, 1),
  BRANCH_SPI2     = BRANCH_FIELD(2, 10, 2),
  BRANCH_SPI3     = BRANCH_FIELD(2, 12, 2),
  BRANCH_SC0      = BRANCH_FIELD(3, 0, 2),
  BRANCH_SC1      = BRANCH_FIELD(3, 2, 2),
  BRANCH_SC2      = BRANCH_FIELD(3, 4, 2),
  BRANCH_RTC      = BRANCH_FIELD(3, 8, 1),
  BRANCH_QSPI1    = BRANCH_FIELD(3, 12, 2),
  BRANCH_I2S0     = BRANCH_FIELD(3, 16, 2),
  BRANCH_UART6    = BRANCH_FIELD(3, 20, 2),
  BRANCH_UART7    = BRANCH_FIELD(3, 22, 2),
  BRANCH_UART2    = BRANCH_FIELD(3, 24, 2),
  BRANCH_UART3    = BRANCH_FIELD(3, 26, 2),
  BRANCH_UART4    = BRANCH_FIELD(3, 28, 2),
  BRANCH_UART5    = BRANCH_FIELD(3, 30, 2)
};

#define DIVIDER_FIELD(index, offset, size) \
    ((size) | ((offset) << 5) | ((index) << 10))
#define EXTRACT_DIVIDER_INDEX(value)    ((value) >> 10)
#define EXTRACT_DIVIDER_OFFSET(value)   (((value) >> 5) & 0x1F)
#define EXTRACT_DIVIDER_SIZE(value)     ((value) & 0x1F)

enum [[gnu::packed]] ClockBranchGroup
{
  BRANCH_GROUP_PWM,
  BRANCH_GROUP_SD,
  BRANCH_GROUP_SPI,
  BRANCH_GROUP_TMR,
  BRANCH_GROUP_UART,

  BRANCH_GROUP_CLKO,
  BRANCH_GROUP_HCLK,
  BRANCH_GROUP_RTC,
  BRANCH_GROUP_STCLK,
  BRANCH_GROUP_USB,
  BRANCH_GROUP_WDT,
  BRANCH_GROUP_WWDT,

  GROUP_COUNT
};

enum [[gnu::packed]] ClockDivider
{
  DIVIDER_HCLK    = DIVIDER_FIELD(0, 0, 4),
  DIVIDER_USB     = DIVIDER_FIELD(0, 4, 4),
  DIVIDER_UART0   = DIVIDER_FIELD(0, 8, 4),
  DIVIDER_UART1   = DIVIDER_FIELD(0, 12, 4),
  DIVIDER_EADC0   = DIVIDER_FIELD(0, 16, 8),
  DIVIDER_SDH0    = DIVIDER_FIELD(0, 24, 8),

  DIVIDER_SC0     = DIVIDER_FIELD(1, 0, 8),
  DIVIDER_SC1     = DIVIDER_FIELD(1, 8, 8),
  DIVIDER_SC2     = DIVIDER_FIELD(1, 16, 8),

  DIVIDER_I2S0    = DIVIDER_FIELD(2, 0, 4),
  DIVIDER_EADC1   = DIVIDER_FIELD(2, 24, 8),

  DIVIDER_CCAP    = DIVIDER_FIELD(3, 0, 8),
  DIVIDER_VSENSE  = DIVIDER_FIELD(3, 8, 8),
  DIVIDER_EMAC    = DIVIDER_FIELD(3, 16, 8),
  DIVIDER_SDH1    = DIVIDER_FIELD(3, 24, 8),

  DIVIDER_UART2   = DIVIDER_FIELD(4, 0, 4),
  DIVIDER_UART3   = DIVIDER_FIELD(4, 4, 4),
  DIVIDER_UART4   = DIVIDER_FIELD(4, 8, 4),
  DIVIDER_UART5   = DIVIDER_FIELD(4, 12, 4),
  DIVIDER_UART6   = DIVIDER_FIELD(4, 16, 4),
  DIVIDER_UART7   = DIVIDER_FIELD(4, 20, 4),

  DIVIDER_APB0    = DIVIDER_FIELD(5, 0, 3),
  DIVIDER_APB1    = DIVIDER_FIELD(5, 4, 3)
};
/*------------------System Power-down Control register------------------------*/
enum
{
  HXTGAIN_8MHZ        = 0,
  HXTGAIN_8MHZ_12MHZ  = 1,
  HXTGAIN_12MHZ_16MHZ = 2,
  HXTGAIN_16MHZ_24MHZ = 3
};

enum
{
  HIRCSTBS_24CLKS = 0,
  HIRCSTBS_64CLKS = 1
};

#define PWRCTL_HXTEN                    BIT(0)
#define PWRCTL_LXTEN                    BIT(1)
#define PWRCTL_HIRCEN                   BIT(2)
#define PWRCTL_LIRCEN                   BIT(3)
#define PWRCTL_PDWKDLY                  BIT(4)
#define PWRCTL_PDWKIEN                  BIT(5)
#define PWRCTL_PDWKIF                   BIT(6)
#define PWRCTL_PDEN                     BIT(7)

#define PWRCTL_HXTGAIN(value)           BIT_FIELD((value), 10)
#define PWRCTL_HXTGAIN_MASK             BIT_FIELD(MASK(2), 10)
#define PWRCTL_HXTGAIN_VALUE(reg) \
    FIELD_VALUE((reg), PWRCTL_HXTGAIN_MASK, 10)

#define PWRCTL_HXTSELTYP                BIT(12)
#define PWRCTL_HXTTBEN                  BIT(13)

#define PWRCTL_HIRCSTBS(value)          BIT_FIELD((value), 16)
#define PWRCTL_HIRCSTBS_MASK            BIT_FIELD(MASK(2), 16)
#define PWRCTL_HIRCSTBS_VALUE(reg) \
    FIELD_VALUE((reg), PWRCTL_HIRCSTBS_MASK, 16)

#define PWRCTL_HIRC48MEN                BIT(18)

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
#define STATUS_HIRC48MSTB               BIT(6)
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
/*------------------Power Manager Control register----------------------------*/
enum
{
  /* Power-down mode */
  PDMSEL_PD             = 0,
  /* Low leakage power-down mode */
  PDMSEL_LOW_LEAKAGE_PD = 1,
  /* Fast wake-up power-down mode */
  PDMSEL_FAST_WAKEUP_PD = 2,
  /* Standby power-down mode with SRAM retention */
  PDMSEL_STANDBY_PD_0   = 4,
  /* Standby power-down mode */
  PDMSEL_STANDBY_PD_1   = 5,
  /* Deep power-down mode */
  PDMSEL_DEEP_PD        = 6
};

enum
{
  SRETSEL_NONE  = 0,
  SRETSEL_16K   = 1,
  SRETSEL_32K   = 2,
  SRETSEL_64K   = 3,
  SRETSEL_128K  = 4
};

enum
{
  WKTMRIS_128     = 0,
  WKTMRIS_256     = 1,
  WKTMRIS_512     = 2,
  WKTMRIS_1024    = 3,
  WKTMRIS_4096    = 4,
  WKTMRIS_8192    = 5,
  WKTMRIS_16384   = 6,
  WKTMRIS_65536   = 7,
  WKTMRIS_131072  = 8,
  WKTMRIS_262144  = 9,
  WKTMRIS_524288  = 10,
  WKTMRIS_1048576 = 11
};

#define PMUCTL_PDMSEL(value)            BIT_FIELD((value), 0)
#define PMUCTL_PDMSEL_MASK              BIT_FIELD(MASK(3), 0)
#define PMUCTL_PDMSEL_VALUE(reg) \
    FIELD_VALUE((reg), PMUCTL_PDMSEL_MASK, 0)

#define PMUCTL_DPDHOLDEN                BIT(3)

#define PMUCTL_SRETSEL(value)           BIT_FIELD((value), 4)
#define PMUCTL_SRETSEL_MASK             BIT_FIELD(MASK(3), 4)
#define PMUCTL_SRETSEL_VALUE(reg) \
    FIELD_VALUE((reg), PMUCTL_SRETSEL_MASK, 4)

#define PMUCTL_WKTMREN                  BIT(8)

#define PMUCTL_WKTMRIS(value)           BIT_FIELD((value), 9)
#define PMUCTL_WKTMRIS_MASK             BIT_FIELD(MASK(4), 9)
#define PMUCTL_WKTMRIS_VALUE(reg) \
    FIELD_VALUE((reg), PMUCTL_WKTMRIS_MASK, 9)

#define PMUCTL_WKPINEN0(value)          BIT_FIELD((value), 16)
#define PMUCTL_WKPINEN0_MASK            BIT_FIELD(MASK(2), 16)
#define PMUCTL_WKPINEN0_VALUE(reg) \
    FIELD_VALUE((reg), PMUCTL_WKPINEN0_MASK, 16)

#define PMUCTL_ACMPSPWK                 BIT(18)
#define PMUCTL_RTCWKEN                  BIT(23)

#define PMUCTL_WKPINEN1(value)          BIT_FIELD((value), 24)
#define PMUCTL_WKPINEN1_MASK            BIT_FIELD(MASK(2), 24)
#define PMUCTL_WKPINEN1_VALUE(reg) \
    FIELD_VALUE((reg), PMUCTL_WKPINEN1_MASK, 24)

#define PMUCTL_WKPINEN2(value)          BIT_FIELD((value), 26)
#define PMUCTL_WKPINEN2_MASK            BIT_FIELD(MASK(2), 26)
#define PMUCTL_WKPINEN2_VALUE(reg) \
    FIELD_VALUE((reg), PMUCTL_WKPINEN2_MASK, 26)

#define PMUCTL_WKPINEN3(value)          BIT_FIELD((value), 28)
#define PMUCTL_WKPINEN3_MASK            BIT_FIELD(MASK(2), 28)
#define PMUCTL_WKPINEN3_VALUE(reg) \
    FIELD_VALUE((reg), PMUCTL_WKPINEN3_MASK, 28)

#define PMUCTL_WKPINEN4(value)          BIT_FIELD((value), 30)
#define PMUCTL_WKPINEN4_MASK            BIT_FIELD(MASK(2), 30)
#define PMUCTL_WKPINEN4_VALUE(reg) \
    FIELD_VALUE((reg), PMUCTL_WKPINEN4_MASK, 30)
/*------------------Power Manager Status register-----------------------------*/
#define PMUSTS_PINWK0                   BIT(0)
#define PMUSTS_TMRWK                    BIT(1)
#define PMUSTS_RTCWK                    BIT(2)
#define PMUSTS_PINWK1                   BIT(3)
#define PMUSTS_PINWK2                   BIT(4)
#define PMUSTS_PINWK3                   BIT(5)
#define PMUSTS_PINWK4                   BIT(6)
#define PMUSTS_GPAWK                    BIT(8)
#define PMUSTS_GPBWK                    BIT(9)
#define PMUSTS_GPCWK                    BIT(10)
#define PMUSTS_GPDWK                    BIT(11)
#define PMUSTS_LVRWK                    BIT(12)
#define PMUSTS_BODWK                    BIT(13)
#define PMUSTS_ACMPWK                   BIT(14)
#define PMUSTS_CLRWK                    BIT(31)
/*------------------LDO Control register--------------------------------------*/
#define LDOCTL_DPDWKSEL                 BIT(0)
/*------------------GPIO Standby Power-down Control register------------------*/
#define IOPDCTL_IOHR                    BIT(0)
/*----------------------------------------------------------------------------*/
// TODO SWKDBCTL PxSWKCTL
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M48X_CLOCKING_DEFS_H_ */
