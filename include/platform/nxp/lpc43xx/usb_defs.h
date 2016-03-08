/*
 * platform/nxp/lpc43xx/usb_defs.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_LPC43XX_USB_DEFS_H_
#define PLATFORM_NXP_LPC43XX_USB_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <bits.h>
/*------------------USB Command register--------------------------------------*/
/* Device mode */
#define USBCMD_D_RS                     BIT(0)
#define USBCMD_D_RST                    BIT(1)
#define USBCMD_D_SUTW                   BIT(13)
#define USBCMD_D_ATDTW                  BIT(14)
#define USBCMD_D_ITC(value)             BIT_FIELD((value), 16)

/* Host mode */
#define USBCMD_H_RS                     BIT(0)
#define USBCMD_H_RST                    BIT(1)
#define USBCMD_H_FS0                    BIT(2)
#define USBCMD_H_FS1                    BIT(3)
#define USBCMD_H_PSE                    BIT(4)
#define USBCMD_H_ASE                    BIT(5)
#define USBCMD_H_IAA                    BIT(6)
#define USBCMD_H_ASP1_0(value)          BIT_FIELD((value), 8)
#define USBCMD_H_ASPE                   BIT(11)
#define USBCMD_H_FS2                    BIT(15)
#define USBCMD_H_ITC(value)             BIT_FIELD((value), 16)
/*------------------USB Status register---------------------------------------*/
/* Device mode */
#define USBSTS_D_UI                     BIT(0)
#define USBSTS_D_UEI                    BIT(1)
#define USBSTS_D_PCI                    BIT(2)
#define USBSTS_D_SEI                    BIT(4)
#define USBSTS_D_AAI                    BIT(5)
#define USBSTS_D_URI                    BIT(6)
#define USBSTS_D_SRI                    BIT(7)
#define USBSTS_D_SLI                    BIT(8)
#define USBSTS_D_NAKI                   BIT(16)

/* Host mode */
#define USBSTS_H_UI                     BIT(0)
#define USBSTS_H_UEI                    BIT(1)
#define USBSTS_H_PCI                    BIT(2)
#define USBSTS_H_FRI                    BIT(3)
#define USBSTS_H_SEI                    BIT(4)
#define USBSTS_H_AAI                    BIT(5)
#define USBSTS_H_SRI                    BIT(7)
#define USBSTS_H_HCH                    BIT(12)
#define USBSTS_H_RCL                    BIT(13)
#define USBSTS_H_PS                     BIT(14)
#define USBSTS_H_AS                     BIT(15)
#define USBSTS_H_UAI                    BIT(18)
#define USBSTS_H_UPI                    BIT(19)
/*------------------USB Interrupt register------------------------------------*/
/* Device mode */
#define USBINTR_D_UE                    BIT(0)
#define USBINTR_D_UEE                   BIT(1)
#define USBINTR_D_PCE                   BIT(2)
#define USBINTR_D_SEE                   BIT(4)
#define USBINTR_D_URE                   BIT(6)
#define USBINTR_D_SRE                   BIT(7)
#define USBINTR_D_SLE                   BIT(8)
#define USBINTR_D_NAKE                  BIT(16)

/* Host mode */
#define USBINTR_H_UE                    BIT(0)
#define USBINTR_H_UEE                   BIT(1)
#define USBINTR_H_PCE                   BIT(2)
#define USBINTR_H_FRE                   BIT(3)
#define USBINTR_H_SEE                   BIT(4)
#define USBINTR_H_AEE                   BIT(5)
#define USBINTR_H_SRE                   BIT(7)
#define USBINTR_H_UAIE                  BIT(18)
#define USBINTR_H_UPIA                  BIT(19)
/*------------------Device Address register-----------------------------------*/
#define DEVICEADDR_USBADRA              BIT(24)
#define DEVICEADDR_USBADR(value)        BIT_FIELD((value), 25)
#define DEVICEADDR_USBADR_MASK          BIT_FIELD(MASK(7), 25)
#define DEVICEADDR_USBADR_VALUE(reg) \
    FIELD_VALUE((reg), DEVICEADDR_USBADR_MASK, 25)
