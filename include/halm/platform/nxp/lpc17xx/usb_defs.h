/*
 * halm/platform/nxp/lpc17xx/usb_defs.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC17XX_USB_DEFS_H_
#define HALM_PLATFORM_NXP_LPC17XX_USB_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------USB Clock Control register--------------------------------*/
#define USBClkCtrl_DEV_CLK_ON           BIT(1)
#define USBClkCtrl_AHB_CLK_ON           BIT(4)
/*------------------USB Clock Status register---------------------------------*/
#define USBClkSt_DEV_CLK_ON             BIT(1)
#define USBClkSt_AHB_CLK_ON             BIT(4)
/*------------------USB Interrupt Status register-----------------------------*/
#define USBIntSt_USB_INT_REQ_LP         BIT(0)
#define USBIntSt_USB_INT_REQ_HP         BIT(1)
#define USBIntSt_USB_INT_REQ_DMA        BIT(2)
#define USBIntSt_USB_NEED_CLK           BIT(8)
#define USBIntSt_EN_USB_INTS            BIT(31)
/*------------------USB Device Interrupt Status register----------------------*/
#define USBDevInt_FRAME                 BIT(0)
#define USBDevInt_EP_FAST               BIT(1)
#define USBDevInt_EP_SLOW               BIT(2)
#define USBDevInt_DEV_STAT              BIT(3)
#define USBDevInt_CCEMPTY               BIT(4)
#define USBDevInt_CDFULL                BIT(5)
#define USBDevInt_RxENDPKT              BIT(6)
#define USBDevInt_TxENDPKT              BIT(7)
#define USBDevInt_EP_RLZED              BIT(8)
#define USBDevInt_ERR_INT               BIT(9)
/*------------------USB Receive Packet Length register------------------------*/
#define USBRxPLen_PKT_LNGTH_MASK        BIT_FIELD(MASK(10), 0)
#define USBRxPLen_PKT_LNGTH(value)      BIT_FIELD((value), 0)
#define USBRxPLen_PKT_LNGTH_VALUE(reg) \
    FIELD_VALUE((reg), USBRxPLen_PKT_LNGTH_MASK, 0)
#define USBRxPLen_DV                    BIT(10)
#define USBRxPLen_PKT_RDY               BIT(11)
/*------------------USB Control register--------------------------------------*/
#define USBCtrl_RD_EN                   BIT(0)
#define USBCtrl_WR_EN                   BIT(1)
#define USBCtrl_LOG_ENDPOINT_MASK       BIT_FIELD(MASK(4), 2)
#define USBCtrl_LOG_ENDPOINT(value)     BIT_FIELD((value), 2)
#define USBCtrl_LOG_ENDPOINT_VALUE(reg) \
    FIELD_VALUE((reg), USBCtrl_LOG_ENDPOINT_MASK, 2)
/*------------------USB Command Code register---------------------------------*/
#define USBCmdCode_CMD_PHASE_MASK       BIT_FIELD(MASK(8), 8)
#define USBCmdCode_CMD_PHASE(value)     BIT_FIELD((value), 8)
#define USBCmdCode_CMD_PHASE_VALUE(reg) \
    FIELD_VALUE((reg), USBCmdCode_CMD_PHASE_MASK, 8)
#define USBCmdCode_CMD_CODE_MASK        BIT_FIELD(MASK(8), 16)
#define USBCmdCode_CMD_CODE(value)      BIT_FIELD((value), 16)
#define USBCmdCode_CMD_CODE_VALUE(reg) \
    FIELD_VALUE((reg), USBCmdCode_CMD_CODE_MASK, 16)
#define USBCmdCode_CMD_WDATA_MASK       USBCmdCode_CMD_CODE_MASK
#define USBCmdCode_CMD_WDATA(value)     USBCmdCode_CMD_CODE(value)
#define USBCmdCode_CMD_WDATA_VALUE(reg) \
    USBCmdCode_CMD_CODE_VALUE(reg)
