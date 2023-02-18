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
