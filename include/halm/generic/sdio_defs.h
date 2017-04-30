/*
 * halm/generic/sdio_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_GENERIC_SDIO_DEFS_H_
#define HALM_GENERIC_SDIO_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#define COMMAND_CODE_MASK               BIT_FIELD(MASK(6), 0)
#define COMMAND_CODE(value)             BIT_FIELD((value), 0)
#define COMMAND_CODE_VALUE(command) \
    FIELD_VALUE((command), COMMAND_CODE_MASK, 0)
#define COMMAND_RESP_MASK               BIT_FIELD(MASK(2), 6)
#define COMMAND_RESP(value)             BIT_FIELD((value), 6)
#define COMMAND_RESP_VALUE(command) \
    FIELD_VALUE((command), COMMAND_RESP_MASK, 6)
#define COMMAND_FLAG_MASK               BIT_FIELD(MASK(16), 8)
#define COMMAND_FLAG(value)             BIT_FIELD((value), 8)
#define COMMAND_FLAG_VALUE(command) \
    FIELD_VALUE((command), COMMAND_FLAG_MASK, 8)
/*----------------------------------------------------------------------------*/
#define SDIO_COMMAND(code, response, flags) \
    (COMMAND_CODE(code) | COMMAND_RESP(response) | COMMAND_FLAG(flags))
/*----------------------------------------------------------------------------*/
enum CardState
{
  CARD_IDLE,
  CARD_READY,
  CARD_IDENT,
  CARD_STANDBY,
  CARD_TRANSFER,
  CARD_DATA,
  CARD_RECEIVE,
  CARD_PROGRAMMING,
  CARD_DISCONNECT
};

#define CURRENT_STATE_MASK              BIT_FIELD(MASK(4), 9)
#define CURRENT_STATE(response) \
    FIELD_VALUE((response), CURRENT_STATE_MASK, 9)
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_SDIO_DEFS_H_ */
