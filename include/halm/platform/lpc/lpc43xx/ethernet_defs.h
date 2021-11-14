/*
 * halm/platform/lpc/ethernet_defs.h
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC43XX_ETHERNET_DEFS_H_
#define HALM_PLATFORM_LPC_LPC43XX_ETHERNET_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
#include <stdint.h>
/*------------------MAC Configuration register--------------------------------*/
/* Receiver Enable */
#define MAC_CONFIG_RE                   BIT(2)
/* Transmitter Enable */
#define MAC_CONFIG_TE                   BIT(3)
/* Deferral Check */
#define MAC_CONFIG_DF                   BIT(4)

/* Back-Off Limit */
#define MAC_CONFIG_BL(value)            BIT_FIELD((value), 5)
#define MAC_CONFIG_BL_MASK              BIT_FIELD(MASK(2), 5)
#define MAC_CONFIG_BL_VALUE(reg) \
    FIELD_VALUE((reg), MAC_CONFIG_BL_MASK, 5)

/* Automatic Pad/CRC Stripping */
#define MAC_CONFIG_ACS                  BIT(7)
/* Link Up/Down */
#define MAC_CONFIG_LINK                 BIT(8)
/* Disable Retry */
#define MAC_CONFIG_DR                   BIT(9)
/* Full-duplex mode */
#define MAC_CONFIG_DM                   BIT(11)
/* Loopback mode */
#define MAC_CONFIG_LM                   BIT(12)
/* Disable receive own */
#define MAC_CONFIG_DO                   BIT(13)
/* Speed: 0 for 10 Mbps, 1 for 100 Mbps */
#define MAC_CONFIG_FES                  BIT(14)
/* Port Select */
#define MAC_CONFIG_PS                   BIT(15)
/* Disable carrier sense during transmission */
#define MAC_CONFIG_DCRS                 BIT(16)

/* Inter-frame gap */
#define MAC_CONFIG_IFG(value)           BIT_FIELD((value), 17)
#define MAC_CONFIG_IFG_MASK             BIT_FIELD(MASK(3), 17)
#define MAC_CONFIG_IFG_VALUE(reg) \
    FIELD_VALUE((reg), MAC_CONFIG_IFG_MASK, 17)

/* Jumbo Frame Enable */
#define MAC_CONFIG_JE                   BIT(20)
/* Jabber Disable */
#define MAC_CONFIG_JD                   BIT(22)
/* Watchdog Disable */
#define MAC_CONFIG_WD                   BIT(23)
/*------------------MAC Frame Filter register---------------------------------*/
enum
{
  PCF_FILTER_ALL        = 0x00,
  PCF_FILTER_PAUSE      = 0x01,
  PCF_FILTER_NONE       = 0x02,
  PCF_FILTER_BY_ADDRESS = 0x03
};

/* Promiscuous Mode */
#define MAC_FILTER_PR                   BIT(0)
/* Hash Unicast */
#define MAC_FILTER_HUC                  BIT(1)
/* Hash Multicast */
#define MAC_FILTER_HMC                  BIT(2)
/* DA Inverse Filtering */
#define MAC_FILTER_DAIF                 BIT(3)
/* Pass All Multicast */
#define MAC_FILTER_PM                   BIT(4)
/* Disable Broadcast Frames */
#define MAC_FILTER_DBF                  BIT(5)

/* Pass Control Frames */
#define MAC_FILTER_PCF(value)           BIT_FIELD((value), 6)
#define MAC_FILTER_PCF_MASK             BIT_FIELD(MASK(2), 6)
#define MAC_FILTER_PCF_VALUE(reg) \
    FIELD_VALUE((reg), MAC_FILTER_PCF_MASK, 6)

/* Hash or perfect filter */
#define MAC_FILTER_HPF                  BIT(10)
/* Receive All */
#define MAC_FILTER_RA                   BIT(31)
/*------------------MAC MII Address register----------------------------------*/
enum
{
  CR_CLOCK_DIV_42   = 0x00,
  CR_CLOCK_DIV_62   = 0x01,
  CR_CLOCK_DIV_16   = 0x02,
  CR_CLOCK_DIV_26   = 0x03,
  CR_CLOCK_DIV_102  = 0x04,
  CR_CLOCK_DIV_124  = 0x05
};

