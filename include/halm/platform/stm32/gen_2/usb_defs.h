/*
 * halm/platform/stm32/gen_2/usb_defs.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_GEN_2_USB_DEFS_H_
#define HALM_PLATFORM_STM32_GEN_2_USB_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Global control and status register------------------------*/
#define GOTGCTL_SRQSCS                  BIT(0)
#define GOTGCTL_SRQ                     BIT(1)
#define GOTGCTL_HNGSCS                  BIT(8)
#define GOTGCTL_HNPRQ                   BIT(9)
#define GOTGCTL_HSHNPEN                 BIT(10)
#define GOTGCTL_DHNPEN                  BIT(11)
#define GOTGCTL_CIDSTS                  BIT(16)
#define GOTGCTL_DBCT                    BIT(17)
#define GOTGCTL_ASVLD                   BIT(18)
#define GOTGCTL_BSVLD                   BIT(19)
/*------------------Global Interrupt register---------------------------------*/
#define GOTGINT_SEDET                   BIT(2)
#define GOTGINT_SRSSCHG                 BIT(8)
#define GOTGINT_HNSSCHG                 BIT(9)
#define GOTGINT_HNGDET                  BIT(17)
#define GOTGINT_ADTOCHG                 BIT(18)
#define GOTGINT_DBCDNE                  BIT(19)
/*------------------Global AHB Configuration register-------------------------*/
enum
{
  HBSTLEN_SINGLE  = 0,
  HBSTLEN_INCR    = 1,
  HBSTLEN_INCR4   = 3,
  HBSTLEN_INCR8   = 5,
  HBSTLEN_INCR16  = 7
};

#define GAHBCFG_GINT                    BIT(0)

#define GAHBCFG_HBSTLEN_MASK            BIT_FIELD(MASK(4), 1)
#define GAHBCFG_HBSTLEN(value)          BIT_FIELD((value), 1)
#define GAHBCFG_HBSTLEN_VALUE(reg) \
    FIELD_VALUE((reg), GAHBCFG_HBSTLEN_MASK, 1)

#define GAHBCFG_DMAEN                   BIT(5)
#define GAHBCFG_TXFELVL                 BIT(7)
#define GAHBCFG_PTXFELVL                BIT(8)
/*------------------Global USB Configuration register-------------------------*/
#define GUSBCFG_TOCAL_MASK              BIT_FIELD(MASK(3), 0)
#define GUSBCFG_TOCAL(value)            BIT_FIELD((value), 0)
#define GUSBCFG_TOCAL_VALUE(reg) \
    FIELD_VALUE((reg), GUSBCFG_TOCAL_MASK, 0)

#define GUSBCFG_PHYSEL                  BIT(7)
#define GUSBCFG_SRPCAP                  BIT(8)
#define GUSBCFG_HNPCAP                  BIT(9)

#define GUSBCFG_TRDT_MASK               BIT_FIELD(MASK(4), 10)
#define GUSBCFG_TRDT(value)             BIT_FIELD((value), 10)
#define GUSBCFG_TRDT_VALUE(reg) \
    FIELD_VALUE((reg), GUSBCFG_TRDT_MASK, 10)

#define GUSBCFG_PHYLPCS                 BIT(15)
#define GUSBCFG_ULPIFSLS                BIT(17)
#define GUSBCFG_ULPIAR                  BIT(18)
#define GUSBCFG_ULPICSM                 BIT(19)
#define GUSBCFG_ULPIEVBUSD              BIT(20)
#define GUSBCFG_ULPIEVBUSI              BIT(21)
#define GUSBCFG_TSDPS                   BIT(22)
#define GUSBCFG_PCCI                    BIT(23)
#define GUSBCFG_PTCI                    BIT(24)
#define GUSBCFG_ULPIIPD                 BIT(25)
#define GUSBCFG_FHMOD                   BIT(29)
#define GUSBCFG_FDMOD                   BIT(30)
#define GUSBCFG_CTXPKT                  BIT(31)
/*------------------Global Reset register-------------------------------------*/
#define GRSTCTL_CSRST                   BIT(0)
#define GRSTCTL_HSRST                   BIT(1)
#define GRSTCTL_FCRST                   BIT(2)
#define GRSTCTL_RXFFLSH                 BIT(4)
#define GRSTCTL_TXFFLSH                 BIT(5)

#define GRSTCTL_TXFNUM_ALL              16
#define GRSTCTL_TXFNUM_MASK             BIT_FIELD(MASK(5), 6)
#define GRSTCTL_TXFNUM(value)           BIT_FIELD((value), 6)
#define GRSTCTL_TXFNUM_VALUE(reg) \
    FIELD_VALUE((reg), GRSTCTL_TXFNUM_MASK, 6)

