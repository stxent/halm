/*
 * halm/platform/bouffalo/bl602/clocking_defs.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_BL602_CLOCKING_DEFS_H_
#define HALM_PLATFORM_BOUFFALO_BL602_CLOCKING_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Clock Configuration 0 register----------------------------*/
enum
{
  ROOT_CLK_SEL_RC32M = 0,
  ROOT_CLK_SEL_XTAL  = 1,
  ROOT_CLK_SEL_PLL   = 2
};

enum
{
  PLL_SEL_48MHZ  = 0,
  PLL_SEL_120MHZ = 1,
  PLL_SEL_160MHZ = 2,
  PLL_SEL_192MHZ = 3
};

#define CLK_CFG0_PLL_EN                 BIT(0)
#define CLK_CFG0_FCLK_EN                BIT(1)
#define CLK_CFG0_HCLK_EN                BIT(2)
#define CLK_CFG0_BCLK_EN                BIT(3)

#define CLK_CFG0_PLL_SEL(value)         BIT_FIELD((value), 4)
#define CLK_CFG0_PLL_SEL_MASK           BIT_FIELD(MASK(2), 4)
#define CLK_CFG0_PLL_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CLK_CFG0_PLL_SEL_MASK, 4)

#define CLK_CFG0_ROOT_CLK_SEL(value)    BIT_FIELD((value), 6)
#define CLK_CFG0_ROOT_CLK_SEL_MASK      BIT_FIELD(MASK(2), 6)
#define CLK_CFG0_ROOT_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CLK_CFG0_ROOT_CLK_SEL_MASK, 6)

#define CLK_CFG0_HCLK_DIV(value)        BIT_FIELD((value), 8)
#define CLK_CFG0_HCLK_DIV_MASK          BIT_FIELD(MASK(8), 8)
#define CLK_CFG0_HCLK_DIV_VALUE(reg) \
    FIELD_VALUE((reg), CLK_CFG0_HCLK_DIV_MASK, 8)
#define CLK_CFG0_HCLK_DIV_MAX           255

#define CLK_CFG0_BCLK_DIV(value)        BIT_FIELD((value), 16)
#define CLK_CFG0_BCLK_DIV_MASK          BIT_FIELD(MASK(8), 16)
#define CLK_CFG0_BCLK_DIV_VALUE(reg) \
    FIELD_VALUE((reg), CLK_CFG0_BCLK_DIV_MASK, 16)

#define CLK_CFG0_FCLK_SW_STATE(value)   BIT_FIELD((value), 24)
#define CLK_CFG0_FCLK_SW_STATE_MASK     BIT_FIELD(MASK(3), 24)
#define CLK_CFG0_FCLK_SW_STATE_VALUE(reg) \
    FIELD_VALUE((reg), CLK_CFG0_FCLK_SW_STATE_MASK, 24)

#define CLK_CFG0_CHIP_RDY               BIT(27)
/*------------------Clock Configuration 2 register----------------------------*/
enum
{
  SF_CLK_SEL_120MHZ = 0,
  SF_CLK_SEL_80MHZ  = 1,
  SF_CLK_SEL_HCLK   = 2,
  SF_CLK_SEL_96MHZ  = 3
};

#define CLK_CFG2_UART_CLK_DIV(value)    BIT_FIELD((value), 0)
#define CLK_CFG2_UART_CLK_DIV_MASK      BIT_FIELD(MASK(3), 0)
#define CLK_CFG2_UART_CLK_DIV_VALUE(reg) \
    FIELD_VALUE((reg), CLK_CFG2_UART_CLK_DIV_MASK, 0)
#define CLK_CFG2_UART_CLK_DIV_MAX       7

#define CLK_CFG2_UART_CLK_EN            BIT(4)
#define CLK_CFG2_HBN_UART_CLK_SEL       BIT(7)

#define CLK_CFG2_SF_CLK_DIV(value)      BIT_FIELD((value), 8)
#define CLK_CFG2_SF_CLK_DIV_MASK        BIT_FIELD(MASK(3), 8)
#define CLK_CFG2_SF_CLK_DIV_VALUE(reg) \
    FIELD_VALUE((reg), CLK_CFG2_SF_CLK_DIV_MASK, 8)

#define CLK_CFG2_SF_CLK_EN              BIT(11)

