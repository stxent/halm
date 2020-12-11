/*
 * rit_base.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/rit_base.h>
/*----------------------------------------------------------------------------*/
uint32_t ritGetClock(void)
{
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
void ritBaseInit(void)
{
}
/*----------------------------------------------------------------------------*/
void ritBaseDeinit(void)
{
}
