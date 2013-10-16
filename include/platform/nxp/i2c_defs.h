/*
 * platform/nxp/i2c_defs.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef I2C_DEFS_H_
#define I2C_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <macro.h>
/*------------------Control Set Register--------------------------------------*/
#define CONSET_AA                       BIT(2) /* Assert acknowledge flag */
#define CONSET_SI                       BIT(3) /* I2C interrupt flag */
#define CONSET_STO                      BIT(4) /* STOP flag */
#define CONSET_STA                      BIT(5) /* START flag */
#define CONSET_I2EN                     BIT(6) /* I2C interface enable */
/*------------------Control Clear Register------------------------------------*/
/* Writing a 1 clears corresponding bit, writing 0 has no effect */
#define CONCLR_AAC                      BIT(2)
#define CONCLR_SIC                      BIT(3)
#define CONCLR_STAC                     BIT(5)
#define CONCLR_I2ENC                    BIT(6)
/*------------------Data Register---------------------------------------------*/
#define DATA_READ                       BIT(0)
#define DATA_WRITE                      (0)
/*------------------Status Register-------------------------------------------*/
/* TODO Remove unused mask? */
#define STAT_MASK                       BIT_FIELD(0x1F, 3)
#define STAT_VALUE(reg)                 (((reg) >> 3) & 0x1F)
/*------------------Monitor Mode Control Register-----------------------------*/
#define MMCTRL_MM_ENA                   BIT(0) /* Monitor mode enable */
#define MMCTRL_ENA_SCL                  BIT(1) /* SCL output enable */
#define MMCTRL_MATCH_ALL                BIT(2) /* Select interrupt match mode */
/*----------------------------------------------------------------------------*/
#endif /* I2C_DEFS_H_ */
