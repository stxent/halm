/*
 * halm/platform/lpc/spifi_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SPIFI_DEFS_H_
#define HALM_PLATFORM_LPC_SPIFI_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Control register------------------------------------------*/
#define CTRL_TIMEOUT_MAX                MASK(16)
#define CTRL_TIMEOUT_MASK               BIT_FIELD(MASK(16), 0)
#define CTRL_TIMEOUT(value)             BIT_FIELD((value), 0)
#define CTRL_TIMEOUT_VALUE(reg) \
    FIELD_VALUE((reg), CTRL_TIMEOUT_MASK, 0)

#define CTRL_CSHIGH_MAX                 MASK(4)
#define CTRL_CSHIGH_MASK                BIT_FIELD(MASK(4), 16)
#define CTRL_CSHIGH(value)              BIT_FIELD((value), 16)
#define CTRL_CSHIGH_VALUE(reg) \
    FIELD_VALUE((reg), CTRL_CSHIGH_MASK, 16)

#define CTRL_D_PRFTCH_DIS               BIT(21)
#define CTRL_INTEN                      BIT(22)
#define CTRL_MODE3                      BIT(23)
#define CTRL_PRFTCH_DIS                 BIT(27)
#define CTRL_DUAL                       BIT(28)
#define CTRL_RFCLK                      BIT(29)
#define CTRL_FBCLK                      BIT(30)
#define CTRL_DMAEN                      BIT(31)
/*------------------Command register------------------------------------------*/
enum
{
  /* All fields of the command are serial */
  FIELDFORM_SERIAL_ALL    = 0x0,
  /* Data field is quad/dual, other fields are serial */
  FIELDFORM_PARALLEL_DATA = 0x1,
  /* Opcode field is serial, other fields are quad/dual */
  FIELDFORM_SERIAL_OPCODE = 0x2,
  /* All fields of the command are quad/dual */
  FIELDFORM_PARALLEL_ALL  = 0x3
};

enum
{
  /* Opcode only, no address field */
  FRAMEFORM_OPCODE            = 0x1,
  /* Opcode and one least significant byte of address field */
  FRAMEFORM_OPCODE_ADDRESS_8  = 0x2,
  /* Opcode and two least significant bytes of address field */
  FRAMEFORM_OPCODE_ADDRESS_16 = 0x3,
  /* Opcode and three least significant bytes of address field */
  FRAMEFORM_OPCODE_ADDRESS_24 = 0x4,
  /* Opcode and four bytes of address field */
  FRAMEFORM_OPCODE_ADDRESS_32 = 0x5,
  /* No opcode, three least significant bytes of address field only */
  FRAMEFORM_ADDRESS_24        = 0x6,
  /* No opcode, four bytes of address field only */
  FRAMEFORM_ADDRESS_32        = 0x7
};

#define CMD_DATALEN_MAX                 MASK(14)
#define CMD_DATALEN_MASK                BIT_FIELD(MASK(14), 0)
#define CMD_DATALEN(value)              BIT_FIELD((value), 0)
#define CMD_DATALEN_VALUE(reg) \
    FIELD_VALUE((reg), CMD_DATALEN_MASK, 0)

#define CMD_POLL                        BIT(14)
#define CMD_DOUT                        BIT(15)

#define CMD_INTLEN_MAX                  MASK(3)
#define CMD_INTLEN_MASK                 BIT_FIELD(MASK(3), 16)
#define CMD_INTLEN(value)               BIT_FIELD((value), 16)
#define CMD_INTLEN_VALUE(reg) \
    FIELD_VALUE((reg), CMD_INTLEN_MASK, 16)

#define CMD_FIELDFORM_MASK              BIT_FIELD(MASK(2), 19)
#define CMD_FIELDFORM(value)            BIT_FIELD((value), 19)
#define CMD_FIELDFORM_VALUE(reg) \
    FIELD_VALUE((reg), CMD_FIELDFORM_MASK, 19)

#define CMD_FRAMEFORM_MASK              BIT_FIELD(MASK(3), 21)
#define CMD_FRAMEFORM(value)            BIT_FIELD((value), 21)
#define CMD_FRAMEFORM_VALUE(reg) \
    FIELD_VALUE((reg), CMD_FRAMEFORM_MASK, 21)

#define CMD_OPCODE_MASK                 BIT_FIELD(MASK(8), 24)
#define CMD_OPCODE(value)               BIT_FIELD((value), 24)
#define CMD_OPCODE_VALUE(reg) \
    FIELD_VALUE((reg), CMD_OPCODE_MASK, 24)
/*------------------Memory Command register-----------------------------------*/
#define MCMD_POLL                       BIT(14)
#define MCMD_DOUT                       BIT(15)

#define MCMD_INTLEN_MASK                BIT_FIELD(MASK(3), 16)
#define MCMD_INTLEN(value)              BIT_FIELD((value), 16)
#define MCMD_INTLEN_VALUE(reg) \
    FIELD_VALUE((reg), MCMD_INTLEN_MASK, 16)

#define MCMD_FIELDFORM_MASK             BIT_FIELD(MASK(2), 19)
#define MCMD_FIELDFORM(value)           BIT_FIELD((value), 19)
#define MCMD_FIELDFORM_VALUE(reg) \
    FIELD_VALUE((reg), MCMD_FIELDFORM_MASK, 19)

#define MCMD_FRAMEFORM_MASK             BIT_FIELD(MASK(3), 21)
#define MCMD_FRAMEFORM(value)           BIT_FIELD((value), 21)
#define MCMD_FRAMEFORM_VALUE(reg) \
    FIELD_VALUE((reg), MCMD_FRAMEFORM_MASK, 21)

#define MCMD_OPCODE_MASK                BIT_FIELD(MASK(8), 24)
#define MCMD_OPCODE(value)              BIT_FIELD((value), 24)
#define MCMD_OPCODE_VALUE(reg) \
    FIELD_VALUE((reg), MCMD_OPCODE_MASK, 24)
/*------------------Status register-------------------------------------------*/
#define STAT_MCINIT                     BIT(0)
#define STAT_CMD                        BIT(1)
#define STAT_RESET                      BIT(4)
#define STAT_INTRQ                      BIT(5)

#define STAT_VERSION_MASK               BIT_FIELD(MASK(8), 24)
#define STAT_VERSION_VALUE(reg) \
    FIELD_VALUE((reg), STAT_VERSION_MASK, 24)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SPIFI_DEFS_H_ */
