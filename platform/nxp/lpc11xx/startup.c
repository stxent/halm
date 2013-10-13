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
  register unsigned char *src, *dst;

  /* Copy the data segment initializers from flash to RAM */
  src = &_etext;
  dst = &_data;
  while (dst < &_edata)
    *dst++ = *src++;

  /* Zero fill the BSS segment */
  dst = &_bss;
  while (dst < &_ebss)
    *dst++ = 0;

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
