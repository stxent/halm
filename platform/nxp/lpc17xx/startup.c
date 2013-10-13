/*
 * startup.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "platform/nxp/device_defs.h"
/*----------------------------------------------------------------------------*/
extern unsigned long _etext;
extern unsigned long _data;
extern unsigned long _edata;
extern unsigned long _bss;
extern unsigned long _ebss;
/*----------------------------------------------------------------------------*/
#ifdef __REDLIB__
extern int __main(void); /* Redlib */
#else
extern int main(void); /* Newlib */
#endif
/*----------------------------------------------------------------------------*/
void RESET_ISR(void)
{
  unsigned long *pulDest, *pulSrc = &_etext;

  /* Copy the data segment initializers from flash to RAM */
  for (pulDest = &_data; pulDest < &_edata;)
    *pulDest++ = *pulSrc++;

  /* Zero fill the BSS segment */
  __asm__(
      "    LDR     r0, =_bss\n"
      "    LDR     r1, =_ebss\n"
      "    MOV     r2, #0\n"
      "    .thumb_func\n"
      "zero_loop:\n"
      "    CMP     r0, r1\n"
      "    IT      lt\n"
      "    STRLT   r2, [r0], #4\n"
      "    BLT     zero_loop"
  );

#if defined(__cplusplus)
  /* Call C++ library initialization */
  __libc_init_array();
#endif

#ifdef __REDLIB__
  __main();
#else
  main();
#endif

  while (1);
}