///*------------------USB Endpoint NAK register---------------------------------*/
//#define ENDPTNAK_EPRN(value)            BIT_FIELD((value), 0)
//#define ENDPTNAK_EPRN_MASK              BIT_FIELD(MASK(6), 0)
//#define ENDPTNAK_EPRN_VALUE(reg) \
//    FIELD_VALUE((reg), ENDPTNAK_EPRN_MASK, 0)
//#define ENDPTNAK_EPTN(value)            BIT_FIELD((value), 16)
//#define ENDPTNAK_EPTN_MASK              BIT_FIELD(MASK(6), 16)
//#define ENDPTNAK_EPTN_VALUE(reg) \
//    FIELD_VALUE((reg), ENDPTNAK_EPTN_MASK, 16)
///*------------------USB Endpoint NAK Enable register--------------------------*/
//#define ENDPTNAKEN_EPRNE(value)         BIT_FIELD((value), 0)
//#define ENDPTNAKEN_EPRNE_MASK           BIT_FIELD(MASK(6), 0)
//#define ENDPTNAKEN_EPRNE_VALUE(reg) \
//    FIELD_VALUE((reg), ENDPTNAKEN_EPRNE_MASK, 0)
//#define ENDPTNAKEN_EPTNE(value)         BIT_FIELD((value), 16)
//#define ENDPTNAKEN_EPTNE_MASK           BIT_FIELD(MASK(6), 16)
//#define ENDPTNAKEN_EPTNE_VALUE(reg) \
//    FIELD_VALUE((reg), ENDPTNAKEN_EPTNE_MASK, 16)
/*------------------Port Status and Control register--------------------------*/
enum
{
  PSPD_FULL_SPEED = 0,
  PSPD_LOW_SPEED  = 1, /* In Host mode only */
  PSPD_HIGH_SPEED = 2
};

/* Device mode */
#define PORTSC1_D_CCS                   BIT(0)
#define PORTSC1_D_PE                    BIT(2)
#define PORTSC1_D_PEC                   BIT(3)
#define PORTSC1_D_FPR                   BIT(6)
#define PORTSC1_D_SUSP                  BIT(7)
#define PORTSC1_D_PR                    BIT(8)
#define PORTSC1_D_HSP                   BIT(9)
#define PORTSC1_D_PIC1_0(value)         BIT_FIELD((value), 14)
#define PORTSC1_D_PTC3_0(value)         BIT_FIELD((value), 16)
#define PORTSC1_D_PHCD                  BIT(23)
#define PORTSC1_D_PFSC                  BIT(24)
#define PORTSC1_D_PSPD(value)           BIT_FIELD((value), 26)

/* Host mode */
#define PORTSC1_H_CCS                   BIT(0)
#define PORTSC1_H_CSC                   BIT(1)
#define PORTSC1_H_PE                    BIT(2)
#define PORTSC1_H_PEC                   BIT(3)
#define PORTSC1_H_OCA                   BIT(4)
#define PORTSC1_H_OCC                   BIT(5)
#define PORTSC1_H_FPR                   BIT(6)
#define PORTSC1_H_SUSP                  BIT(7)
#define PORTSC1_H_PR                    BIT(8)
#define PORTSC1_H_HSP                   BIT(9)

#define PORTSC1_H_LS(value)             BIT_FIELD((value), 10)
#define PORTSC1_H_LS_MASK               BIT_FIELD(MASK(2), 10)
#define PORTSC1_H_LS_VALUE(reg) \
    FIELD_VALUE((reg), PORTSC1_H_LS_MASK, 10)