/* MII busy */
#define MAC_MII_ADDR_GB                 BIT(0)
/* MII write */
#define MAC_MII_ADDR_W                  BIT(1)

/* CSR clock range */
#define MAC_MII_ADDR_CR(value)          BIT_FIELD((value), 2)
#define MAC_MII_ADDR_CR_MASK            BIT_FIELD(MASK(4), 2)
#define MAC_MII_ADDR_CR_VALUE(reg) \
    FIELD_VALUE((reg), MAC_MII_ADDR_CR_MASK, 2)

/* MII register */
#define MAC_MII_ADDR_GR(value)          BIT_FIELD((value), 6)
#define MAC_MII_ADDR_GR_MASK            BIT_FIELD(MASK(5), 6)
#define MAC_MII_ADDR_GR_VALUE(reg) \
    FIELD_VALUE((reg), MAC_MII_ADDR_GR_MASK, 6)

/* Physical layer address */
#define MAC_MII_ADDR_PA(value)          BIT_FIELD((value), 11)
#define MAC_MII_ADDR_PA_MASK            BIT_FIELD(MASK(5), 11)
#define MAC_MII_ADDR_PA_VALUE(reg) \
    FIELD_VALUE((reg), MAC_MII_ADDR_PA_MASK, 11)
/*------------------MAC Flow Control register---------------------------------*/
/* Flow Control Busy/Backpressure Activate */
#define MAC_FLOW_CTRL_FCB               BIT(0)
/* Transmit Flow Control Enable */
#define MAC_FLOW_CTRL_TFE               BIT(1)
/* Receive Flow Control Enable */
#define MAC_FLOW_CTRL_RFE               BIT(2)
/* Unicast Pause Frame Detect */
#define MAC_FLOW_CTRL_UP                BIT(3)

/* Pause Low Threshold */
#define MAC_FLOW_CTRL_PLT(value)        BIT_FIELD((value), 4)
#define MAC_FLOW_CTRL_PLT_MASK          BIT_FIELD(MASK(2), 4)
#define MAC_FLOW_CTRL_PLT_VALUE(reg) \
    FIELD_VALUE((reg), MAC_FLOW_CTRL_PLT_MASK, 4)

/* Disable Zero-Quanta Pause */
#define MAC_FLOW_CTRL_DZPQ              BIT(7)

/* Physical layer address */
#define MAC_FLOW_CTRL_PT(value)         BIT_FIELD((value), 16)
#define MAC_FLOW_CTRL_PT_MASK           BIT_FIELD(MASK(16), 16)
#define MAC_FLOW_CTRL_PT_VALUE(reg) \
    FIELD_VALUE((reg), MAC_FLOW_CTRL_PT_MASK, 16)
/*------------------MAC VLAN Tag register-------------------------------------*/
/* VLAN Tag Identifier for Receive Frames */
#define MAC_VLAN_TAG_VL(value)         BIT_FIELD((value), 0)
#define MAC_VLAN_TAG_VL_MASK           BIT_FIELD(MASK(16), 0)
#define MAC_VLAN_TAG_VL_VALUE(reg) \
    FIELD_VALUE((reg), MAC_VLAN_TAG_VL_MASK, 0)

/* Enable 12-bit VLAN Tag Comparison */
#define MAC_VLAN_TAG_ETV                BIT(16)
/*------------------MAC Debug register----------------------------------------*/
/* MAC MII receive protocol engine is active */
#define MAC_DEBUG_RXIDLESTAT            BIT(0)

/* State of the small FIFO Read and Write controllers */
#define MAC_DEBUG_FIFOSTAT0(value)      BIT_FIELD((value), 1)
#define MAC_DEBUG_FIFOSTAT0_MASK        BIT_FIELD(MASK(2), 1)
#define MAC_DEBUG_FIFOSTAT0_VALUE(reg) \
    FIELD_VALUE((reg), MAC_DEBUG_FIFOSTAT0_MASK, 1)

