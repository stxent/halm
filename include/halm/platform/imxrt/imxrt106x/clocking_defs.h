/*
 * halm/platform/imxrt/imxrt106x/clocking_defs.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_IMXRT_IMXRT106X_CLOCKING_DEFS_H_
#define HALM_PLATFORM_IMXRT_IMXRT106X_CLOCKING_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------CCM Control Register--------------------------------------*/
#define CCR_OSCNT_MASK                  BIT_FIELD(MASK(8), 0)
#define CCR_OSCNT(value)                BIT_FIELD((value), 0)
#define CCR_OSCNT_VALUE(reg)            FIELD_VALUE((reg), CCR_OSCNT_MASK, 0)

#define CCR_COSC_EN                     BIT(12)

#define CCR_REG_BYPASS_COUNT_MASK       BIT_FIELD(MASK(6), 21)
#define CCR_REG_BYPASS_COUNT(value)     BIT_FIELD((value), 21)
#define CCR_REG_BYPASS_COUNT_VALUE(reg) \
    FIELD_VALUE((reg), CCR_REG_BYPASS_COUNT_MASK, 21)

#define CCR_RBC_EN                      BIT(27)
/*------------------CCM Status Register---------------------------------------*/
#define CSR_REF_EN_B                    BIT(0)
#define CSR_CAMP2_READY                 BIT(3)
#define CSR_COSC_READY                  BIT(5)
/*------------------CCM Clock Switcher Register-------------------------------*/
/* Set to enable PLL3 bypass clock, clear to enable PLL3 main clock */
#define CCSR_PLL3_SW_CLK_SEL            BIT(0)
/*------------------CCM Arm Clock Root Register-------------------------------*/
#define CACRR_ARM_PODF_MASK             BIT_FIELD(MASK(3), 0)
#define CACRR_ARM_PODF(value)           BIT_FIELD((value), 0)
#define CACRR_ARM_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CACRR_ARM_PODF_MASK, 0)
/*------------------CCM Bus Clock Divider Register----------------------------*/
/* Set to select SEMC alternative clock, clear to select periph clock */
#define CBCDR_SEMC_CLK_SEL              BIT(6)
/* Set to use PLL3 PFD1, clear to use PLL2 PFD2 */
#define CBCDR_SEMC_ALT_CLK_SEL          BIT(7)

#define CBCDR_IPG_PODF_MASK             BIT_FIELD(MASK(2), 8)
#define CBCDR_IPG_PODF(value)           BIT_FIELD((value), 8)
#define CBCDR_IPG_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CBCDR_IPG_PODF_MASK, 8)

#define CBCDR_AHB_PODF_MASK             BIT_FIELD(MASK(3), 10)
#define CBCDR_AHB_PODF(value)           BIT_FIELD((value), 10)
#define CBCDR_AHB_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CBCDR_AHB_PODF_MASK, 10)

#define CBCDR_SEMC_PODF_MASK            BIT_FIELD(MASK(3), 16)
#define CBCDR_SEMC_PODF(value)          BIT_FIELD((value), 16)
#define CBCDR_SEMC_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CBCDR_SEMC_PODF_MASK, 16)

/* Set to use divided periph clock 2, clear to use pre-periph clock */
#define CBCDR_PERIPH_CLK_SEL            BIT(25)

#define CBCDR_PERIPH_CLK2_PODF_MASK     BIT_FIELD(MASK(3), 27)
#define CBCDR_PERIPH_CLK2_PODF(value)   BIT_FIELD((value), 27)
#define CBCDR_PERIPH_CLK2_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CBCDR_PERIPH_CLK2_PODF_MASK, 27)
/*------------------CCM Bus Clock Multiplexer Register------------------------*/
enum
{
  LPSPI_CLK_SEL_PLL3_PFD1 = 0,
  LPSPI_CLK_SEL_PLL3_PFD0 = 1,
  LPSPI_CLK_SEL_PLL2      = 2,
  LPSPI_CLK_SEL_PLL2_PFD2 = 3
};

enum
{
  FLEXSPI2_CLK_SEL_PLL2_PFD2  = 0,
  FLEXSPI2_CLK_SEL_PLL3_PFD0  = 1,
  FLEXSPI2_CLK_SEL_PLL3_PFD1  = 2,
  FLEXSPI2_CLK_SEL_PLL2       = 3
};

enum
{
  PERIPH_CLK2_SEL_PLL3_SW     = 0,
  PERIPH_CLK2_SEL_OSC         = 1,
  PERIPH_CLK2_SEL_PLL2_BYPASS = 2
};

enum
{
  TRACE_CLK_SEL_PLL2      = 0,
  TRACE_CLK_SEL_PLL2_PFD2 = 1,
  TRACE_CLK_SEL_PLL2_PFD0 = 2,
  TRACE_CLK_SEL_PLL2_PFD1 = 3
};

enum
{
  PRE_PERIPH_CLK_SEL_PLL2       = 0,
  PRE_PERIPH_CLK_SEL_PLL2_PFD2  = 1,
  PRE_PERIPH_CLK_SEL_PLL2_PFD0  = 2,
  PRE_PERIPH_CLK_SEL_PLL1_DIV   = 3
};

#define CBCMR_LPSPI_CLK_SEL_MASK        BIT_FIELD(MASK(2), 4)
#define CBCMR_LPSPI_CLK_SEL(value)      BIT_FIELD((value), 4)
#define CBCMR_LPSPI_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CBCMR_LPSPI_CLK_SEL_MASK, 4)

#define CBCMR_FLEXSPI2_CLK_SEL_MASK     BIT_FIELD(MASK(2), 8)
#define CBCMR_FLEXSPI2_CLK_SEL(value)   BIT_FIELD((value), 8)
#define CBCMR_FLEXSPI2_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CBCMR_FLEXSPI2_CLK_SEL_MASK, 8)

#define CBCMR_PERIPH_CLK2_SEL_MASK      BIT_FIELD(MASK(2), 12)
#define CBCMR_PERIPH_CLK2_SEL(value)    BIT_FIELD((value), 12)
#define CBCMR_PERIPH_CLK2_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CBCMR_PERIPH_CLK2_SEL_MASK, 12)

#define CBCMR_TRACE_CLK_SEL_MASK        BIT_FIELD(MASK(2), 14)
#define CBCMR_TRACE_CLK_SEL(value)      BIT_FIELD((value), 14)
#define CBCMR_TRACE_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CBCMR_TRACE_CLK_SEL_MASK, 14)

