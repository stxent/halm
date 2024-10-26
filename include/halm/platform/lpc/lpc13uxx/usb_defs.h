/*
 * halm/platform/lpc/lpc13uxx/usb_defs.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC13UXX_USB_DEFS_H_
#define HALM_PLATFORM_LPC_LPC13UXX_USB_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
/* Aligned along 64-byte boundary */
#define BUFFER_ALIGNMENT  64
//#define SETUP_BUFFER_SIZE BUFFER_ALIGNMENT

#define USB_EP_NUMBER     10
#define USB_EP_LIST_SIZE  (sizeof(uint32_t) * USB_EP_NUMBER * 2)
/*------------------Device Command/Status register----------------------------*/
#define DEVCMDSTAT_DEV_ADDR_MASK        BIT_FIELD(MASK(7), 0)
#define DEVCMDSTAT_DEV_ADDR(value)      BIT_FIELD((value), 0)
#define DEVCMDSTAT_DEV_ADDR_VALUE(reg) \
    FIELD_VALUE((reg), DEVCMDSTAT_DEV_ADDR_MASK, 0)

#define DEVCMDSTAT_DEV_EN               BIT(7)
#define DEVCMDSTAT_SETUP                BIT(8)
#define DEVCMDSTAT_PLL_ON               BIT(9)
#define DEVCMDSTAT_LPM_SUP              BIT(11)
#define DEVCMDSTAT_INTONNAK_AO          BIT(12)
#define DEVCMDSTAT_INTONNAK_AI          BIT(13)
#define DEVCMDSTAT_INTONNAK_CO          BIT(14)
#define DEVCMDSTAT_INTONNAK_CI          BIT(15)
#define DEVCMDSTAT_DCON                 BIT(16)
#define DEVCMDSTAT_DSUS                 BIT(17)
#define DEVCMDSTAT_LPM_SUS              BIT(19)
#define DEVCMDSTAT_LPM_REWP             BIT(20)
#define DEVCMDSTAT_DCON_C               BIT(24)
#define DEVCMDSTAT_DSUS_C               BIT(25)
#define DEVCMDSTAT_DRES_C               BIT(26)
#define DEVCMDSTAT_VBUSDEBOUNCED        BIT(28)
/*------------------Info register---------------------------------------------*/
#define INFO_FRAME_NR_MASK              BIT_FIELD(MASK(11), 0)
#define INFO_FRAME_NR(value)            BIT_FIELD((value), 0)
#define INFO_FRAME_NR_VALUE(reg) \
    FIELD_VALUE((reg), INFO_FRAME_NR_MASK, 0)

#define INFO_ERR_CODE_MASK              BIT_FIELD(MASK(4), 11)
#define INFO_ERR_CODE(value)            BIT_FIELD((value), 11)
#define INFO_ERR_CODE_VALUE(reg) \
    FIELD_VALUE((reg), INFO_ERR_CODE_MASK, 11)
/*------------------Link Power Management register----------------------------*/
#define LPM_HIRD_HW_MASK                BIT_FIELD(MASK(4), 0)
#define LPM_HIRD_HW(value)              BIT_FIELD((value), 0)
#define LPM_HIRD_HW_VALUE(reg)          FIELD_VALUE((reg), LPM_HIRD_HW_MASK, 0)

#define LPM_HIRD_SW_MASK                BIT_FIELD(MASK(4), 4)
#define LPM_HIRD_SW(value)              BIT_FIELD((value), 4)
#define LPM_HIRD_SW_VALUE(reg)          FIELD_VALUE((reg), LPM_HIRD_SW_MASK, 4)

