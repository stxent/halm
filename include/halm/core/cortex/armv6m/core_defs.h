/*
 * halm/core/cortex/armv6m/core_defs.h
 * Based on original from ARM Limited
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_CORTEX_CORE_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_CORE_CORTEX_ARMV6M_CORE_DEFS_H_
#define HALM_CORE_CORTEX_ARMV6M_CORE_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*------------------System Control Block--------------------------------------*/
typedef struct
{
  /* Offset 0xED00 */
  __ro__ uint32_t CPUID; /* CPU ID Base Register */
  __rw__ uint32_t ICSR; /* Interrupt Control State Register */

  /* Offset 0xED08: Cortex-M0+ */
  __ne__ uint32_t VTOR; /* Vector Table Offset Register */

  /* Offset 0xED0C */
  __rw__ uint32_t AIRCR; /* Application Interrupt and Reset Control Register */
  __rw__ uint32_t SCR; /* System Control Register */
  __rw__ uint32_t CCR; /* Configuration Control Register */
  __ne__ uint32_t RESERVED1;

  union
  {
    struct
    {
      __rw__ uint32_t SHPR2; /* System Handler Priority Register 2 */
      __rw__ uint32_t SHPR3; /* System Handler Priority Register 3 */
    };

    __rw__ uint32_t SHPR[8];
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
  __rw__ uint32_t ISER; /* Interrupt Set Enable Register */
  __ne__ uint32_t RESERVED0[31];
  __rw__ uint32_t ICER; /* Interrupt Clear Enable Register */
  __ne__ uint32_t RESERVED1[31];
  __rw__ uint32_t ISPR; /* Interrupt Set Pending Register */
  __ne__ uint32_t RESERVED2[31];
  __rw__ uint32_t ICPR; /* Interrupt Clear Pending Register */
  __ne__ uint32_t RESERVED3[95];
  __rw__ uint32_t IPR[8]; /* Interrupt Priority Registers */
} NVIC_Type;
/*----------------------------------------------------------------------------*/
typedef struct
{
  __ne__ uint8_t RESERVED0[0xE010];
  SYSTICK_Type SYSTICK;
  __ne__ uint8_t RESERVED1[0xF0 - sizeof(SYSTICK_Type)];
  NVIC_Type NVIC;
  __ne__ uint8_t RESERVED2[0xC00 - sizeof(NVIC_Type)];
  SCB_Type SCB;
} PPB_DOMAIN_Type;
/*----------------------------------------------------------------------------*/
extern PPB_DOMAIN_Type PPB_DOMAIN;
/*----------------------------------------------------------------------------*/
#define SYSTICK   (&PPB_DOMAIN.SYSTICK)
#define NVIC      (&PPB_DOMAIN.NVIC)
#define SCB       (&PPB_DOMAIN.SCB)
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_ARMV6M_CORE_DEFS_H_ */