#define CLK_CFG2_SF_CLK_SEL(value)      BIT_FIELD((value), 12)
#define CLK_CFG2_SF_CLK_SEL_MASK        BIT_FIELD(MASK(2), 12)
#define CLK_CFG2_SF_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CLK_CFG2_SF_CLK_SEL_MASK, 12)

#define CLK_CFG2_SF_CLK_SEL2(value)     BIT_FIELD((value), 14)
#define CLK_CFG2_SF_CLK_SEL2_MASK       BIT_FIELD(MASK(2), 14)
#define CLK_CFG2_SF_CLK_SEL2_VALUE(reg) \
    FIELD_VALUE((reg), CLK_CFG2_SF_CLK_SEL2_MASK, 14)

#define CLK_CFG2_IR_CLK_DIV(value)      BIT_FIELD((value), 16)
#define CLK_CFG2_IR_CLK_DIV_MASK        BIT_FIELD(MASK(6), 16)
#define CLK_CFG2_IR_CLK_DIV_VALUE(reg) \
    FIELD_VALUE((reg), CLK_CFG2_IR_CLK_DIV_MASK, 16)

#define CLK_CFG2_IR_CLK_EN              BIT(23)

#define CLK_CFG2_DMA_CLK_EN(value)      BIT_FIELD((value), 24)
#define CLK_CFG2_DMA_CLK_EN_MASK        BIT_FIELD(MASK(8), 24)
#define CLK_CFG2_DMA_CLK_EN_VALUE(reg) \
    FIELD_VALUE((reg), CLK_CFG2_DMA_CLK_EN_MASK, 24)
/*------------------HBN GLB register------------------------------------------*/
#define HBN_GLB_ROOT_CLK_SEL(value)     BIT_FIELD((value), 0)
#define HBN_GLB_ROOT_CLK_SEL_MASK       BIT_FIELD(MASK(2), 0)
#define HBN_GLB_ROOT_CLK_SEL_VALUE(reg) \
    FIELD_VALUE((reg), HBN_GLB_ROOT_CLK_SEL_MASK, 0)

/* Clear to select root clock, set to select 160 MHz PLL */
#define HBN_GLB_UART_CLK_SEL            BIT(2)

#define HBN_GLB_F32K_SEL(value)         BIT_FIELD((value), 3)
#define HBN_GLB_F32K_SEL_MASK           BIT_FIELD(MASK(2), 3)
#define HBN_GLB_F32K_SEL_VALUE(reg) \
    FIELD_VALUE((reg), HBN_GLB_F32K_SEL_MASK, 3)

#define HBN_GLB_LDO11SOC_VOUT_SEL_AON(value) \
    BIT_FIELD((value), 16)
#define HBN_GLB_LDO11SOC_VOUT_SEL_AON_MASK \
    BIT_FIELD(MASK(4), 16)
#define HBN_GLB_LDO11SOC_VOUT_SEL_AON_VALUE(reg) \
    FIELD_VALUE((reg), HBN_GLB_LDO11SOC_VOUT_SEL_AON_MASK, 16)

#define HBN_GLB_LDO11_RT_VOUT_SEL(value) \
    BIT_FIELD((value), 24)
#define HBN_GLB_LDO11_RT_VOUT_SEL_MASK \
    BIT_FIELD(MASK(4), 24)
#define HBN_GLB_LDO11_RT_VOUT_SEL_VALUE(reg) \
    FIELD_VALUE((reg), HBN_GLB_LDO11_RT_VOUT_SEL_MASK, 24)

#define HBN_GLB_LDO11_AON_VOUT_SEL(value) \
    BIT_FIELD((value), 28)
#define HBN_GLB_LDO11_AON_VOUT_SEL_MASK \
    BIT_FIELD(MASK(4), 28)
#define HBN_GLB_LDO11_AON_VOUT_SEL_VALUE(reg) \
    FIELD_VALUE((reg), HBN_GLB_LDO11_AON_VOUT_SEL_MASK, 28)
/*------------------AON RF TOP register---------------------------------------*/
#define RF_TOP_AON_PU_MBG_AON           BIT(0)
#define RF_TOP_AON_PU_LDO15RF_AON       BIT(1)
#define RF_TOP_AON_PU_SFREG_AON         BIT(2)
#define RF_TOP_AON_PU_XTAL_BUF_AON      BIT(4)
#define RF_TOP_AON_PU_XTAL_AON          BIT(5)
#define RF_TOP_AON_LDO15RF_SSTART_SEL_AON \
    BIT(8)

#define RF_TOP_AON_LDO15RF_SSTART_DELAY_AON(value) \
    BIT_FIELD((value), 9)