/*----------------------------------------------------------------------------*/
enum UsbCommandPhase
{
  USB_CMD_PHASE_WRITE   = 0x01,
  USB_CMD_PHASE_READ    = 0x02,
  USB_CMD_PHASE_COMMAND = 0x05
};

enum UsbCommand
{
  /* Device commands */
  USB_CMD_SET_ADDRESS               = 0xD0,
  USB_CMD_CONFIGURE_DEVICE          = 0xD8,
  USB_CMD_SET_MODE                  = 0xF3,
  USB_CMD_READ_CURRENT_FRAME_NUMBER = 0xF5,
  USB_CMD_READ_TEST_REGISTER        = 0xFD,
  USB_CMD_SET_DEVICE_STATUS         = 0xFE,
  USB_CMD_GET_DEVICE_STATUS         = 0xFE,
  USB_CMD_GET_ERROR_CODE            = 0xFF,
  USB_CMD_READ_ERROR_STATUS         = 0xFB,

  /* Endpoint commands */
  USB_CMD_SELECT_ENDPOINT           = 0x00,
  /* Select endpoint and clear interrupt */
  USB_CMD_CLEAR_INTERRUPT           = 0x40,
  USB_CMD_SET_ENDPOINT_STATUS       = 0x40,
  USB_CMD_CLEAR_BUFFER              = 0xF2,
  USB_CMD_VALIDATE_BUFFER           = 0xFA
};
/*----------------------------------------------------------------------------*/
/* Set Address command */
#define SET_ADDRESS_DEV_ADDR_MASK       BIT_FIELD(MASK(7), 0)
#define SET_ADDRESS_DEV_ADDR(value)     BIT_FIELD((value), 0)
#define SET_ADDRESS_DEV_ADDR_VALUE(reg) \
    FIELD_VALUE((reg), SET_ADDRESS_DEV_ADDR_MASK, 0)
#define SET_ADDRESS_DEV_EN              BIT(7)

/* Configure Device command */
#define CONFIGURE_DEVICE_CONF_DEVICE    BIT(0)

/* Set Mode command */
#define SET_MODE_AP_CLK                 BIT(0)
#define SET_MODE_INAK_CI                BIT(1)
#define SET_MODE_INAK_CO                BIT(2)
#define SET_MODE_INAK_II                BIT(3)
#define SET_MODE_INAK_IO                BIT(4)
#define SET_MODE_INAK_BI                BIT(5)
#define SET_MODE_INAK_BO                BIT(6)

/* Set or Get Device Status command */
#define DEVICE_STATUS_CON               BIT(0)
#define DEVICE_STATUS_CON_CH            BIT(1)
#define DEVICE_STATUS_SUS               BIT(2)
#define DEVICE_STATUS_SUS_CH            BIT(3)
#define DEVICE_STATUS_RST               BIT(4)

/* Select Endpoint command */
#define SELECT_ENDPOINT_FE              BIT(0)
#define SELECT_ENDPOINT_ST              BIT(1)
#define SELECT_ENDPOINT_STP             BIT(2)
#define SELECT_ENDPOINT_PO              BIT(3)
#define SELECT_ENDPOINT_EPN             BIT(4)
#define SELECT_ENDPOINT_B1FULL          BIT(5)
#define SELECT_ENDPOINT_B2FULL          BIT(6)

/* Set Endpoint Status command */
#define SET_ENDPOINT_STATUS_ST          BIT(0)
#define SET_ENDPOINT_STATUS_DA          BIT(5)
#define SET_ENDPOINT_STATUS_RF_MO       BIT(6)
#define SET_ENDPOINT_STATUS_CND_ST      BIT(7)

