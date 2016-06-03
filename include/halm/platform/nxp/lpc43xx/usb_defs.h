/*
 * halm/platform/nxp/lpc43xx/usb_defs.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC43XX_USB_DEFS_H_
#define HALM_PLATFORM_NXP_LPC43XX_USB_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <xcore/bits.h>
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
#define PORTSC1_D_PSPD_MASK             BIT_FIELD(MASK(2), 26)
#define PORTSC1_D_PSPD_VALUE(reg) \
    FIELD_VALUE((reg), PORTSC1_D_PSPD_MASK, 26)

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
#define ENDPT_BIT(ep)                   BIT(((ep) & 0x07) + ((ep) >> 3))
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
/*------------------System Bus interface configuration register---------------*/
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

#define SBUSCFG_AHB_BRST(value)         BIT_FIELD((value), 0)
#define SBUSCFG_AHB_BRST_MASK           BIT_FIELD(MASK(3), 0)
#define SBUSCFG_AHB_BRST_VALUE(reg) \
    FIELD_VALUE((reg), SBUSCFG_AHB_BRST_MASK, 0)
/*------------------USB1 pin configuration register---------------------------*/
#define SFSUSB_AIM                      BIT(0)
#define SFSUSB_ESEA                     BIT(1)
#define SFSUSB_EPD                      BIT(2)
#define SFSUSB_EPWR                     BIT(4)
#define SFSUSB_VBUS                     BIT(5)
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
#endif /* HALM_PLATFORM_NXP_LPC43XX_USB_DEFS_H_ */
