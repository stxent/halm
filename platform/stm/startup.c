/*
 * startup.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

extern unsigned long _sdata;
extern unsigned long _edata;
extern unsigned long _sidata;
extern unsigned long _sbss;
extern unsigned long _ebss;
/*----------------------------------------------------------------------------*/
extern int main(void);
extern void platformStartup(void);
/*----------------------------------------------------------------------------*/
void RESET_ISR(void)
{
  register unsigned long *dst __asm__ ("r0");
  register unsigned long *src __asm__ ("r1");

  /* Copy the data segment initializers from flash to RAM */
  for (dst = &_sdata, src = &_sidata; dst < &_edata;)
    *dst++ = *src++;

  /* Zero fill the BSS segment */
  for (dst = &_sbss; dst < &_ebss;)
    *dst++ = 0;

  platformStartup();
  main();

  while (1);
}