#define RF_TOP_AON_LDO15RF_SSTART_DELAY_AON_MASK \
    BIT_FIELD(MASK(2), 9)
#define RF_TOP_AON_LDO15RF_SSTART_DELAY_AON_VALUE(reg) \
    FIELD_VALUE((reg), RF_TOP_AON_LDO15RF_SSTART_DELAY_AON_MASK, 9)

#define RF_TOP_AON_LDO15RF_PULLDOWN_AON \
    BIT(12)
#define RF_TOP_AON_LDO15RF_PULLDOWN_SEL_AON \
    BIT(13)

#define RF_TOP_AON_LDO15RF_VOUT_SEL_AON(value) \
    BIT_FIELD((value), 16)
#define RF_TOP_AON_LDO15RF_VOUT_SEL_AON_MASK \
    BIT_FIELD(MASK(3), 16)
#define RF_TOP_AON_LDO15RF_VOUT_SEL_AON_VALUE(reg) \
    FIELD_VALUE((reg), RF_TOP_AON_LDO15RF_VOUT_SEL_AON_MASK, 16)

#define RF_TOP_AON_LDO15RF_CC_AON(value) \
    BIT_FIELD((value), 24)
#define RF_TOP_AON_LDO15RF_CC_AON_MASK \
    BIT_FIELD(MASK(2), 24)
#define RF_TOP_AON_LDO15RF_CC_AON_VALUE(reg) \
    FIELD_VALUE((reg), RF_TOP_AON_LDO15RF_CC_AON_MASK, 24)

#define RF_TOP_AON_LDO15RF_BYPASS_AON   BIT(28)
/*------------------AON TSEN register-----------------------------------------*/
#define TSEN_TSEN_REFCODE_CORNER(value) \
    BIT_FIELD((value), 0)
#define TSEN_TSEN_REFCODE_CORNER_MASK \
    BIT_FIELD(MASK(12), 0)
#define TSEN_TSEN_REFCODE_CORNER_VALUE(reg) \
    FIELD_VALUE((reg), TSEN_TSEN_REFCODE_CORNER_MASK, 0)

#define TSEN_TSEN_REFCODE_RFCAL(value) \
    BIT_FIELD((value), 16)
#define TSEN_TSEN_REFCODE_RFCAL_MASK \
    BIT_FIELD(MASK(12), 16)
#define TSEN_TSEN_REFCODE_RFCAL_VALUE(reg) \
    FIELD_VALUE((reg), TSEN_TSEN_REFCODE_RFCAL_MASK, 16)

#define TSEN_XTAL_RDY                   BIT(28)
#define TSEN_XTAL_INN_CFG_EN_AON        BIT(29)

#define TSEN_XTAL_RDY_INT_SEL_AON(value) \
    BIT_FIELD((value), 30)
#define TSEN_XTAL_RDY_INT_SEL_AON_MASK \
    BIT_FIELD(MASK(2), 30)
#define TSEN_XTAL_RDY_INT_SEL_AON_VALUE(reg) \
    FIELD_VALUE((reg), TSEN_XTAL_RDY_INT_SEL_AON_MASK, 30)
/*------------------PDS PU RST CLKPLL register--------------------------------*/
#define PU_RST_CLKPLL_CLKPLL_RESET_SDM  BIT(0)
#define PU_RST_CLKPLL_CLKPLL_RESET_POSTDIV \
    BIT(1)
#define PU_RST_CLKPLL_CLKPLL_RESET_FBDV BIT(2)
#define PU_RST_CLKPLL_CLKPLL_RESET_REFDIV \
    BIT(3)
#define PU_RST_CLKPLL_CLKPLL_PU_POSTDIV BIT(4)
#define PU_RST_CLKPLL_CLKPLL_PU_FBDV    BIT(5)
#define PU_RST_CLKPLL_CLKPLL_PU_CLAMP_OP \
    BIT(6)
#define PU_RST_CLKPLL_CLKPLL_PU_PFD     BIT(7)
#define PU_RST_CLKPLL_CLKPLL_PU_CP      BIT(8)
#define PU_RST_CLKPLL_PU_CLKPLL_SFREG   BIT(9)
#define PU_RST_CLKPLL_PU_CLKPLL         BIT(10)