#define CBCMR_PRE_PERIPH_CLK_SEL_MASK   BIT_FIELD(MASK(2), 18)
#define CBCMR_PRE_PERIPH_CLK_SEL(value) BIT_FIELD((value), 18)
#define CBCMR_PRE_PERIPH_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CBCMR_PRE_PERIPH_CLK_SEL_MASK, 18)

#define CBCMR_LCDIF_PODF_MASK           BIT_FIELD(MASK(3), 23)
#define CBCMR_LCDIF_PODF(value)         BIT_FIELD((value), 23)
#define CBCMR_LCDIF_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CBCMR_LCDIF_PODF_MASK, 23)

#define CBCMR_LPSPI_PODF_MASK           BIT_FIELD(MASK(3), 26)
#define CBCMR_LPSPI_PODF(value)         BIT_FIELD((value), 26)
#define CBCMR_LPSPI_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CBCMR_LPSPI_PODF_MASK, 26)

#define CBCMR_FLEXSPI2_PODF_MASK        BIT_FIELD(MASK(3), 29)
#define CBCMR_FLEXSPI2_PODF(value)      BIT_FIELD((value), 29)
#define CBCMR_FLEXSPI2_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CBCMR_FLEXSPI2_PODF_MASK, 29)
/*------------------CCM Serial Clock Multiplexer Register 1-------------------*/
enum
{
  SAI_CLK_SEL_PLL3_PFD2 = 0,
  SAI_CLK_SEL_PLL5      = 1,
  SAI_CLK_SEL_PLL4      = 2
};

enum
{
  FLEXSPI_CLK_SEL_SEMC_ROOT_PRE = 0,
  FLEXSPI_CLK_SEL_PLL3_SW       = 1,
  FLEXSPI_CLK_SEL_PLL2_PFD2     = 2,
  FLEXSPI_CLK_SEL_PLL3_PFD0     = 3
};

#define CSCMR1_PERCLK_PODF_MASK         BIT_FIELD(MASK(6), 0)
#define CSCMR1_PERCLK_PODF(value)       BIT_FIELD((value), 0)
#define CSCMR1_PERCLK_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CSCMR1_PERCLK_PODF_MASK, 0)

/* Set to use OSC clock, clear to derive from IPG clock root */
#define CSCMR1_PERCLK_CLK_SEL           BIT(6)

#define CSCMR1_SAI1_CLK_SEL_MASK        BIT_FIELD(MASK(2), 10)
#define CSCMR1_SAI1_CLK_SEL(value)      BIT_FIELD((value), 10)
#define CSCMR1_SAI1_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CSCMR1_SAI1_CLK_SEL_MASK, 10)

#define CSCMR1_SAI2_CLK_SEL_MASK        BIT_FIELD(MASK(2), 12)
#define CSCMR1_SAI2_CLK_SEL(value)      BIT_FIELD((value), 12)
#define CSCMR1_SAI2_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CSCMR1_SAI2_CLK_SEL_MASK, 12)

#define CSCMR1_SAI3_CLK_SEL_MASK        BIT_FIELD(MASK(2), 14)
#define CSCMR1_SAI3_CLK_SEL(value)      BIT_FIELD((value), 14)
#define CSCMR1_SAI3_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CSCMR1_SAI3_CLK_SEL_MASK, 14)

/* Set to use PLL2 PFD0, clear to use PLL2 PFD2 */
#define CSCMR1_USDHC1_CLK_SEL           BIT(16)
/* Set to use PLL2 PFD0, clear to use PLL2 PFD2 */
#define CSCMR1_USDHC2_CLK_SEL           BIT(17)

#define CSCMR1_FLEXSPI_PODF_MASK        BIT_FIELD(MASK(3), 23)
#define CSCMR1_FLEXSPI_PODF(value)      BIT_FIELD((value), 23)
#define CSCMR1_FLEXSPI_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CSCMR1_FLEXSPI_PODF_MASK, 23)

#define CSCMR1_FLEXSPI_CLK_SEL_MASK     BIT_FIELD(MASK(2), 29)
#define CSCMR1_FLEXSPI_CLK_SEL(value)   BIT_FIELD((value), 29)
#define CSCMR1_FLEXSPI_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CSCMR1_FLEXSPI_CLK_SEL_MASK, 29)
/*------------------CCM Serial Clock Multiplexer Register 2-------------------*/
enum
{
  CAN_CLK_SEL_PLL3_SW_60M = 0,
  CAN_CLK_SEL_OSC         = 1,
  CAN_CLK_SEL_PLL3_SW_80M = 2,
  CAN_CLK_SEL_DISABLE     = 3
};

enum
{
  FLEXIO_CLK_SEL_PLL4_DIV  = 0,
  FLEXIO_CLK_SEL_PLL3_PFD2 = 1,
  FLEXIO_CLK_SEL_PLL5      = 2,
  FLEXIO_CLK_SEL_PLL3_SW   = 3
};

#define CSCMR2_CAN_CLK_PODF_MASK        BIT_FIELD(MASK(6), 2)
#define CSCMR2_CAN_CLK_PODF(value)      BIT_FIELD((value), 2)
#define CSCMR2_CAN_CLK_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CSCMR2_CAN_CLK_PODF_MASK, 2)

#define CSCMR2_CAN_CLK_SEL_MASK         BIT_FIELD(MASK(2), 8)
#define CSCMR2_CAN_CLK_SEL(value)       BIT_FIELD((value), 8)
#define CSCMR2_CAN_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CSCMR2_CAN_CLK_SEL_MASK, 8)

#define CSCMR2_FLEXIO2_CLK_SEL_MASK     BIT_FIELD(MASK(2), 19)
#define CSCMR2_FLEXIO2_CLK_SEL(value)   BIT_FIELD((value), 19)
#define CSCMR2_FLEXIO2_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CSCMR2_FLEXIO2_CLK_SEL_MASK, 19)
/*------------------CCM Serial Clock Divider Register 1-----------------------*/
#define CSCDR1_UART_CLK_PODF_MASK       BIT_FIELD(MASK(6), 0)
#define CSCDR1_UART_CLK_PODF(value)     BIT_FIELD((value), 0)
#define CSCDR1_UART_CLK_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CSCDR1_UART_CLK_PODF_MASK, 0)