/* RxFIFO Write Controller is active */
#define MAC_DEBUG_RXFIFOSTAT1           BIT(4)

/* State of the RxFIFO Read controller */
#define MAC_DEBUG_RXFIFOSTAT(value)     BIT_FIELD((value), 5)
#define MAC_DEBUG_RXFIFOSTAT_MASK       BIT_FIELD(MASK(2), 5)
#define MAC_DEBUG_RXFIFOSTAT_VALUE(reg) \
    FIELD_VALUE((reg), MAC_DEBUG_RXFIFOSTAT_MASK, 5)

/* Status of the RxFIFO fill-level */
#define MAC_DEBUG_RXFIFOLVL(value)      BIT_FIELD((value), 8)
#define MAC_DEBUG_RXFIFOLVL_MASK        BIT_FIELD(MASK(2), 8)
#define MAC_DEBUG_RXFIFOLVL_VALUE(reg) \
    FIELD_VALUE((reg), MAC_DEBUG_RXFIFOLVL_MASK, 8)

/* MAC MII transmit protocol engine is active */
#define MAC_DEBUG_TXIDLESTAT            BIT(16)

/* State of the MAC Transmit Frame controller */
#define MAC_DEBUG_TXSTAT(value)         BIT_FIELD((value), 17)
#define MAC_DEBUG_TXSTAT_MASK           BIT_FIELD(MASK(2), 17)
#define MAC_DEBUG_TXSTAT_VALUE(reg) \
    FIELD_VALUE((reg), MAC_DEBUG_TXSTAT_MASK, 17)

/* MAC transmitter is in PAUSE condition */
#define MAC_DEBUG_PAUSE                 BIT(19)

/* State of the TxFIFO Read controller */
#define MAC_DEBUG_TXFIFOSTAT(value)     BIT_FIELD((value), 20)
#define MAC_DEBUG_TXFIFOSTAT_MASK       BIT_FIELD(MASK(2), 20)
#define MAC_DEBUG_TXFIFOSTAT_VALUE(reg) \
    FIELD_VALUE((reg), MAC_DEBUG_TXFIFOSTAT_MASK, 20)

/* TxFIFO Write Controller is active */
#define MAC_DEBUG_TXFIFOSTAT1           BIT(22)
/* TxFIFO is not empty */
#define MAC_DEBUG_TXFIFOLVL             BIT(24)
/* TxFIFO is full */
#define MAC_DEBUG_TXFIFOFULL            BIT(25)
/*------------------MAC PMT Control and Status register-----------------------*/
/* Power-down */
#define MAC_PMT_CTRL_STAT_PD            BIT(0)
/* Magic packet enable */
#define MAC_PMT_CTRL_STAT_MPE           BIT(1)
/* Wake-up frame enable */
#define MAC_PMT_CTRL_STAT_WFE           BIT(2)
/* Magic packet received */
#define MAC_PMT_CTRL_STAT_MPR           BIT(5)
/* Wake-up frame received */
#define MAC_PMT_CTRL_STAT_WFR           BIT(6)
/* Global Unicast */
#define MAC_PMT_CTRL_STAT_GU            BIT(9)
/* Wake-up frame filter register pointer reset */
#define MAC_PMT_CTRL_STAT_WFFRPR        BIT(31)
/*------------------MAC Interrupt Status register-----------------------------*/
/* PMT Interrupt Status */
#define MAC_INTR_PMT                    BIT(3)
/* Timestamp Interrupt Status */
#define MAC_INTR_TS                     BIT(9)
/*------------------MAC Interrupt Mask register-------------------------------*/
/* PMT Interrupt Mask */
#define MAC_INTR_MASK_PMTIM             BIT(3)
/* Timestamp Interrupt Mask */
#define MAC_INTR_MASK_TSIM              BIT(9)
/*------------------MAC Address 0 High register-------------------------------*/
#define MAC_ADDR0_HIGH_ADDRESS(value)   BIT_FIELD((value)), 0)
#define MAC_ADDR0_HIGH_ADDRESS_MASK     BIT_FIELD(MASK(16), 0)
#define MAC_ADDR0_HIGH_ADDRESS_VALUE(reg) \
    FIELD_VALUE((reg), MAC_ADDR0_HIGH_ADDRESS_MASK, 0)