#define PU_RST_CLKPLL_ENABLE_MASK (\
    PU_RST_CLKPLL_CLKPLL_PU_POSTDIV \
    | PU_RST_CLKPLL_CLKPLL_PU_FBDV \
    | PU_RST_CLKPLL_CLKPLL_PU_PFD \
    | PU_RST_CLKPLL_CLKPLL_PU_CP \
    | PU_RST_CLKPLL_PU_CLKPLL_SFREG \
    | PU_RST_CLKPLL_PU_CLKPLL \
)
/*------------------PDS CLKPLL TOP CTRL register------------------------------*/
#define CLKPLL_TOP_CTRL_CLKPLL_POSTDIV(value) \
    BIT_FIELD((value), 0)
#define CLKPLL_TOP_CTRL_CLKPLL_POSTDIV_MASK \
    BIT_FIELD(MASK(7), 0)
#define CLKPLL_TOP_CTRL_CLKPLL_POSTDIV_VALUE(reg) \
    FIELD_VALUE((reg), CLKPLL_TOP_CTRL_CLKPLL_POSTDIV_MASK, 0)

#define CLKPLL_TOP_CTRL_CLKPLL_REFDIV_RATIO(value) \
    BIT_FIELD((value), 8)
#define CLKPLL_TOP_CTRL_CLKPLL_REFDIV_RATIO_MASK \
    BIT_FIELD(MASK(4), 8)
#define CLKPLL_TOP_CTRL_CLKPLL_REFDIV_RATIO_VALUE(reg) \
    FIELD_VALUE((reg), CLKPLL_TOP_CTRL_CLKPLL_REFDIV_RATIO_MASK, 8)

#define CLKPLL_TOP_CTRL_CLKPLL_XTAL_RC32M_SEL \
    BIT(12)
#define CLKPLL_TOP_CTRL_CLKPLL_REFCLK_SEL \
    BIT(16)

#define CLKPLL_TOP_CTRL_CLKPLL_VG11_SEL(value) \
    BIT_FIELD((value), 20)
#define CLKPLL_TOP_CTRL_CLKPLL_VG11_SEL_MASK \
    BIT_FIELD(MASK(2), 20)
#define CLKPLL_TOP_CTRL_CLKPLL_VG11_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CLKPLL_TOP_CTRL_CLKPLL_VG11_SEL_MASK, 20)

#define CLKPLL_TOP_CTRL_CLKPLL_VG13_SEL(value) \
    BIT_FIELD((value), 24)
#define CLKPLL_TOP_CTRL_CLKPLL_VG13_SEL_MASK \
    BIT_FIELD(MASK(2), 24)
#define CLKPLL_TOP_CTRL_CLKPLL_VG13_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CLKPLL_TOP_CTRL_CLKPLL_VG13_SEL_MASK, 24)
/*------------------PDS CLKPLL CP register------------------------------------*/
#define CLKPLL_CP_CLKPLL_SEL_CP_BIAS    BIT(0)

#define CLKPLL_CP_CLKPLL_ICP_5U(value)  BIT_FIELD((value), 4)
#define CLKPLL_CP_CLKPLL_ICP_5U_MASK    BIT_FIELD(MASK(2), 4)
#define CLKPLL_CP_CLKPLL_ICP_5U_VALUE(reg) \
    FIELD_VALUE((reg), CLKPLL_CP_CLKPLL_ICP_5U_MASK, 4)

#define CLKPLL_CP_CLKPLL_ICP_1U(value)  BIT_FIELD((value), 6)
#define CLKPLL_CP_CLKPLL_ICP_1U_MASK    BIT_FIELD(MASK(2), 6)
#define CLKPLL_CP_CLKPLL_ICP_1U_VALUE(reg) \
    FIELD_VALUE((reg), CLKPLL_CP_CLKPLL_ICP_1U_MASK, 6)

#define CLKPLL_CP_CLKPLL_INT_FRAC_SW    BIT(8)
#define CLKPLL_CP_CLKPLL_CP_STARTUP_EN  BIT(9)
#define CLKPLL_CP_CLKPLL_CP_OPAMP_EN    BIT(10)
/*------------------PDS CLKPLL RZ register------------------------------------*/
#define CLKPLL_RZ_CLKPLL_C4_EN          BIT(0)

#define CLKPLL_RZ_CLKPLL_R4(value)      BIT_FIELD((value), 4)
#define CLKPLL_RZ_CLKPLL_R4_MASK        BIT_FIELD(MASK(2), 4)
#define CLKPLL_RZ_CLKPLL_R4_VALUE(reg) \
    FIELD_VALUE((reg), CLKPLL_RZ_CLKPLL_R4_MASK, 4)

