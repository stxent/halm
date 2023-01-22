/*
 * halm/platform/numicro/m03x/usb_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_M03X_USB_DEFS_H_
#define HALM_PLATFORM_NUMICRO_M03X_USB_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Interrupt Enable register---------------------------------*/
#define INTEN_BUSIEN                    BIT(0)
#define INTEN_USBIEN                    BIT(1)
#define INTEN_VBDETIEN                  BIT(2)
#define INTEN_NEVWKIEN                  BIT(3)
#define INTEN_SOFIEN                    BIT(4)
#define INTEN_WKEN                      BIT(8)
#define INTEN_INNAKEN                   BIT(15)
/*------------------Interrupt Event Status register---------------------------*/
#define INTSTS_BUSIF                    BIT(0)
#define INTSTS_USBIF                    BIT(1)
#define INTSTS_VBDETIF                  BIT(2)
#define INTSTS_NEVWKIF                  BIT(3)
#define INTSTS_SOFIF                    BIT(4)

#define INTSTS_EPEVT_MASK               BIT_FIELD(MASK(8), 16)
#define INTSTS_EPEVT(value)             BIT_FIELD((value), 16)
#define INTSTS_EPEVT_VALUE(reg) \
    FIELD_VALUE((reg), INTSTS_EPEVT_MASK, 16)

#define INTSTS_SETUP                    BIT(31)
/*------------------Endpoint Status register----------------------------------*/
#define EPSTS_OV                        BIT(7)
/*------------------Bus Status and Attribution register-----------------------*/
#define ATTR_USBRST                     BIT(0)
#define ATTR_SUSPEND                    BIT(1)
#define ATTR_RESUME                     BIT(2)
#define ATTR_TOUT                       BIT(3)
#define ATTR_PHYEN                      BIT(4)
#define ATTR_RWAKEUP                    BIT(5)
#define ATTR_USBEN                      BIT(7)
#define ATTR_DPPUEN                     BIT(8)
#define ATTR_PWRDN                      BIT(9)
#define ATTR_BYTEM                      BIT(10)
#define ATTR_LPMACK                     BIT(11)
#define ATTR_L1SUSPEND                  BIT(12)
#define ATTR_L1RESUME                   BIT(13)
/*------------------Device VBUS Detection register----------------------------*/
#define VBUSDET_VBUSDET                 BIT(0)
/*------------------SETUP Token Buffer Segmentation register------------------*/
#define STBUFSEG_SIZE                   8
#define STBUFSEG_ADDRESS_MASK           BIT_FIELD(MASK(6), 3)
/*------------------Endpoint Status register 0--------------------------------*/
#define EPSTS_EP_MASK(index)            BIT_FIELD(MASK(4), (index) * 4)
#define EPSTS_EP(index, value)          BIT_FIELD((value), (index) * 4)
#define EPSTS_EP_VALUE(index, reg) \
    FIELD_VALUE((reg), EPSTS_EP_MASK(index), (index) * 4)
/*------------------LPM Attribution register----------------------------------*/
#define LPMATTR_LPMLINKSTS_MASK         BIT_FIELD(MASK(4), 0)
#define LPMATTR_LPMLINKSTS(value)       BIT_FIELD((value), 0)
#define LPMATTR_LPMLINKSTS_VALUE(reg) \
    FIELD_VALUE((reg), LPMATTR_LPMLINKSTS_MASK, 0)

#define LPMATTR_BESL_MASK               BIT_FIELD(MASK(4), 4)
#define LPMATTR_BESL(value)             BIT_FIELD((value), 4)
#define LPMATTR_BESL_VALUE(reg) \
    FIELD_VALUE((reg), LPMATTR_BESL_MASK, 4)

#define LPMATTR_LPMRWAKUP               BIT(8)
/*------------------Drive DE0 register----------------------------------------*/
#define SE0_SE0                         BIT(0)
/*------------------Buffer Segmentation registers-----------------------------*/
#define BUFSEG_ADDRESS_ALIGNMENT        8
#define BUFSEG_ADDRESS_MASK             BIT_FIELD(MASK(6), 3)
/*------------------Configuration registers-----------------------------------*/
enum
{
  STATE_EP_DISABLED = 0,
  STATE_EP_OUT      = 1,
  STATE_EP_IN       = 2
};

#define CFG_EPNUM_MASK                  BIT_FIELD(MASK(4), 0)
#define CFG_EPNUM(value)                BIT_FIELD((value), 0)
#define CFG_EPNUM_VALUE(reg)            FIELD_VALUE((reg), CFG_EPNUM_MASK, 0)

#define CFG_ISOCH                       BIT(4)

#define CFG_STATE_MASK                  BIT_FIELD(MASK(2), 5)
#define CFG_STATE(value)                BIT_FIELD((value), 5)
#define CFG_STATE_VALUE(reg)            FIELD_VALUE((reg), CFG_STATE_MASK, 5)

#define CFG_DSQSYNC                     BIT(7)
#define CFG_CSTALL                      BIT(9)
/*------------------Extra Configuration registers-----------------------------*/
#define CFGP_CLRRDY                     BIT(0)
#define CFGP_SSTALL                     BIT(1)
/*----------------------------------------------------------------------------*/
#define EP_TO_NUMBER(ep)                ((ep) & 0x0F)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M03X_USB_DEFS_H_ */
