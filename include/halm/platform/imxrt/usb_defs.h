/*
 * halm/platform/imxrt/usb_defs.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_IMXRT_USB_DEFS_H_
#define HALM_PLATFORM_IMXRT_USB_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
#include <stdint.h>
/*------------------General Purpose Timer Load register-----------------------*/
#define GPTIMERLD_GPTLD_MAX             MASK(24)
/*------------------General Purpose Timer Control register--------------------*/
#define GPTIMERCTRL_GPTCNT_MASK         BIT_FIELD(MASK(24), 0)
#define GPTIMERCTRL_GPTCNT(value)       BIT_FIELD((value), 0)
#define GPTIMERCTRL_GPTCNT_VALUE(reg) \
    FIELD_VALUE((reg), GPTIMERCTRL_GPTCNT_MASK, 0)

#define GPTIMERCTRL_GPTMODE             MASK(24)
#define GPTIMERCTRL_GPTRST              MASK(30)
#define GPTIMERCTRL_GPTRUN              MASK(31)
/*------------------System Bus Config register--------------------------------*/
enum
{
  /* Bursts of unspecified length */
  AHB_BRST_INCR_UNSPECIFIED   = 0,

  /* Non-multiple transfers will be decomposed into singles */
  AHB_BRST_INCR4              = 1,
  /* Non-multiple transfers will be decomposed into INCR4 or singles */
  AHB_BRST_INCR8              = 2,
  /* Non-multiple transfers will be decomposed into INCR8, INCR4 or singles */
  AHB_BRST_INCR16             = 3,

  /* Non-multiple transfers will be decomposed into unspecified length bursts */
  AHB_BRST_INCR4_UNSPECIFIED  = 5,
  AHB_BRST_INCR8_UNSPECIFIED  = 6,
  AHB_BRST_INCR16_UNSPECIFIED = 7
};

#define SBUSCFG_AHBBRST(value)          BIT_FIELD((value), 0)
#define SBUSCFG_AHBBRST_MASK            BIT_FIELD(MASK(3), 0)
#define SBUSCFG_AHBBRST_VALUE(reg) \
    FIELD_VALUE((reg), SBUSCFG_AHBBRST_MASK, 0)
/*------------------USB Command register--------------------------------------*/
/* Device mode */
#define USBCMD_D_RS                     BIT(0)
#define USBCMD_D_RST                    BIT(1)
#define USBCMD_D_SUTW                   BIT(13)
#define USBCMD_D_ATDTW                  BIT(14)
#define USBCMD_D_ITC_MASK               BIT_FIELD(MASK(8), 16)
#define USBCMD_D_ITC(value)             BIT_FIELD((value), 16)
#define USBCMD_D_ITC_VALUE(reg) \
    FIELD_VALUE((reg), USBCMD_D_ITC_MASK, 16)

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
#define USBSTS_D_ULPII                  BIT(10)
#define USBSTS_D_NAKI                   BIT(16)
#define USBSTS_D_TI0                    BIT(24)
#define USBSTS_D_TI1                    BIT(25)

/* Host mode */
#define USBSTS_H_UI                     BIT(0)
#define USBSTS_H_UEI                    BIT(1)
#define USBSTS_H_PCI                    BIT(2)
#define USBSTS_H_FRI                    BIT(3)
#define USBSTS_H_SEI                    BIT(4)
#define USBSTS_H_AAI                    BIT(5)
#define USBSTS_H_SRI                    BIT(7)
#define USBSTS_H_ULPII                  BIT(10)
#define USBSTS_H_HCH                    BIT(12)
#define USBSTS_H_RCL                    BIT(13)
#define USBSTS_H_PS                     BIT(14)
#define USBSTS_H_AS                     BIT(15)
#define USBSTS_H_UAI                    BIT(18)
#define USBSTS_H_UPI                    BIT(19)
#define USBSTS_H_TI0                    BIT(24)
#define USBSTS_H_TI1                    BIT(25)
/*------------------USB Interrupt register------------------------------------*/
/* Device mode */
#define USBINTR_D_UE                    BIT(0)
#define USBINTR_D_UEE                   BIT(1)
#define USBINTR_D_PCE                   BIT(2)
#define USBINTR_D_SEE                   BIT(4)
#define USBINTR_D_URE                   BIT(6)
#define USBINTR_D_SRE                   BIT(7)
#define USBINTR_D_SLE                   BIT(8)
#define USBINTR_D_ULPIE                 BIT(10)
#define USBINTR_D_NAKE                  BIT(16)
#define USBINTR_D_TI0                   BIT(24)
#define USBINTR_D_TI1                   BIT(25)

