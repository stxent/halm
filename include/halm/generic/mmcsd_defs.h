/*
 * halm/generic/mmcsd_defs.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_GENERIC_MMCSD_DEFS_H_
#define HALM_GENERIC_MMCSD_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/sdio.h>
/*----------------------------------------------------------------------------*/
#define BLOCK_POW                       9
#define ENUM_RATE                       400000
/*------------------CMD6------------------------------------------------------*/
#define MMC_BUS_WIDTH_PATTERN           0x03B70000UL
#define MMC_BUS_WIDTH_4BIT              0x00000100
#define MMC_BUS_WIDTH_8BIT              0x00000200
/*------------------CMD8------------------------------------------------------*/
#define CONDITION_PATTERN               0x000001AAUL
/*------------------CMD59-----------------------------------------------------*/
#define CRC_ENABLED                     0x00000001UL
/*------------------ACMD6-----------------------------------------------------*/
#define SD_BUS_WIDTH_4BIT               0x00000002UL
/*------------------OCR register----------------------------------------------*/
/* Voltage range from 2.7V to 3.6V */
#define OCR_VOLTAGE_MASK                0x00FF8000UL
/* Card power up status bit (active low) */
#define OCR_BUSY                        BIT(31)

/* MMC: Sector addressing mode */
#define OCR_MMC_SECTOR_MODE             BIT(30)

/* SD: Card Capacity Status */
#define OCR_SD_CCS                      BIT(30)
/* SD: Host Capacity Support */
#define OCR_SD_HCS                      BIT(30)
/*----------------------------------------------------------------------------*/
enum MMCSDCapacity
{
  MMCSD_SC,
  MMCSD_HC,
  MMCSD_XC,
  MMCSD_UC
};

enum MMCSDCommand
{
  /* Common commands */
  CMD0_GO_IDLE_STATE          = 0,
  CMD8_SEND_IF_COND           = 8,
  CMD9_SEND_CSD               = 9,
  CMD10_SEND_CID              = 10,
  CMD12_STOP_TRANSMISSION     = 12,
  CMD13_SEND_STATUS           = 13,
  CMD16_SET_BLOCKLEN          = 16,
  CMD17_READ_SINGLE_BLOCK     = 17,
  CMD18_READ_MULTIPLE_BLOCK   = 18,
  CMD24_WRITE_BLOCK           = 24,
  CMD25_WRITE_MULTIPLE_BLOCK  = 25,
  CMD55_APP_CMD               = 55,
  ACMD41_SD_SEND_OP_COND      = 41,
  ACMD42_SET_CLR_CARD_DETECT  = 42,

  /* Commands available only for MMC cards */
  CMD1_SEND_OP_COND           = 1,
  CMD3_SET_RELATIVE_ADDR      = 3,
  CMD6_SWITCH                 = 6,
  CMD8_SEND_EXT_CSD           = 8,

  /* Commands available only in SDIO mode */
  CMD2_ALL_SEND_CID           = 2,
  CMD3_SEND_RELATIVE_ADDR     = 3,
  CMD7_SELECT_CARD            = 7,
  ACMD6_SET_BUS_WIDTH         = 6,

  /* Commands available only in SPI mode */
  CMD58_READ_OCR              = 58,
  CMD59_CRC_ON_OFF            = 59
};

enum MMCSDResponse
{
  MMCSD_RESPONSE_NONE = SDIO_RESPONSE_NONE,
  MMCSD_RESPONSE_R1   = SDIO_RESPONSE_SHORT,
  MMCSD_RESPONSE_R1B  = SDIO_RESPONSE_SHORT,
  MMCSD_RESPONSE_R2   = SDIO_RESPONSE_LONG,
  MMCSD_RESPONSE_R3   = SDIO_RESPONSE_SHORT,
  MMCSD_RESPONSE_R4   = SDIO_RESPONSE_SHORT,
  MMCSD_RESPONSE_R5   = SDIO_RESPONSE_SHORT,
  MMCSD_RESPONSE_R6   = SDIO_RESPONSE_SHORT,
  MMCSD_RESPONSE_R7   = SDIO_RESPONSE_SHORT
};

enum MMCSDType
{
  CARD_SD_1_0,
  CARD_SD_2_0,
  CARD_MMC
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_MMCSD_DEFS_H_ */