#define MAC_ADDR0_HIGH_MO               BIT(31)
/*------------------Time Stamp Control register-------------------------------*/
/* Time stamp Enable */
#define MAC_TIMESTP_CTRL_TSENA          BIT(0)
/* Time stamp Fine or Coarse Update */
#define MAC_TIMESTP_CTRL_TSCFUPDT       BIT(1)
/* Time stamp Initialize */
#define MAC_TIMESTP_CTRL_TSINIT         BIT(2)
/* Time stamp Update */
#define MAC_TIMESTP_CTRL_TSUPDT         BIT(3)
/* Time stamp Interrupt Trigger Enable */
#define MAC_TIMESTP_CTRL_TSTRIG         BIT(4)
/* Addend Reg Update */
#define MAC_TIMESTP_CTRL_TSADDREG       BIT(5)
/* Enable Time stamp for all frames */
#define MAC_TIMESTP_CTRL_TSENALL        BIT(8)
/* Time stamp digital or binary rollover control */
#define MAC_TIMESTP_CTRL_TSCTRLSSR      BIT(9)
/* Enable PTP packet snooping for version 2 format */
#define MAC_TIMESTP_CTRL_TSVER2ENA      BIT(10)
/* Enable Time stamp snapshot for PTP over Ethernet frames */
#define MAC_TIMESTP_CTRL_TSIPENA        BIT(11)
/* Enable Time stamp snapshot for IPv6 frames */
#define MAC_TIMESTP_CTRL_TSIPV6ENA      BIT(12)
/* Enable Time stamp snapshot for IPv4 frames */
#define MAC_TIMESTP_CTRL_TSIPV4ENA      BIT(13)
/* Enable Time stamp snapshot for Event Messages */
#define MAC_TIMESTP_CTRL_TSEVNTENA      BIT(14)
/* Enable snapshot for messages relevant to master */
#define MAC_TIMESTP_CTRL_TSMSTRENA      BIT(15)

/* Select the type of clock node */
#define MAC_TIMESTP_CTRL_TSCLKTYPE(value) \
    BIT_FIELD((value), 16)
#define MAC_TIMESTP_CTRL_TSCLKTYPE_MASK \
    BIT_FIELD(MASK(2), 16)
#define MAC_TIMESTP_CTRL_TSCLKTYPE_VALUE(reg) \
    FIELD_VALUE((reg), MAC_TIMESTP_CTRL_TSCLKTYPE_MASK, 16)

/* Enable MAC address for PTP frame filtering */
#define MAC_TIMESTP_CTRL_TSENMACADDR    BIT(18)
/*------------------System time Nanoseconds register--------------------------*/
/* Time stamp sub seconds */
#define NANOSECONDS_TSSS(value)         BIT_FIELD((value), 0)
#define NANOSECONDS_TSSS_MASK           BIT_FIELD(MASK(31), 0)
#define NANOSECONDS_TSSS_VALUE(reg) \
    FIELD_VALUE((reg), NANOSECONDS_TSSS_MASK, 0)

/* Positive or negative time */
#define NANOSECONDS_PSNT                BIT(31)
/*------------------System time Nanoseconds Update register-------------------*/
/* Time stamp sub seconds */
#define NANOSECONDSUPDATE_TSSS(value) \
    BIT_FIELD((value), 0)
#define NANOSECONDSUPDATE_TSSS_MASK \
    BIT_FIELD(MASK(31), 0)
#define NANOSECONDSUPDATE_TSSS_VALUE(reg) \
    FIELD_VALUE((reg), NANOSECONDSUPDATE_TSSS_MASK, 0)

