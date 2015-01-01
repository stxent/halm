/*
 * platform/nxp/platform_defs.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_PLATFORM_DEFS_H_
#define PLATFORM_NXP_PLATFORM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <libhalm/target.h>
/*----------------------------------------------------------------------------*/
/* No effect or reserved registers */
#define __ne__ __attribute__((deprecated))
/* Registers with read and write access types */
#define __rw__ volatile
/* Read-only registers */
#define __ro__ const volatile
/* Write-only registers */
#define __wo__ volatile
/*------------------Analog-to-Digital Converter-------------------------------*/
typedef struct
{
  __rw__ uint32_t CR;
  __rw__ uint32_t GDR;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t INTEN;
  union
  {
    __ro__ uint32_t DR[8];
    struct
    {
      __ro__ uint32_t DR0;
      __ro__ uint32_t DR1;
      __ro__ uint32_t DR2;
      __ro__ uint32_t DR3;
      __ro__ uint32_t DR4;
      __ro__ uint32_t DR5;
      __ro__ uint32_t DR6;
      __ro__ uint32_t DR7;
    };
  };
  __ro__ uint32_t STAT;
  __rw__ uint32_t TRIM; /* May be unavailable on some parts */
} LPC_ADC_Type;
/*------------------Digital-to-Analog Converter-------------------------------*/
typedef struct
{
  __rw__ uint32_t CR; /* Converter Register */
  __rw__ uint32_t CTRL; /* Control register */
  __rw__ uint16_t CNTVAL; /* Counter Value register */
} LPC_DAC_Type;
/*------------------Inter-Integrated Circuit----------------------------------*/
typedef struct
{
  __rw__ uint32_t CONSET;
  __ro__ uint32_t STAT;
  __rw__ uint32_t DAT;
  __rw__ uint32_t ADR0;
  __rw__ uint32_t SCLH;
  __rw__ uint32_t SCLL;
  __rw__ uint32_t CONCLR;
  __rw__ uint32_t MMCTRL;
  __rw__ uint32_t ADR1;
  __rw__ uint32_t ADR2;
  __rw__ uint32_t ADR3;
  __ro__ uint32_t DATA_BUFFER;
  __rw__ uint32_t MASK0;
  __rw__ uint32_t MASK1;
  __rw__ uint32_t MASK2;
  __rw__ uint32_t MASK3;
} LPC_I2C_Type;
/*------------------Synchronous Serial Port-----------------------------------*/
typedef struct
{
  __rw__ uint32_t CR0;
  __rw__ uint32_t CR1;
  __rw__ uint32_t DR;
  __ro__ uint32_t SR;
  __rw__ uint32_t CPSR;
  __rw__ uint32_t IMSC;
  __rw__ uint32_t RIS;
  __rw__ uint32_t MIS;
  __wo__ uint32_t ICR;
  __rw__ uint32_t DMACR; /* Unavailable on parts without DMA controller */
} LPC_SSP_Type;
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/PLATFORM/platform_defs.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
#undef __wo__
#undef __ro__
#undef __rw__
#undef __ne__
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_PLATFORM_DEFS_H_ */
