/*
 * startup.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/system.h>
/*----------------------------------------------------------------------------*/
void platformStartup(void)
{
  sysResetBlock(RST_GPIO);
}