/* Set to use OSC clock, clear to use PLL3 80M */
#define CSCDR1_UART_CLK_SEL             BIT(6)

#define CSCDR1_USDHC1_PODF_MASK         BIT_FIELD(MASK(3), 11)
#define CSCDR1_USDHC1_PODF(value)       BIT_FIELD((value), 11)
#define CSCDR1_USDHC1_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CSCDR1_USDHC1_PODF_MASK, 11)

#define CSCDR1_USDHC2_PODF_MASK         BIT_FIELD(MASK(3), 16)
#define CSCDR1_USDHC2_PODF(value)       BIT_FIELD((value), 16)
#define CSCDR1_USDHC2_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CSCDR1_USDHC2_PODF_MASK, 16)

#define CSCDR1_TRACE_PODF_MASK          BIT_FIELD(MASK(2), 25)
#define CSCDR1_TRACE_PODF(value)        BIT_FIELD((value), 25)
#define CSCDR1_TRACE_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CSCDR1_TRACE_PODF_MASK, 25)
/*------------------CCM Clock Divider Register 1------------------------------*/
#define CS1CDR_SAI1_CLK_PODF_MASK       BIT_FIELD(MASK(6), 0)
#define CS1CDR_SAI1_CLK_PODF(value)     BIT_FIELD((value), 0)
#define CS1CDR_SAI1_CLK_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CS1CDR_SAI1_CLK_PODF_MASK, 0)

#define CS1CDR_SAI1_CLK_PRED_MASK       BIT_FIELD(MASK(3), 6)
#define CS1CDR_SAI1_CLK_PRED(value)     BIT_FIELD((value), 6)
#define CS1CDR_SAI1_CLK_PRED_VALUE(reg) \
    FIELD_VALUE((reg), CS1CDR_SAI1_CLK_PRED_MASK, 6)

#define CS1CDR_FLEXIO2_CLK_PRED_MASK    BIT_FIELD(MASK(3), 9)
#define CS1CDR_FLEXIO2_CLK_PRED(value)  BIT_FIELD((value), 9)
#define CS1CDR_FLEXIO2_CLK_PRED_VALUE(reg) \
    FIELD_VALUE((reg), CS1CDR_FLEXIO2_CLK_PRED_MASK, 9)

#define CS1CDR_SAI3_CLK_PODF_MASK       BIT_FIELD(MASK(6), 16)
#define CS1CDR_SAI3_CLK_PODF(value)     BIT_FIELD((value), 16)
#define CS1CDR_SAI3_CLK_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CS1CDR_SAI3_CLK_PODF_MASK, 16)

#define CS1CDR_SAI3_CLK_PRED_MASK       BIT_FIELD(MASK(3), 22)
#define CS1CDR_SAI3_CLK_PRED(value)     BIT_FIELD((value), 22)
#define CS1CDR_SAI3_CLK_PRED_VALUE(reg) \
    FIELD_VALUE((reg), CS1CDR_SAI3_CLK_PRED_MASK, 22)

#define CS1CDR_FLEXIO2_CLK_PODF_MASK    BIT_FIELD(MASK(3), 25)
#define CS1CDR_FLEXIO2_CLK_PODF(value)  BIT_FIELD((value), 25)
#define CS1CDR_FLEXIO2_CLK_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CS1CDR_SAI1_CLK_PODF_MASK, 25)
/*------------------CCM Clock Divider Register 2------------------------------*/
#define CS2CDR_SAI2_CLK_PODF_MASK       BIT_FIELD(MASK(6), 0)
#define CS2CDR_SAI2_CLK_PODF(value)     BIT_FIELD((value), 0)
#define CS2CDR_SAI2_CLK_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CS2CDR_SAI2_CLK_PODF_MASK, 0)

#define CS2CDR_SAI2_CLK_PRED_MASK       BIT_FIELD(MASK(3), 6)
#define CS2CDR_SAI2_CLK_PRED(value)     BIT_FIELD((value), 6)
#define CS2CDR_SAI2_CLK_PRED_VALUE(reg) \
    FIELD_VALUE((reg), CS2CDR_SAI2_CLK_PRED_MASK, 6)
/*------------------CCM D1 Clock Divider Register-----------------------------*/
enum
{
  SPDIF0_CLK_SEL_PLL4       = 0,
  SPDIF0_CLK_SEL_PLL3_PFD2  = 1,
  SPDIF0_CLK_SEL_PLL5       = 2,
  SPDIF0_CLK_SEL_PLL3_SW    = 3
};

#define CDCDR_FLEXIO1_CLK_SEL_MASK      BIT_FIELD(MASK(2), 7)
#define CDCDR_FLEXIO1_CLK_SEL(value)    BIT_FIELD((value), 7)
#define CDCDR_FLEXIO1_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CDCDR_FLEXIO1_CLK_SEL_MASK, 7)

#define CDCDR_FLEXIO1_CLK_PODF_MASK     BIT_FIELD(MASK(3), 9)
#define CDCDR_FLEXIO1_CLK_PODF(value)   BIT_FIELD((value), 9)
#define CDCDR_FLEXIO1_CLK_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CDCDR_FLEXIO1_CLK_PODF_MASK, 9)

#define CDCDR_FLEXIO1_CLK_PRED_MASK     BIT_FIELD(MASK(3), 12)
#define CDCDR_FLEXIO1_CLK_PRED(value)   BIT_FIELD((value), 12)
#define CDCDR_FLEXIO1_CLK_PRED_VALUE(reg) \
    FIELD_VALUE((reg), CDCDR_FLEXIO1_CLK_PRED_MASK, 12)

#define CDCDR_SPDIF0_CLK_SEL_MASK       BIT_FIELD(MASK(2), 20)
#define CDCDR_SPDIF0_CLK_SEL(value)     BIT_FIELD((value), 20)
#define CDCDR_SPDIF0_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CDCDR_SPDIF0_CLK_SEL_MASK, 20)

#define CDCDR_SPDIF0_CLK_PODF_MASK      BIT_FIELD(MASK(3), 22)
#define CDCDR_SPDIF0_CLK_PODF(value)    BIT_FIELD((value), 22)
#define CDCDR_SPDIF0_CLK_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CDCDR_SPDIF0_CLK_PODF_MASK, 22)