/* Host mode */
#define USBINTR_H_UE                    BIT(0)
#define USBINTR_H_UEE                   BIT(1)
#define USBINTR_H_PCE                   BIT(2)
#define USBINTR_H_FRE                   BIT(3)
#define USBINTR_H_SEE                   BIT(4)
#define USBINTR_H_AEE                   BIT(5)
#define USBINTR_H_SRE                   BIT(7)
#define USBINTR_H_ULPIE                 BIT(10)
#define USBINTR_H_UAIE                  BIT(18)
#define USBINTR_H_UPIA                  BIT(19)
#define USBINTR_H_TI0                   BIT(24)
#define USBINTR_H_TI1                   BIT(25)
/*------------------Device Address register-----------------------------------*/
#define DEVICEADDR_USBADRA              BIT(24)
#define DEVICEADDR_USBADR(value)        BIT_FIELD((value), 25)
#define DEVICEADDR_USBADR_MASK          BIT_FIELD(MASK(7), 25)
#define DEVICEADDR_USBADR_VALUE(reg) \
    FIELD_VALUE((reg), DEVICEADDR_USBADR_MASK, 25)
/*------------------Programmable Burst Size register--------------------------*/
#define BURSTSIZE_RXPBURST(value)       BIT_FIELD((value), 0)
#define BURSTSIZE_RXPBURST_MASK         BIT_FIELD(MASK(8), 0)
#define BURSTSIZE_RXPBURST_VALUE(reg) \
    FIELD_VALUE((reg), BURSTSIZE_RXPBURST_MASK, 0)

#define BURSTSIZE_TXPBURST(value)       BIT_FIELD((value), 8)
#define BURSTSIZE_TXPBURST_MASK         BIT_FIELD(MASK(9), 8)
#define BURSTSIZE_TXPBURST_VALUE(reg) \
    FIELD_VALUE((reg), BURSTSIZE_TXPBURST_MASK, 8)
/*------------------TX FIFO Fill Tuning register------------------------------*/
#define TXFILLTUNING_TXSCHOH(value)       BIT_FIELD((value), 0)
#define TXFILLTUNING_TXSCHOH_MASK         BIT_FIELD(MASK(8), 0)
#define TXFILLTUNING_TXSCHOH_VALUE(reg) \
    FIELD_VALUE((reg), TXFILLTUNING_TXSCHOH_MASK, 0)

#define TXFILLTUNING_TXSCHHEALTH(value)   BIT_FIELD((value), 8)
#define TXFILLTUNING_TXSCHHEALTH_MASK     BIT_FIELD(MASK(5), 8)
#define TXFILLTUNING_TXSCHHEALTH_VALUE(reg) \
    FIELD_VALUE((reg), TXFILLTUNING_TXSCHHEALTH_MASK, 8)

#define TXFILLTUNING_TXFIFOTHRES(value)   BIT_FIELD((value), 16)
#define TXFILLTUNING_TXFIFOTHRES_MASK     BIT_FIELD(MASK(6), 16)
#define TXFILLTUNING_TXFIFOTHRES_VALUE(reg) \
    FIELD_VALUE((reg), TXFILLTUNING_TXFIFOTHRES_MASK, 16)
/*------------------Configure Flash register----------------------------------*/
#define CONFIGFLAG_CF                   BIT(0)
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

#define PORTSC1_D_PIC(value)            BIT_FIELD((value), 14)
#define PORTSC1_D_PIC_MASK              BIT_FIELD(MASK(2), 14)
#define PORTSC1_D_PIC_VALUE(reg) \
    FIELD_VALUE((reg), PORTSC1_D_PIC_MASK, 14)

#define PORTSC1_D_PTC(value)            BIT_FIELD((value), 16)
#define PORTSC1_D_PTC_MASK              BIT_FIELD(MASK(4), 16)
#define PORTSC1_D_PTC_VALUE(reg) \
    FIELD_VALUE((reg), PORTSC1_D_PTC_MASK, 16)

