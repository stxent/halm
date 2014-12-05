/*
 * platform/nxp/sdio.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef MODULES_SDIO_H_
#define MODULES_SDIO_H_
/*----------------------------------------------------------------------------*/
#include <interface.h>
/*----------------------------------------------------------------------------*/
enum sdioCommand
{
  CMD_GO_IDLE_STATE         = 0,
  CMD_ALL_SEND_CID          = 2,
  CMD_SEND_RELATIVE_ADDR    = 3,
  CMD_SELECT_CARD           = 7,
  CMD_SEND_IF_COND          = 8,
  CMD_SEND_CSD              = 9,
  CMD_SEND_CID              = 10,
  CMD_STOP_TRANSMISSION     = 12,
  CMD_SEND_STATUS           = 13,
  CMD_SET_BLOCKLEN          = 16,
  CMD_READ_SINGLE_BLOCK     = 17,
  CMD_READ_MULTIPLE_BLOCK   = 18,
  CMD_WRITE_BLOCK           = 24,
  CMD_WRITE_MULTIPLE_BLOCK  = 25,
  CMD_APP_CMD               = 55,
  CMD_READ_OCR              = 58,
  ACMD_SD_SEND_OP_COND      = 41
};
/*----------------------------------------------------------------------------*/
enum sdioFlags
{
  /** Send initialization sequence to device. */
  SDIO_INITIALIZE   = 0x01,
  /** Generate and check checksum. */
  SDIO_CHECK_CRC    = 0x02,
  /** Enable data transfer mode. */
  SDIO_DATA_MODE    = 0x04,
  /** Clear to read data from device, set to write data to device. */
  SDIO_WRITE_MODE   = 0x08,
  /** Enable stream transfer mode instead of block mode. */
  SDIO_STREAM_MODE  = 0x10,
  /** Send stop command at the end of data transfer. */
  SDIO_AUTO_STOP    = 0x20,
  /** Wait for previous data transfer completion. */
  SDIO_WAIT_DATA    = 0x40
};
/*----------------------------------------------------------------------------*/
enum sdioMode
{
  SDIO_SPI,
  SDIO_1BIT,
  SDIO_4BIT
};
/*----------------------------------------------------------------------------*/
enum sdioOption
{
  IF_SDIO_MODE = IF_OPTION_END,
  IF_SDIO_EXECUTE,
  IF_SDIO_ARGUMENT,
  IF_SDIO_COMMAND,
  IF_SDIO_RESPONSE,
  IF_SDIO_BLOCK_LENGTH
};
/*----------------------------------------------------------------------------*/
enum sdioResponse
{
  SDIO_RESPONSE_NONE,
  SDIO_RESPONSE_SHORT,
  SDIO_RESPONSE_LONG
};
/*----------------------------------------------------------------------------*/
#endif /* MODULES_SDIO_H_ */