#define CDCDR_SPDIF0_CLK_PRED_MASK      BIT_FIELD(MASK(3), 25)
#define CDCDR_SPDIF0_CLK_PRED(value  )  BIT_FIELD((value), 25)
#define CDCDR_SPDIF0_CLK_PRED_VALUE(reg) \
    FIELD_VALUE((reg), CDCDR_SPDIF0_CLK_PRED_MASK, 25)
/*------------------CCM Serial Clock Divider Register 2-----------------------*/
enum
{
  LCDIF_PRE_CLK_SEL_PLL2      = 0,
  LCDIF_PRE_CLK_SEL_PLL3_PFD3 = 1,
  LCDIF_PRE_CLK_SEL_PLL5      = 2,
  LCDIF_PRE_CLK_SEL_PLL2_PFD0 = 3,
  LCDIF_PRE_CLK_SEL_PLL2_PFD1 = 4,
  LCDIF_PRE_CLK_SEL_PLL3_PFD1 = 5
};

#define CSCDR2_LCDIF_PRED_MASK          BIT_FIELD(MASK(3), 12)
#define CSCDR2_LCDIF_PRED(value)        BIT_FIELD((value), 12)
#define CSCDR2_LCDIF_PRED_VALUE(reg) \
    FIELD_VALUE((reg), CSCDR2_LCDIF_PRED_MASK, 12)

#define CSCDR2_LCDIF_PRE_CLK_SEL_MASK   BIT_FIELD(MASK(3), 15)
#define CSCDR2_LCDIF_PRE_CLK_SEL(value) BIT_FIELD((value), 15)
#define CSCDR2_LCDIF_PRE_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CSCDR2_LCDIF_PRE_CLK_SEL_MASK, 15)

/* Set to use PLL3 60M, clear to use OSC clock */
#define CSCDR2_LPI2C_CLK_SEL            BIT(18)

#define CSCDR2_LPI2C_CLK_PODF_MASK      BIT_FIELD(MASK(6), 19)
#define CSCDR2_LPI2C_CLK_PODF(value)    BIT_FIELD((value), 19)
#define CSCDR2_LPI2C_CLK_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CSCDR2_LPI2C_CLK_PODF_MASK, 19)
/*------------------CCM Serial Clock Divider Register 3-----------------------*/
enum
{
  CSI_CLK_SEL_OSC       = 0,
  CSI_CLK_SEL_PLL2_PFD2 = 1,
  CSI_CLK_SEL_PLL3_120M = 2,
  CSI_CLK_SEL_PLL3_PFD1 = 3
};

#define CSCDR3_CSI_CLK_SEL_MASK         BIT_FIELD(MASK(2), 9)
#define CSCDR3_CSI_CLK_SEL(value)       BIT_FIELD((value), 9)
#define CSCDR3_CSI_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CSCDR3_CSI_CLK_SEL_MASK, 9)

#define CSCDR3_CSI_PODF_MASK            BIT_FIELD(MASK(3), 11)
#define CSCDR3_CSI_PODF(value)          BIT_FIELD((value), 11)
#define CSCDR3_CSI_PODF_VALUE(reg) \
    FIELD_VALUE((reg), CSCDR3_CSI_PODF_MASK, 11)
/*------------------CCM Divider Handshake In-Process Register-----------------*/
#define CDHIPR_SEMC_PODF_BUSY           BIT(0)
#define CDHIPR_AHB_PODF_BUSY            BIT(1)
#define CDHIPR_PERIPH2_CLK_SEL_BUSY     BIT(3)
#define CDHIPR_PERIPH_CLK_SEL_BUSY      BIT(5)
#define CDHIPR_ARM_PODF_BUSY            BIT(16)
/*------------------CCM Low Power Control Register----------------------------*/
enum
{
  LPM_RUN_MODE  = 0,
  LPM_WAIT_MODE = 1,
  LPM_STOP_MODE = 2
};

enum
{
  STBY_COUNT_1_PMIC_DELAY   = 0,
  STBY_COUNT_3_PMIC_DELAY   = 1,
  STBY_COUNT_7_PMIC_DELAY   = 2,
  STBY_COUNT_15_PMIC_DELAY  = 3
};

#define CLPCR_LPM_MASK                  BIT_FIELD(MASK(2), 0)
#define CLPCR_LPM(value)                BIT_FIELD((value), 0)
#define CLPCR_LPM_VALUE(reg)            FIELD_VALUE((reg), CLPCR_LPM_MASK, 0)

#define CLPCR_ARM_CLK_DIS_ON_LPM        BIT(5)
#define CLPCR_SBYOS                     BIT(6)
#define CLPCR_DIS_REF_OSC               BIT(7)
#define CLPCR_VSTBY                     BIT(8)

#define CLPCR_STBY_COUNT_MASK           BIT_FIELD(MASK(2), 9)
#define CLPCR_STBY_COUNT(value)         BIT_FIELD((value), 9)
#define CLPCR_STBY_COUNT_VALUE(reg) \
    FIELD_VALUE((reg), CLPCR_STBY_COUNT_MASK, 9)

#define CLPCR_COSC_PWRDOWN              BIT(11)
#define CLPCR_BYPASS_LPM_HS1            BIT(19)
#define CLPCR_BYPASS_LPC_HS0            BIT(21)
#define CLPCR_MASK_CORE0_WFI            BIT(22)
/*------------------CCM Interrupt Status Register-----------------------------*/
#define CISR_LRF_PLL                    BIT(0)
#define CISR_COSC_READY                 BIT(6)
#define CISR_SEMC_PODF_LOADED           BIT(17)
#define CISR_PERIPH2_CLK_SEL_LOADED     BIT(19)
#define CISR_AHB_PODF_LOADED            BIT(20)
#define CISR_PERIPH_CLK_SEL_LOADED      BIT(22)
#define CISR_ARM_PODF_LOADED            BIT(26)
/*------------------CCM Interrupt Mask Register-------------------------------*/
#define CIMR_MASK_LRF_PLL               BIT(0)
#define CIMR_MASK_COSC_READY            BIT(6)
#define CIMR_MASK_SEMC_PODF_LOADED      BIT(17)
#define CIMR_MASK_PERIPH2_CLK_SEL_LOADED \
    BIT(19)
