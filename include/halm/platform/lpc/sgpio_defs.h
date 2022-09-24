/*
 * halm/platform/lpc/sgpio_defs.h
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SGPIO_DEFS_H_
#define HALM_PLATFORM_LPC_SGPIO_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Pin multiplexer configuration registers-------------------*/
enum
{
  OUT_DOUTM1   = 0x0,
  OUT_DOUTM2A  = 0x1,
  OUT_DOUTM2B  = 0x2,
  OUT_DOUTM2C  = 0x3,
  OUT_GPIO     = 0x4,
  OUT_DOUTM4A  = 0x5,
  OUT_DOUTM4B  = 0x6,
  OUT_DOUTM4C  = 0x7,
  OUT_CLK      = 0x8,
  OUT_DOUTM8A  = 0x9,
  OUT_DOUTM8B  = 0xA,
  OUT_DOUTM8C  = 0xB
};

enum
{
  OE_GPIO = 0x0,
  OE_OEM1 = 0x4,
  OE_OEM2 = 0x5,
  OE_OEM4 = 0x6,
  OE_OEM8 = 0x7,
};

#define OUT_MUX_CFG_P_OUT_CFG_MASK      BIT_FIELD(MASK(4), 0)
#define OUT_MUX_CFG_P_OUT_CFG(value)    BIT_FIELD((value), 0)
#define OUT_MUX_CFG_P_OUT_CFG_VALUE(reg) \
    FIELD_VALUE((reg), OUT_MUX_CFG_P_OUT_CFG_MASK, 0)

#define OUT_MUX_CFG_P_OE_CFG_MASK       BIT_FIELD(MASK(3), 4)
#define OUT_MUX_CFG_P_OE_CFG(value)     BIT_FIELD((value), 4)
#define OUT_MUX_CFG_P_OE_CFG_VALUE(reg) \
    FIELD_VALUE((reg), OUT_MUX_CFG_P_OE_CFG_MASK, 4)
/*------------------SGPIO multiplexer configuration registers-----------------*/
enum
{
  CONCAT_SELF_LOOP  = 0x0,
  CONCAT_2_SLICES   = 0x1,
  CONCAT_4_SLICES   = 0x2,
  CONCAT_8_SLICES   = 0x3
};

enum
{
  QUALIFIER_ENABLE  = 0x0,
  QUALIFIER_DISABLE = 0x1,
  QUALIFIER_SLICE   = 0x2,
  QUALIFIER_PIN     = 0x3
};

#define SGPIO_MUX_CFG_INT_CLK_ENABLE    0
#define SGPIO_MUX_CFG_EXT_CLK_ENABLE    BIT(0)
#define SGPIO_MUX_CFG_CONCAT_ENABLE     BIT(11)

#define SGPIO_MUX_CFG_CLK_SOURCE_PIN_MODE_MASK \
    BIT_FIELD(MASK(2), 1)
#define SGPIO_MUX_CFG_CLK_SOURCE_PIN_MODE(value) \
    BIT_FIELD((value), 1)
#define SGPIO_MUX_CFG_CLK_SOURCE_PIN_MODE_VALUE(reg) \
    FIELD_VALUE((reg), SGPIO_MUX_CFG_CLK_SOURCE_PIN_MODE_MASK, 1)

#define SGPIO_MUX_CFG_CLK_SOURCE_SLICE_MODE_MASK \
    BIT_FIELD(MASK(2), 3)
#define SGPIO_MUX_CFG_CLK_SOURCE_SLICE_MODE(value) \
    BIT_FIELD((value), 3)
#define SGPIO_MUX_CFG_CLK_SOURCE_SLICE_MODE_VALUE(reg) \
    FIELD_VALUE((reg), SGPIO_MUX_CFG_CLK_SOURCE_SLICE_MODE_MASK, 3)

#define SGPIO_MUX_CFG_QUALIFIER_MODE_MASK \
    BIT_FIELD(MASK(2), 5)
#define SGPIO_MUX_CFG_QUALIFIER_MODE(value) \
    BIT_FIELD((value), 5)
#define SGPIO_MUX_CFG_QUALIFIER_MODE_VALUE(reg) \
    FIELD_VALUE((reg), SGPIO_MUX_CFG_QUALIFIER_MODE_MASK, 5)

#define SGPIO_MUX_CFG_QUALIFIER_PIN_MODE_MASK \
    BIT_FIELD(MASK(2), 7)
#define SGPIO_MUX_CFG_QUALIFIER_PIN_MODE(value) \
    BIT_FIELD((value), 7)