#define PORTSC1_D_PHCD                  BIT(23)
#define PORTSC1_D_PFSC                  BIT(24)
#define PORTSC1_D_PTS_2                 BIT(25)

#define PORTSC1_D_PSPD_MASK             BIT_FIELD(MASK(2), 26)
#define PORTSC1_D_PSPD_VALUE(reg) \
    FIELD_VALUE((reg), PORTSC1_D_PSPD_MASK, 26)

#define PORTSC1_D_PTW                   BIT(28)
#define PORTSC1_D_STS                   BIT(29)
#define PORTSC1_D_PTS_0                 BIT(30)
#define PORTSC1_D_PTS_1                 BIT(31)

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
#define PORTSC1_H_PO                    BIT(13)

#define PORTSC1_H_PIC(value)            BIT_FIELD((value), 14)
#define PORTSC1_H_PIC_MASK              BIT_FIELD(MASK(2), 14)
#define PORTSC1_H_PIC_VALUE(reg) \
    FIELD_VALUE((reg), PORTSC1_H_PIC_MASK, 14)

#define PORTSC1_H_PTC(value)            BIT_FIELD((value), 16)
#define PORTSC1_H_PTC_MASK              BIT_FIELD(MASK(4), 16)
#define PORTSC1_H_PTC_VALUE(reg) \
    FIELD_VALUE((reg), PORTSC1_H_PTC_MASK, 16)

#define PORTSC1_H_WKCN                  BIT(20)
#define PORTSC1_H_WKDC                  BIT(21)
#define PORTSC1_H_WKOC                  BIT(22)
#define PORTSC1_H_PHCD                  BIT(23)
#define PORTSC1_H_PFSC                  BIT(24)
#define PORTSC1_H_PTS_2                 BIT(25)

#define PORTSC1_H_PSPD(value)           BIT_FIELD((value), 26)
#define PORTSC1_H_PSPD_MASK             BIT_FIELD(MASK(2), 26)
#define PORTSC1_H_PSPD_VALUE(reg) \
    FIELD_VALUE((reg), PORTSC1_H_PSPD_MASK, 26)

#define PORTSC1_H_PTW                   BIT(28)
#define PORTSC1_H_STS                   BIT(29)
#define PORTSC1_H_PTS_0                 BIT(30)
#define PORTSC1_H_PTS_1                 BIT(31)
/*------------------OTG Status and Control register---------------------------*/
#define OTGSC_VD                        BIT(0)
#define OTGSC_VC                        BIT(1)
#define OTGSC_OT                        BIT(3)
#define OTGSC_DP                        BIT(4)
#define OTGSC_IDPU                      BIT(5)
#define OTGSC_ID                        BIT(8)
#define OTGSC_AVV                       BIT(9)
#define OTGSC_ASV                       BIT(10)
#define OTGSC_BSV                       BIT(11)
#define OTGSC_BSE                       BIT(12)
#define OTGSC_TOG_1MS                   BIT(13)
#define OTGSC_DPS                       BIT(14)
#define OTGSC_IDIS                      BIT(16)
#define OTGSC_AVVIS                     BIT(17)
#define OTGSC_ASVIS                     BIT(18)
#define OTGSC_BSVIS                     BIT(19)
#define OTGSC_BSEIS                     BIT(20)
#define OTGSC_STATUS_1MS                BIT(21)
#define OTGSC_DPIS                      BIT(22)
#define OTGSC_IDIE                      BIT(24)
#define OTGSC_AVVIE                     BIT(25)
#define OTGSC_ASVIE                     BIT(26)
#define OTGSC_BSVIE                     BIT(27)
#define OTGSC_BSEIE                     BIT(28)
#define OTGSC_EN_1MS                    BIT(29)
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
/*------------------All endpoint-related registers----------------------------*/
#define ENDPT_BIT(ep)                   BIT(((ep) & 0x07) + ((ep) >> 3))
/*------------------OTG Control register--------------------------------------*/
#define OTG_CTRL_OVER_CUR_DIS           BIT(7)
#define OTG_CTRL_OVER_CUR_POL           BIT(8)
#define OTG_CTRL_PWR_POL                BIT(9)
#define OTG_CTRL_WIE                    BIT(10)
#define OTG_CTRL_WKUP_SW_EN             BIT(14)
#define OTG_CTRL_WKUP_SW                BIT(15)
#define OTG_CTRL_WKUP_ID_EN             BIT(16)
#define OTG_CTRL_WKUP_VBUS_EN           BIT(17)
#define OTG_CTRL_WKUP_DPDM_EN           BIT(29)
#define OTG_CTRL_WIR                    BIT(31)
/*------------------OTG UTMI PHY Control register-----------------------------*/
#define OTG_PHY_CTRL_UTMI_CLK_VLD       BIT(31)
/*------------------PHY Power-Down register-----------------------------------*/
#define PHY_PWD_TXPWDFS                 BIT(10)
#define PHY_PWD_TXPWDIBIAS              BIT(11)
#define PHY_PWD_TXPWDV2I                BIT(12)
#define PHY_PWD_RXPWDENV                BIT(17)
#define PHY_PWD_RXPWD1PT1               BIT(18)
#define PHY_PWD_RXPWDDIFF               BIT(19)
#define PHY_PWD_RXPWDRX                 BIT(20)
#define PHY_PWD_MASK \
    (BIT_FIELD(0x7, 10) | BIT_FIELD(0xF, 17))