#define LPM_DATA_PENDING                BIT(8)
/*------------------Endpoint Skip---------------------------------------------*/
#define EPSKIP_SKIP(index)              BIT(index)
#define EPSKIP_MASK                     MASK(30)
/*------------------Endpoint Toggle-------------------------------------------*/
#define EPTOGGLE_TOGGLE(index)          BIT(index)
/*------------------Endpoint Buffer in use------------------------------------*/
#define EPINUSE_BUF(index)              BIT(index)
/*------------------Endpoint Buffer Configuration-----------------------------*/
#define EPBUFCFG_BUF_SB(index)          BIT(index)
/*------------------Interrupt Status register---------------------------------*/
#define INTSTAT_EP_INT_MASK             MASK(10)
#define INTSTAT_EP_INT(index)           BIT(index)
#define INTSTAT_OUT_INT(ep)             BIT((ep) * 2)
#define INTSTAT_IN_INT(ep)              BIT((ep) * 2 + 1)
#define INTSTAT_FRAME_INT               BIT(30)
#define INTSTAT_DEV_INT                 BIT(31)
#define INTSTAT_MASK \
    (INTSTAT_EP_INT_MASK | INTSTAT_FRAME_INT | INTSTAT_DEV_INT)
/*------------------Interrupt Enable register---------------------------------*/
#define INTEN_EP_INT_EN(index)          BIT(index)
#define INTEN_FRAME_INT_EN              BIT(30)
#define INTEN_DEV_INT_EN                BIT(31)
/*------------------Set Interrupt Status register-----------------------------*/
#define INTSETSTAT_EP_SET_INT(index)    BIT(index)
#define INTSETSTAT_FRAME_SET_INT        BIT(30)
#define INTSETSTAT_DEV_SET_INT          BIT(31)
/*------------------Interrupt Routing register--------------------------------*/
#define INTROUTING_ROUTE_EP_INT(index)  BIT(index)
#define INTROUTING_ROUTE_FRAME_INT      BIT(30)
#define INTROUTING_ROUTE_DEV_INT        BIT(31)
/*------------------EP Command/Status List start address----------------------*/
#define EPLISTSTART_EP_LIST_MASK        BIT_FIELD(MASK(24), 8)
#define EPLISTSTART_EP_LIST(value)      ((value) & EPLISTSTART_EP_LIST_MASK)
/*------------------Data buffer start address---------------------------------*/
#define DATABUFSTART_DA_BUF_MASK        BIT_FIELD(MASK(10), 22)
#define DATABUFSTART_DA_BUF(value)      ((value) & DATABUFSTART_DA_BUF_MASK)
/*------------------Endpoint Command/Status List------------------------------*/
#define EPCS_NBytes_MASK                BIT_FIELD(MASK(10), 16)
#define EPCS_NBytes(value)              BIT_FIELD((value), 16)
#define EPCS_NBytes_VALUE(reg)          FIELD_VALUE((reg), EPCS_NBytes_MASK, 16)

#define EPCS_AddressOffset_MASK         BIT_FIELD(MASK(16), 0)
#define EPCS_AddressOffset(value)       BIT_FIELD((value), 0)
#define EPCS_AddressOffset_VALUE(reg) \
    FIELD_VALUE((reg), EPCS_AddressOffset_MASK, 0)

/* Type */
#define EPCS_T                          BIT(26)
/* Rate feedback mode / Toggle value */
#define EPCS_RF_TV                      BIT(27)
/* Toggle Reset */
#define EPCS_TR                         BIT(28)
/* Stall */
#define EPCS_S                          BIT(29)
/* Disabled */
#define EPCS_D                          BIT(30)
/* Active */
#define EPCS_A                          BIT(31)
/*----------------------------------------------------------------------------*/
static inline uint16_t epcsAlignSize(uint16_t size)
{
  return (size + (BUFFER_ALIGNMENT - 1)) & ~(BUFFER_ALIGNMENT - 1);
}

static inline uint32_t epcsSetupTransfer(uint32_t value, uint16_t size,
    uint16_t offset)
{
  return (value & ~(EPCS_NBytes_MASK | EPCS_AddressOffset_MASK))
      | EPCS_NBytes(size)
      | EPCS_AddressOffset(offset >> 6);
}
/*----------------------------------------------------------------------------*/
#define EP_TO_INDEX(ep)     ((((ep) & 0x0F) << 1) | (((ep) & 0x80) >> 7))
#define INDEX_TO_EP(index)  ((((index) << 7) & 0x80) | (((index) >> 1) & 0x0F))
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC13UXX_USB_DEFS_H_ */
