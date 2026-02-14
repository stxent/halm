/*
 * halm/platform/numicro/m48x/system_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_SYSTEM_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_M48X_SYSTEM_DEFS_H_
#define HALM_PLATFORM_NUMICRO_M48X_SYSTEM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Multiple Function Control registers-----------------------*/
#define MFP_FUNCTION_MASK(pin)          BIT_FIELD(MASK(4), (pin) * 4)
#define MFP_FUNCTION(value, pin)        BIT_FIELD((value), (pin) * 4)
/*------------------Brown-out Detector Control register-----------------------*/
#define BODCTL_BODEN                    BIT(0)
#define BODCTL_BODRSTEN                 BIT(3)
#define BODCTL_BODIF                    BIT(4)
#define BODCTL_BODLPM                   BIT(5)
#define BODCTL_BODOUT                   BIT(6)
#define BODCTL_LVREN                    BIT(7)

#define BODCTL_BODDGSEL_MASK            BIT_FIELD(MASK(3), 8)
#define BODCTL_BODDGSEL(value)          BIT_FIELD((value), 8)
#define BODCTL_BODDGSEL_VALUE(reg) \
    FIELD_VALUE((reg), BODCTL_BODDGSEL_MASK, 8)

#define BODCTL_LVRDGSEL_MASK            BIT_FIELD(MASK(3), 12)
#define BODCTL_LVRDGSEL(value)          BIT_FIELD((value), 12)
#define BODCTL_LVRDGSEL_VALUE(reg) \
    FIELD_VALUE((reg), BODCTL_LVRDGSEL_MASK, 12)

#define BODCTL_BODVL_MASK               BIT_FIELD(MASK(3), 16)
#define BODCTL_BODVL(value)             BIT_FIELD((value), 16)
#define BODCTL_BODVL_VALUE(reg) \
    FIELD_VALUE((reg), BODCTL_BODVL_MASK, 16)
/*------------------Power Level Control register------------------------------*/
enum
{
  /* Turbo mode */
  PLSEL_PL0 = 0,
  /* Normal mode */
  PLSEL_PL1 = 1
};

#define PLCTL_PLSEL_MASK                BIT_FIELD(MASK(2), 0)
#define PLCTL_PLSEL(value)              BIT_FIELD((value), 0)
#define PLCTL_PLSEL_VALUE(reg)          FIELD_VALUE((reg), PLCTL_PLSEL_MASK, 0)

#define PLCTL_LVSSTEP_MASK              BIT_FIELD(MASK(6), 16)
#define PLCTL_LVSSTEP(value)            BIT_FIELD((value), 16)
#define PLCTL_LVSSTEP_VALUE(reg) \
    FIELD_VALUE((reg), PLCTL_LVSSTEP_MASK, 16)

#define PLCTL_LVSPRD_MASK               BIT_FIELD(MASK(8), 24)
#define PLCTL_LVSPRD(value)             BIT_FIELD((value), 24)
#define PLCTL_LVSPRD_VALUE(reg) \
    FIELD_VALUE((reg), PLCTL_LVSPRD_MASK, 24)
/*------------------Power Level Status register-------------------------------*/
enum
{
  /* Turbo mode */
  PLSTATUS_PL0 = 0,
  /* Normal mode */
  PLSTATUS_PL1 = 1
};

#define PLSTS_PLCBUSY                   BIT(0)

#define PLCTL_PLSTATUS_MASK             BIT_FIELD(MASK(2), 8)
#define PLCTL_PLSTATUS(value)           BIT_FIELD((value), 8)
#define PLCTL_PLSTATUS_VALUE(reg) \
    FIELD_VALUE((reg), PLCTL_PLSTATUS_MASK, 8)
/*------------------Register Lock Control register----------------------------*/
#define REGLCTL_MAGIC_NUMBER_0          0x59
#define REGLCTL_MAGIC_NUMBER_1          0x16
#define REGLCTL_MAGIC_NUMBER_2          0x88
/*------------------VREF Control register-------------------------------------*/
enum
{
  PRELOAD_SEL_60US    = 0, /* For 0.1 uF capacitors */
  PRELOAD_SEL_310US   = 1, /* For 1 uF capacitors */
  PRELOAD_SEL_1270US  = 2, /* For 4.7 uF capacitors */
  PRELOAD_SEL_2650US  = 3  /* For 10 uF capacitors */
};

enum
{
  VBGISEL_4UA2  = 0,
  VBGISEL_7UA3  = 1,
  VBGISEL_10UA4 = 2,
  VBGISEL_13UA5 = 3
};

enum
{
  VREFCTL_EXT     = 0,
  VREFCTL_INT_1V6 = 3,
  VREFCTL_INT_2V0 = 7,
  VREFCTL_INT_2V5 = 11,
  VREFCTL_INT_3V0 = 15
};

#define VREFCTL_VREFCTL_MASK            BIT_FIELD(MASK(5), 0)
#define VREFCTL_VREFCTL(value)          BIT_FIELD((value), 0)
#define VREFCTL_VREFCTL_VALUE(reg) \
    FIELD_VALUE((reg), VREFCTL_VREFCTL_MASK, 0)

#define VREFCTL_PRELOAD_SEL_MASK        BIT_FIELD(MASK(2), 6)
#define VREFCTL_PRELOAD_SEL(value)      BIT_FIELD((value), 6)
#define VREFCTL_PRELOAD_SEL_VALUE(reg) \
    FIELD_VALUE((reg), VREFCTL_PRELOAD_SEL_MASK, 0)

#define VREFCTL_VBGFEN                  BIT(24)

#define VREFCTL_VBGISEL_MASK            BIT_FIELD(MASK(2), 25)
#define VREFCTL_VBGISEL(value)          BIT_FIELD((value), 25)
#define VREFCTL_VBGISEL_VALUE(reg) \
    FIELD_VALUE((reg), VREFCTL_VBGISEL_MASK, 25)
/*------------------USB PHY control register----------------------------------*/
enum
{
  USBROLE_DEVICE        = 0,
  USBROLE_HOST          = 1,
  USBROLE_ID_DEPENDENT  = 2,
  USBROLE_OTG_DEVICE    = 3
};

#define USBPHY_USBROLE_MASK             BIT_FIELD(MASK(2), 0)
#define USBPHY_USBROLE(value)           BIT_FIELD((value), 0)
#define USBPHY_USBROLE_VALUE(reg) \
    FIELD_VALUE((reg), USBPHY_USBROLE_MASK, 0)

#define USBPHY_SBO                      BIT(2)
#define USBPHY_USBEN                    BIT(8)

#define USBPHY_HSUSBROLE_MASK           BIT_FIELD(MASK(2), 16)
#define USBPHY_HSUSBROLE(value)         BIT_FIELD((value), 16)
#define USBPHY_HSUSBROLE_VALUE(reg) \
    FIELD_VALUE((reg), USBPHY_HSUSBROLE_MASK, 16)

#define USBPHY_HSUSBEN                  BIT(24)
#define USBPHY_HSUSBACT                 BIT(25)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M48X_SYSTEM_DEFS_H_ */
