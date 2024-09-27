/*
 * edma_common.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/imxrt/edma_base.h>
#include <halm/platform/imxrt/edma_defs.h>
#include <halm/platform/platform_defs.h>
#include <xcore/asm.h>
/*----------------------------------------------------------------------------*/
void edmaResetMux(struct EdmaBase *channel)
{
  ((IMX_DMAMUX_Type *)channel->mux)->CHCFG[channel->number] = 0;
}
/*----------------------------------------------------------------------------*/
void edmaSetMux(struct EdmaBase *channel)
{
  IMX_DMAMUX_Type * const mux = channel->mux;
  uint32_t value = CHCFG_ENBL;

  if (channel->event != EDMA_MEMORY)
    value |= CHCFG_SOURCE(channel->event);
  else
    value |= CHCFG_A_ON;

  mux->CHCFG[channel->number] = value;
}
/*----------------------------------------------------------------------------*/
void edmaStartTransfer(struct EdmaBase *channel)
{
  IMX_EDMA_Type * const reg = channel->reg;
  const uint8_t number = channel->number;

  /* Clear pending flags */
  reg->CDNE = CDNE_CDNE(number);
  reg->CERR = CERR_CERR(number);
  reg->CINT = CINT_CINT(number);

  /* Enable error interrupt */
  reg->SEEI = SEEI_SEEI(number);

  __dmb();

  /* Enable request processing */
  reg->SERQ = SERQ_SERQ(number);
}