#define GRSTCTL_DMAREQ                  BIT(30)
#define GRSTCTL_AHBIDL                  BIT(31)
/*------------------Global Interrupt Status register--------------------------*/
#define GINTSTS_CMOD                    BIT(0)
#define GINTSTS_MMIS                    BIT(1)
#define GINTSTS_OTGINT                  BIT(2)
#define GINTSTS_SOF                     BIT(3)
#define GINTSTS_RXFLVL                  BIT(4)
#define GINTSTS_NPTXFE                  BIT(5)
#define GINTSTS_GINAKEFF                BIT(6)
#define GINTSTS_GONAKEFF                BIT(7)
#define GINTSTS_ESUSP                   BIT(10)
#define GINTSTS_USBSUSP                 BIT(11)
#define GINTSTS_USBRST                  BIT(12)
#define GINTSTS_ENUMDNE                 BIT(13)
#define GINTSTS_ISOODRP                 BIT(14)
#define GINTSTS_EOPF                    BIT(15)
#define GINTSTS_IEPINT                  BIT(18)
#define GINTSTS_OEPINT                  BIT(19)
#define GINTSTS_IISOIXFR                BIT(20)
#define GINTSTS_IPXFR                   BIT(21)
#define GINTSTS_IISOOXFR                GINTSTS_IPXFR
#define GINTSTS_DATAFSUSP               BIT(22)
#define GINTSTS_HPRTINT                 BIT(24)
#define GINTSTS_HCINT                   BIT(25)
#define GINTSTS_PTXFE                   BIT(26)
#define GINTSTS_LPMINT                  BIT(27)
#define GINTSTS_CIDSCHG                 BIT(28)
#define GINTSTS_DISCINT                 BIT(29)
#define GINTSTS_SRQINT                  BIT(30)
#define GINTSTS_WKUPINT                 BIT(31)
/*------------------Global Interrupt Mask register----------------------------*/
#define GINTMSK_MMISM                   BIT(1)
#define GINTMSK_OTGINT                  BIT(2)
#define GINTMSK_SOFM                    BIT(3)
#define GINTMSK_RXFLVLM                 BIT(4)
#define GINTMSK_NPTXFEM                 BIT(5)
#define GINTMSK_GINAKEFFM               BIT(6)
#define GINTMSK_GONAKEFFM               BIT(7)
#define GINTMSK_ESUSPM                  BIT(10)
#define GINTMSK_USBSUSPM                BIT(11)
#define GINTMSK_USBRST                  BIT(12)
#define GINTMSK_ENUMDNEM                BIT(13)
#define GINTMSK_ISOODRPM                BIT(14)
#define GINTMSK_EOPFM                   BIT(15)
#define GINTMSK_IEPINT                  BIT(18)
#define GINTMSK_OEPINT                  BIT(19)
#define GINTMSK_IISOIXFRM               BIT(20)
#define GINTMSK_IPXFRM                  BIT(21)
#define GINTMSK_IISOOXFRM               GINTMSK_IPXFRM
#define GINTMSK_FSUSPM                  BIT(22)
#define GINTMSK_PRTIM                   BIT(24)
#define GINTMSK_HCIM                    BIT(25)
#define GINTMSK_PTXFEM                  BIT(26)
#define GINTMSK_CIDSCHGM                BIT(28)
#define GINTMSK_DISCINT                 BIT(29)
#define GINTMSK_SRQIM                   BIT(30)
#define GINTMSK_WUIM                    BIT(31)
/*------------------Receive status debug read/read and pop registers----------*/
enum
{
  DPID_DATA0,
  DPID_DATA1,
  DPID_DATA2,
  DPID_MDATA
};

enum
{
  /* Host mode */
  PKTSTS_IN_PACKET_RECEIVED       = 2,
  PKTSTS_IN_TRANSFER_COMPLETED    = 3,
  PKTSTS_DATA_TOGGLE_ERROR        = 5,
  PKTSTS_CHANNEL_HALTED           = 7,

  /* Device mode */
  PKTSTS_OUT_NAK                  = 1,
  PKTSTS_OUT_PACKET_RECEIVED      = 2,
  PKTSTS_OUT_TRANSFER_COMPLETED   = 3,
  PKTSTS_SETUP_TRANSFER_COMPLETED = 4,
  PKTSTS_SETUP_PACKET_RECEIVED    = 6
};

/* Host mode */
#define GRXSTSR_CHNUM_MASK              BIT_FIELD(MASK(4), 0)
#define GRXSTSR_CHNUM(value)            BIT_FIELD((value), 0)
#define GRXSTSR_CHNUM_VALUE(reg) \
    FIELD_VALUE((reg), GRXSTSR_CHNUM_MASK, 0)

/* Device mode */
#define GRXSTSR_EPNUM_MASK              BIT_FIELD(MASK(4), 0)
#define GRXSTSR_EPNUM(value)            BIT_FIELD((value), 0)
#define GRXSTSR_EPNUM_VALUE(reg) \
    FIELD_VALUE((reg), GRXSTSR_EPNUM_MASK, 0)

#define GRXSTSR_BCNT_MASK               BIT_FIELD(MASK(11), 4)
#define GRXSTSR_BCNT(value)             BIT_FIELD((value), 4)
#define GRXSTSR_BCNT_VALUE(reg) \
    FIELD_VALUE((reg), GRXSTSR_BCNT_MASK, 4)

#define GRXSTSR_DPID_MASK               BIT_FIELD(MASK(2), 15)
#define GRXSTSR_DPID(value)             BIT_FIELD((value), 15)
#define GRXSTSR_DPID_VALUE(reg) \
    FIELD_VALUE((reg), GRXSTSR_DPID_MASK, 15)

#define GRXSTSR_PKTSTS_MASK             BIT_FIELD(MASK(4), 17)
#define GRXSTSR_PKTSTS(value)           BIT_FIELD((value), 17)
#define GRXSTSR_PKTSTS_VALUE(reg) \
    FIELD_VALUE((reg), GRXSTSR_PKTSTS_MASK, 17)

/* Device mode */
#define GRXSTSR_FRMNUM_MASK             BIT_FIELD(MASK(4), 21)
#define GRXSTSR_FRMNUM(value)           BIT_FIELD((value), 21)
#define GRXSTSR_FRMNUM_VALUE(reg) \
    FIELD_VALUE((reg), GRXSTSR_FRMNUM_MASK, 21)
/*------------------Nonperiodic TX FIFO size/EP0 TX FIFO size register--------*/
/* Host mode */
#define GNPTXFSIZ_NPTXFSA_MASK          BIT_FIELD(MASK(16), 0)
#define GNPTXFSIZ_NPTXFSA(value)        BIT_FIELD((value), 0)
#define GNPTXFSIZ_NPTXFSA_VALUE(reg) \
    FIELD_VALUE((reg), GNPTXFSIZ_NPTXFSA_MASK, 0)

