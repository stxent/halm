/*
 * sct_common.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/sct_base.h>
#include <halm/platform/nxp/sct_defs.h>
/*----------------------------------------------------------------------------*/
void sctSetFrequency(struct SctBase *timer, uint32_t frequency)
{
  const unsigned int part = timer->part == SCT_HIGH;
  LPC_SCT_Type * const reg = timer->reg;

  /* Counter reset is recommended by the user manual */
  const uint32_t value = (reg->CTRL_PART[part] & ~CTRL_PRE_MASK) | CTRL_CLRCTR;

  if (frequency)
  {
    /* TODO Check whether the clock is from internal source */
    const uint32_t apbClock = sctGetClock(timer);
    const uint16_t prescaler = apbClock / frequency - 1;

    assert(prescaler < 256);

    reg->CTRL_PART[part] = value | CTRL_PRE(prescaler);
  }
  else
    reg->CTRL_PART[part] = 0;
}
