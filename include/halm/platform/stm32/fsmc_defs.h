/*
 * halm/platform/stm32/fsmc_defs.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_FSMC_DEFS_H_
#define HALM_PLATFORM_STM32_FSMC_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#define FSMC_SUBBANK_SIZE               0x04000000UL
/*------------------SRAM/PSRAM chip-select Control Registers------------------*/
#define BCR_MBKEN                       BIT(0)
#define BCR_MUXEN                       BIT(1)

#define BCR_MTYP(value)                 BIT_FIELD(value, 2)
#define BCR_MTYP_MASK                   BIT_FIELD(MASK(2), 2)
#define BCR_MTYP_VALUE(reg)             BCR_MTYP_MASK

#define BCR_MWID(value)                 BIT_FIELD(value, 4)
#define BCR_MWID_MASK                   BIT_FIELD(MASK(2), 4)
#define BCR_MWID_VALUE(reg)             BCR_MWID_MASK

#define BCR_FACCEN                      BIT(6)
#define BCR_BURSTEN                     BIT(8)
#define BCR_WAITPOL                     BIT(9)
#define BCR_WRAPMOD                     BIT(10)
#define BCR_WAITCFG                     BIT(11)
#define BCR_WREN                        BIT(12)
#define BCR_WAITEN                      BIT(13)
#define BCR_EXTMOD                      BIT(14)
#define BCR_ASYNCWAIT                   BIT(15)
#define BCR_CBBURST                     BIT(19)

/* Memory Type configurations */
enum
{
  BCR_MTYP_SRAM  = 0,
  BCR_MTYP_PSRAM = 1,
  BCR_MTYP_NOR   = 2
};

/* Memory Width configurations */
enum
{
  BCR_MWID_8   = 0,
  BCR_MWID_16  = 1
};
/*------------------SRAM/PSRAM chip-select Timing Registers-------------------*/
#define BTR_ADDSET(value)               BIT_FIELD(value, 0)
#define BTR_ADDSET_MASK                 BIT_FIELD(MASK(4), 0)
#define BTR_ADDSET_VALUE(reg)           FIELD_VALUE(reg, BTR_ADDSET_MASK, 0)
#define BTR_ADDSET_MAX                  15 /* N cycles */

#define BTR_ADDHLD(value)               BIT_FIELD(value, 4)
#define BTR_ADDHLD_MASK                 BIT_FIELD(MASK(4), 4)
#define BTR_ADDHLD_VALUE(reg)           FIELD_VALUE(reg, BTR_ADDHLD_MASK, 4)
#define BTR_ADDHLD_MAX                  15 /* N cycles */
#define BTR_ADDHLD_MIN                  1

#define BTR_DATAST(value)               BIT_FIELD(value, 8)
#define BTR_DATAST_MASK                 BIT_FIELD(MASK(8), 8)
#define BTR_DATAST_VALUE(reg)           FIELD_VALUE(reg, BTR_DATAST_MASK, 8)
#define BTR_DATAST_MAX                  255 /* N (R) or N + 1 (W) cycles */
#define BTR_DATAST_MIN                  1

#define BTR_BUSTURN(value)              BIT_FIELD(value, 16)
#define BTR_BUSTURN_MASK                BIT_FIELD(MASK(4), 16)
#define BTR_BUSTURN_VALUE(reg)          FIELD_VALUE(reg, BTR_BUSTURN_MASK, 16)
#define BTR_BUSTURN_MAX                 15 /* N + 1 (R) or N + 2 (W) cycles */

#define BTR_CLKDIV(value)               BIT_FIELD(value, 20)
#define BTR_CLKDIV_MASK                 BIT_FIELD(MASK(4), 20)
#define BTR_CLKDIV_VALUE(reg)           FIELD_VALUE(reg, BTR_CLKDIV_MASK, 20)
#define BTR_CLKDIV_MAX                  15 /* N + 1 cycles */
#define BTR_CLKDIV_MIN                  1

#define BTR_DATLAT(value)               BIT_FIELD(value, 24)
#define BTR_DATLAT_MASK                 BIT_FIELD(MASK(4), 24)
#define BTR_DATLAT_VALUE(reg)           FIELD_VALUE(reg, BTR_DATLAT_MASK, 24)
#define BTR_DATLAT_MAX                  15 /* N + 2 cycles */

#define BTR_ACCMOD(value)               BIT_FIELD(value, 28)
#define BTR_ACCMOD_MASK                 BIT_FIELD(MASK(2), 28)
#define BTR_ACCMOD_VALUE(reg)           FIELD_VALUE(reg, BTR_ACCMOD_MASK, 28)