/* Add or subtract time */
#define NANOSECONDSUPDATE_ADDSUB        BIT(31)
/*------------------Time stamp Status register--------------------------------*/
/* Time stamp seconds overflow */
#define TIMESTAMPSTAT_TSSOVF            BIT(0)
/* Time stamp target reached */
#define TIMESTAMPSTAT_TSTARGT           BIT(1)
/*------------------DMA Bus Mode register-------------------------------------*/
/* Software reset */
#define DMA_BUS_MODE_SWR                BIT(0)
/* DMA arbitration scheme */
#define DMA_BUS_MODE_DA                 BIT(1)

/* Descriptor skip length */
#define DMA_BUS_MODE_DSL(value)         BIT_FIELD((value), 2)
#define DMA_BUS_MODE_DSL_MASK           BIT_FIELD(MASK(5), 2)
#define DMA_BUS_MODE_DSL_VALUE(reg) \
    FIELD_VALUE((reg), DMA_BUS_MODE_DSL_MASK, 2)

/* Alternate descriptor size */
#define DMA_BUS_MODE_ATDS               BIT(7)

/* Programmable burst length */
#define DMA_BUS_MODE_PBL(value)         BIT_FIELD((value), 8)
#define DMA_BUS_MODE_PBL_MASK           BIT_FIELD(MASK(6), 8)
#define DMA_BUS_MODE_PBL_VALUE(reg) \
    FIELD_VALUE((reg), DMA_BUS_MODE_PBL_MASK, 8)

/* Rx-to-Tx priority ratio */
#define DMA_BUS_MODE_PR(value)          BIT_FIELD((value), 14)
#define DMA_BUS_MODE_PR_MASK            BIT_FIELD(MASK(2), 14)
#define DMA_BUS_MODE_PR_VALUE(reg) \
    FIELD_VALUE((reg), DMA_BUS_MODE_PR_MASK, 14)

/* Fixed burst */
#define DMA_BUS_MODE_FB                 BIT(16)

/* RxDMA PBL */
#define DMA_BUS_MODE_RPBL(value)        BIT_FIELD((value), 17)
#define DMA_BUS_MODE_RPBL_MASK          BIT_FIELD(MASK(6), 17)
#define DMA_BUS_MODE_RPBL_VALUE(reg) \
    FIELD_VALUE((reg), DMA_BUS_MODE_RPBL_MASK, 17)

/* Use separate PBL */
#define DMA_BUS_MODE_USP                BIT(23)
/* 8 x PBL mode */
#define DMA_BUS_MODE_PBL8X              BIT(24)
/* Address-aligned beats */
#define DMA_BUS_MODE_AAL                BIT(25)
/* Mixed burst */
#define DMA_BUS_MODE_MB                 BIT(26)
/* Transmit DMA has higher priority than the Receive DMA */
#define DMA_BUS_MODE_TXPR               BIT(27)
/*------------------DMA Status register---------------------------------------*/
enum
{
  RS_STOPPED              = 0x00,
  RS_FETCHING_DESCRIPTOR  = 0x01,
  RS_WAITING_FOR_PACKET   = 0x03,
  RS_SUSPENDED            = 0x04,
  RS_CLOSING_DESCRIPTOR   = 0x05,
  RS_TIME_STAMP           = 0x06,
  RS_WRITIN_TO_MEMORY     = 0x07
};

enum
{
  TS_STOPPED              = 0x00,
  TS_FETCHING_DESCRIPTOR  = 0x01,
  TS_WAITING_FOR_STATUS   = 0x02,
  TS_READING_FROM_MEMORY  = 0x03,
  TS_TIME_STAMP           = 0x04,
  TS_SUSPENDED            = 0x06,
  TS_CLOSING_DESCRIPTOR   = 0x07
};