/* Device mode */
#define TX0FSIZ_TX0FSA_MASK             BIT_FIELD(MASK(16), 0)
#define TX0FSIZ_TX0FSA(value)           BIT_FIELD((value), 0)
#define TX0FSIZ_TX0FSA_VALUE(reg) \
    FIELD_VALUE((reg), TX0FSIZ_TX0FSA_MASK, 0)

/* Host mode */
#define GNPTXFSIZ_NPTXFD_MASK           BIT_FIELD(MASK(16), 16)
#define GNPTXFSIZ_NPTXFD(value)         BIT_FIELD((value), 16)
#define GNPTXFSIZ_NPTXFD_VALUE(reg) \
    FIELD_VALUE((reg), GNPTXFSIZ_NPTXFD_MASK, 16)

/* Device mode */
#define TX0FSIZ_TX0FD_MASK              BIT_FIELD(MASK(16), 16)
#define TX0FSIZ_TX0FD(value)            BIT_FIELD((value), 16)
#define TX0FSIZ_TX0FD_VALUE(reg) \
    FIELD_VALUE((reg), TX0FSIZ_TX0FD_MASK, 16)
/*------------------Nonperiodic TX FIFO/queue status register-----------------*/
#define GNPTXSTS_NPTXFSAV_MASK          BIT_FIELD(MASK(16), 0)
#define GNPTXSTS_NPTXFSAV(value)        BIT_FIELD((value), 0)
#define GNPTXSTS_NPTXFSAV_VALUE(reg) \
    FIELD_VALUE((reg), GNPTXSTS_NPTXFSAV_MASK, 0)

#define GNPTXSTS_NPTXQSAV_MASK          BIT_FIELD(MASK(8), 16)
#define GNPTXSTS_NPTXQSAV(value)        BIT_FIELD((value), 16)
#define GNPTXSTS_NPTXQSAV_VALUE(reg) \
    FIELD_VALUE((reg), GNPTXSTS_NPTXQSAV_MASK, 16)

#define GNPTXSTS_NPTXQTOP_MASK          BIT_FIELD(MASK(8), 24)
#define GNPTXSTS_NPTXQTOP(value)        BIT_FIELD((value), 24)
#define GNPTXSTS_NPTXQTOP_VALUE(reg) \
    FIELD_VALUE((reg), GNPTXSTS_NPTXQTOP_MASK, 24)
/*------------------Global I2C access register--------------------------------*/
#define GI2CCTL_RWDATA_MASK             BIT_FIELD(MASK(8), 0)
#define GI2CCTL_RWDATA(value)           BIT_FIELD((value), 0)
#define GI2CCTL_RWDATA_VALUE(reg) \
    FIELD_VALUE((reg), GI2CCTL_RWDATA_MASK, 0)

#define GI2CCTL_REGADDR_MASK            BIT_FIELD(MASK(8), 8)
#define GI2CCTL_REGADDR(value)          BIT_FIELD((value), 8)
#define GI2CCTL_REGADDR_VALUE(reg) \
    FIELD_VALUE((reg), GI2CCTL_REGADDR_MASK, 8)

#define GI2CCTL_ADDR_MASK               BIT_FIELD(MASK(7), 16)
#define GI2CCTL_ADDR(value)             BIT_FIELD((value), 16)
#define GI2CCTL_ADDR_VALUE(reg) \
    FIELD_VALUE((reg), GI2CCTL_ADDR_MASK, 16)

#define GI2CCTL_I2CEN                   BIT(23)
#define GI2CCTL_ACK                     BIT(24)

#define GI2CCTL_I2CDEVADR_MASK          BIT_FIELD(MASK(2), 26)
#define GI2CCTL_I2CDEVADR(value)        BIT_FIELD((value), 26)
#define GI2CCTL_I2CDEVADR_VALUE(reg) \
    FIELD_VALUE((reg), GI2CCTL_I2CDEVADR_MASK, 26)

#define GI2CCTL_I2CDATSE0               BIT(28)
#define GI2CCTL_RW                      BIT(30)
#define GI2CCTL_BSYDNE                  BIT(31)
/*------------------General Core Configuration register-----------------------*/
#define GCCFG_PWRDWN                    BIT(16)
#define GCCFG_I2CPADEN                  BIT(17)
#define GCCFG_VBUSASEN                  BIT(18)
#define GCCFG_VBUSBSEN                  BIT(19)
#define GCCFG_SOFOUTEN                  BIT(20)
#define GCCFG_NOVBUSSENS                BIT(21)
/*------------------Host periodic TX FIFO size register-----------------------*/
#define HPTXFSIZ_PTXFSA_MASK            BIT_FIELD(MASK(16), 0)
#define HPTXFSIZ_PTXFSA(value)          BIT_FIELD((value), 0)
#define HPTXFSIZ_PTXFSA_VALUE(reg) \
    FIELD_VALUE((reg), HPTXFSIZ_PTXFSA_MASK, 0)

#define HPTXFSIZ_PTXFD_MASK             BIT_FIELD(MASK(16), 16)
#define HPTXFSIZ_PTXFD(value)           BIT_FIELD((value), 16)
#define HPTXFSIZ_PTXFD_VALUE(reg) \
    FIELD_VALUE((reg), HPTXFSIZ_PTXFD_MASK, 16)
/*------------------Device IN endpoint TX FIFO size registers-----------------*/
#define DIEPTXF_INEPTXSA_MASK           BIT_FIELD(MASK(16), 0)
#define DIEPTXF_INEPTXSA(value)         BIT_FIELD((value), 0)
#define DIEPTXF_INEPTXSA_VALUE(reg) \
    FIELD_VALUE((reg), DIEPTXF_INEPTXSA_MASK, 0)

#define DIEPTXF_INEPTXFD_MASK           BIT_FIELD(MASK(16), 16)
#define DIEPTXF_INEPTXFD(value)         BIT_FIELD((value), 16)
#define DIEPTXF_INEPTXFD_VALUE(reg) \
    FIELD_VALUE((reg), DIEPTXF_INEPTXFD_MASK, 16)