/*------------------PHY Transmitter control register--------------------------*/
#define PHY_TX_D_CAL(value)             BIT_FIELD((value), 0)
#define PHY_TX_D_CAL_MASK               BIT_FIELD(MASK(4), 0)
#define PHY_TX_D_CAL_VALUE(reg) \
    FIELD_VALUE((reg), PHY_TX_D_CAL_MASK, 0)

#define PHY_TX_TXCAL45DN(value)         BIT_FIELD((value), 8)
#define PHY_TX_TXCAL45DN_MASK           BIT_FIELD(MASK(4), 8)
#define PHY_TX_TXCAL45DN_VALUE(reg) \
    FIELD_VALUE((reg), PHY_TX_TXCAL45DN_MASK, 8)

#define PHY_TX_TXCAL45DP(value)         BIT_FIELD((value), 16)
#define PHY_TX_TXCAL45DP_MASK           BIT_FIELD(MASK(4), 16)
#define PHY_TX_TXCAL45DP_VALUE(reg) \
    FIELD_VALUE((reg), PHY_TX_TXCAL45DP_MASK, 16)

#define PHY_TX_USBPHY_TX_EDGECTRL(value) \
    BIT_FIELD((value), 26)
#define PHY_TX_USBPHY_TX_EDGECTRL_MASK \
    BIT_FIELD(MASK(3), 26)
#define PHY_TX_USBPHY_TX_EDGECTRL_VALUE(reg) \
    FIELD_VALUE((reg), PHY_TX_USBPHY_TX_EDGECTRL_MASK, 26)
/*------------------PHY Receiver control register-----------------------------*/
#define PHY_RX_ENVADJ(value)            BIT_FIELD((value), 0)
#define PHY_RX_ENVADJ_MASK              BIT_FIELD(MASK(3), 0)
#define PHY_RX_ENVADJ_VALUE(reg) \
    FIELD_VALUE((reg), PHY_RX_ENVADJ_MASK, 0)

#define PHY_RX_DISCONADJ(value)         BIT_FIELD((value), 4)
#define PHY_RX_DISCONADJ_MASK           BIT_FIELD(MASK(3), 4)
#define PHY_RX_DISCONADJ_VALUE(reg) \
    FIELD_VALUE((reg), PHY_RX_DISCONADJ_MASK, 4)