#define PORTSC1_H_PP                    BIT(12)
#define PORTSC1_H_PIC1_0(value)         BIT_FIELD((value), 14)
#define PORTSC1_H_PTC3_0(value)         BIT_FIELD((value), 16)
#define PORTSC1_H_WKCN                  BIT(20)
#define PORTSC1_H_WKDC                  BIT(21)
#define PORTSC1_H_WKOC                  BIT(22)
#define PORTSC1_H_PHCD                  BIT(23)
#define PORTSC1_H_PFSC                  BIT(24)
#define PORTSC1_H_PSPD(value)           BIT_FIELD((value), 26)
/*------------------OTG Status and Control register---------------------------*/
#define OTGSC_VD                        BIT(0)
#define OTGSC_VC                        BIT(1)
#define OTGSC_HAAR                      BIT(2)
#define OTGSC_OT                        BIT(3)
#define OTGSC_DP                        BIT(4)
#define OTGSC_IDPU                      BIT(5)
#define OTGSC_HADP                      BIT(6)
#define OTGSC_HABA                      BIT(7)
#define OTGSC_ID                        BIT(8)
#define OTGSC_AVV                       BIT(9)
#define OTGSC_ASV                       BIT(10)
#define OTGSC_BSV                       BIT(11)
#define OTGSC_BSE                       BIT(12)
#define OTGSC_MS1T                      BIT(13)
#define OTGSC_DPS                       BIT(14)
#define OTGSC_IDIS                      BIT(16)
#define OTGSC_AVVIS                     BIT(17)
#define OTGSC_ASVIS                     BIT(18)
#define OTGSC_BSVIS                     BIT(19)
#define OTGSC_BSEIS                     BIT(20)
#define OTGSC_MS1S                      BIT(21)
#define OTGSC_DPIS                      BIT(22)
#define OTGSC_IDIE                      BIT(24)
#define OTGSC_AVVIE                     BIT(25)
#define OTGSC_ASVIE                     BIT(26)
#define OTGSC_BSVIE                     BIT(27)
#define OTGSC_BSEIE                     BIT(28)
#define OTGSC_MS1E                      BIT(29)
#define OTGSC_DPIE                      BIT(30)
/*------------------USB Mode register-----------------------------------------*/
enum
{
  CM_IDLE               = 0,
  CM_DEVICE_CONTROLLER  = 2,
  CM_HOST_CONTROLLER    = 3
};

/* Device mode */
#define USBMODE_D_CM(value)             BIT_FIELD((value), 0)
#define USBMODE_D_ES                    BIT(2)
#define USBMODE_D_SLOM                  BIT(3)
#define USBMODE_D_SDIS                  BIT(4)