#define DIEPTXF_INEPTXFD_MIN            16
/*------------------Host Configuration register-------------------------------*/
enum
{
  FSLSPCS_48MHZ = 1
};

#define HCFG_FSLSPCS_MASK               BIT_FIELD(MASK(2), 0)
#define HCFG_FSLSPCS(value)             BIT_FIELD((value), 0)
#define HCFG_FSLSPCS_VALUE(reg)         FIELD_VALUE((reg), HCFG_FSLSPCS_MASK, 0)

#define HCFG_FSLSS                      BIT(2)
/*------------------Host frame number/frame time register---------------------*/
#define HFNUM_FRNUM_MASK                BIT_FIELD(MASK(16), 0)
#define HFNUM_FRNUM(value)              BIT_FIELD((value), 0)
#define HFNUM_FRNUM_VALUE(reg)          FIELD_VALUE((reg), HFNUM_FRNUM_MASK, 0)

#define HFNUM_FTREM_MASK                BIT_FIELD(MASK(16), 16)
#define HFNUM_FTREM(value)              BIT_FIELD((value), 16)
#define HFNUM_FTREM_VALUE(reg)          FIELD_VALUE((reg), HFNUM_FTREM_MASK, 16)
/*------------------Host periodic TX FIFO/queue status register---------------*/
#define HPTXSTS_PTXFSAV_MASK            BIT_FIELD(MASK(16), 0)
#define HPTXSTS_PTXFSAV(value)          BIT_FIELD((value), 0)
#define HPTXSTS_PTXFSAV_VALUE(reg) \
    FIELD_VALUE((reg), HPTXSTS_PTXFSAV_MASK, 0)

#define HPTXSTS_PTXQSAV_MASK            BIT_FIELD(MASK(8), 16)
#define HPTXSTS_PTXQSAV(value)          BIT_FIELD((value), 16)
#define HPTXSTS_PTXQSAV_VALUE(reg) \
    FIELD_VALUE((reg), HPTXSTS_PTXQSAV_MASK, 16)

#define HPTXSTS_PTXQTOP_MASK            BIT_FIELD(MASK(8), 24)
#define HPTXSTS_PTXQTOP(value)          BIT_FIELD((value), 24)
#define HPTXSTS_PTXQTOP_VALUE(reg) \
    FIELD_VALUE((reg), HPTXSTS_PTXQTOP_MASK, 24)
/*------------------Host all channels interrupt register----------------------*/
#define HAINT_CH(channel)               BIT(channel)
/*------------------Host all channels interrupt mask register-----------------*/
#define HAINTMSK_CH(channel)            BIT(channel)
/*------------------Host port control and status register---------------------*/
enum
{
  PTCTL_DISABLED          = 0,
  PTCTL_TEST_J            = 1,
  PTCTL_TEST_K            = 2,
  PTCTL_TEST_SE0_NAK      = 3,
  PTCTL_TEST_PACKET       = 4,
  PTCTL_TEST_FORCE_ENABLE = 5
};

#define HPRT_PCSTS                      BIT(0)
#define HPRT_PCDET                      BIT(1)
#define HPRT_PENA                       BIT(2)
#define HPRT_PENCHNG                    BIT(3)
#define HPRT_POCA                       BIT(4)
#define HPRT_OICCHNG                    BIT(5)
#define HPRT_PRES                       BIT(6)
#define HPRT_PSUSP                      BIT(7)
#define HPRT_PRST                       BIT(8)

#define HPRT_PLSTS_DP                   BIT(10)
#define HPRT_PLSTS_DM                   BIT(11)

#define HPRT_PLSTS_MASK                 BIT_FIELD(MASK(2), 10)
#define HPRT_PLSTS(value)               BIT_FIELD((value), 10)
#define HPRT_PLSTS_VALUE(reg)           FIELD_VALUE((reg), HPRT_PLSTS_MASK, 10)

#define HPRT_PPWR                       BIT(12)

#define HPRT_PTCTL_MASK                 BIT_FIELD(MASK(4), 13)
#define HPRT_PTCTL(value)               BIT_FIELD((value), 13)
#define HPRT_PTCTL_VALUE(reg)           FIELD_VALUE((reg), HPRT_PTCTL_MASK, 13)

#define HPRT_PSPD_MASK                  BIT_FIELD(MASK(2), 17)
#define HPRT_PSPD(value)                BIT_FIELD((value), 17)
#define HPRT_PSPD_VALUE(reg)            FIELD_VALUE((reg), HPRT_PSPD_MASK, 17)
/*------------------Host Channel Characteristics registers--------------------*/
#define HCCHAR_MPSIZ_MASK               BIT_FIELD(MASK(11), 0)
#define HCCHAR_MPSIZ(value)             BIT_FIELD((value), 0)
#define HCCHAR_MPSIZ_VALUE(reg) \
    FIELD_VALUE((reg), HCCHAR_MPSIZ_MASK, 0)

#define HCCHAR_EPNUM_MASK               BIT_FIELD(MASK(4), 11)
#define HCCHAR_EPNUM(value)             BIT_FIELD((value), 11)
#define HCCHAR_EPNUM_VALUE(reg) \
    FIELD_VALUE((reg), HCCHAR_EPNUM_MASK, 11)

#define HCCHAR_EPDIR_OUT                BIT(15)
#define HCCHAR_EPDIR_IN                 0

#define HCCHAR_LSDEV                    BIT(17)

#define HCCHAR_EPTYP_MASK               BIT_FIELD(MASK(2), 18)
#define HCCHAR_EPTYP(value)             BIT_FIELD((value), 18)
#define HCCHAR_EPTYP_VALUE(reg) \
    FIELD_VALUE((reg), HCCHAR_EPTYP_MASK, 18)

#define HCCHAR_MC_MASK                  BIT_FIELD(MASK(2), 20)
#define HCCHAR_MC(value)                BIT_FIELD((value), 20)
#define HCCHAR_MC_VALUE(reg)            FIELD_VALUE((reg), HCCHAR_MC_MASK, 20)

