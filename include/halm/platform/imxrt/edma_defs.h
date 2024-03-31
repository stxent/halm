/*
 * halm/platform/imxrt/edma_defs.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_IMXRT_EDMA_DEFS_H_
#define HALM_PLATFORM_IMXRT_EDMA_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
#include <limits.h>
/*------------------Control Register------------------------------------------*/
#define CR_EDBG                         BIT(1)
#define CR_ERCA                         BIT(2)
#define CR_ERGA                         BIT(3)
#define CR_HOE                          BIT(4)
#define CR_HALT                         BIT(5)
#define CR_CLM                          BIT(6)
#define CR_EMLM                         BIT(7)
#define CR_GRP0PRI                      BIT(8)
#define CR_GRP1PRI                      BIT(10)
#define CR_ECX                          BIT(16)
#define CR_CX                           BIT(17)
#define CR_ACTIVE                       BIT(31)
/*------------------Error Status register-------------------------------------*/
#define ES_DBE                          BIT(0)
#define ES_SBE                          BIT(1)
#define ES_SGE                          BIT(2)
#define ES_NCE                          BIT(3)
#define ES_DOE                          BIT(4)
#define ES_DAE                          BIT(5)
#define ES_SOE                          BIT(6)
#define ES_SAE                          BIT(7)

#define ES_ERRCHN_MASK                  BIT_FIELD(MASK(5), 8)
#define ES_ERRCHN(value)                BIT_FIELD((value), 8)
#define ES_ERRCHN_VALUE(reg)            FIELD_VALUE((reg), ES_ERRCHN_MASK, 8)

#define ES_CPE                          BIT(14)
#define ES_GPE                          BIT(15)
#define ES_ECX                          BIT(16)
#define ES_VLD                          BIT(31)
/*------------------Clear Enable Error Interrupt register---------------------*/
#define CEEI_CEEI_MASK                  BIT_FIELD(MASK(5), 0)
#define CEEI_CEEI(value)                BIT_FIELD((value), 0)
#define CEEI_CEEI_VALUE(reg)            FIELD_VALUE((reg), CEEI_CEEI_MASK, 0)

#define CEEI_CAEE                       BIT(6)
#define CEEI_NOP                        BIT(7)
/*------------------Set Enable Error Interrupt register-----------------------*/
#define SEEI_SEEI_MASK                  BIT_FIELD(MASK(5), 0)
#define SEEI_SEEI(value)                BIT_FIELD((value), 0)
#define SEEI_SEEI_VALUE(reg)            FIELD_VALUE((reg), SEEI_SEEI_MASK, 0)

#define SEEI_SAEE                       BIT(6)
#define SEEI_NOP                        BIT(7)
/*------------------Clear Enable Request register-----------------------------*/
#define CERQ_CERQ_MASK                  BIT_FIELD(MASK(5), 0)
#define CERQ_CERQ(value)                BIT_FIELD((value), 0)
#define CERQ_CERQ_VALUE(reg)            FIELD_VALUE((reg), CERQ_CERQ_MASK, 0)

#define CERQ_CAER                       BIT(6)
#define CERQ_NOP                        BIT(7)
/*------------------Set Enable Request register-------------------------------*/
#define SERQ_SERQ_MASK                  BIT_FIELD(MASK(5), 0)
#define SERQ_SERQ(value)                BIT_FIELD((value), 0)
#define SERQ_SERQ_VALUE(reg)            FIELD_VALUE((reg), SERQ_SERQ_MASK, 0)

#define SERQ_SAER                       BIT(6)
#define SERQ_NOP                        BIT(7)
/*------------------Clear DONE Status Bit register----------------------------*/
#define CDNE_CDNE_MASK                  BIT_FIELD(MASK(5), 0)
#define CDNE_CDNE(value)                BIT_FIELD((value), 0)
#define CDNE_CDNE_VALUE(reg)            FIELD_VALUE((reg), CDNE_CDNE_MASK, 0)

#define CDNE_CADN                       BIT(6)
#define CDNE_NOP                        BIT(7)
/*------------------Set START Bit register------------------------------------*/
#define SSRT_SSRT_MASK                  BIT_FIELD(MASK(5), 0)
#define SSRT_SSRT(value)                BIT_FIELD((value), 0)
#define SSRT_SSRT_VALUE(reg)            FIELD_VALUE((reg), SSRT_SSRT_MASK, 0)

#define SSRT_SAST                       BIT(6)
#define SSRT_NOP                        BIT(7)
/*------------------Clear Error register--------------------------------------*/
#define CERR_CERR_MASK                  BIT_FIELD(MASK(5), 0)
#define CERR_CERR(value)                BIT_FIELD((value), 0)
#define CERR_CERR_VALUE(reg)            FIELD_VALUE((reg), CERR_CERR_MASK, 0)

