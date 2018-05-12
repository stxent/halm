/*
 * halm/platform/stm/stm32f1xx/usb_defs.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_STM_STM32F1XX_USB_DEFS_H_
#define HALM_PLATFORM_STM_STM32F1XX_USB_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------USB Control Register--------------------------------------*/
#define CNTR_FRES                       BIT(0)
#define CNTR_PDWN                       BIT(1)
#define CNTR_LP_MODE                    BIT(2)
#define CNTR_FSUSP                      BIT(3)
#define CNTR_RESUME                     BIT(4)
#define CNTR_ESOFM                      BIT(8)
#define CNTR_SOFM                       BIT(9)
#define CNTR_RESETM                     BIT(10)
#define CNTR_SUSPM                      BIT(11)
#define CNTR_WKUPM                      BIT(12)
#define CNTR_ERRM                       BIT(13)
#define CNTR_PMAOVRM                    BIT(14)
#define CNTR_CTRM                       BIT(15)
/*------------------Interrupt Status Register---------------------------------*/
#define ISTR_EP_ID_MASK                 BIT_FIELD(MASK(4), 0)
#define ISTR_EP_ID(value)               BIT_FIELD((value), 0)
#define ISTR_EP_ID_VALUE(reg)           FIELD_VALUE((reg), ISTR_EP_ID_MASK, 0)

#define ISTR_DIR                        BIT(4)
#define ISTR_ESOF                       BIT(8)
#define ISTR_SOF                        BIT(9)
#define ISTR_RESET                      BIT(10)
#define ISTR_SUSP                       BIT(11)
#define ISTR_WKUP                       BIT(12)
#define ISTR_ERR                        BIT(13)
#define ISTR_PMAOVR                     BIT(14)
#define ISTR_CTR                        BIT(15)

#define ISTR_MASK \
    (ISTR_ESOF | ISTR_SOF | ISTR_RESET | ISTR_SUSP | ISTR_WKUP \
        | ISTR_ERR | ISTR_PMAOVR)
/*------------------Frame Number Register-------------------------------------*/
#define FNR_FN_MASK                     BIT_FIELD(MASK(11), 0)
#define FNR_FN(value)                   BIT_FIELD((value), 0)
#define FNR_FN_VALUE(reg)               FIELD_VALUE((reg), FNR_FN_MASK, 0)

#define FNR_LSOF_MASK                   BIT_FIELD(MASK(2), 11)
#define FNR_LSOF(value)                 BIT_FIELD((value), 11)
#define FNR_LSOF_VALUE(reg)             FIELD_VALUE((reg), FNR_LSOF_MASK, 11)

#define FNR_LCK                         BIT(13)
#define FNR_RXDM                        BIT(14)
#define FNR_RXDP                        BIT(15)
/*------------------Device Address Register-----------------------------------*/
#define DADDR_ADD_MASK                  BIT_FIELD(MASK(7), 0)
#define DADDR_ADD(value)                BIT_FIELD((value), 0)
#define DADDR_ADD_VALUE(reg)            FIELD_VALUE((reg), DADDR_ADD_MASK, 0)

#define DADDR_EF                        BIT(7)
/*------------------Endpoint Register-----------------------------------------*/
enum
{
  EPR_STAT_DISABLED,
  EPR_STAT_STALL,
  EPR_STAT_NAK,
  EPR_STAT_VALID
};

enum
{
  EPR_TYPE_BULK,
  EPR_TYPE_CONTROL,
  EPR_TYPE_ISO,
  EPR_TYPE_INTERRUPT
};

#define EPR_EA_MASK                     BIT_FIELD(MASK(4), 0)
#define EPR_EA(value)                   BIT_FIELD((value), 0)
#define EPR_EA_VALUE(reg)               FIELD_VALUE((reg), EPR_EA_MASK, 0)

#define EPR_DTOG_TX                     BIT(6)
#define EPR_CTR_TX                      BIT(7)
#define EPR_EP_KIND                     BIT(8)

#define EPR_STAT_TX_MASK                BIT_FIELD(MASK(2), 4)
#define EPR_STAT_TX(value)              BIT_FIELD((value), 4)
#define EPR_STAT_TX_VALUE(reg)          FIELD_VALUE((reg), EPR_STAT_TX_MASK, 4)

#define EPR_SETUP                       BIT(11)

#define EPR_EP_TYPE_MASK                BIT_FIELD(MASK(2), 9)
#define EPR_EP_TYPE(value)              BIT_FIELD((value), 9)
#define EPR_EP_TYPE_VALUE(reg)          FIELD_VALUE((reg), EPR_EP_TYPE_MASK, 9)

#define EPR_STAT_RX_MASK                BIT_FIELD(MASK(2), 12)
#define EPR_STAT_RX(value)              BIT_FIELD((value), 12)
#define EPR_STAT_RX_VALUE(reg)          FIELD_VALUE((reg), EPR_STAT_RX_MASK, 12)

#define EPR_DTOG_RX                     BIT(14)
#define EPR_CTR_RX                      BIT(15)

#define EPR_CTR_MASK                    (EPR_CTR_TX | EPR_CTR_RX)
#define EPR_TOGGLE_MASK \
    (EPR_EA_MASK | EPR_EP_TYPE_MASK | EPR_EP_KIND | EPR_SETUP)
/*------------------Reception byte count--------------------------------------*/
#define COUNT_RX_MASK                   BIT_FIELD(MASK(10), 0)
#define COUNT_RX(value)                 BIT_FIELD((value), 0)
#define COUNT_RX_VALUE(reg)             FIELD_VALUE((reg), COUNT_RX_MASK, 0)

#define COUNT_RX_NUM_BLOCK_MASK         BIT_FIELD(MASK(5), 10)
#define COUNT_RX_NUM_BLOCK(value)       BIT_FIELD((value), 10)
#define COUNT_RX_NUM_BLOCK_VALUE(reg) \
    FIELD_VALUE((reg), COUNT_RX_NUM_BLOCK_MASK, 10)

#define COUNT_RX_BLSIZE                 BIT(15)
/*----------------------------------------------------------------------------*/
#define DESCRIPTOR_TABLE_SIZE 0x40
#define EP_TO_INDEX(ep)       ((((ep) & 0x0F) << 1) | (((ep) & 0x80) >> 7))
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM_STM32F1XX_USB_DEFS_H_ */