/* Access Mode configurations */
enum
{
  BTR_ACCMOD_A = 0,
  BTR_ACCMOD_B = 1,
  BTR_ACCMOD_C = 2,
  BTR_ACCMOD_D = 3
};
/*------------------SRAM/PSRAM Write Timing Registers-------------------------*/
#define BWTR_ADDSET(value)              BIT_FIELD(value, 0)
#define BWTR_ADDSET_MASK                BIT_FIELD(MASK(4), 0)
#define BWTR_ADDSET_VALUE(reg)          FIELD_VALUE(reg, BWTR_ADDSET_MASK, 0)
#define BWTR_ADDSET_MAX                 15 /* N cycles */

#define BWTR_ADDHLD(value)              BIT_FIELD(value, 4)
#define BWTR_ADDHLD_MASK                BIT_FIELD(MASK(4), 4)
#define BWTR_ADDHLD_VALUE(reg)          FIELD_VALUE(reg, BWTR_ADDHLD_MASK, 4)
#define BWTR_ADDHLD_MAX                 15 /* N cycles */
#define BWTR_ADDHLD_MIN                 1

#define BWTR_DATAST(value)              BIT_FIELD(value, 8)
#define BWTR_DATAST_MASK                BIT_FIELD(MASK(8), 8)
#define BWTR_DATAST_VALUE(reg)          FIELD_VALUE(reg, BWTR_DATAST_MASK, 8)
#define BWTR_DATAST_MAX                 255 /* N + 1 cycles */
#define BWTR_DATAST_MIN                 1

#define BWTR_BUSTURN(value)             BIT_FIELD(value, 16)
#define BWTR_BUSTURN_MASK               BIT_FIELD(MASK(4), 16)
#define BWTR_BUSTURN_VALUE(reg)         FIELD_VALUE(reg, BWTR_BUSTURN_MASK, 16)
#define BWTR_BUSTURN_MAX                15 /* N + 2 cycles */

#define BWTR_CLKDIV(value)              BIT_FIELD(value, 20)
#define BWTR_CLKDIV_MASK                BIT_FIELD(MASK(4), 20)
#define BWTR_CLKDIV_VALUE(reg)          FIELD_VALUE(reg, BWTR_CLKDIV_MASK, 20)
#define BWTR_CLKDIV_MAX                 15 /* N + 1 cycles */
#define BWTR_CLKDIV_MIN                 1

#define BWTR_DATLAT(value)              BIT_FIELD(value, 24)
#define BWTR_DATLAT_MASK                BIT_FIELD(MASK(4), 24)
#define BWTR_DATLAT_VALUE(reg)          FIELD_VALUE(reg, BWTR_DATLAT_MASK, 24)
#define BWTR_DATLAT_MAX                 15 /* N + 2 cycles */

#define BWTR_ACCMOD(value)              BIT_FIELD(value, 28)
#define BWTR_ACCMOD_MASK                BIT_FIELD(MASK(2), 28)
#define BWTR_ACCMOD_VALUE(reg)          FIELD_VALUE(reg, BWTR_ACCMOD_MASK, 28)

/* Access Mode configurations */
enum
{
  BWTR_ACCMOD_A = 0,
  BWTR_ACCMOD_B = 1,
  BWTR_ACCMOD_C = 2,
  BWTR_ACCMOD_D = 3
};
/*------------------NAND Flash/PC Card Control Registers----------------------*/
#define PCR_PWAITEN                     BIT(1)
#define PCR_PBKEN                       BIT(2)
#define PCR_PTYP                        BIT(3)

#define PCR_PWID(value)                 BIT_FIELD(value, 4)
#define PCR_PWID_MASK                   BIT_FIELD(MASK(2), 4)
#define PCR_PWID_VALUE(reg)             FIELD_VALUE(reg, PCR_PWID_MASK, 4)

#define PCR_ECCEN                       BIT(6)

#define PCR_TCLR(value)                 BIT_FIELD(value, 9)
#define PCR_TCLR_MASK                   BIT_FIELD(MASK(4), 9)
#define PCR_TCLR_VALUE(reg)             FIELD_VALUE(reg, PCR_TCLR_MASK, 9)

#define PCR_TAR(value)                  BIT_FIELD(value, 13)
#define PCR_TAR_MASK                    BIT_FIELD(MASK(4), 13)
#define PCR_TAR_VALUE(reg)              FIELD_VALUE(reg, PCR_TAR_MASK, 13)

#define PCR_ECCPS(value)                BIT_FIELD(value, 17)
#define PCR_ECCPS_MASK                  BIT_FIELD(MASK(3), 17)
#define PCR_ECCPS_VALUE(reg)            FIELD_VALUE(reg, PCR_ECCPS_MASK, 17)