#define CERR_CAEI                       BIT(6)
#define CERR_NOP                        BIT(7)
/*------------------Clear Interrupt Request register--------------------------*/
#define CINT_CINT_MASK                  BIT_FIELD(MASK(5), 0)
#define CINT_CINT(value)                BIT_FIELD((value), 0)
#define CINT_CINT_VALUE(reg)            FIELD_VALUE((reg), CINT_CINT_MASK, 0)

#define CINT_CAIR                       BIT(6)
#define CINT_NOP                        BIT(7)
/*------------------TCD Signed Source and Destination Offsets-----------------*/
#define TCD_OFF_MAX                     INT16_MAX
#define TCD_OFF_MIN                     INT16_MIN
/*------------------TCD Transfer Attributes-----------------------------------*/
enum
{
  SIZE_8BIT         = 0,
  SIZE_16BIT        = 1,
  SIZE_32BIT        = 2,
  SIZE_64BIT        = 3,
  SIZE_32BIT_BURST  = 5
};

#define TCD_ATTR_DSIZE_MASK             BIT_FIELD(MASK(3), 0)
#define TCD_ATTR_DSIZE(value)           BIT_FIELD((value), 0)
#define TCD_ATTR_DSIZE_VALUE(reg) \
    FIELD_VALUE((reg), TCD_ATTR_DSIZE_MASK, 0)

#define TCD_ATTR_DMOD_MASK              BIT_FIELD(MASK(5), 3)
#define TCD_ATTR_DMOD(value)            BIT_FIELD((value), 3)
#define TCD_ATTR_DMOD_VALUE(reg) \
    FIELD_VALUE((reg), TCD_ATTR_DMOD_MASK, 3)

#define TCD_ATTR_SSIZE_MASK             BIT_FIELD(MASK(3), 8)
#define TCD_ATTR_SSIZE(value)           BIT_FIELD((value), 8)
#define TCD_ATTR_SSIZE_VALUE(reg) \
    FIELD_VALUE((reg), TCD_ATTR_DSIZE_MASK, 8)

#define TCD_ATTR_SMOD_MASK              BIT_FIELD(MASK(5), 11)
#define TCD_ATTR_SMOD(value)            BIT_FIELD((value), 11)
#define TCD_ATTR_SMOD_VALUE(reg) \
    FIELD_VALUE((reg), TCD_ATTR_SMOD_MASK, 11)
/*------------------TCD Signed Minor Loop Offset (MLOFF disabled)-------------*/
#define TCD_NBYTES_MLOFFNO_NBYTES_MASK  BIT_FIELD(MASK(30), 0)
#define TCD_NBYTES_MLOFFNO_NBYTES(value) \
    BIT_FIELD((value), 0)
#define TCD_NBYTES_MLOFFNO_NBYTES_VALUE(reg) \
    FIELD_VALUE((reg), TCD_NBYTES_MLOFFNO_NBYTES_MASK, 0)
#define TCD_NBYTES_MLOFFNO_NBYTES_MAX   MASK(30)

#define TCD_NBYTES_MLOFFNO_DMLOE        BIT(30)
#define TCD_NBYTES_MLOFFNO_SMLOE        BIT(31)
/*------------------TCD Signed Minor Loop Offset (MLOFF enabled)--------------*/
#define TCD_NBYTES_MLOFFYES_NBYTES_MASK BIT_FIELD(MASK(10), 0)
#define TCD_NBYTES_MLOFFYES_NBYTES(value) \
    BIT_FIELD((value), 0)
#define TCD_NBYTES_MLOFFYES_NBYTES_VALUE(reg) \
    FIELD_VALUE((reg), TCD_NBYTES_MLOFFYES_NBYTES_MASK, 0)
#define TCD_NBYTES_MLOFFYES_NBYTES_MAX  MASK(10)

#define TCD_NBYTES_MLOFFYES_MLOFF_MASK BIT_FIELD(MASK(20), 10)
#define TCD_NBYTES_MLOFFYES_MLOFF(value) \
    BIT_FIELD((value), 10)
#define TCD_NBYTES_MLOFFYES_MLOFF_VALUE(reg) \
    FIELD_VALUE((reg), TCD_NBYTES_MLOFFYES_MLOFF_MASK, 10)
#define TCD_NBYTES_MLOFFYES_MLOFF_MAX   MASK(20)

#define TCD_NBYTES_MLOFFYES_DMLOE       BIT(30)
#define TCD_NBYTES_MLOFFYES_SMLOE       BIT(31)
/*------------------TCD Current Minor Loop Link, Major Loop Count-------------*/
/* Channel Linking Disabled */
#define TCD_CITER_ELINKNO_CITER_MASK    BIT_FIELD(MASK(15), 0)
#define TCD_CITER_ELINKNO_CITER(value)  BIT_FIELD((value), 0)
#define TCD_CITER_ELINKNO_CITER_VALUE(reg) \
    FIELD_VALUE((reg), TCD_CITER_ELINKNO_CITER_MASK, 0)
#define TCD_CITER_ELINKNO_CITER_MAX     MASK(15)

