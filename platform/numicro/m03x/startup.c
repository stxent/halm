/*
 * startup.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/system.h>
// #include <stddef.h>
/*----------------------------------------------------------------------------*/
void platformStartup(void)
{
  // TODO
  // static const enum SysResetBlock blocksToReset[] = {
  //     RST_GPIO
  // };

  // for (size_t index = 0; index < ARRAY_SIZE(blocksToReset); ++index)
  //   sysResetBlock(blocksToReset[index]);

  sysResetBlock(RST_GPIO);
}