/* Transmit interrupt */
#define DMA_STAT_TI                     BIT(0)
/* Transmit process stopped */
#define DMA_STAT_TPS                    BIT(1)
/* Transmit buffer unavailable */
#define DMA_STAT_TU                     BIT(2)
/* Transmit jabber timeout */
#define DMA_STAT_TJT                    BIT(3)
/* Receive overflow */
#define DMA_STAT_OVF                    BIT(4)
/* Transmit underflow */
#define DMA_STAT_UNF                    BIT(5)
/* Receive interrupt */
#define DMA_STAT_RI                     BIT(6)
/* Receive buffer unavailable */
#define DMA_STAT_RU                     BIT(7)
/* Receive process stopped */
#define DMA_STAT_RPS                    BIT(8)
/* Receive watchdog timeout */
#define DMA_STAT_RWT                    BIT(9)
/* Early transmit interrupt */
#define DMA_STAT_ETI                    BIT(10)
/* Fatal bus error interrupt */
#define DMA_STAT_FBI                    BIT(13)
/* Early receive interrupt */
#define DMA_STAT_ERI                    BIT(14)
/* Abnormal interrupt summary */
#define DMA_STAT_AIE                    BIT(15)
/* Normal interrupt summary */
#define DMA_STAT_NIS                    BIT(16)

/* Receive process state */
#define DMA_STAT_RS(value)              BIT_FIELD((value), 17)
#define DMA_STAT_RS_MASK                BIT_FIELD(MASK(3), 17)
#define DMA_STAT_RS_VALUE(reg)          FIELD_VALUE((reg), DMA_STAT_RS_MASK, 17)

/* Transmit process state */
#define DMA_STAT_TS(value)              BIT_FIELD((value), 20)
#define DMA_STAT_TS_MASK                BIT_FIELD(MASK(3), 20)
#define DMA_STAT_TS_VALUE(reg)          FIELD_VALUE((reg), DMA_STAT_TS_MASK, 20)

/* Error bit 1 */
#define DMA_STAT_EB1                    BIT(23)
/* Error bit 2 */
#define DMA_STAT_EB2                    BIT(24)
/* Error bit 3 */
#define DMA_STAT_EB3                    BIT(25)
/*------------------DMA Operation Mode register-------------------------------*/
/* Start/stop receive */
#define DMA_OP_MODE_SR                  BIT(1)
/* Operate on second frame */
#define DMA_OP_MODE_OSF                 BIT(2)

/* Receive threshold control */
#define DMA_OP_MODE_RTC(value)          BIT_FIELD((value), 3)
#define DMA_OP_MODE_RTC_MASK            BIT_FIELD(MASK(2), 3)
#define DMA_OP_MODE_RTC_VALUE(reg) \
    FIELD_VALUE((reg), DMA_OP_MODE_RTC_MASK, 3)

/* Forward undersized good frames */
#define DMA_OP_MODE_FUF                 BIT(6)
/* Forward error frames */
#define DMA_OP_MODE_FEF                 BIT(7)
/* Start/stop transmission command */
#define DMA_OP_MODE_ST                  BIT(13)

/* Transmit threshold control */
#define DMA_OP_MODE_TTC(value)          BIT_FIELD((value), 14)
#define DMA_OP_MODE_TTC_MASK            BIT_FIELD(MASK(3), 14)
#define DMA_OP_MODE_TTC_VALUE(reg) \
    FIELD_VALUE((reg), DMA_OP_MODE_TTC_MASK, 14)

/* Flush transmit FIFO */
#define DMA_OP_MODE_FTF                 BIT(20)
/* Disable flushing of received frames */
#define DMA_OP_MODE_DFF                 BIT(24)
/*------------------DMA Interrupt Enable register-----------------------------*/
/* Transmit interrupt enable */
#define DMA_INT_EN_TIE                  BIT(0)
/* Transmit stopped enable */
#define DMA_INT_EN_TSE                  BIT(1)
/* Transmit buffer unavailable enable */
#define DMA_INT_EN_TUE                  BIT(2)
/* Transmit jabber timeout enable */
#define DMA_INT_EN_TJE                  BIT(3)
/* Overflow interrupt enable */
#define DMA_INT_EN_OVE                  BIT(4)
/* Underflow interrupt enable */
#define DMA_INT_EN_UNE                  BIT(5)
/* Receive interrupt enable */
#define DMA_INT_EN_RIE                  BIT(6)
/* Receive buffer unavailable enable */
#define DMA_INT_EN_RUE                  BIT(7)
/* Receive stopped enable */
#define DMA_INT_EN_RSE                  BIT(8)
/* Receive watchdog timeout enable */
#define DMA_INT_EN_RWE                  BIT(9)
/* Early transmit interrupt enable */
#define DMA_INT_EN_ETE                  BIT(10)
/* Fatal bus error enable */
#define DMA_INT_EN_FBE                  BIT(13)
/* Early receive interrupt enable */
#define DMA_INT_EN_ERE                  BIT(14)
/* Abnormal interrupt summary enable */
#define DMA_INT_EN_AIE                  BIT(15)
/* Normal interrupt summary enable */
#define DMA_INT_EN_NIE                  BIT(16)
/*------------------DMA Missed Frame and Buffer Overflow Counter register-----*/
/* Number of frames missed */
#define DMA_MFRM_BUFOF_FMC(value)       BIT_FIELD((value), 0)
#define DMA_MFRM_BUFOF_FMC_MASK         BIT_FIELD(MASK(16), 0)
#define DMA_MFRM_BUFOF_FMC_VALUE(reg) \
    FIELD_VALUE((reg), DMA_MFRM_BUFOF_FMC_MASK, 0)