#define TCD_CITER_ELINKNO_CITER_ELINK   BIT(15)
/*------------------TCD Current Minor Loop Link, Major Loop Count-------------*/
/* Channel Linking Enabled */
#define TCD_CITER_ELINKYES_CITER_MASK   BIT_FIELD(MASK(9), 0)
#define TCD_CITER_ELINKYES_CITER(value) \
    BIT_FIELD((value), 0)
#define TCD_CITER_ELINKYES_CITER_VALUE(reg) \
    FIELD_VALUE((reg), TCD_CITER_ELINKYES_CITER_MASK, 0)
#define TCD_CITER_ELINKYES_CITER_MAX    MASK(9)

#define TCD_CITER_ELINKYES_LINKCH_MASK  BIT_FIELD(MASK(5), 9)
#define TCD_CITER_ELINKYES_LINKCH(value) \
    BIT_FIELD((value), 9)
#define TCD_CITER_ELINKYES_LINKCH_VALUE(reg) \
    FIELD_VALUE((reg), TCD_CITER_ELINKYES_LINKCH_MASK, 9)

#define TCD_CITER_ELINKYES_CITER_ELINK  BIT(15)
/*------------------TCD Control and Status------------------------------------*/
enum
{
  BWC_NO_STALL        = 0,
  BWC_STALL_4_CYCLES  = 2,
  BWC_STALL_8_CYCLES  = 3
};

#define TCD_CSR_START                   BIT(0)
#define TCD_CSR_INTMAJOR                BIT(1)
#define TCD_CSR_INTHALF                 BIT(2)
#define TCD_CSR_DREQ                    BIT(3)
#define TCD_CSR_ESG                     BIT(4)
#define TCD_CSR_MAJORELINK              BIT(5)
#define TCD_CSR_ACTIVE                  BIT(6)
#define TCD_CSR_DONE                    BIT(7)

#define TCD_CSR_MAJORLINKCH_MASK        BIT_FIELD(MASK(5), 8)
#define TCD_CSR_MAJORLINKCH(value)      BIT_FIELD((value), 8)
#define TCD_CSR_MAJORLINKCH_VALUE(reg) \
    FIELD_VALUE((reg), TCD_CSR_MAJORLINKCH_MASK, 8)

#define TCD_CSR_BWC_MASK                BIT_FIELD(MASK(2), 14)
#define TCD_CSR_BWC(value)              BIT_FIELD((value), 14)
#define TCD_CSR_BWC_VALUE(reg) \
    FIELD_VALUE((reg), TCD_CSR_BWC_MASK, 14)
/*------------------TCD Beginning Minor Loop Link, Major Loop Count-----------*/
/* Channel Linking Disabled */
#define TCD_BITER_ELINKNO_BITER_MASK    BIT_FIELD(MASK(15), 0)
#define TCD_BITER_ELINKNO_BITER(value)  BIT_FIELD((value), 0)
#define TCD_BITER_ELINKNO_BITER_VALUE(reg) \
    FIELD_VALUE((reg), TCD_BITER_ELINKNO_BITER_MASK, 0)
#define TCD_BITER_ELINKNO_BITER_MAX     MASK(15)

#define TCD_BITER_ELINKNO_BITER_ELINK   BIT(15)
/*------------------TCD Beginning Minor Loop Link, Major Loop Count-----------*/
/* Channel Linking Enabled */
#define TCD_BITER_ELINKYES_BITER_MASK   BIT_FIELD(MASK(9), 0)
#define TCD_BITER_ELINKYES_BITER(value) \
    BIT_FIELD((value), 0)
#define TCD_BITER_ELINKYES_BITER_VALUE(reg) \
    FIELD_VALUE((reg), TCD_BITER_ELINKYES_BITER_MASK, 0)
#define TCD_BITER_ELINKYES_BITER_MAX    MASK(9)

#define TCD_BITER_ELINKYES_LINKCH_MASK  BIT_FIELD(MASK(5), 9)
#define TCD_BITER_ELINKYES_LINKCH(value) \
    BIT_FIELD((value), 9)
#define TCD_BITER_ELINKYES_LINKCH_VALUE(reg) \
    FIELD_VALUE((reg), TCD_BITER_ELINKYES_LINKCH_MASK, 9)

#define TCD_BITER_ELINKYES_BITER_ELINK  BIT(15)
/*------------------DMAMUX Channel Configuration------------------------------*/
#define CHCFG_SOURCE_MASK               BIT_FIELD(MASK(8), 0)
#define CHCFG_SOURCE(value)             BIT_FIELD((value), 0)
#define CHCFG_SOURCE_VALUE(reg)         FIELD_VALUE((reg), CHCFG_SOURCE_MASK, 0)

#define CHCFG_A_ON                      BIT(29)
#define CHCFG_TRIG                      BIT(30)
#define CHCFG_ENBL                      BIT(31)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_IMXRT_EDMA_DEFS_H_ */