#define HCCHAR_DAD_MASK                 BIT_FIELD(MASK(7), 22)
#define HCCHAR_DAD(value)               BIT_FIELD((value), 22)
#define HCCHAR_DAD_VALUE(reg)           FIELD_VALUE((reg), HCCHAR_DAD_MASK, 22)

#define HCCHAR_ODDFRM                   BIT(29)
#define HCCHAR_CHDIS                    BIT(30)
#define HCCHAR_CHENA                    BIT(31)
/*------------------Host Channel Split control registers----------------------*/
#define HCSPLT_PRTADDR_MASK             BIT_FIELD(MASK(7), 0)
#define HCSPLT_PRTADDR(value)           BIT_FIELD((value), 0)
#define HCSPLT_PRTADDR_VALUE(reg) \
    FIELD_VALUE((reg), HCSPLT_PRTADDR_MASK, 0)

#define HCSPLT_HUBADDR_MASK             BIT_FIELD(MASK(7), 7)
#define HCSPLT_HUBADDR(value)           BIT_FIELD((value), 7)
#define HCSPLT_HUBADDR_VALUE(reg) \
    FIELD_VALUE((reg), HCSPLT_HUBADDR_MASK, 7)

#define HCSPLT_XACTPOS_MASK             BIT_FIELD(MASK(2), 14)
#define HCSPLT_XACTPOS(value)           BIT_FIELD((value), 14)
#define HCSPLT_XACTPOS_VALUE(reg) \
    FIELD_VALUE((reg), HCSPLT_XACTPOS_MASK, 14)

#define HCSPLT_COMPLSPLT                BIT(16)
#define HCSPLT_SPLITEN                  BIT(31)
/*------------------Host Channel Interrupt registers--------------------------*/
#define HCINT_XFRC                      BIT(0)
#define HCINT_CHH                       BIT(1)
#define HCINT_AHBERR                    BIT(2)
#define HCINT_STALL                     BIT(3)
#define HCINT_NAK                       BIT(4)
#define HCINT_ACK                       BIT(5)
#define HCINT_NYET                      BIT(6)
#define HCINT_TXERR                     BIT(7)
#define HCINT_BBERR                     BIT(8)
#define HCINT_FRMOR                     BIT(9)
#define HCINT_DTERR                     BIT(10)
/*------------------Host Channel Interrupt Mask registers---------------------*/
#define HCINTMSK_XFRCM                  BIT(0)
#define HCINTMSK_CHHM                   BIT(1)
#define HCINTMSK_AHBERR                 BIT(2)
#define HCINTMSK_STALLM                 BIT(3)
#define HCINTMSK_NAKM                   BIT(4)
#define HCINTMSK_ACKM                   BIT(5)
#define HCINTMSK_NYET                   BIT(6)
#define HCINTMSK_TXERRM                 BIT(7)
#define HCINTMSK_BBERRM                 BIT(8)
#define HCINTMSK_FRMORM                 BIT(9)
#define HCINTMSK_DTERRM                 BIT(10)
/*------------------Host Channel Transfer Size registers----------------------*/
#define HCTSIZ_XFRSIZ_MASK              BIT_FIELD(MASK(19), 0)
#define HCTSIZ_XFRSIZ(value)            BIT_FIELD((value), 0)
#define HCTSIZ_XFRSIZ_VALUE(reg) \
    FIELD_VALUE((reg), HCTSIZ_XFRSIZ_MASK, 0)

#define HCTSIZ_PKTCNT_MASK              BIT_FIELD(MASK(10), 19)
#define HCTSIZ_PKTCNT(value)            BIT_FIELD((value), 19)
#define HCTSIZ_PKTCNT_VALUE(reg) \
    FIELD_VALUE((reg), HCTSIZ_PKTCNT_MASK, 19)

#define HCTSIZ_DPID_MASK                BIT_FIELD(MASK(2), 29)
#define HCTSIZ_DPID(value)              BIT_FIELD((value), 29)
#define HCTSIZ_DPID_VALUE(reg) \
    FIELD_VALUE((reg), HCTSIZ_DPID_MASK, 29)

#define HCTSIZ_DOPING                   BIT(31)
/*------------------Device Configuration register-----------------------------*/
enum
{
  DSPD_HS = 0,
  DSPD_FS = 3
};

enum
{
  PFIVL_80P,
  PFIVL_85P,
  PFIVL_90P,
  PFIVL_95P
};

#define DCFG_DSPD_MASK                  BIT_FIELD(MASK(2), 0)
#define DCFG_DSPD(value)                BIT_FIELD((value), 0)
#define DCFG_DSPD_VALUE(reg)            FIELD_VALUE((reg), DCFG_DSPD_MASK, 0)

#define DCFG_NZLSOHSK                   BIT(2)

#define DCFG_DAD_MASK                   BIT_FIELD(MASK(7), 4)
#define DCFG_DAD(value)                 BIT_FIELD((value), 4)
#define DCFG_DAD_VALUE(reg)             FIELD_VALUE((reg), DCFG_DAD_MASK, 4)

#define DCFG_PFIVL_MASK                 BIT_FIELD(MASK(2), 11)
#define DCFG_PFIVL(value)               BIT_FIELD((value), 11)
#define DCFG_PFIVL_VALUE(reg)           FIELD_VALUE((reg), DCFG_PFIVL_MASK, 11)

#define DCFG_PERSHIVL_MASK              BIT_FIELD(MASK(2), 24)
#define DCFG_PERSHIVL(value)            BIT_FIELD((value), 24)
#define DCFG_PERSHIVL_VALUE(reg) \
    FIELD_VALUE((reg), DCFG_PERSHIVL_MASK, 24)