#define CIMR_MASK_AHB_PODF_LOADED       BIT(20)
#define CIMR_MASK_PERIPH_CLK_SEL_LOADED BIT(22)
#define CIMR_MASK_ARM_PODF_LOADED       BIT(26)
/*------------------CCM Clock Output Source Register--------------------------*/
enum
{
  CLKO1_SEL_USB1_PLL_DIV2   = 0,
  CLKO1_SEL_SYS_PLL_DIV2    = 1,
  CLKO1_SEL_VIDEO_PLL_DIV2  = 3,
  CLKO1_SEL_SEMC            = 5,
  CLKO1_SEL_LCDIF_PIX       = 10,
  CLKO1_SEL_AHB             = 11,
  CLKO1_SEL_IPG             = 12,
  CLKO1_SEL_PERCLK          = 13,
  CLKO1_SEL_CKIL_SYNC       = 14,
  CLKO1_SEL_PLL4            = 15
};

enum
{
  CLKO2_SEL_USDHC1  = 3,
  CLKO2_SEL_LPI2C   = 6,
  CLKO2_SEL_CSI     = 11,
  CLKO2_SEL_OSC     = 14,
  CLKO2_SEL_USDHC2  = 17,
  CLKO2_SEL_SAI1    = 18,
  CLKO2_SEL_SAI2    = 19,
  CLKO2_SEL_SAI3    = 20,
  CLKO2_SEL_CAN     = 23,
  CLKO2_SEL_FLEXSPI = 27,
  CLKO2_SEL_UART    = 28,
  CLKO2_SEL_SPDIF0  = 29
};

#define CCOSR_CLKO1_SEL_MASK            BIT_FIELD(MASK(4), 0)
#define CCOSR_CLKO1_SEL(value)          BIT_FIELD((value), 0)
#define CCOSR_CLKO1_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CCOSR_CLKO1_SEL_MASK, 0)

#define CCOSR_CLKO1_DIV_MASK            BIT_FIELD(MASK(3), 4)
#define CCOSR_CLKO1_DIV(value)          BIT_FIELD((value), 4)
#define CCOSR_CLKO1_DIV_VALUE(reg) \
    FIELD_VALUE((reg), CCOSR_CLKO1_DIV_MASK, 4)

#define CCOSR_CLKO1_EN                  BIT(7)
/* Use CCM_CLKO2 pin to output CCM_CLKO1 clock */
#define CCOSR_CLK_OUT_SEL               BIT(8)

#define CCOSR_CLKO2_SEL_MASK            BIT_FIELD(MASK(5), 16)
#define CCOSR_CLKO2_SEL(value)          BIT_FIELD((value), 16)
#define CCOSR_CLKO2_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CCOSR_CLKO2_SEL_MASK, 16)

#define CCOSR_CLKO2_DIV_MASK            BIT_FIELD(MASK(3), 21)
#define CCOSR_CLKO2_DIV(value)          BIT_FIELD((value), 21)
#define CCOSR_CLKO2_DIV_VALUE(reg) \
    FIELD_VALUE((reg), CCOSR_CLKO2_DIV_MASK, 21)

#define CCOSR_CLKO2_EN                  BIT(24)
/*------------------CCM General Purpose Register------------------------------*/
enum
{
  SYS_MEM_DS_CTRL_DISABLE               = 0,
  SYS_MEM_DS_CTRL_STOP_AND_PLL_DISABLED = 1,
  SYS_MEM_DS_CTRL_STOP                  = 2
};

#define CGPR_PMIC_DELAY_SCALER          BIT(0)
#define CGPR_EFUSE_PROG_SUPPLY_GATE     BIT(4)

#define CGPR_SYS_MEM_DS_CTRL_MASK       BIT_FIELD(MASK(2), 14)
#define CGPR_SYS_MEM_DS_CTRL(value)     BIT_FIELD((value), 14)
#define CGPR_SYS_MEM_DS_CTRL_VALUE(reg) \
    FIELD_VALUE((reg), CGPR_SYS_MEM_DS_CTRL_MASK, 14)

#define CGPR_FPL                        BIT(16)
#define CGPR_INT_MEM_CLK_LPM            BIT(17)
/*------------------CCM Module Enable Override Register-----------------------*/
#define CMEOR_MOD_EN_OV_GPT             BIT(5)
#define CMEOR_MOD_EN_OV_PIT             BIT(6)
#define CMEOR_MOD_EN_OV_USDHC           BIT(7)
#define CMEOR_MOD_EN_OV_TRNG            BIT(9)
#define CMEOR_MOD_EN_OV_CANFD_CPI       BIT(10)
#define CMEOR_MOD_EN_OV_CAN2_CPI        BIT(28)
#define CMEOR_MOD_EN_OV_CAN1_CPI        BIT(30)
/*------------------Analog PLL control registers------------------------------*/
enum
{
  BYPASS_CLK_SRC_REF_CLK_24M  = 0,
  BYPASS_CLK_SRC_CLK1         = 1
};

#define PLL_POWERDOWN                   BIT(12)
#define PLL_ENABLE                      BIT(13)

#define PLL_BYPASS_CLK_SRC_MASK         BIT_FIELD(MASK(2), 14)
#define PLL_BYPASS_CLK_SRC(value)       BIT_FIELD((value), 14)
#define PLL_BYPASS_CLK_SRC_VALUE(reg) \
    FIELD_VALUE((reg), PLL_BYPASS_CLK_SRC_MASK, 14)

#define PLL_BYPASS                      BIT(16)
#define PLL_LOCK                        BIT(31)
/*------------------Analog ARM PLL control register---------------------------*/
#define PLL_ARM_DIV_SELECT_MASK         BIT_FIELD(MASK(7), 0)
#define PLL_ARM_DIV_SELECT(value)       BIT_FIELD((value), 0)
#define PLL_ARM_DIV_SELECT_VALUE(reg) \
    FIELD_VALUE((reg), PLL_ARM_DIV_SELECT_MASK, 0)
/*------------------Analog USB PLL control registers--------------------------*/
#define PLL_USB_DIV_SELECT              BIT(1)
#define PLL_USB_EN_USB_CLKS             BIT(6)
#define PLL_USB_POWER                   BIT(12)
/*------------------Analog SYS PLL control registers--------------------------*/
#define PLL_SYS_DIV_SELECT              BIT(0)
/*-----------------Analog SYS PLL Spread Spectrum Register--------------------*/
#define PLL_SYS_SS_STEP_MASK            BIT_FIELD(MASK(15), 0)
#define PLL_SYS_SS_STEP(value)          BIT_FIELD((value), 0)
#define PLL_SYS_SS_STEP_VALUE(reg) \
    FIELD_VALUE((reg), PLL_SYS_SS_STEP_MASK, 0)