/* Host mode */
#define USBMODE_H_CM(value)             BIT_FIELD((value), 0)
#define USBMODE_H_ES                    BIT(2)
#define USBMODE_H_SDIS                  BIT(4)
#define USBMODE_H_VBPS                  BIT(5)
///*------------------USB Endpoint Prime register-------------------------------*/
//#define ENDPTPRIME_PERB(value)          BIT_FIELD((value), 0)
//#define ENDPTPRIME_PERB_MASK            BIT_FIELD(MASK(6), 0)
//#define ENDPTPRIME_PERB_VALUE(reg) \
//    FIELD_VALUE((reg), ENDPTPRIME_PERB_MASK, 0)
//#define ENDPTPRIME_PETB(value)          BIT_FIELD((value), 16)
//#define ENDPTPRIME_PETB_MASK            BIT_FIELD(MASK(6), 16)
//#define ENDPTPRIME_PETB_VALUE(reg) \
//    FIELD_VALUE((reg), ENDPTPRIME_PETB_MASK, 16)
///*------------------USB Endpoint Flush register-------------------------------*/
//#define ENDPTFLUSH_FERB(value)          BIT_FIELD((value), 0)
//#define ENDPTFLUSH_FERB_MASK            BIT_FIELD(MASK(6), 0)
//#define ENDPTFLUSH_FERB_VALUE(reg) \
//    FIELD_VALUE((reg), ENDPTFLUSH_FERB_MASK, 0)
//#define ENDPTFLUSH_FETB(value)          BIT_FIELD((value), 16)
//#define ENDPTFLUSH_FETB_MASK            BIT_FIELD(MASK(6), 16)
//#define ENDPTFLUSH_FETB_VALUE(reg) \
//    FIELD_VALUE((reg), ENDPTFLUSH_FETB_MASK, 16)
///*------------------USB Endpoint Status register------------------------------*/
//#define ENDPTSTAT_ERBR(value)           BIT_FIELD((value), 0)
//#define ENDPTSTAT_ERBR_MASK             BIT_FIELD(MASK(6), 0)
//#define ENDPTSTAT_ERBR_VALUE(reg) \
//    FIELD_VALUE((reg), ENDPTSTAT_ERBR_MASK, 0)
//#define ENDPTSTAT_ETBR(value)           BIT_FIELD((value), 16)
//#define ENDPTSTAT_ETBR_MASK             BIT_FIELD(MASK(6), 16)
//#define ENDPTSTAT_ETBR_VALUE(reg) \
//    FIELD_VALUE((reg), ENDPTSTAT_ETBR_MASK, 16)
///*------------------USB Endpoint Complete register----------------------------*/
//#define ENDPTCOMPLETE_ERCE(value)       BIT_FIELD((value), 0)
//#define ENDPTCOMPLETE_ERCE_MASK         BIT_FIELD(MASK(6), 0)
//#define ENDPTCOMPLETE_ERCE_VALUE(reg) \
//    FIELD_VALUE((reg), ENDPTCOMPLETE_ERCE_MASK, 0)
//#define ENDPTCOMPLETE_ETCE(value)       BIT_FIELD((value), 16)
//#define ENDPTCOMPLETE_ETCE_MASK         BIT_FIELD(MASK(6), 16)
//#define ENDPTCOMPLETE_ETCE_VALUE(reg) \
//    FIELD_VALUE((reg), ENDPTCOMPLETE_ETCE_MASK, 16)
///*------------------USB Endpoint 0 Control register---------------------------*/
//#define ENDPTCTRL0_RXS                  BIT(0)
//#define ENDPTCTRL0_RXT1_0(value)        BIT_FIELD((value), 2)
//#define ENDPTCTRL0_RXE                  BIT(7)
//#define ENDPTCTRL0_TXS                  BIT(16)
//#define ENDPTCTRL0_TXT1_0(value)        BIT_FIELD((value), 18)
//#define ENDPTCTRL0_TXE                  BIT(23)
///*------------------USB Endpoint 1 to 5 Control registers---------------------*/
/*------------------USB Endpoint Control registers----------------------------*/
#define ENDPTCTRL_RXS                   BIT(0)
#define ENDPTCTRL_RXT(value)            BIT_FIELD((value), 2)
#define ENDPTCTRL_RXI                   BIT(5)
#define ENDPTCTRL_RXR                   BIT(6)
#define ENDPTCTRL_RXE                   BIT(7)
#define ENDPTCTRL_TXS                   BIT(16)
#define ENDPTCTRL_TXT(value)            BIT_FIELD((value), 18)
#define ENDPTCTRL_TXI                   BIT(21)
#define ENDPTCTRL_TXR                   BIT(22)
#define ENDPTCTRL_TXE                   BIT(23)
/*------------------All endpoint registers------------------------------------*/
#define ENDPT_BIT(ep) \
    BIT(((ep) & 0x0F) + (((ep) & 0x80) >> 3))
/*----------------------------------------------------------------------------*/
#define EP_TO_INDEX(ep)     ((((ep) & 0xF) << 1) | (((ep) & 0x80) >> 7))
#define INDEX_TO_EP(index)  ((((index) << 7) & 0x80) | (((index) >> 1) & 0xF))
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC43XX_USB_DEFS_H_ */
