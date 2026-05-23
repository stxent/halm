/*
 * halm/platform/bouffalo/bl602/glb_defs.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_BL602_GLB_DEFS_H_
#define HALM_PLATFORM_BOUFFALO_BL602_GLB_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------GLB Parameters register-----------------------------------*/
#define GLB_PARM_REG_BD_EN              BIT(0)
#define GLB_PARM_REG_EXT_RST_SMT        BIT(1)

#define GLB_PARM_JTAG_SWAP_SET(value)   BIT_FIELD((value), 2)
#define GLB_PARM_JTAG_SWAP_SET_MASK     BIT_FIELD(MASK(6), 2)
#define GLB_PARM_JTAG_SWAP_SET_VALUE(reg) \
    FIELD_VALUE((reg), GLB_PARM_JTAG_SWAP_SET_MASK, 2)

#define GLB_PARM_SWAP_SFLASH_IO3_IO0    BIT(8)
#define GLB_PARM_SEL_EMBEDDED_SFLASH    BIT(9)
#define GLB_PARM_REG_SPI_0_MASTER_MODE  BIT(12)
#define GLB_PARM_REG_SPI_0_SWAP         BIT(13)
#define GLB_PARM_REG_CCI_USE_JTAG_PIN   BIT(15)
#define GLB_PARM_REG_CCI_USE_SDIO_PIN   BIT(16)
#define GLB_PARM_P1_ADC_TEST_WITH_CCI   BIT(17)
#define GLB_PARM_P2_DAC_TEST_WITH_CCI   BIT(18)
#define GLB_PARM_P3_CCI_USE_IO2_IO5     BIT(19)
#define GLB_PARM_P4_ADC_TEST_WITH_JTAG  BIT(20)
#define GLB_PARM_P5_DAC_TEST_WITH_JTAG  BIT(21)
#define GLB_PARM_P6_SDIO_USE_IO0_IO5    BIT(22)
#define GLB_PARM_P7_JTAG_USE_IO2_IO5    BIT(23)

#define GLB_PARM_UART_SWAP_SET(value)   BIT_FIELD((value), 24)
#define GLB_PARM_UART_SWAP_SET_MASK     BIT_FIELD(MASK(3), 24)
#define GLB_PARM_UART_SWAP_SET_VALUE(reg) \
    FIELD_VALUE((reg), GLB_PARM_UART_SWAP_SET_MASK, 24)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_BL602_GLB_DEFS_H_ */