/* ECC Page Size configurations */
enum 
{
  PCR_ECCPS_256  = 0,
  PCR_ECCPS_512  = 1,
  PCR_ECCPS_1024 = 2,
  PCR_ECCPS_2048 = 3
};
/*------------------NAND Flash/PC Card Status Registers-----------------------*/
#define SR_IRS                          BIT(0)
#define SR_ILS                          BIT(1)
#define SR_IFS                          BIT(2)
#define SR_IREN                         BIT(3)
#define SR_ILEN                         BIT(4)
#define SR_IFEN                         BIT(5)
#define SR_FEMPT                        BIT(6)
/*------------------Common Memory space timing registers----------------------*/
#define PMEM_MEMSET(value)              BIT_FIELD(value, 0)
#define PMEM_MEMSET_MASK                BIT_FIELD(MASK(8), 0)
#define PMEM_MEMSET_VALUE(reg)          FIELD_VALUE(reg, PMEM_MEMSET_MASK, 0)

#define PMEM_MEMWAIT(value)             BIT_FIELD(value, 8)
#define PMEM_MEMWAIT_MASK               BIT_FIELD(MASK(8), 8)
#define PMEM_MEMWAIT_VALUE(reg)         FIELD_VALUE(reg, PMEM_MEMWAIT_MASK, 8)

#define PMEM_MEMHOLD(value)             BIT_FIELD(value, 16)
#define PMEM_MEMHOLD_MASK               BIT_FIELD(MASK(8), 16)
#define PMEM_MEMHOLD_VALUE(reg)         FIELD_VALUE(reg, PMEM_MEMHOLD_MASK, 16)

#define PMEM_MEMHIZ(value)              BIT_FIELD(value, 24)
#define PMEM_MEMHIZ_MASK                BIT_FIELD(MASK(8), 24)
#define PMEM_MEMHIZ_VALUE(reg)          FIELD_VALUE(reg, PMEM_MEMHIZ_MASK, 24)
/*------------------Attribute memory space timing registers-------------------*/
#define PATT_ATTSET(value)              BIT_FIELD(value, 0)
#define PATT_ATTSET_MASK                BIT_FIELD(MASK(8), 0)
#define PATT_ATTSET_VALUE(reg)          FIELD_VALUE(reg, PATT_ATTSET_MASK, 0)

#define PATT_ATTWAIT(value)             BIT_FIELD(value, 8)
#define PATT_ATTWAIT_MASK               BIT_FIELD(MASK(8), 8)
#define PATT_ATTWAIT_VALUE(reg)         FIELD_VALUE(reg, PATT_ATTWAIT_MASK, 8)

#define PATT_ATTHOLD(value)             BIT_FIELD(value, 16)
#define PATT_ATTHOLD_MASK               BIT_FIELD(MASK(8), 16)
#define PATT_ATTHOLD_VALUE(reg)         FIELD_VALUE(reg, PATT_ATTHOLD_MASK, 16)

#define PATT_ATTHIZ(value)              BIT_FIELD(value, 24)
#define PATT_ATTHIZ_MASK                BIT_FIELD(MASK(8), 24)
#define PATT_ATTHIZ_VALUE(reg)          FIELD_VALUE(reg, PATT_ATTHIZ_MASK, 24)
/*------------------I/O space timing register 4-------------------------------*/
#define PIO4_IOSET(value)               BIT_FIELD(value, 0)
#define PIO4_IOSET_MASK                 BIT_FIELD(MASK(8), 0)
#define PIO4_IOSET_VALUE(reg)           FIELD_VALUE(reg, PIO4_IOSET_MASK, 0)

#define PIO4_IOHOLD(value)              BIT_FIELD(value, 8)
#define PIO4_IOHOLD_MASK                BIT_FIELD(MASK(8), 8)
#define PIO4_IOHOLD_VALUE(reg)          FIELD_VALUE(reg, PIO4_IOHOLD_MASK, 8)

#define PIO4_WAIT(value)                BIT_FIELD(value, 16)
#define PIO4_WAIT_MASK                  BIT_FIELD(MASK(8), 16)
#define PIO4_WAIT_VALUE(reg)            FIELD_VALUE(reg, PIO4_WAIT_MASK, 16)

#define PIO4_SET(value)                 BIT_FIELD(value, 24)
#define PIO4_SET_MASK                   BIT_FIELD(MASK(8), 24)
#define PIO4_SET_VALUE(reg)             FIELD_VALUE(reg, PIO4_SET_MASK, 24)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_FSMC_DEFS_H_ */