/*------------------Device Control register-----------------------------------*/
enum
{
  TCTL_DISABLED           = 0,
  TCTL_TEST_J             = 1,
  TCTL_TEST_K             = 2,
  TCTL_TEST_SE0_NAK       = 3,
  TCTL_TEST_PACKET        = 4,
  TCTL_TEST_FORCE_ENABLE  = 5
};

#define DCTL_RWUSIG                     BIT(0)
#define DCTL_SDIS                       BIT(1)
#define DCTL_GINSTS                     BIT(2)
#define DCTL_GONSTS                     BIT(3)

#define DCTL_TCTL_MASK                  BIT_FIELD(MASK(2), 4)
#define DCTL_TCTL(value)                BIT_FIELD((value), 4)
#define DCTL_TCTL_VALUE(reg)            FIELD_VALUE((reg), DCTL_TCTL_MASK, 4)

#define DCTL_SGINAK                     BIT(7)
#define DCTL_CGINAK                     BIT(8)
#define DCTL_SGONAK                     BIT(9)
#define DCTL_CGONAK                     BIT(10)
#define DCTL_POPRGDNE                   BIT(11)
/*------------------Device Status register------------------------------------*/
enum
{
  ENUMSPD_HS  = 0,
  ENUMSPD_FS  = 3
};

#define DSTS_SUSPSTS                    BIT(0)

#define DSTS_ENUMSPD_MASK               BIT_FIELD(MASK(2), 1)
#define DSTS_ENUMSPD(value)             BIT_FIELD((value), 1)
#define DSTS_ENUMSPD_VALUE(reg)         FIELD_VALUE((reg), DSTS_ENUMSPD_MASK, 1)

#define DSTS_EERR                       BIT(3)

#define DSTS_FNSOF_MASK                 BIT_FIELD(MASK(14), 8)
#define DSTS_FNSOF(value)               BIT_FIELD((value), 8)
#define DSTS_FNSOF_VALUE(reg)           FIELD_VALUE((reg), DSTS_FNSOF_MASK, 8)
#define DSTS_FNSOF_LSB                  BIT(8)
/*------------------Device IN EP common interrupt mask register---------------*/
#define DIEPMSK_XFRCM                   BIT(0)
#define DIEPMSK_EPDM                    BIT(1)
#define DIEPMSK_TOM                     BIT(3)
#define DIEPMSK_ITTXFEMSK               BIT(4)
#define DIEPMSK_INEPNMM                 BIT(5)
#define DIEPMSK_INEPNEM                 BIT(6)
#define DIEPMSK_TXFURM                  BIT(8)
#define DIEPMSK_BIM                     BIT(9)
/*------------------Device OUT EP common interrupt mask register--------------*/
#define DOEPMSK_XFRCM                   BIT(0)
#define DOEPMSK_EPDM                    BIT(1)
#define DOEPMSK_STUPM                   BIT(3)
#define DOEPMSK_OTEPDM                  BIT(4)
#define DOEPMSK_B2BSTUP                 BIT(6)
#define DOEPMSK_OPEM                    BIT(8)
#define DOEPMSK_BOIM                    BIT(9)
/*------------------Device all EP interrupt register--------------------------*/
#define DAINT_IEPINT_MASK               BIT_FIELD(MASK(16), 0)
#define DAINT_IEPINT(channel)           BIT(channel)
#define DAINT_IEPINT_VALUE(reg) \
    FIELD_VALUE((reg), DAINT_IEPINT_MASK, 0)

#define DAINT_OEPINT_MASK               BIT_FIELD(MASK(16), 16)
#define DAINT_OEPINT(channel)           BIT((channel) + 16)
#define DAINT_OEPINT_VALUE(reg) \
    FIELD_VALUE((reg), DAINT_OEPINT_MASK, 16)
/*------------------Device all EP interrupt mask register---------------------*/
#define DAINTMSK_IEPM(channel)          BIT(channel)
#define DAINTMSK_IEPM_FIELD(value)      BIT_FIELD((value), 0)
#define DAINTMSK_IEPM_MASK              BIT_FIELD(MASK(16), 0)
#define DAINTMSK_OEPM(channel)          BIT((channel) + 16)
#define DAINTMSK_OEPM_FIELD(value)      BIT_FIELD((value), 16)
#define DAINTMSK_OEPM_MASK              BIT_FIELD(MASK(16), 16)
/*------------------Device Threshold Control register-------------------------*/
#define DTHRCTL_NONISOTHREN             BIT(0)
#define DTHRCTL_ISOTHREN                BIT(1)

#define DTHRCTL_TXTHRLEN_MASK           BIT_FIELD(MASK(9), 2)
#define DTHRCTL_TXTHRLEN(value)         BIT_FIELD((value), 2)
#define DTHRCTL_TXTHRLEN_VALUE(reg) \
    FIELD_VALUE((reg), DTHRCTL_TXTHRLEN_MASK, 2)

#define DTHRCTL_RXTHREN                 BIT(16)

#define DTHRCTL_RXTHRLEN_MASK           BIT_FIELD(MASK(9), 17)
#define DTHRCTL_RXTHRLEN(value)         BIT_FIELD((value), 17)
#define DTHRCTL_RXTHRLEN_VALUE(reg) \
    FIELD_VALUE((reg), DTHRCTL_RXTHRLEN_MASK, 17)

