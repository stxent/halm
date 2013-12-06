/*
 * platform/nxp/platform_defs.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_DEFS_TOP_H_
#define PLATFORM_DEFS_TOP_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <mcu.h>
/*----------------------------------------------------------------------------*/
#define __rw__  volatile
#define __r__   const volatile
#define __w__   volatile
/*------------------Timer/Counter---------------------------------------------*/
typedef struct
{
  __rw__ uint32_t IR;
  __rw__ uint32_t TCR;
  __rw__ uint32_t TC;
  __rw__ uint32_t PR;
  __rw__ uint32_t PC;
  __rw__ uint32_t MCR;
  union
  {
    __rw__ uint32_t MR[4];
    struct
    {
      __rw__ uint32_t MR0;
      __rw__ uint32_t MR1;
      __rw__ uint32_t MR2;
      __rw__ uint32_t MR3;
    };
  };
  __rw__ uint32_t CCR;
  union
  {
    __r__  uint32_t CR[4];
    struct
    {
      __r__  uint32_t CR0;
      __r__  uint32_t CR1;
      __r__  uint32_t CR2;
      __r__  uint32_t CR3;
    };
  };
  __rw__ uint32_t EMR;
         uint32_t RESERVED2[12];
  __rw__ uint32_t CTCR;
  __rw__ uint32_t PWMC; /* Chip-specific register */
} LPC_TIMER_Type;
/*------------------Universal Asynchronous Receiver Transmitter---------------*/
typedef struct
{
  union {
    __r__  uint32_t RBR;
    __w__  uint32_t THR;
    __rw__ uint32_t DLL;
  };
  union {
    __rw__ uint32_t DLM;
    __rw__ uint32_t IER;
  };
  union {
    __r__  uint32_t IIR;
    __w__  uint32_t FCR;
  };
  __rw__ uint32_t LCR;
         uint32_t RESERVED0;
  __r__  uint32_t LSR;
         uint32_t RESERVED1;
  __rw__ uint32_t SCR;
  __rw__ uint32_t ACR;
  __rw__ uint32_t ICR;
  __rw__ uint32_t FDR;
         uint32_t RESERVED2;
  __rw__ uint32_t TER;
} LPC_UART_Type;
/*------------------Extended Universal Asynchronous Receiver Transmitter------*/
/* UART block with modem control, RS485 support and IrDA mode */
typedef struct
{
  union
  {
    __r__  uint32_t RBR;
    __w__  uint32_t THR;
    __rw__ uint32_t DLL;
  };
  union
  {
    __rw__ uint32_t DLM;
    __rw__ uint32_t IER;
  };
  union
  {
    __r__  uint32_t IIR;
    __w__  uint32_t FCR;
  };
  __rw__ uint32_t LCR;
  __rw__ uint32_t MCR;
  __r__  uint32_t LSR;
  __r__  uint32_t MSR;
  __rw__ uint32_t SCR;
  __rw__ uint32_t ACR;
  __rw__ uint32_t ICR;
  __rw__ uint32_t FDR;
         uint32_t RESERVED0;
  __rw__ uint32_t TER;
         uint32_t RESERVED1[6];
  __rw__ uint32_t RS485CTRL;
  __rw__ uint32_t ADRMATCH;
  __rw__ uint32_t RS485DLY;
} LPC_UART_MODEM_Type;
/*------------------Synchronous Serial Port-----------------------------------*/
typedef struct
{
  __rw__ uint32_t CR0;
  __rw__ uint32_t CR1;
  __rw__ uint32_t DR;
  __r__  uint32_t SR;
  __rw__ uint32_t CPSR;
  __rw__ uint32_t IMSC;
  __rw__ uint32_t RIS;
  __rw__ uint32_t MIS;
  __w__  uint32_t ICR;
  __rw__ uint32_t DMACR; /* May not be available on some parts */
} LPC_SSP_Type;
/*------------------Inter-Integrated Circuit----------------------------------*/
typedef struct
{
  __rw__ uint32_t CONSET;
  __r__  uint32_t STAT;
  __rw__ uint32_t DAT;
  __rw__ uint32_t ADR0;
  __rw__ uint32_t SCLH;
  __rw__ uint32_t SCLL;
  __rw__ uint32_t CONCLR;
  __rw__ uint32_t MMCTRL;
  __rw__ uint32_t ADR1;
  __rw__ uint32_t ADR2;
  __rw__ uint32_t ADR3;
  __r__  uint32_t DATA_BUFFER;
  __rw__ uint32_t MASK0;
  __rw__ uint32_t MASK1;
  __rw__ uint32_t MASK2;
  __rw__ uint32_t MASK3;
} LPC_I2C_Type;
/*------------------Analog-to-Digital Converter-------------------------------*/
typedef struct
{
  __rw__ uint32_t CR;
  __rw__ uint32_t GDR;
         uint32_t RESERVED0;
  __rw__ uint32_t INTEN;
  union
  {
    __rw__ uint32_t DR[8];
    struct
    {
      __rw__ uint32_t DR0;
      __rw__ uint32_t DR1;
      __rw__ uint32_t DR2;
      __rw__ uint32_t DR3;
      __rw__ uint32_t DR4;
      __rw__ uint32_t DR5;
      __rw__ uint32_t DR6;
      __rw__ uint32_t DR7;
    };
  };
  __r__  uint32_t STAT;
  __rw__ uint32_t TRIM;
} LPC_ADC_Type;
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/PLATFORM/platform_defs.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
#undef __w__
#undef __r__
#undef __rw__
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_DEFS_TOP_H_ */