#define PHY_RX_RXDBYPASS                BIT(22)
/*------------------PHY General Control register------------------------------*/
#define PHY_CTRL_ENOTG_ID_CHG_IRQ       BIT(0)
#define PHY_CTRL_ENHOSTDISCONDETECT     BIT(1)
#define PHY_CTRL_ENIRQHOSTDISCON        BIT(2)
#define PHY_CTRL_HOSTDISCONDETECT_IRQ   BIT(3)
#define PHY_CTRL_ENDEVPLUGINDETECT      BIT(4)
#define PHY_CTRL_DEVPLUGIN_POLARITY     BIT(5)
#define PHY_CTRL_OTG_ID_CHG_IRQ         BIT(6)
#define PHY_CTRL_ENOTGIDDETECT          BIT(7)
#define PHY_CTRL_RESUMEIRQSTICKY        BIT(8)
#define PHY_CTRL_ENIRQRESUMEDETECT      BIT(9)
#define PHY_CTRL_RESUME_IRQ             BIT(10)
#define PHY_CTRL_ENIRQDEVPLUGIN         BIT(11)
#define PHY_CTRL_DEVPLUGIN_IRQ          BIT(12)
#define PHY_CTRL_DATA_ON_LRADC          BIT(13)
#define PHY_CTRL_ENUTMILEVEL2           BIT(14)
#define PHY_CTRL_ENUTMILEVEL3           BIT(15)
#define PHY_CTRL_ENIRQWAKEUP            BIT(16)
#define PHY_CTRL_WAKEUP_IRQ             BIT(17)
#define PHY_CTRL_ENAUTO_PWRON_PLL       BIT(18)
#define PHY_CTRL_ENAUTOCLR_CLKGATE      BIT(19)
#define PHY_CTRL_ENAUTOCLR_PHY_PWD      BIT(20)
#define PHY_CTRL_ENDPDMCHG_WKUP         BIT(21)
#define PHY_CTRL_ENIDCHG_WKUP           BIT(22)
#define PHY_CTRL_ENVBUSCHG_WKUP         BIT(23)
#define PHY_CTRL_FSDLL_RST_EN           BIT(24)
#define PHY_CTRL_OTG_ID_VALUE           BIT(27)
#define PHY_CTRL_HOST_FORCE_LS_SE0      BIT(28)
#define PHY_CTRL_UTMI_SUSPENDM          BIT(29)
#define PHY_CTRL_CLKGATE                BIT(30)
#define PHY_CTRL_SFTRST                 BIT(31)
/*------------------PHY Status register---------------------------------------*/
#define PHY_STATUS_HOSTDISCONDETECT_STATUS \
    BIT(3)
#define PHY_STATUS_DEVPLUGIN_STATUS     BIT(6)
#define PHY_STATUS_OTGID_STATUS         BIT(8)
#define PHY_STATUS_RESUME_STATUS        BIT(10)
/*------------------PHY Debug register----------------------------------------*/
#define PHY_DEBUG_OTGIDPIOLOCK          BIT(0)
#define PHY_DEBUG_DEBUG_INTERFACE_HOLD  BIT(1)

#define PHY_DEBUG_HSTPULLDOWN(value)    BIT_FIELD((value), 2)
#define PHY_DEBUG_HSTPULLDOWN_MASK      BIT_FIELD(MASK(2), 2)
#define PHY_DEBUG_HSTPULLDOWN_VALUE(reg) \
    FIELD_VALUE((reg), PHY_DEBUG_HSTPULLDOWN_MASK, 2)

#define PHY_DEBUG_ENHSTPULLDOWN(value)  BIT_FIELD((value), 4)
#define PHY_DEBUG_ENHSTPULLDOWN_MASK    BIT_FIELD(MASK(2), 4)
#define PHY_DEBUG_ENHSTPULLDOWN_VALUE(reg) \
    FIELD_VALUE((reg), PHY_DEBUG_ENHSTPULLDOWN_MASK, 4)

#define PHY_DEBUG_TX2RXCOUNT(value)     BIT_FIELD((value), 8)
#define PHY_DEBUG_TX2RXCOUNT_MASK       BIT_FIELD(MASK(4), 8)
#define PHY_DEBUG_TX2RXCOUNT_VALUE(reg) \
    FIELD_VALUE((reg), PHY_DEBUG_TX2RXCOUNT_MASK, 8)

#define PHY_DEBUG_ENTX2RXCOUNT          BIT(0)

#define PHY_DEBUG_SQUELCHRESETCOUNT(value) \
    BIT_FIELD((value), 16)
#define PHY_DEBUG_SQUELCHRESETCOUNT_MASK \
    BIT_FIELD(MASK(5), 16)
#define PHY_DEBUG_SQUELCHRESETCOUNT_VALUE(reg) \
    FIELD_VALUE((reg), PHY_DEBUG_SQUELCHRESETCOUNT_MASK, 16)