#define PLL_SYS_SS_ENABLE               BIT(15)

#define PLL_SYS_SS_STOP_MASK            BIT_FIELD(MASK(16), 16)
#define PLL_SYS_SS_STOP(value)          BIT_FIELD((value), 16)
#define PLL_SYS_SS_STOP_VALUE(reg) \
    FIELD_VALUE((reg), PLL_SYS_SS_STOP_MASK, 16)
/*------------------Analog AUDIO PLL control register-------------------------*/
#define PLL_AUDIO_DIV_SELECT_MASK       BIT_FIELD(MASK(7), 0)
#define PLL_AUDIO_DIV_SELECT(value)     BIT_FIELD((value), 0)
#define PLL_AUDIO_DIV_SELECT_VALUE(reg) \
    FIELD_VALUE((reg), PLL_AUDIO_DIV_SELECT_MASK, 0)

#define PLL_AUDIO_POST_DIV_SELECT_MASK  BIT_FIELD(MASK(2), 19)
#define PLL_AUDIO_POST_DIV_SELECT(value) \
    BIT_FIELD((value), 19)
#define PLL_AUDIO_POST_DIV_SELECT_VALUE(reg) \
    FIELD_VALUE((reg), PLL_AUDIO_POST_DIV_SELECT_MASK, 19)
/*------------------Analog VIDEO PLL control register-------------------------*/
#define PLL_VIDEO_DIV_SELECT_MASK       BIT_FIELD(MASK(7), 0)
#define PLL_VIDEO_DIV_SELECT(value)     BIT_FIELD((value), 0)
#define PLL_VIDEO_DIV_SELECT_VALUE(reg) \
    FIELD_VALUE((reg), PLL_VIDEO_DIV_SELECT_MASK, 0)

#define PLL_VIDEO_POST_DIV_SELECT_MASK  BIT_FIELD(MASK(2), 19)
#define PLL_VIDEO_POST_DIV_SELECT(value) \
    BIT_FIELD((value), 19)
#define PLL_VIDEO_POST_DIV_SELECT_VALUE(reg) \
    FIELD_VALUE((reg), PLL_VIDEO_POST_DIV_SELECT_MASK, 19)
/*------------------Analog ENET PLL control register--------------------------*/
enum
{
  ENET_DIV_SELECT_25M   = 0,
  ENET_DIV_SELECT_50M   = 1,
  ENET_DIV_SELECT_100M  = 2,
  ENET_DIV_SELECT_125M  = 3
};

#define PLL_ENET_ENET_DIV_SELECT_MASK   BIT_FIELD(MASK(2), 0)
#define PLL_ENET_ENET_DIV_SELECT(value) \
    BIT_FIELD((value), 0)
#define PLL_ENET_ENET_DIV_SELECT_VALUE(reg) \
    FIELD_VALUE((reg), PLL_ENET_ENET_DIV_SELECT_MASK, 0)

#define PLL_ENET_ENET2_DIV_SELECT_MASK  BIT_FIELD(MASK(2), 2)
#define PLL_ENET_ENET2_DIV_SELECT(value) \
    BIT_FIELD((value), 2)
#define PLL_ENET_ENET2_DIV_SELECT_VALUE(reg) \
    FIELD_VALUE((reg), PLL_ENET_ENET2_DIV_SELECT_MASK, 2)

#define PLL_ENET_ENET2_REF_EN           BIT(20)
#define PLL_ENET_ENET_25M_REF_EN        BIT(21)
/*------------------Analog PLL2 and PLL3 PFD control registers----------------*/
#define PLL_PFD_PFDn_FRAC_MASK(channel) \
    BIT_FIELD(MASK(6), (channel) * 8)
#define PLL_PFD_PFDn_FRAC(value, channel) \
    BIT_FIELD((value), (channel) * 8)
#define PLL_PFD_PFDn_FRAC_VALUE(reg, channel) \
    FIELD_VALUE((reg), PLL_PFD_PFDn_FRAC_MASK(channel), (channel) * 8)

#define PLL_PFD_PFDn_STABLE(channel)    BIT((channel) * 8 + 6)
#define PLL_PFD_PFDn_CLKGATE(channel)   BIT((channel) * 8 + 7)
/*------------------XTALOSC 24MHz Miscellaneous register 0--------------------*/
enum
{
  OSC_I_NOMINAL     = 0,
  OSC_I_MINUS_12P5  = 1,
  OSC_I_MINUS_25P   = 2,
  OSC_I_MINUS_37P5  = 3
};

enum
{
  REFTOP_VBGADJ_NOMINAL     = 0,
  REFTOP_VBGADJ_PLUS_0P78   = 1,
  REFTOP_VBGADJ_PLUS_1P56   = 2,
  REFTOP_VBGADJ_PLUS_2P34   = 3,
  REFTOP_VBGADJ_MINUS_0P78  = 4,
  REFTOP_VBGADJ_MINUS_1P56  = 5,
  REFTOP_VBGADJ_MINUS_2P34  = 6,
  REFTOP_VBGADJ_MINUS_3P12  = 7
};

enum
{
  /* All except RTC powered down */
  STOP_MODE_CONFIG_0,
  /* RTC, analog bandgap, 1V1 and 2V5 regulators are on */
  STOP_MODE_CONFIG_1,
  /* RTC, 1V1 and 2V5 regulators are on, low-power bandgap is used */
  STOP_MODE_CONFIG_2,
  /* RTC and low-power bandgap are on */
  STOP_MODE_CONFIG_3
};

#define MISC0_REFTOP_PWD                BIT(0)
#define MISC0_REFTOP_SELFBIASOFF        BIT(3)

#define MISC0_REFTOP_VBGADJ_MASK        BIT_FIELD(MASK(3), 4)
#define MISC0_REFTOP_VBGADJ(value)      BIT_FIELD((value), 4)
#define MISC0_REFTOP_VBGADJ_VALUE(reg) \
    FIELD_VALUE((reg), MISC0_REFTOP_VBGADJ_MASK, 4)

#define MISC0_REFTOP_VBGUP              BIT(7)

