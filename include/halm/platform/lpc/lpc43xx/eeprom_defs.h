/*
 * halm/platform/lpc/lpc43xx/eeprom_defs.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LPC_LPC43XX_EEPROM_DEFS_H_
#define HALM_PLATFORM_LPC_LPC43XX_EEPROM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Command register------------------------------------------*/
#define CMD_ERASE_PROGRAM               BIT_FIELD(6, 0)
/*------------------Read Wait State register----------------------------------*/
#define RWSTATE_RPHASE2_MASK            BIT_FIELD(MASK(8), 0)
#define RWSTATE_RPHASE2(value)          BIT_FIELD((value), 0)
#define RWSTATE_RPHASE2_VALUE(reg) \
    FIELD_VALUE((reg), RWSTATE_RPHASE2_MASK, 0)

#define RWSTATE_RPHASE1_MASK            BIT_FIELD(MASK(8), 8)
#define RWSTATE_RPHASE1(value)          BIT_FIELD((value), 8)
#define RWSTATE_RPHASE1_VALUE(reg) \
    FIELD_VALUE((reg), RWSTATE_RPHASE1_MASK, 8)
/*------------------Auto Programming register---------------------------------*/
enum
{
  AUTOPROG_OFF  = 0,
  AUTOPROG_WORD = 1,
  AUTOPROG_PAGE = 2
};

#define AUTOPROG_MODE_MASK              BIT_FIELD(MASK(2), 0)
#define AUTOPROG_MODE(value)            BIT_FIELD((value), 0)
#define AUTOPROG_MODE_VALUE(reg) \
    FIELD_VALUE((reg), AUTOPROG_MODE_MASK, 0)
/*------------------Wait State register---------------------------------------*/
#define WSTATE_PHASE3_MASK              BIT_FIELD(MASK(8), 0)
#define WSTATE_PHASE3(value)            BIT_FIELD((value), 0)
#define WSTATE_PHASE3_VALUE(reg) \
    FIELD_VALUE((reg), WSTATE_PHASE3_MASK, 0)

#define WSTATE_PHASE2_MASK              BIT_FIELD(MASK(8), 8)
#define WSTATE_PHASE2(value)            BIT_FIELD((value), 8)
#define WSTATE_PHASE2_VALUE(reg) \
    FIELD_VALUE((reg), WSTATE_PHASE2_MASK, 8)

#define WSTATE_PHASE1_MASK              BIT_FIELD(MASK(8), 16)
#define WSTATE_PHASE1(value)            BIT_FIELD((value), 16)
#define WSTATE_PHASE1_VALUE(reg) \
    FIELD_VALUE((reg), WSTATE_PHASE1_MASK, 16)

#define WSTATE_LCK_PARWEP               BIT(31)
/*------------------Power Down register---------------------------------------*/
#define PWRDWN_MODE                     BIT(0)
/*------------------Interrupt registers---------------------------------------*/
#define INT_PROG_DONE                   BIT(2)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_EEPROM_DEFS_H_ */
