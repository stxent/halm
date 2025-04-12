/*
 * sct_common.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/sct_base.h>
#include <halm/platform/lpc/sct_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
void sctSetFrequency(struct SctBase *timer, uint32_t frequency)
{
  const unsigned int part = timer->part == SCT_HIGH;
  LPC_SCT_Type * const reg = timer->reg;

  /* Counter reset is recommended by the user manual */
  const uint16_t ctrl = (reg->CTRL_PART[part] & ~CTRL_PRE_MASK) | CTRL_CLRCTR;

  if (frequency)
  {
    const uint32_t apbClock = sctGetClock(timer);
    const uint16_t prescaler = apbClock / frequency - 1;

    assert(prescaler < 256);

    reg->CTRL_PART[part] = ctrl | CTRL_PRE(prescaler);
  }
  else
    reg->CTRL_PART[part] = ctrl;
}
