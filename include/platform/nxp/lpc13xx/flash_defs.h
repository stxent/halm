/*
 * platform/nxp/lpc13xx/flash_defs.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_LPC13XX_FLASH_DEFS_H_
#define PLATFORM_NXP_LPC13XX_FLASH_DEFS_H_
/*----------------------------------------------------------------------------*/
#define CODE_LPC1311          0x2C42502B
#define CODE_LPC1311_01       0x1816902B
#define CODE_LPC1313          0x2C40102B
#define CODE_LPC1313_01       0x1830102B
#define CODE_LPC1342          0x3D01402B
#define CODE_LPC1343          0x3D00002B
/*----------------------------------------------------------------------------*/
#define FLASH_PAGE_SIZE       256
#define FLASH_SECTOR_SIZE     4096
/*----------------------------------------------------------------------------*/
#define IAP_BASE              0x1FFF1FF1U
/*----------------------------------------------------------------------------*/
enum iapCommand
{
  CMD_PREPARE_FOR_WRITE       = 50,
  CMD_COPY_RAM_TO_FLASH       = 51,
  CMD_ERASE_SECTORS           = 52,
  CMD_BLANK_CHECK_SECTORS     = 53,
  CMD_READ_PART_ID            = 54,
  CMD_READ_BOOT_CODE_VERSION  = 55,
  CMD_COMPARE                 = 56,
  CMD_REINVOKE_ISP            = 57,
  CMD_READ_UID                = 58
};

enum iapResult
{
  RES_CMD_SUCCESS,
  RES_INVALID_COMMAND,
  RES_SRC_ADDR_ERROR,
  RES_DST_ADDR_ERROR,
  RES_SRC_ADDR_NOT_MAPPED,
  RES_DST_ADDR_NOT_MAPPED,
  RES_COUNT_ERROR,
  RES_INVALID_SECTOR,
  RES_SECTOR_NOT_BLANK,
  RES_SECTOR_NOT_PREPARED,
  RES_COMPARE_ERROR,
  RES_BUSY
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC13XX_FLASH_DEFS_H_ */