#define SGPIO_MUX_CFG_QUALIFIER_PIN_MODE_VALUE(reg) \
    FIELD_VALUE((reg), SGPIO_MUX_CFG_QUALIFIER_PIN_MODE_MASK, 7)

#define SGPIO_MUX_CFG_QUALIFIER_SLICE_MODE_MASK \
    BIT_FIELD(MASK(2), 9)
#define SGPIO_MUX_CFG_QUALIFIER_SLICE_MODE(value) \
    BIT_FIELD((value), 9)
#define SGPIO_MUX_CFG_QUALIFIER_SLICE_MODE_VALUE(reg) \
    FIELD_VALUE((reg), SGPIO_MUX_CFG_QUALIFIER_SLICE_MODE_MASK, 9)

#define SGPIO_MUX_CFG_CONCAT_ORDER_MASK \
    BIT_FIELD(MASK(2), 12)
#define SGPIO_MUX_CFG_CONCAT_ORDER(value) \
    BIT_FIELD((value), 12)
#define SGPIO_MUX_CFG_CONCAT_ORDER_VALUE(reg) \
    FIELD_VALUE((reg), SGPIO_MUX_CFG_CONCAT_ORDER_MASK, 12)
/*------------------Slice multiplexer configuration registers-----------------*/
enum
{
  CLK_CAP_RISING      = 0,
  CLK_CAP_FALLING     = 1
};

enum
{
  CLK_GEN_INTERNAL    = 0,
  CLK_GEN_EXTERNAL    = 1
};

enum
{
  PARALLEL_MODE_1_BIT = 0,
  PARALLEL_MODE_2_BIT = 1,
  PARALLEL_MODE_4_BIT = 2,
  PARALLEL_MODE_8_BIT = 3
};

#define SLICE_MUX_CFG_MATCH_MODE        BIT(0)
#define SLICE_MUX_CFG_INV_OUT_CLK       BIT(3)
#define SLICE_MUX_CFG_INV_QUALIFIER     BIT(8)

#define SLICE_MUX_CFG_CLK_CAPTURE_MODE_MASK \
    BIT_FIELD(MASK(1), 1)
#define SLICE_MUX_CFG_CLK_CAPTURE_MODE(value) \
    BIT_FIELD((value), 1)
#define SLICE_MUX_CFG_CLK_CAPTURE_MODE_VALUE(reg) \
    FIELD_VALUE((reg), SLICE_MUX_CFG_CLK_CAPTURE_MODE_MASK, 1)

#define SLICE_MUX_CFG_CLKGEN_MODE_MASK \
    BIT_FIELD(MASK(1), 2)
#define SLICE_MUX_CFG_CLKGEN_MODE(value) \
    BIT_FIELD((value), 2)
#define SLICE_MUX_CFG_CLKGEN_MODE_VALUE(reg) \
    FIELD_VALUE((reg), SLICE_MUX_CFG_CLKGEN_MODE_MASK, 2)

#define SLICE_MUX_CFG_DATA_CAPTURE_MODE_MASK \
    BIT_FIELD(MASK(2), 4)
#define SLICE_MUX_CFG_DATA_CAPTURE_MODE(value) \
    BIT_FIELD((value), 4)
#define SLICE_MUX_CFG_DATA_CAPTURE_MODE_VALUE(reg) \
    FIELD_VALUE((reg), SLICE_MUX_CFG_DATA_CAPTURE_MODE_MASK, 4)

#define SLICE_MUX_CFG_PARALLEL_MODE_MASK \
    BIT_FIELD(MASK(2), 6)
#define SLICE_MUX_CFG_PARALLEL_MODE(value) \
    BIT_FIELD((value), 6)
#define SLICE_MUX_CFG_PARALLEL_MODE_VALUE(reg) \
    FIELD_VALUE((reg), SLICE_MUX_CFG_PARALLEL_MODE_MASK, 6)
/*------------------Position registers----------------------------------------*/
#define POS_POS_MASK                    BIT_FIELD(MASK(8), 0)
#define POS_POS(value)                  BIT_FIELD((value), 0)
#define POS_POS_VALUE(reg)              FIELD_VALUE((reg), POS_POS_MASK, 0)

#define POS_POS_RESET_MASK \
    BIT_FIELD(MASK(8), 8)
#define POS_POS_RESET(value) \
    BIT_FIELD((value), 8)
#define POS_POS_RESET_VALUE(reg) \
    FIELD_VALUE((reg), POS_POS_RESET_MASK, 8)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SGPIO_DEFS_H_ */