/* Overflow bit for missed frame counter */
#define DMA_MFRM_BUFOF_OC               BIT(16)

/* Number of frames missed by the application */
#define DMA_MFRM_BUFOF_FMA(value)       BIT_FIELD((value), 17)
#define DMA_MFRM_BUFOF_FMA_MASK         BIT_FIELD(MASK(11), 17)
#define DMA_MFRM_BUFOF_FMA_VALUE(reg) \
    FIELD_VALUE((reg), DMA_MFRM_BUFOF_FMA_MASK, 17)

/* Overflow bit for FIFO overflow counter */
#define DMA_MFRM_BUFOF_OF               BIT(28)
/*----------------------------------------------------------------------------*/
struct TransmitDescriptor
{
  volatile uint32_t TDES0;
  volatile uint32_t TDES1;
  volatile uint32_t TDES2;
  volatile uint32_t TDES3;
  volatile uint32_t TDES4;
  volatile uint32_t TDES5;
  volatile uint32_t TDES6;
  volatile uint32_t TDES7;
};

struct TransmitDescriptorAlt
{
  volatile uint32_t TDES0;
  volatile uint32_t TDES1;
  volatile uint32_t TDES2;
  volatile uint32_t TDES3;
};

/* Deferred Bit */
#define TDES0_DB                        BIT(0)
/* Underflow Error */
#define TDES0_UF                        BIT(1)
/* Excessive Deferral */
#define TDES0_ED                        BIT(2)

/* Collision Count or Slot Number control bits */
#define TDES0_CC_SLOTNUM(value)         BIT_FIELD((value), 3)
#define TDES0_CC_SLOTNUM_MASK           BIT_FIELD(MASK(4), 3)
#define TDES0_CC_SLOTNUM_VALUE(reg) \
    FIELD_VALUE((reg), TDES0_CC_SLOTNUM_MASK, 3)

/* VLAN Frame */
#define TDES0_VF                        BIT(7)
/* Excessive Collision */
#define TDES0_EC                        BIT(8)
/* Late Collision */
#define TDES0_LC                        BIT(9)
/* No Carrier */
#define TDES0_NC                        BIT(10)
/* Loss of Carrier */
#define TDES0_LOC                       BIT(11)
/* IP Payload Error */
#define TDES0_IPE                       BIT(12)
/* Frame Flushed */
#define TDES0_FF                        BIT(13)
/* Jabber Timeout */
#define TDES0_JT                        BIT(14)
/* Error Summary */
#define TDES0_ES                        BIT(15)
/* IP Header Error */
#define TDES0_IHE                       BIT(16)
/* Transmit Timestamp Status */
#define TDES0_TTSS                      BIT(17)
/* Second Address Chained */
#define TDES0_TCH                       BIT(20)
/* Transmit End of Ring */
#define TDES0_TER                       BIT(21)
/* Transmit Timestamp Enable */
#define TDES0_TTSE                      BIT(25)
/* Disable Pad */
#define TDES0_DP                        BIT(26)
/* Disable CRC */
#define TDES0_DC                        BIT(27)
/* First Segment */
#define TDES0_FS                        BIT(28)
/* Last Segment */
#define TDES0_LS                        BIT(29)
/* Interrupt on Completion */
#define TDES0_IC                        BIT(30)
/* Own Bit */
#define TDES0_OWN                       BIT(31)