#define DTHRCTL_ARPEN                   BIT(27)
/*------------------Device IN EP FIFO empty interrupt mask register-----------*/
#define DIEPEMPMSK_INEPTXFEM(channel)   BIT(channel)
#define DIEPEMPMSK_INEPTXFEM_MASK       BIT_FIELD(MASK(16), 0)
/*------------------Device each EP interrupt register-------------------------*/
#define DEACHINT_IEP1INT                BIT(1)
#define DEACHINT_OEP1INT                BIT(17)
/*------------------Device each EP interrupt mask register--------------------*/
#define DEACHINTMSK_IEP1INTM            BIT(1)
#define DEACHINTMSK_OEP1INTM            BIT(17)
/*------------------Device each IN EP interrupt register 1--------------------*/
#define DIEPEACHMSK1_XFRCM              BIT(0)
#define DIEPEACHMSK1_EPDM               BIT(1)
#define DIEPEACHMSK1_TOM                BIT(3)
#define DIEPEACHMSK1_ITTXFEMSK          BIT(4)
#define DIEPEACHMSK1_INEPNMM            BIT(5)
#define DIEPEACHMSK1_INEPNEM            BIT(6)
#define DIEPEACHMSK1_TXFURM             BIT(8)
#define DIEPEACHMSK1_BIM                BIT(9)
#define DIEPEACHMSK1_NAKM               BIT(13)
/*------------------Device each OUT EP interrupt register 1-------------------*/
#define DOEPEACHMSK1_XFRCM              BIT(0)
#define DOEPEACHMSK1_EPDM               BIT(1)
#define DOEPEACHMSK1_AHBERRM            BIT(2)
#define DOEPEACHMSK1_OPEM               BIT(8)
#define DOEPEACHMSK1_BIM                BIT(9)
#define DOEPEACHMSK1_BERRM              BIT(12)
#define DOEPEACHMSK1_NAKM               BIT(13)
#define DOEPEACHMSK1_NYETM              BIT(14)
/*------------------Device IN EP control registers----------------------------*/
#define DIEPCTL_MPSIZ_MASK              BIT_FIELD(MASK(11), 0)
#define DIEPCTL_MPSIZ(value)            BIT_FIELD((value), 0)
#define DIEPCTL_MPSIZ_VALUE(reg) \
    FIELD_VALUE((reg), DIEPCTL_MPSIZ_MASK, 0)

#define DIEPCTL_USBAEP                  BIT(15)
#define DIEPCTL_EONUM                   BIT(16)
#define DIEPCTL_DPID_DATA0              BIT(16)
#define DIEPCTL_DPID_DATA1              0
#define DIEPCTL_NAKSTS                  BIT(17)

#define DIEPCTL_EPTYP_MASK              BIT_FIELD(MASK(2), 18)
#define DIEPCTL_EPTYP(value)            BIT_FIELD((value), 18)
#define DIEPCTL_EPTYP_VALUE(reg) \
    FIELD_VALUE((reg), DIEPCTL_EPTYP_MASK, 18)

#define DIEPCTL_STALL                   BIT(21)

#define DIEPCTL_TXFNUM_MASK             BIT_FIELD(MASK(4), 22)
#define DIEPCTL_TXFNUM(value)           BIT_FIELD((value), 22)
#define DIEPCTL_TXFNUM_VALUE(reg) \
    FIELD_VALUE((reg), DIEPCTL_TXFNUM_MASK, 22)

#define DIEPCTL_CNAK                    BIT(26)
#define DIEPCTL_SNAK                    BIT(27)
#define DIEPCTL_SD0PID                  BIT(28)
#define DIEPCTL_SEVNFRM                 BIT(28)
#define DIEPCTL_SODDFRM                 BIT(29)
#define DIEPCTL_EPDIS                   BIT(30)
#define DIEPCTL_EPENA                   BIT(31)
/*------------------Device IN EP0 control register----------------------------*/
#define DIEPCTL0_MPSIZ_MASK             BIT_FIELD(MASK(2), 0)
#define DIEPCTL0_MPSIZ(value)           BIT_FIELD((value), 0)
#define DIEPCTL0_MPSIZ_VALUE(reg) \
    FIELD_VALUE((reg), DIEPCTL0_MPSIZ_MASK, 0)
/*------------------Device OUT EP control registers---------------------------*/
#define DOEPCTL_MPSIZ_MASK              BIT_FIELD(MASK(11), 0)
#define DOEPCTL_MPSIZ(value)            BIT_FIELD((value), 0)
#define DOEPCTL_MPSIZ_VALUE(reg) \
    FIELD_VALUE((reg), DOEPCTL_MPSIZ_MASK, 0)

#define DOEPCTL_USBAEP                  BIT(15)
#define DOEPCTL_EONUM                   BIT(16)
#define DOEPCTL_DPID_DATA0              BIT(16)
#define DOEPCTL_DPID_DATA1              0
#define DOEPCTL_NAKSTS                  BIT(17)

#define DOEPCTL_EPTYP_MASK              BIT_FIELD(MASK(2), 18)
#define DOEPCTL_EPTYP(value)            BIT_FIELD((value), 18)
#define DOEPCTL_EPTYP_VALUE(reg) \
    FIELD_VALUE((reg), DOEPCTL_EPTYP_MASK, 18)

#define DOEPCTL_SNPM                    BIT(20)
#define DOEPCTL_STALL                   BIT(21)

#define DOEPCTL_CNAK                    BIT(26)
#define DOEPCTL_SNAK                    BIT(27)
#define DOEPCTL_SD0PID                  BIT(28)
#define DOEPCTL_SEVNFRM                 BIT(28)
#define DOEPCTL_SD1PID                  BIT(29)
#define DOEPCTL_SODDFRM                 BIT(29)
#define DOEPCTL_EPDIS                   BIT(30)
#define DOEPCTL_EPENA                   BIT(31)
/*------------------Device OUT EP0 control register---------------------------*/
#define DOEPCTL0_MPSIZ_MASK             BIT_FIELD(MASK(2), 0)
#define DOEPCTL0_MPSIZ(value)           BIT_FIELD((value), 0)
#define DOEPCTL0_MPSIZ_VALUE(reg) \
    FIELD_VALUE((reg), DOEPCTL0_MPSIZ_MASK, 0)