#define MISC0_STOP_MODE_CONFIG_MASK     BIT_FIELD(MASK(2), 10)
#define MISC0_STOP_MODE_CONFIG(value)   BIT_FIELD((value), 10)
#define MISC0_STOP_MODE_CONFIG_VALUE(reg) \
    FIELD_VALUE((reg), MISC0_STOP_MODE_CONFIG_MASK, 10)

#define MISC0_DISCON_HIGH_SNVS          BIT(12)

#define MISC0_OSC_I_MASK                BIT_FIELD(MASK(2), 13)
#define MISC0_OSC_I(value)              BIT_FIELD((value), 13)
#define MISC0_OSC_I_VALUE(reg) \
    FIELD_VALUE((reg), MISC0_OSC_I_MASK, 13)

#define MISC0_OSC_XTALOK                BIT(15)
#define MISC0_OSC_XTALOK_EN             BIT(16)
#define MISC0_CLKGATE_CTRL              BIT(25)

#define MISC0_CLKGATE_DELAY_MASK        BIT_FIELD(MASK(3), 26)
#define MISC0_CLKGATE_DELAY(value)      BIT_FIELD((value), 26)
#define MISC0_CLKGATE_DELAY_VALUE(reg) \
    FIELD_VALUE((reg), MISC0_CLKGATE_DELAY_MASK, 26)

/* Set to use RTC crystal oscillator, clear to use internal ring oscillator */
#define MISC0_RTC_XTAL_SOURCE           BIT(29)
/* Power down 24MHz crystal oscillator */
#define MISC0_XTAL_24M_PWD              BIT(30)
#define MISC0_VID_PLL_PREDIV            BIT(31)
/*------------------Analog Miscellaneous register 1---------------------------*/
enum
{
  LVDS1_CLK_SEL_ARM_PLL   = 0,
  LVDS1_CLK_SEL_SYS_PLL   = 1,
  LVDS1_CLK_SEL_PLL2_PFD0 = 2,
  LVDS1_CLK_SEL_PLL2_PFD1 = 3,
  LVDS1_CLK_SEL_PLL2_PFD2 = 4,
  LVDS1_CLK_SEL_PLL2_PFD3 = 5,
  LVDS1_CLK_SEL_AUDIO_PLL = 6,
  LVDS1_CLK_SEL_VIDEO_PLL = 7,
  LVDS1_CLK_SEL_ENET_PLL  = 9,
  LVDS1_CLK_SEL_USB1_PLL  = 12,
  LVDS1_CLK_SEL_USB2_PLL  = 13,
  LVDS1_CLK_SEL_PLL3_PFD0 = 14,
  LVDS1_CLK_SEL_PLL3_PFD1 = 15,
  LVDS1_CLK_SEL_PLL3_PFD2 = 16,
  LVDS1_CLK_SEL_PLL3_PFD3 = 17,
  LVDS1_CLK_SEL_OSC       = 18
};

#define MISC1_LVDS1_CLK_SEL_MASK        BIT_FIELD(MASK(5), 0)
#define MISC1_LVDS1_CLK_SEL(value)      BIT_FIELD((value), 0)
#define MISC1_LVDS1_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), MISC1_LVDS1_CLK_SEL_MASK, 0)

#define MISC1_LVDSCLK1_OBEN             BIT(10)
#define MISC1_LVDSCLK1_IBEN             BIT(12)

#define MISC1_PFD_480_AUTOGATE_EN       BIT(16)
#define MISC1_PFD_528_AUTOGATE_EN       BIT(17)
#define MISC1_IRQ_TEMPPANIC             BIT(27)
#define MISC1_IRQ_TEMPLOW               BIT(28)
#define MISC1_IRQ_TEMPHIGH              BIT(29)
#define MISC1_IRQ_ANA_BO                BIT(30)
#define MISC1_IRQ_DIG_BO                BIT(31)
/*------------------Analog Miscellaneous register 2---------------------------*/
enum
{
  REG_STEP_TIME_64_CLOCKS   = 0,
  REG_STEP_TIME_128_CLOCKS  = 1,
  REG_STEP_TIME_256_CLOCKS  = 2,
  REG_STEP_TIME_512_CLOCKS  = 3
};

// TODO More Enums

#define MISC2_REG0_BO_OFFSET_MASK       BIT_FIELD(MASK(3), 0)
#define MISC2_REG0_BO_OFFSET(value)     BIT_FIELD((value), 0)
#define MISC2_REG0_BO_OFFSET_VALUE(reg) \
    FIELD_VALUE((reg), MISC2_REG0_BO_OFFSET_MASK, 0)

#define MISC2_REG0_BO_STATUS            BIT(3)
#define MISC2_REG0_ENABLE_BO            BIT(5)
#define MISC2_REG0_OK                   BIT(6)
#define MISC2_PLL3_DISABLE              BIT(7)

#define MISC2_REG1_BO_OFFSET_MASK       BIT_FIELD(MASK(3), 8)
#define MISC2_REG1_BO_OFFSET(value)     BIT_FIELD((value), 8)
#define MISC2_REG1_BO_OFFSET_VALUE(reg) \
    FIELD_VALUE((reg), MISC2_REG1_BO_OFFSET_MASK, 8)

#define MISC2_REG1_BO_STATUS            BIT(11)
#define MISC2_REG1_ENABLE_BO            BIT(13)
#define MISC2_REG1_OK                   BIT(14)
#define MISC2_AUDIO_DIV_LSB             BIT(15)

#define MISC2_REG2_BO_OFFSET_MASK       BIT_FIELD(MASK(3), 16)
#define MISC2_REG2_BO_OFFSET(value)     BIT_FIELD((value), 16)
#define MISC2_REG2_BO_OFFSET_VALUE(reg) \
    FIELD_VALUE((reg), MISC2_REG2_BO_OFFSET_MASK, 16)

#define MISC2_REG2_BO_STATUS            BIT(19)
#define MISC2_REG2_ENABLE_BO            BIT(21)
#define MISC2_REG2_OK                   BIT(22)

#define MISC2_AUDIO_DIV_MSB             BIT(23)

#define MISC2_REG0_STEP_TIME_MASK       BIT_FIELD(MASK(2), 24)
#define MISC2_REG0_STEP_TIME(value)     BIT_FIELD((value), 24)
#define MISC2_REG0_STEP_TIME_VALUE(reg) \
    FIELD_VALUE((reg), MISC2_REG0_STEP_TIME_MASK, 24)