#define CLKPLL_RZ_CLKPLL_R4_SHORT       BIT(8)

#define CLKPLL_RZ_CLKPLL_C3(value)      BIT_FIELD((value), 12)
#define CLKPLL_RZ_CLKPLL_C3_MASK        BIT_FIELD(MASK(2), 12)
#define CLKPLL_RZ_CLKPLL_C3_VALUE(reg) \
    FIELD_VALUE((reg), CLKPLL_RZ_CLKPLL_C3_MASK, 12)

#define CLKPLL_RZ_CLKPLL_CZ(value)      BIT_FIELD((value), 14)
#define CLKPLL_RZ_CLKPLL_CZ_MASK        BIT_FIELD(MASK(2), 14)
#define CLKPLL_RZ_CLKPLL_CZ_VALUE(reg) \
    FIELD_VALUE((reg), CLKPLL_RZ_CLKPLL_CZ_MASK, 14)

#define CLKPLL_RZ_CLKPLL_RZ(value)      BIT_FIELD((value), 16)
#define CLKPLL_RZ_CLKPLL_RZ_MASK        BIT_FIELD(MASK(3), 16)
#define CLKPLL_RZ_CLKPLL_RZ_VALUE(reg) \
    FIELD_VALUE((reg), CLKPLL_RZ_CLKPLL_RZ_MASK, 16)
/*------------------PDS CLKPLL FBDV register----------------------------------*/
#define CLKPLL_FBDV_CLKPLL_SEL_SAMPLE_CLK(value) \
    BIT_FIELD((value), 0)
#define CLKPLL_FBDV_CLKPLL_SEL_SAMPLE_CLK_MASK \
    BIT_FIELD(MASK(2), 0)
#define CLKPLL_FBDV_CLKPLL_SEL_SAMPLE_CLK_VALUE(reg) \
    FIELD_VALUE((reg), CLKPLL_FBDV_CLKPLL_SEL_SAMPLE_CLK_MASK, 0)

#define CLKPLL_FBDV_CLKPLL_SEL_FB_CLK(value) \
    BIT_FIELD((value), 2)
#define CLKPLL_FBDV_CLKPLL_SEL_FB_CLK_MASK \
    BIT_FIELD(MASK(2), 2)
#define CLKPLL_FBDV_CLKPLL_SEL_FB_CLK_VALUE(reg) \
    FIELD_VALUE((reg), CLKPLL_FBDV_CLKPLL_SEL_FB_CLK_MASK, 2)
/*------------------PDS CLKPLL VCO register-----------------------------------*/
#define CLKPLL_VCO_CLKPLL_VCO_SPEED(value) \
    BIT_FIELD((value), 0)
#define CLKPLL_VCO_CLKPLL_VCO_SPEED_MASK \
    BIT_FIELD(MASK(3), 0)
#define CLKPLL_VCO_CLKPLL_VCO_SPEED_VALUE(reg) \
    FIELD_VALUE((reg), CLKPLL_VCO_CLKPLL_VCO_SPEED_MASK, 0)

#define CLKPLL_VCO_CLKPLL_SHRTR         BIT(3)
/*------------------PDS CLKPLL SDM register-----------------------------------*/
#define CLKPLL_SDM_CLKPLL_SDMIN(value)  BIT_FIELD((value), 0)
#define CLKPLL_SDM_CLKPLL_SDMIN_MASK    BIT_FIELD(MASK(24), 0)
#define CLKPLL_SDM_CLKPLL_SDMIN_VALUE(reg) \
    FIELD_VALUE((reg), CLKPLL_SDM_CLKPLL_SDMIN_MASK, 0)

#define CLKPLL_SDM_CLKPLL_DITHER_SEL(value) \
    BIT_FIELD((value), 24)
#define CLKPLL_SDM_CLKPLL_DITHER_SEL_MASK \
    BIT_FIELD(MASK(2), 24)
#define CLKPLL_SDM_CLKPLL_DITHER_SEL_VALUE(reg) \
    FIELD_VALUE((reg), CLKPLL_SDM_CLKPLL_DITHER_SEL_MASK, 24)

#define CLKPLL_SDM_CLKPLL_SDM_FLAG      BIT(28)
#define CLKPLL_SDM_CLKPLL_SDM_BYPASS    BIT(29)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_BL602_CLOCKING_DEFS_H_ */