/*------------------Device IN EP interrupt registers--------------------------*/
#define DIEPINT_XFRC                    BIT(0)
#define DIEPINT_EPDISD                  BIT(1)
#define DIEPINT_TOC                     BIT(3)
#define DIEPINT_ITTXFE                  BIT(4)
#define DIEPINT_INEPNE                  BIT(6)
#define DIEPINT_TXFE                    BIT(7)
#define DIEPINT_TXFIFOUDRN              BIT(8)
#define DIEPINT_BNA                     BIT(9)
#define DIEPINT_PKTDRPSTS               BIT(11)
#define DIEPINT_BERR                    BIT(12)
#define DIEPINT_NAK                     BIT(13)
/*------------------Device OUT EP interrupt registers-------------------------*/
#define DOEPINT_XFRC                    BIT(0)
#define DOEPINT_EPDISD                  BIT(1)
#define DOEPINT_STUP                    BIT(3)
#define DOEPINT_OTEPDIS                 BIT(4)
#define DOEPINT_B2BSTUP                 BIT(6)
#define DOEPINT_NYET                    BIT(14)
/*------------------Device IN EP0 transfer size register----------------------*/
#define DIEPTSIZ0_XFRSIZ_MASK           BIT_FIELD(MASK(7), 0)
#define DIEPTSIZ0_XFRSIZ(value)         BIT_FIELD((value), 0)
#define DIEPTSIZ0_XFRSIZ_VALUE(reg) \
    FIELD_VALUE((reg), DIEPTSIZ0_XFRSIZ_MASK, 0)

#define DIEPTSIZ0_PKTCNT_MASK           BIT_FIELD(MASK(2), 19)
#define DIEPTSIZ0_PKTCNT(value)         BIT_FIELD((value), 19)
#define DIEPTSIZ0_PKTCNT_VALUE(reg) \
    FIELD_VALUE((reg), DIEPTSIZ0_PKTCNT_MASK, 19)
#define DIEPTSIZ0_PKTCNT_MAX            MASK(2)
/*------------------Device OUT EP0 transfer size register---------------------*/
#define DOEPTSIZ0_XFRSIZ_MASK           BIT_FIELD(MASK(7), 0)
#define DOEPTSIZ0_XFRSIZ(value)         BIT_FIELD((value), 0)
#define DOEPTSIZ0_XFRSIZ_VALUE(reg) \
    FIELD_VALUE((reg), DOEPTSIZ0_XFRSIZ_MASK, 0)

#define DOEPTSIZ0_PKTCNT                BIT(19)

#define DOEPTSIZ0_STUPCNT_MASK          BIT_FIELD(MASK(2), 29)
#define DOEPTSIZ0_STUPCNT(value)        BIT_FIELD((value), 29)
#define DOEPTSIZ0_STUPCNT_VALUE(reg) \
    FIELD_VALUE((reg), DOEPTSIZ0_STUPCNT_MASK, 29)
/*------------------Device IN EP transfer size registers----------------------*/
#define DIEPTSIZ_XFRSIZ_MASK            BIT_FIELD(MASK(19), 0)
#define DIEPTSIZ_XFRSIZ(value)          BIT_FIELD((value), 0)
#define DIEPTSIZ_XFRSIZ_VALUE(reg) \
    FIELD_VALUE((reg), DIEPTSIZ_XFRSIZ_MASK, 0)

#define DIEPTSIZ_PKTCNT_MASK            BIT_FIELD(MASK(10), 19)
#define DIEPTSIZ_PKTCNT(value)          BIT_FIELD((value), 19)
#define DIEPTSIZ_PKTCNT_VALUE(reg) \
    FIELD_VALUE((reg), DIEPTSIZ_PKTCNT_MASK, 19)
#define DIEPTSIZ_PKTCNT_MAX             MASK(10)

#define DIEPTSIZ_MCNT_MASK              BIT_FIELD(MASK(2), 29)
#define DIEPTSIZ_MCNT(value)            BIT_FIELD((value), 29)
#define DIEPTSIZ_MCNT_VALUE(reg) \
    FIELD_VALUE((reg), DIEPTSIZ_MCNT_MASK, 29)
/*------------------Device OUT EP transfer size registers---------------------*/
#define DOEPTSIZ_XFRSIZ_MASK            BIT_FIELD(MASK(19), 0)
#define DOEPTSIZ_XFRSIZ(value)          BIT_FIELD((value), 0)
#define DOEPTSIZ_XFRSIZ_VALUE(reg) \
    FIELD_VALUE((reg), DOEPTSIZ_XFRSIZ_MASK, 0)

#define DOEPTSIZ_PKTCNT_MASK            BIT_FIELD(MASK(10), 19)
#define DOEPTSIZ_PKTCNT(value)          BIT_FIELD((value), 19)
#define DOEPTSIZ_PKTCNT_VALUE(reg) \
    FIELD_VALUE((reg), DOEPTSIZ_PKTCNT_MASK, 19)

#define DOEPTSIZ_STUPCNT_MASK           BIT_FIELD(MASK(2), 29)
#define DOEPTSIZ_STUPCNT(value)         BIT_FIELD((value), 29)
#define DOEPTSIZ_STUPCNT_VALUE(reg) \
    FIELD_VALUE((reg), DOEPTSIZ_STUPCNT_MASK, 29)

#define DOEPTSIZ_RXDPID_MASK            BIT_FIELD(MASK(2), 29)
#define DOEPTSIZ_RXDPID(value)          BIT_FIELD((value), 29)
#define DOEPTSIZ_RXDPID_VALUE(reg) \
    FIELD_VALUE((reg), DOEPTSIZ_RXDPID_MASK, 29)
/*------------------Power and Clock Gating Control register-------------------*/
#define PCGCCTL_STPPCLK                 BIT(0)
#define PCGCCTL_GATEHCLK                BIT(1)
#define PCGCCTL_PHYSUSP                 BIT(4)
/*----------------------------------------------------------------------------*/
#define EP_TO_INDEX(ep) ((((ep) & 0x0F) << 1) | (((ep) & 0x80) >> 7))
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GEN_2_USB_DEFS_H_ */