#define MISC2_REG1_STEP_TIME_MASK       BIT_FIELD(MASK(2), 26)
#define MISC2_REG1_STEP_TIME(value)     BIT_FIELD((value), 26)
#define MISC2_REG1_STEP_TIME_VALUE(reg) \
    FIELD_VALUE((reg), MISC2_REG1_STEP_TIME_MASK, 26)

#define MISC2_REG2_STEP_TIME_MASK       BIT_FIELD(MASK(2), 28)
#define MISC2_REG2_STEP_TIME(value)     BIT_FIELD((value), 28)
#define MISC2_REG2_STEP_TIME_VALUE(reg) \
    FIELD_VALUE((reg), MISC2_REG2_STEP_TIME_MASK, 28)

#define MISC2_VIDEO_DIV_MASK            BIT_FIELD(MASK(2), 30)
#define MISC2_VIDEO_DIV(value)          BIT_FIELD((value), 30)
#define MISC2_VIDEO_DIV_VALUE(reg) \
    FIELD_VALUE((reg), MISC2_VIDEO_DIV_MASK, 30)
/*------------------XTALOSC 24MHz Low Power Control---------------------------*/
enum
{
  XTALOSC_PWRUP_DELAY_0MS25 = 0,
  XTALOSC_PWRUP_DELAY_0MS5  = 1,
  XTALOSC_PWRUP_DELAY_1MS   = 2,
  XTALOSC_PWRUP_DELAY_2MS   = 3
};

#define LOWPWR_CTRL_RC_OSC_EN           BIT(0)
#define LOWPWR_CTRL_OSC_SEL             BIT(4)
#define LOWPWR_CTRL_LPBG_SEL            BIT(5)
#define LOWPWR_CTRL_LPBG_TEST           BIT(6)
#define LOWPWR_CTRL_REFTOP_IBIAS_OFF    BIT(7)
#define LOWPWR_CTRL_L1_PWRGATE          BIT(8)
#define LOWPWR_CTRL_L2_PWRGATE          BIT(9)
#define LOWPWR_CTRL_CPU_PWRGATE         BIT(10)
#define LOWPWR_CTRL_DISPLAY_PWRGATE     BIT(11)
#define LOWPWR_CTRL_RCOSC_CG_OVERRIDE   BIT(13)

#define LOWPWR_CTRL_XTALOSC_PWRUP_DELAY_MASK \
    BIT_FIELD(MASK(2), 14)
#define LOWPWR_CTRL_XTALOSC_PWRUP_DELAY(value) \
    BIT_FIELD((value), 14)
#define LOWPWR_CTRL_XTALOSC_PWRUP_DELAY_VALUE(reg) \
    FIELD_VALUE((reg), LOWPWR_CTRL_XTALOSC_PWRUP_DELAY_MASK, 14)

#define LOWPWR_CTRL_XTALOSC_PWRUP_STAT  BIT(16)
#define LOWPWR_CTRL_MIX_PWRGATE         BIT(17)
#define LOWPWR_CTRL_GPU_PWRGATE         BIT(18)
/*------------------XTALOSC 24MHz Oscillator Configuration Register 0---------*/
#define OSC_CONFIG0_START               BIT(0)
#define OSC_CONFIG0_ENABLE              BIT(1)
#define OSC_CONFIG0_BYPASS              BIT(2)
#define OSC_CONFIG0_INVERT              BIT(3)

#define OSC_CONFIG0_RC_OSC_PROG_MASK    BIT_FIELD(MASK(8), 4)
#define OSC_CONFIG0_RC_OSC_PROG(value)  BIT_FIELD((value), 4)
#define OSC_CONFIG0_RC_OSC_PROG_VALUE(reg) \
    FIELD_VALUE((reg), OSC_CONFIG0_RC_OSC_PROG_MASK, 4)

#define OSC_CONFIG0_HYST_PLUS_MASK      BIT_FIELD(MASK(4), 12)
#define OSC_CONFIG0_HYST_PLUS(value)    BIT_FIELD((value), 12)
#define OSC_CONFIG0_HYST_PLUS_VALUE(reg) \
    FIELD_VALUE((reg), OSC_CONFIG0_HYST_PLUS_MASK, 12)

#define OSC_CONFIG0_HYST_MINUS_MASK     BIT_FIELD(MASK(4), 16)
#define OSC_CONFIG0_HYST_MINUS(value)   BIT_FIELD((value), 16)
#define OSC_CONFIG0_HYST_MINUS_VALUE(reg) \
    FIELD_VALUE((reg), OSC_CONFIG0_HYST_MINUS_MASK, 16)

#define OSC_CONFIG0_RC_OSC_PROG_CUR_MASK \
    BIT_FIELD(MASK(8), 24)
#define OSC_CONFIG0_RC_OSC_PROG_CUR(value) \
    BIT_FIELD((value), 24)
#define OSC_CONFIG0_RC_OSC_PROG_CUR_VALUE(reg) \
    FIELD_VALUE((reg), OSC_CONFIG0_RC_OSC_PROG_CUR_MASK, 24)
/*------------------XTALOSC 24MHz Oscillator Configuration Register 1---------*/
#define OSC_CONFIG1_COUNT_RC_TRG_MASK   BIT_FIELD(MASK(12), 0)
#define OSC_CONFIG1_COUNT_RC_TRG(value) BIT_FIELD((value), 0)
#define OSC_CONFIG1_COUNT_RC_TRG_VALUE(reg) \
    FIELD_VALUE((reg), OSC_CONFIG1_COUNT_RC_TRG_MASK, 0)

#define OSC_CONFIG1_COUNT_RC_CUR_MASK   BIT_FIELD(MASK(12), 20)
#define OSC_CONFIG1_COUNT_RC_CUR(value) BIT_FIELD((value), 20)
#define OSC_CONFIG1_COUNT_RC_CUR_VALUE(reg) \
    FIELD_VALUE((reg), OSC_CONFIG1_COUNT_RC_CUR_MASK, 20)
/*------------------XTALOSC 24MHz Oscillator Configuration Register 2---------*/
#define OSC_CONFIG2_COUNT_1M_TRG        BIT(0)
#define OSC_CONFIG2_ENABLE_1M           BIT(16)
#define OSC_CONFIG2_MUX_1M              BIT(17)
#define OSC_CONFIG2_CLK_1M_ERR_FL       BIT(31)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_IMXRT_IMXRT106X_CLOCKING_DEFS_H_ */
