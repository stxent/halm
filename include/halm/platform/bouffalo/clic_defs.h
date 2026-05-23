/*
 * halm/platform/bouffalo/clic_defs.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_CLIC_DEFS_H_
#define HALM_PLATFORM_BOUFFALO_CLIC_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Machine Status register-----------------------------------*/
#define MSTATUS_UIE                     BIT(0)
#define MSTATUS_SIE                     BIT(1)
#define MSTATUS_HIE                     BIT(2)
#define MSTATUS_MIE                     BIT(3)
#define MSTATUS_UPIE                    BIT(4)
#define MSTATUS_SPIE                    BIT(5)
#define MSTATUS_HPIE                    BIT(6)
#define MSTATUS_MPIE                    BIT(7)
#define MSTATUS_SPP                     BIT(8)
#define MSTATUS_MPP                     BIT_FIELD(MASK(2), 11)
#define MSTATUS_FS                      BIT_FIELD(MASK(2), 13)
#define MSTATUS_XS                      BIT_FIELD(MASK(2), 15)
#define MSTATUS_MPRV                    BIT(17)
#define MSTATUS_PUM                     BIT(18)
#define MSTATUS_MXR                     BIT(19)
#define MSTATUS_VM                      BIT_FIELD(MASK(5), 24)
#define MSTATUS_SD                      BIT(31)
/*------------------Machine Trap-Vector Base-Address register-----------------*/
enum
{
  MODE_CLINT_DIRECT   = 0,
  MODE_CLINT_VECTORED = 1,
  MODE_CLIC_DIRECT    = 2,
  MODE_CLIC_VECTORED  = 3
};

#define MTVEC_MODE(value)               BIT_FIELD((value), 0)
#define MTVEC_MODE_MASK                 BIT_FIELD(MASK(6), 0)
#define MTVEC_MODE_VALUE(reg)           FIELD_VALUE((reg), MTVEC_MODE_MASK, 0)

#define MTVEC_CLINT_BASE(value)         BIT_FIELD((value), 2)
#define MTVEC_CLINT_BASE_MASK           BIT_FIELD(MASK(30), 2)
#define MTVEC_CLINT_BASE_VALUE(reg) \
    FIELD_VALUE((reg), MTVEC_CLINT_BASE_MASK, 2)

#define MTVEC_CLIC_BASE(value)          BIT_FIELD((value), 6)
#define MTVEC_CLIC_BASE_MASK            BIT_FIELD(MASK(26), 6)
#define MTVEC_CLIC_BASE_VALUE(reg) \
    FIELD_VALUE((reg), MTVEC_CLIC_BASE_MASK, 6)
/*------------------Machine Cause register------------------------------------*/
#define MCAUSE_EXCCODE(value)           BIT_FIELD((value), 0)
#define MCAUSE_EXCCODE_MASK             BIT_FIELD(MASK(10), 0)
#define MCAUSE_EXCCODE_VALUE(reg) \
    FIELD_VALUE((reg), MCAUSE_EXCCODE_MASK, 0)

#define MCAUSE_MPIL(value)              BIT_FIELD((value), 16)
#define MCAUSE_MPIL_MASK                BIT_FIELD(MASK(8), 16)
#define MCAUSE_MPIL_VALUE(reg)          FIELD_VALUE((reg), MCAUSE_MPIL_MASK, 16)

#define MCAUSE_MPIE                     BIT(27)

#define MCAUSE_MPP(value)               BIT_FIELD((value), 28)
#define MCAUSE_MPP_MASK                 BIT_FIELD(MASK(2), 28)
#define MCAUSE_MPP_VALUE(reg) \         FIELD_VALUE((reg), MCAUSE_MPP_MASK, 28)

#define MCAUSE_MINHV                    BIT(30)
#define MCAUSE_INTERRUPT                BIT(31)
/*------------------Machine Interrupt Pending register------------------------*/
#define MIP_SSIP                        BIT(1)
#define MIP_HSIP                        BIT(2)
#define MIP_MSIP                        BIT(3)
#define MIP_STIP                        BIT(5)
#define MIP_HTIP                        BIT(6)
#define MIP_MTIP                        BIT(7)
#define MIP_SEIP                        BIT(9)
#define MIP_HEIP                        BIT(10)
#define MIP_MEIP                        BIT(11)
/*------------------Machine Interrupt Enable register-------------------------*/
#define MIE_SSIE                        BIT(1)
#define MIE_HSIE                        BIT(2)
#define MIE_MSIE                        BIT(3)
#define MIE_STIE                        BIT(5)
#define MIE_HTIE                        BIT(6)
#define MIE_MTIE                        BIT(7)
#define MIE_SEIE                        BIT(9)
#define MIE_HEIE                        BIT(10)
#define MIE_MEIE                        BIT(11)
/*------------------CLIC Configuration register-------------------------------*/
#define CLICCFG_NVBITS                  BIT(0)

#define CLICCFG_NLBITS(value)           BIT_FIELD((value), 1)
#define CLICCFG_NLBITS_MASK             BIT_FIELD(MASK(4), 1)
#define CLICCFG_NLBITS_VALUE(reg) \
    FIELD_VALUE((reg), CLICCFG_NLBITS_MASK, 1)

#define CLICCFG_NMBITS(value)           BIT_FIELD((value), 5)
#define CLICCFG_NMBITS_MASK             BIT_FIELD(MASK(2), 5)
#define CLICCFG_NMBITS_VALUE(reg) \
    FIELD_VALUE((reg), CLICCFG_NMBITS_MASK, 5)
/*------------------CLIC Interrupt Pending register---------------------------*/
#define CLICINTIP_PENDING               BIT(0)
/*------------------CLIC Interrupt Enable register----------------------------*/
#define CLICINTIE_ENABLE                BIT(0)
/*------------------CLIC Interrupt Configuration register---------------------*/
#define CLICINTCFG_DIRECT_LEVEL(value) \
    BIT_FIELD((value), 4)
#define CLICINTCFG_DIRECT_LEVEL_MASK \
    BIT_FIELD(MASK(4), 4)
#define CLICINTCFG_DIRECT_LEVEL_VALUE(reg) \
    FIELD_VALUE((reg), CLICINTCFG_DIRECT_LEVEL_MASK, 4)

#define CLICINTCFG_VECTORED_LEVEL(value) \
    BIT_FIELD((value), 5)
#define CLICINTCFG_VECTORED_LEVEL_MASK \
    BIT_FIELD(MASK(3), 5)
#define CLICINTCFG_VECTORED_LEVEL_VALUE(reg) \
    FIELD_VALUE((reg), CLICINTCFG_VECTORED_LEVEL_MASK, 5)

#define CLICINTCFG_VECTORED             BIT(4)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_CLIC_DEFS_H_ */
