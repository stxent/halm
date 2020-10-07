/*
 * halm/core/cortex/armv7m/core_defs.h
 * Based on original from ARM Limited
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_CORE_CORTEX_ARMV7M_CORE_DEFS_H_
#define HALM_CORE_CORTEX_ARMV7M_CORE_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*------------------System Control Block--------------------------------------*/
typedef struct
{
  __ne__ uint32_t RESERVED0;
  __ro__ uint32_t ICTR; /* Interrupt Controller Type Register */
  __rw__ uint32_t ACTLR; /* Auxiliary Control Register */
  __ne__ uint32_t RESERVED1[829];
  __ro__ uint32_t CPUID; /* CPU ID Base Register */
  __rw__ uint32_t ICSR; /* Interrupt Control State Register */
  __rw__ uint32_t VTOR; /* Vector Table Offset Register */
  __rw__ uint32_t AIRCR; /* Application Interrupt and Reset Control Register */
  __rw__ uint32_t SCR; /* System Control Register */
  __rw__ uint32_t CCR; /* Configuration Control Register */
  __rw__ uint8_t SHP[12]; /* System Handlers Priority Registers */
  __rw__ uint32_t SHCSR; /* System Handler Control and State Register */
  __rw__ uint32_t CFSR; /* Configurable Fault Status Register */
  __rw__ uint32_t HFSR; /* Hard Fault Status Register */
  __rw__ uint32_t DFSR; /* Debug Fault Status Register */
  __rw__ uint32_t MMFAR; /* Memory Manage Address Register */
  __rw__ uint32_t BFAR; /* Bus Fault Address Register */
  __rw__ uint32_t AFSR; /* Auxiliary Fault Status Register */
  __ro__ uint32_t PFR[2]; /* Processor Feature Register */
  __ro__ uint32_t DFR; /* Debug Feature Register */
  __ro__ uint32_t ADR; /* Auxiliary Feature Register */
  __ro__ uint32_t MMFR[4]; /* Memory Model Feature Register */
  __ro__ uint32_t ISAR[5]; /* ISA Feature Register */
  __ne__ uint32_t RESERVED2[5];
  __rw__ uint32_t CPACR; /* Coprocessor Access Control Register */
  __ne__ uint32_t RESERVED3[93];
  __wo__ uint32_t STIR; /* Software Trigger Interrupt Register */
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
  __ro__ uint32_t MVFR0; /* Media and FP Feature Register 0 */
  __ro__ uint32_t MVFR1; /* Media and FP Feature Register 1 */
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
#define SCB       (&PPB_DOMAIN.SCB)
#define SYSTICK   (&PPB_DOMAIN.SYSTICK)
#define NVIC      (&PPB_DOMAIN.NVIC)
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_ARMV7M_CORE_DEFS_H_ */