#define PHY_DEBUG_ENSQUELCHRESET        BIT(24)

#define PHY_DEBUG_SQUELCHRESETLENGTH(value) \
    BIT_FIELD((value), 25)
#define PHY_DEBUG_SQUELCHRESETLENGTH_MASK \
    BIT_FIELD(MASK(4), 25)
#define PHY_DEBUG_SQUELCHRESETLENGTH_VALUE(reg) \
    FIELD_VALUE((reg), PHY_DEBUG_TX2RXCOUNT_MASK, 25)

#define PHY_DEBUG_HOST_RESUME_DEBUG     BIT(29)
#define PHY_DEBUG_CLKGATE               BIT(30)
/*------------------UTMI Debug Status register 0------------------------------*/
#define PHY_DEBUG0_LOOP_BACK_FAIL_COUNT(value) \
    BIT_FIELD((value), 0)
#define PHY_DEBUG0_LOOP_BACK_FAIL_COUNT_MASK \
    BIT_FIELD(MASK(16), 0)
#define PHY_DEBUG0_LOOP_BACK_FAIL_COUNT_VALUE(reg) \
    FIELD_VALUE((reg), PHY_DEBUG0_LOOP_BACK_FAIL_COUNT_MASK, 0)

#define PHY_DEBUG0_UTMI_RXERROR_FAIL_COUNT(value) \
    BIT_FIELD((value), 16)
#define PHY_DEBUG0_UTMI_RXERROR_FAIL_COUNT_MASK \
    BIT_FIELD(MASK(10), 16)
#define PHY_DEBUG0_UTMI_RXERROR_FAIL_COUNT_VALUE(reg) \
    FIELD_VALUE((reg), PHY_DEBUG0_UTMI_RXERROR_FAIL_COUNT_MASK, 16)

#define PHY_DEBUG_SQUELCH_COUNT(value) \
    BIT_FIELD((value), 26)
#define PHY_DEBUG_SQUELCH_COUNT_MASK \
    BIT_FIELD(MASK(6), 26)
#define PHY_DEBUG_SQUELCH_COUNT_VALUE(reg) \
    FIELD_VALUE((reg), PHY_DEBUG_SQUELCH_COUNT_MASK, 26)
/*------------------UTMI Debug Status register 1------------------------------*/
enum
{
  ENTAILADJVD_NOMINAL   = 0,
  ENTAILADJVD_PLUS_20P  = 1,
  ENTAILADJVD_MINUS_20P = 2,
  ENTAILADJVD_MINUS_40P = 3
};

#define PHY_DEBUG1_ENTAILADJVD(value)   BIT_FIELD((value), 13)
#define PHY_DEBUG1_ENTAILADJVD_MASK     BIT_FIELD(MASK(2), 13)
#define PHY_DEBUG1_ENTAILADJVD_VALUE(reg) \
    FIELD_VALUE((reg), PHY_DEBUG1_ENTAILADJVD_MASK, 13)
/*------------------ANALOG VBUS Detect register-------------------------------*/
enum
{
  VBUSVALID_THRESH_4V0  = 0,
  VBUSVALID_THRESH_4V1  = 1,
  VBUSVALID_THRESH_4V2  = 2,
  VBUSVALID_THRESH_4V3  = 3,
  VBUSVALID_THRESH_4V4  = 4,
  VBUSVALID_THRESH_4V5  = 5,
  VBUSVALID_THRESH_4V6  = 6,
  VBUSVALID_THRESH_4V7  = 7
};

#define VBUS_DETECT_VBUSVALID_THRESH(value) \
    BIT_FIELD((value), 0)
#define VBUS_DETECT_VBUSVALID_THRESH_MASK \
    BIT_FIELD(MASK(3), 0)
#define VBUS_DETECT_VBUSVALID_THRESH_VALUE(reg) \
    FIELD_VALUE((reg), VBUS_DETECT_VBUSVALID_THRESH_MASK, 0)

#define VBUS_DETECT_VBUSVALID_PWRUP_CMPS \
    BIT(20)

