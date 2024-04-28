/*
 * halm/core/cortex/armv7m/core_defs.h
 * Based on original from ARM Limited
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_CORTEX_CORE_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_CORE_CORTEX_ARMV7M_CORE_DEFS_H_
#define HALM_CORE_CORTEX_ARMV7M_CORE_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*------------------System Control Block--------------------------------------*/
typedef struct
{
  /* Offset 0xE000 */
  __ne__ uint32_t RESERVED0;
  __ro__ uint32_t ICTR; /* Interrupt Controller Type Register */
  __rw__ uint32_t ACTLR; /* Auxiliary Control Register */
  __ne__ uint32_t RESERVED1[829];

  /* Offset 0xED00 */
  __ro__ uint32_t CPUID; /* CPU ID Base Register */
  __rw__ uint32_t ICSR; /* Interrupt Control State Register */
  __rw__ uint32_t VTOR; /* Vector Table Offset Register */
  __rw__ uint32_t AIRCR; /* Application Interrupt and Reset Control Register */
  __rw__ uint32_t SCR; /* System Control Register */
  __rw__ uint32_t CCR; /* Configuration Control Register */

  union
  {
    struct
    {
      __rw__ uint32_t SHPR1; /* System Handler Priority Register 1 */
      __rw__ uint32_t SHPR2; /* System Handler Priority Register 2 */
      __rw__ uint32_t SHPR3; /* System Handler Priority Register 3 */
    };

    __rw__ uint8_t SHPR[12];
  };

  /* Offset 0xED24 */
  __rw__ uint32_t SHCSR; /* System Handler Control and State Register */
  __rw__ uint32_t CFSR; /* Configurable Fault Status Register */
  __rw__ uint32_t HFSR; /* Hard Fault Status Register */
  __rw__ uint32_t DFSR; /* Debug Fault Status Register */
  __rw__ uint32_t MMFAR; /* Memory Manage Address Register */
  __rw__ uint32_t BFAR; /* Bus Fault Address Register */
  __rw__ uint32_t AFSR; /* Auxiliary Fault Status Register */

  /* Offset 0xED40 */
  union
  {
    struct
    {
      __ro__ uint32_t PFR0; /* Processor Feature Register 0 */
      __ro__ uint32_t PFR1; /* Processor Feature Register 1 */
    };

    __ro__ uint32_t PFR[2];
  };

  /* Offset 0xED48 */
  __ro__ uint32_t DFR0; /* Debug Feature Register 0 */
  __ro__ uint32_t AFR0; /* Auxiliary Feature Register 0 */

  /* Offset 0xED50 */
  union
  {
    struct
    {
      __ro__ uint32_t MMFR0; /* Memory Model Feature Register 0 */
      __ro__ uint32_t MMFR1; /* Memory Model Feature Register 1 */
      __ro__ uint32_t MMFR2; /* Memory Model Feature Register 2 */
      __ro__ uint32_t MMFR3; /* Memory Model Feature Register 3 */
    };

    __ro__ uint32_t MMFR[4];
  };

  /* Offset 0xED60 */
  union
  {
    struct
    {
      __ro__ uint32_t ISAR0; /* Instruction Set Attributes Register 0 */
      __ro__ uint32_t ISAR1; /* Instruction Set Attributes Register 1 */
      __ro__ uint32_t ISAR2; /* Instruction Set Attributes Register 2 */
      __ro__ uint32_t ISAR3; /* Instruction Set Attributes Register 3 */
      __ro__ uint32_t ISAR4; /* Instruction Set Attributes Register 4 */
    };

    __ro__ uint32_t ISAR[5];
  };
  __ne__ uint32_t RESERVED2;

  /* Offset 0xED78: Cortex-M7 */
  __ro__ uint32_t CLIDR; /* Cache Level ID Register */
  __ro__ uint32_t CTR; /* Cache Type Register */
  __ro__ uint32_t CCSIDR; /* Cache Size ID Register */
  __rw__ uint32_t CSSELR; /* Cache Size Selection Register */

  /* Offset 0xED88: Cortex-M4 */
  __rw__ uint32_t CPACR; /* Coprocessor Access Control Register */
  __ne__ uint32_t RESERVED3[93];

  /* Offset 0xEF00 */
  __wo__ uint32_t STIR; /* Software Trigger Interrupt Register */
  __ne__ uint32_t RESERVED4[19];

  /* Offset 0xEF50: Cortex-M7 */
  __wo__ uint32_t ICIALLU; /* Instruction Cache Invalidate All */
  __ne__ uint32_t RESERVED5;
  __wo__ uint32_t ICIMVAU; /* Instruction Cache Invalidate by Address */
  __wo__ uint32_t DCIMVAC; /* Data Cache Invalidate by Address */
  __wo__ uint32_t DCISW; /* Data Cache Invalidate by Set/Way */
  __wo__ uint32_t DCCMVAU; /* Data Cache Invalidate by Address */
  __wo__ uint32_t DCCMVAC; /* Data Cache Clean by Address */
  __wo__ uint32_t DCCSW; /* Data Cache Clean by Set/Way */
  __wo__ uint32_t DCCIMVAC; /* Data Cache Clean and Invalidate by Address */
  __wo__ uint32_t DCCISW; /* Data Cache Clean and Invalidate by Set/Way */
  __ne__ uint32_t BPIALL;
  __ne__ uint32_t RESERVED6[5];

  /* Offset 0xEF90: Cortex-M7 */
  __rw__ uint32_t ITCMCR; /* Instruction TCM Control Register */
  __rw__ uint32_t DTCMCR; /* Data TCM Control Register */
  __rw__ uint32_t AHBPCR; /* AHBP Control Register */
  __rw__ uint32_t CACR; /* L1 Cache Control Register */
  __rw__ uint32_t AHBSCR; /* AHB Slave Control Register */
  __ne__ uint32_t RESERVED7;
  __rw__ uint32_t ABFSR; /* Auxiliary Bus Fault Status Register */
  __ne__ uint32_t RESERVED8;

  /* Offset 0xEFB0: Cortex-M7 */
  union
  {
    struct
    {
      __rw__ uint32_t IEBR0; /* Instruction Error Bank Register 0 */
      __rw__ uint32_t IEBR1; /* Instruction Error Bank Register 1 */
    };

    __rw__ uint32_t IEBR[2];
  };

  /* Offset 0xEFB8: Cortex-M7 */
  union
  {
    struct
    {
      __rw__ uint32_t DEBR0; /* Data Error Bank Register 0 */
      __rw__ uint32_t DEBR1; /* Data Error Bank Register 1 */
    };

    __rw__ uint32_t DEBR[2];
  };
} SCB_Type;
/*------------------System Tick Timer-----------------------------------------*/
typedef struct
{
  __rw__ uint32_t CTRL; /* SysTick Control and Status Register */
  __rw__ uint32_t LOAD; /* SysTick Reload Value Register */
  __rw__ uint32_t VAL; /* SysTick Current Value Register */
  __ro__ uint32_t CALIB; /* SysTick Calibration Register */
} SYSTICK_Type;
/*------------------Power Management Unit-------------------------------------*/
typedef struct
{
  __rw__ uint32_t ISER[8]; /* Interrupt Set Enable Register */
  __ne__ uint32_t RESERVED0[24];
  __rw__ uint32_t ICER[8]; /* Interrupt Clear Enable Register */
  __ne__ uint32_t RESERVED1[24];
  __rw__ uint32_t ISPR[8]; /* Interrupt Set Pending Register */
  __ne__ uint32_t RESERVED2[24];
  __rw__ uint32_t ICPR[8]; /* Interrupt Clear Pending Register */
  __ne__ uint32_t RESERVED3[24];
  __rw__ uint32_t IABR[8]; /* Interrupt Active bit Register */
  __ne__ uint32_t RESERVED4[56];
  __rw__ uint8_t IPR[240]; /* Interrupt Priority Registers */
} NVIC_Type;
/*----------------------------------------------------------------------------*/
typedef struct
{
  __ro__ uint32_t TYPE;
  __rw__ uint32_t CTRL;
  __rw__ uint32_t RNR;
  __rw__ uint32_t RBAR;
  __rw__ uint32_t RASR;
  __rw__ uint32_t RBAR_A1;
  __rw__ uint32_t RASR_A1;
  __rw__ uint32_t RBAR_A2;
  __rw__ uint32_t RASR_A2;
  __rw__ uint32_t RBAR_A3;
  __rw__ uint32_t RASR_A3;
} MPU_Type;
/*----------------------------------------------------------------------------*/
typedef struct
{
  __rw__ uint32_t FPCCR; /* Floating-Point Context Control Register */
  __rw__ uint32_t FPCAR; /* Floating-Point Context Address Register */
  __rw__ uint32_t FPDSCR; /* Floating-Point Default Status Control Register */

  union
  {
    struct
    {
      __ro__ uint32_t MVFR0; /* Media and FP Feature Register 0 */
      __ro__ uint32_t MVFR1; /* Media and FP Feature Register 1 */
      __ro__ uint32_t MVFR2; /* Media and FP Feature Register 2 */
    };

    __ro__ uint32_t MVFR[3];
  };
} FPU_Type;
/*----------------------------------------------------------------------------*/
typedef struct
{
  union
  {
    struct
    {
      __ne__ uint8_t RESERVED0[0xE010];
      SYSTICK_Type SYSTICK;
      __ne__ uint8_t RESERVED1[0xF0 - sizeof(SYSTICK_Type)];
      NVIC_Type NVIC;
      __ne__ uint8_t RESERVED2[0xC90 - sizeof(NVIC_Type)];
      MPU_Type MPU;
      __ne__ uint8_t RESERVED3[0x1A4 - sizeof(MPU_Type)];
      FPU_Type FPU;
    };

    struct
    {
      __ne__ uint8_t RESERVED4[0xE000];
      SCB_Type SCB;
    };
  };
} PPB_DOMAIN_Type;
/*----------------------------------------------------------------------------*/
extern PPB_DOMAIN_Type PPB_DOMAIN;
/*----------------------------------------------------------------------------*/
#define SYSTICK   (&PPB_DOMAIN.SYSTICK)
#define NVIC      (&PPB_DOMAIN.NVIC)
#define MPU       (&PPB_DOMAIN.MPU)
#define SCB       (&PPB_DOMAIN.SCB)
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_ARMV7M_CORE_DEFS_H_ */