/* Transmit buffer 1 size */
#define TDES1_TBS1(value)               BIT_FIELD((value), 0)
#define TDES1_TBS1_MASK                 BIT_FIELD(MASK(13), 0)
#define TDES1_TBS1_VALUE(reg)           FIELD_VALUE((reg), TDES1_TBS1_MASK, 0)

/* Transmit buffer 2 size */
#define TDES1_TBS2(value)               BIT_FIELD((value), 16)
#define TDES1_TBS2_MASK                 BIT_FIELD(MASK(13), 16)
#define TDES1_TBS2_VALUE(reg)           FIELD_VALUE((reg), TDES1_TBS2_MASK, 16)
/*----------------------------------------------------------------------------*/
struct ReceiveDescriptor
{
  volatile uint32_t RDES0;
  volatile uint32_t RDES1;
  volatile uint32_t RDES2;
  volatile uint32_t RDES3;
  volatile uint32_t RDES4;
  volatile uint32_t RDES5;
  volatile uint32_t RDES6;
  volatile uint32_t RDES7;
};

struct ReceiveDescriptorAlt
{
  volatile uint32_t RDES0;
  volatile uint32_t RDES1;
  volatile uint32_t RDES2;
  volatile uint32_t RDES3;
};

/* Extended Status Available/Rx MAC Address */
#define RDES0_ESA                       BIT(0)
/* CRC Error */
#define RDES0_CE                        BIT(1)
/* Dribble Bit Error */
#define RDES0_DBE                       BIT(2)
/* Receive Error */
#define RDES0_RE                        BIT(3)
/* Receive Watchdog Timeout */
#define RDES0_RWT                       BIT(4)
/* Frame Type */
#define RDES0_FT                        BIT(5)
/* Late Collision */
#define RDES0_LC                        BIT(6)
/* Timestamp Available */
#define RDES0_TSA                       BIT(7)
/* Last Descriptor */
#define RDES0_LS                        BIT(8)
/* First Descriptor */
#define RDES0_FS                        BIT(9)
/* VLAN Tag */
#define RDES0_VLAN                      BIT(10)
/* Overflow Error */
#define RDES0_OE                        BIT(11)
/* Length Error */
#define RDES0_LE                        BIT(12)
/* Source Address Filter Fail */
#define RDES0_SAF                       BIT(13)
/* Descriptor Error */
#define RDES0_DE                        BIT(14)
/* Error Summary */
#define RDES0_ES                        BIT(15)

/* Frame Length */
#define RDES0_FL(value)                 BIT_FIELD((value), 16)
#define RDES0_FL_MASK                   BIT_FIELD(MASK(14), 16)
#define RDES0_FL_VALUE(reg)             FIELD_VALUE((reg), RDES0_FL_MASK, 16)

/* Destination Address Filter Fail */
#define RDES0_AFM                       BIT(30)
/* Own Bit */
#define RDES0_OWN                       BIT(31)

/* Receive buffer 1 size */
#define RDES1_RBS1(value)               BIT_FIELD((value), 0)
#define RDES1_RBS1_MASK                 BIT_FIELD(MASK(13), 0)
#define RDES1_RBS1_VALUE(reg)           FIELD_VALUE((reg), RDES1_RBS1_MASK, 0)

/* Second Address Chained */
#define RDES1_RCH                       BIT(14)
/* Receive End of Ring */
#define RDES1_RER                       BIT(15)

/* Receive buffer 2 size */
#define RDES1_RBS2(value)               BIT_FIELD((value), 16)
#define RDES1_RBS2_MASK                 BIT_FIELD(MASK(13), 16)
#define RDES1_RBS2_VALUE(reg)           FIELD_VALUE((reg), RDES1_RBS2_MASK, 16)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_ETHERNET_DEFS_H_ */