/* Read Error Status command */
#define READ_ERROR_STATUS_PID_ERR       BIT(0)
#define READ_ERROR_STATUS_UEPKT         BIT(1)
#define READ_ERROR_STATUS_DCRC          BIT(2)
#define READ_ERROR_STATUS_TIMEOUT       BIT(3)
#define READ_ERROR_STATUS_EOP           BIT(4)
#define READ_ERROR_STATUS_B_OVRN        BIT(5)
#define READ_ERROR_STATUS_BTSTF         BIT(6)
#define READ_ERROR_STATUS_TGL_ERR       BIT(7)
/*------------------USB DMA Interrupt Enable register-------------------------*/
#define USBDMAIntEn_EOT                 BIT(0)
#define USBDMAIntEn_NDDR                BIT(1)
#define USBDMAIntEn_ERR                 BIT(2)
/*----------------------------------------------------------------------------*/
struct DmaDescriptor
{
  volatile uint32_t next;
  volatile uint32_t control;
  volatile uint32_t buffer;
  volatile uint32_t status;
  volatile uint32_t size; /* Isochronous endpoints only */
  volatile uint32_t request; /* Project-specific field */
};
/*----------------------------------------------------------------------------*/
enum
{
  DMA_MODE_NORMAL = 0,
  DMA_MODE_ATLE   = 1
};

enum
{
  DD_NOT_SERVICED       = 0,
  DD_BEING_SERVICES     = 1,
  DD_NORMAL_COMPLETION  = 2,
  DD_DATA_UNDERRUN      = 3,
  DD_DATA_OVERRUN       = 8,
  DD_SYSTEM_ERROR       = 9
};

#define DD_CONTROL_DMA_MODE(value)      BIT_FIELD((value), 0)
#define DD_CONTROL_NEXT_DD_VALID        BIT(2)
#define DD_CONTROL_ISOCHRONOUS_EP       BIT(4)

#define DD_CONTROL_MAX_PACKET_SIZE(value) \
    BIT_FIELD((value), 5)
#define DD_CONTROL_MAX_PACKET_SIZE_MASK \
    BIT_FIELD(MASK(10), 5)
#define DD_CONTROL_MAX_PACKET_SIZE_VALUE(reg) \
    FIELD_VALUE((reg), DD_CONTROL_MAX_PACKET_SIZE_MASK, 5)

#define DD_CONTROL_DMA_BUFFER_LENGTH(value) \
    BIT_FIELD((value), 16)
#define DD_CONTROL_DMA_BUFFER_LENGTH_MASK \
    BIT_FIELD(MASK(16), 16)
#define DD_CONTROL_DMA_BUFFER_LENGTH_VALUE(reg) \
    FIELD_VALUE((reg), DD_CONTROL_DMA_BUFFER_LENGTH_MASK, 16)

#define DD_STATUS_RETIRED               BIT(0)
#define DD_STATUS_PACKET_VALID          BIT(5)
#define DD_STATUS_LS_BYTE_EXTRACTED     BIT(6)
#define DD_STATUS_MS_BYTE_EXTRACTED     BIT(7)

#define DD_STATUS_MASK                  BIT_FIELD(MASK(4), 1)
#define DD_STATUS_VALUE(reg)            FIELD_VALUE((reg), DD_STATUS_MASK, 1)

#define DD_STATUS_MESSAGE_LENGTH_POSITION(value) \
    BIT_FIELD((value), 8)
#define DD_STATUS_MESSAGE_LENGTH_POSITION_MASK \
    BIT_FIELD(MASK(6), 8)
#define DD_STATUS_MESSAGE_LENGTH_POSITION_VALUE(reg) \
    FIELD_VALUE((reg), DD_STATUS_MESSAGE_LENGTH_POSITION_MASK, 8)

#define DD_STATUS_PRESENT_DMA_COUNT_MASK \
    BIT_FIELD(MASK(16), 16)
#define DD_STATUS_PRESENT_DMA_COUNT_VALUE(reg) \
    FIELD_VALUE((reg), DD_STATUS_PRESENT_DMA_COUNT_MASK, 16)
/*----------------------------------------------------------------------------*/
#define EP_TO_INDEX(ep)     ((((ep) & 0x0F) << 1) | (((ep) & 0x80) >> 7))
#define INDEX_TO_EP(index)  ((((index) << 7) & 0x80) | (((index) >> 1) & 0x0F))
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC17XX_USB_DEFS_H_ */