#define VBUS_DETECT_DISCHARGE_VBUS      BIT(26)
#define VBUS_DETECT_CHARGE_VBUS         BIT(27)
/*------------------ANALOG Charger Detect register----------------------------*/
#define CHRG_DETECT_CHK_CONTACT         BIT(18)
#define CHRG_DETECT_CHK_CHRG_B          BIT(19)
#define CHRG_DETECT_EN_B                BIT(20)
/*------------------ANALOG VBUS Detect Status register------------------------*/
#define VBUS_DETECT_STAT_SESSEND        BIT(0)
#define VBUS_DETECT_STAT_BVALID         BIT(1)
#define VBUS_DETECT_STAT_AVALID         BIT(2)
#define VBUS_DETECT_STAT_VBUS_VALID     BIT(3)
/*------------------ANALOG Charger Detect Status register---------------------*/
#define CHRG_DETECT_STAT_PLUG_CONTACT   BIT(0)
#define CHRG_DETECT_STAT_CHRG_DETECTED  BIT(1)
#define CHRG_DETECT_STAT_DM_STATE       BIT(2)
#define CHRG_DETECT_STAT_DP_STATE       BIT(3)
/*----------------------------------------------------------------------------*/
enum
{
  TOKEN_STATUS_TRANSACTION_ERROR  = 0x08,
  TOKEN_STATUS_BUFFER_ERROR       = 0x20,
  TOKEN_STATUS_HALTED             = 0x40,
  TOKEN_STATUS_ACTIVE             = 0x80,
};

struct TransferDescriptor
{
  volatile uint32_t next;
  volatile uint32_t token;
  volatile uint32_t buffer0;
  volatile uint32_t buffer1;
  volatile uint32_t buffer2;
  volatile uint32_t buffer3;
  volatile uint32_t buffer4;

  /* Project-specific fields */
  volatile uint32_t listNode;
};

struct QueueHead
{
  volatile uint32_t capabilities;
  volatile uint32_t current;
  volatile uint32_t next;
  volatile uint32_t token;
  volatile uint32_t buffer0;
  volatile uint32_t buffer1;
  volatile uint32_t buffer2;
  volatile uint32_t buffer3;
  volatile uint32_t buffer4;
  volatile uint32_t reserved;
  volatile uint32_t setup[2];

  /* Project-specific fields */
  volatile uint32_t listHead;
  volatile uint32_t listTail;

  volatile uint32_t gap[2];
};

#define QH_IOS                          BIT(15)
#define QH_MAX_PACKET_LENGTH(value)     BIT_FIELD((value), 16)
#define QH_MAX_PACKET_LENGTH_MASK       BIT_FIELD(MASK(10), 16)
#define QH_MAX_PACKET_LENGTH_VALUE(reg) \
    FIELD_VALUE((reg), QH_MAX_PACKET_LENGTH_MASK, 16)
#define QH_ZLT                          BIT(29)
#define QH_MULT(value)                  BIT_FIELD((value), 30)
#define QH_MULT_MASK                    BIT_FIELD(MASK(2), 30)
#define QH_MULT_VALUE(reg)              FIELD_VALUE((reg), QH_MULT_MASK, 30)

#define TD_NEXT_TERMINATE               BIT(0)

#define TD_TOKEN_STATUS(value)          BIT_FIELD((value), 0)
#define TD_TOKEN_STATUS_MASK            BIT_FIELD(MASK(8), 0)
#define TD_TOKEN_STATUS_VALUE(reg) \
    FIELD_VALUE((reg), TD_TOKEN_STATUS_MASK, 0)
#define TD_TOKEN_MULTO(value)           BIT_FIELD((value), 10)
#define TD_TOKEN_IOC                    BIT(15)
#define TD_TOKEN_TOTAL_BYTES(value)     BIT_FIELD((value), 16)
#define TD_TOKEN_TOTAL_BYTES_MASK       BIT_FIELD(MASK(15), 16)
#define TD_TOKEN_TOTAL_BYTES_VALUE(reg) \
    FIELD_VALUE((reg), TD_TOKEN_TOTAL_BYTES_MASK, 16)
/*----------------------------------------------------------------------------*/
#define EP_TO_DESCRIPTOR_NUMBER(ep) \
    ((((ep) & 0x0F) << 1) | (((ep) & 0x80) >> 7))
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_IMXRT_USB_DEFS_H_ */
