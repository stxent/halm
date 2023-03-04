/*
 * halm/platform/numicro/m48x/system_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_M48X_SYSTEM_DEFS_H_
#define HALM_PLATFORM_NUMICRO_M48X_SYSTEM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Multiple Function Control registers-----------------------*/
#define MFP_FUNCTION_MASK(pin)          BIT_FIELD(MASK(4), (pin) * 4)
#define MFP_FUNCTION(value, pin)        BIT_FIELD((value), (pin) * 4)
/*------------------Flash Access Cycle Control register-----------------------*/
#define CYCCTL_CYCLE_MAX                8
#define CYCCTL_CYCLE_MASK               BIT_FIELD(MASK(4), 0)
#define CYCCTL_CYCLE(value)             BIT_FIELD((value), 0)
#define CYCCTL_CYCLE_VALUE(reg)         FIELD_VALUE((reg), CYCCTL_CYCLE_MASK, 0)
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
