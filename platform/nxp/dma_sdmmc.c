/*
 * dma_sdmmc.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
#include <halm/platform/nxp/dma_sdmmc.h>
#include <halm/platform/nxp/sdmmc_defs.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
static enum result appendItem(void *object, uintptr_t address, size_t size);
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *, const void *);
static void channelDeinit(void *);

static void channelCallback(void *, void (*)(void *), void *);
static void channelConfigure(void *, const void *);

static enum result channelEnable(void *);
static void channelDisable(void *);
static size_t channelPending(const void *);
static size_t channelResidue(const void *);
static enum result channelStatus(const void *);

static void channelAppend(void *, void *, const void *, size_t);
static void channelClear(void *);
/*----------------------------------------------------------------------------*/
static const struct DmaClass channelTable = {
    .size = sizeof(struct DmaSdmmc),
    .init = channelInit,
    .deinit = channelDeinit,

    .callback = channelCallback,
    .configure = channelConfigure,

    .enable = channelEnable,
    .disable = channelDisable,
    .pending = channelPending,
    .residue = channelResidue,
    .status = channelStatus,

    .append = channelAppend,
    .clear = channelClear
};
/*----------------------------------------------------------------------------*/
const struct DmaClass * const DmaSdmmc = &channelTable;
/*----------------------------------------------------------------------------*/
static enum result appendItem(void *object, uintptr_t address, size_t size)
{
  struct DmaSdmmc * const channel = object;
  struct DmaSdmmcEntry * const entry = channel->list + channel->length;
  uint32_t control = DESC_CONTROL_OWN | DESC_CONTROL_CH | DESC_CONTROL_LD;

  if (channel->length++)
  {
    struct DmaSdmmcEntry * const previous = entry - 1;

    /* Link current element to the previous one */
    previous->buffer2 = (uint32_t)entry;
    previous->control = (previous->control & ~DESC_CONTROL_LD)
        | DESC_CONTROL_DIC;
  }
  else
  {
    control |= DESC_CONTROL_FS;
  }

  entry->control = control;
  entry->size = DESC_SIZE_BS1(size);
  entry->buffer1 = address;
  entry->buffer2 = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *object, const void *configBase)
{
  const struct DmaSdmmcConfig * const config = configBase;
  assert(config);

  struct DmaSdmmc * const channel = object;

  assert(config->number);
  assert(config->burst <= DMA_BURST_256 && config->burst != DMA_BURST_2);

  /* Allocation should produce memory chunks aligned along 4-byte boundary */
  channel->list = memalign(4, sizeof(struct DmaSdmmcEntry) * config->number);
  if (!channel->list)
    return E_MEMORY;

  channel->capacity = config->number;
  channel->length = 0;
  channel->reg = config->parent->base.reg;

  LPC_SDMMC_Type * const reg = channel->reg;
  const enum dmaBurst burst = config->burst >= DMA_BURST_4 ?
      config->burst - 1 : config->burst;

  /* Control register is originally initialized in parent class */

  /* Reset DMA channel */
  reg->BMOD = BMOD_SWR;
  /* Reset DMA interface */
  reg->CTRL |= CTRL_DMA_RESET;
  while (reg->CTRL & CTRL_DMA_RESET);

  /* Use internal DMA */
  reg->CTRL |= CTRL_USE_INTERNAL_DMAC;
  /* Enable internal DMA */
  reg->BMOD = BMOD_DE | BMOD_DSL(4);
  /* Disable all DMA interrupts */
  reg->IDINTEN = 0;

  reg->FIFOTH = FIFOTH_DMA_MTS(burst) | FIFOTH_TX_WMARK(FIFO_SIZE / 2)
      | FIFOTH_RX_WMARK(FIFO_SIZE / 2 - 1);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  struct DmaSdmmc * const channel = object;
  LPC_SDMMC_Type * const reg = channel->reg;

  reg->CTRL &= ~CTRL_USE_INTERNAL_DMAC;
  reg->BMOD = 0;

  free(channel->list);
}
/*----------------------------------------------------------------------------*/
static void channelCallback(void *object __attribute__((unused)),
    void (*callback)(void *) __attribute__((unused)),
    void *argument __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static void channelConfigure(void *object __attribute__((unused)),
    const void *settingsBase __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static enum result channelEnable(void *object)
{
  struct DmaSdmmc * const channel = object;
  LPC_SDMMC_Type * const reg = channel->reg;

  /* Clear status flags */
  reg->IDSTS = IDINTEN_TI | IDINTEN_RI | IDINTEN_FBE | IDINTEN_DU | IDINTEN_CES;
  /* Set DMA descriptor base address */
  reg->DBADDR = (uint32_t)channel->list;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDisable(void *object __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static size_t channelPending(const void *object)
{
  const struct DmaSdmmc * const channel = object;

  return channel->length;
}
/*----------------------------------------------------------------------------*/
static size_t channelResidue(const void *object)
{
  const struct DmaSdmmc * const channel = object;
  const LPC_SDMMC_Type * const reg = channel->reg;
  const struct DmaSdmmcEntry * const current =
      (const struct DmaSdmmcEntry *)reg->DSCADDR;

  if (!current)
    return 0;

  return channel->length - (current - channel->list);
}
/*----------------------------------------------------------------------------*/
static enum result channelStatus(const void *object)
{
  const struct DmaSdmmc * const channel = object;
  const LPC_SDMMC_Type * const reg = channel->reg;
  const uint32_t status = reg->IDINTEN;

  if (status & IDINTEN_AIS)
    return E_INTERFACE;
  else if (status & IDINTEN_NIS)
    return E_OK;
  else
    return E_BUSY;
}
/*----------------------------------------------------------------------------*/
static void channelAppend(void *object, void *destination, const void *source,
    size_t size)
{
  struct DmaSdmmc * const channel = object;
  LPC_SDMMC_Type * const reg = channel->reg;

  assert(size);
  assert(size <= channel->capacity * DESC_SIZE_MAX);
  assert(!destination ^ !source);

  const uintptr_t address = destination != 0 ?
      (uintptr_t)destination : (uintptr_t)source;

  /* Address and size must be aligned along 4-byte boundary */
  assert(!(address & 0x03));
  assert(!(size & 0x03));

  /* Reset DMA */
  reg->CTRL |= CTRL_DMA_RESET | CTRL_FIFO_RESET;
  while (reg->CTRL & (CTRL_DMA_RESET | CTRL_FIFO_RESET));

  size_t offset = 0;

  channel->length = 0;
  while (offset < size)
  {
    const size_t chunkLength = (size - offset >= DESC_SIZE_MAX) ?
        DESC_SIZE_MAX : (size - offset);

    appendItem(channel, address + offset, chunkLength);
    offset += chunkLength;
  }
}
/*----------------------------------------------------------------------------*/
static void channelClear(void *object __attribute__((unused)))
{

}
