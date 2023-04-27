/*
 * dma_sdmmc.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/dma_sdmmc.h>
#include <halm/platform/lpc/sdmmc_defs.h>
#include <halm/platform/platform_defs.h>
#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
/*----------------------------------------------------------------------------*/
static enum Result appendItem(void *object, uintptr_t address, size_t size);
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *, const void *);
static void channelDeinit(void *);

static enum Result channelEnable(void *);
static enum Result channelResidue(const void *, size_t *);
static enum Result channelStatus(const void *);

static void channelAppend(void *, void *, const void *, size_t);
static size_t channelQueued(const void *);
/*----------------------------------------------------------------------------*/
const struct DmaClass * const DmaSdmmc = &(const struct DmaClass){
    .size = sizeof(struct DmaSdmmc),
    .init = channelInit,
    .deinit = channelDeinit,

    .configure = NULL,
    .setCallback = NULL,

    .enable = channelEnable,
    .disable = NULL,
    .residue = channelResidue,
    .status = channelStatus,

    .append = channelAppend,
    .clear = NULL,
    .queued = channelQueued
};
/*----------------------------------------------------------------------------*/
static enum Result appendItem(void *object, uintptr_t address, size_t size)
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
static enum Result channelInit(void *object, const void *configBase)
{
  const struct DmaSdmmcConfig * const config = configBase;
  assert(config != NULL);
  assert(config->number);
  assert(config->burst <= DMA_BURST_256);

  struct DmaSdmmc * const channel = object;

  /* Memory chunks should be aligned along 4-byte boundary */
  channel->list = memalign(4, sizeof(struct DmaSdmmcEntry) * config->number);
  if (channel->list == NULL)
    return E_MEMORY;

  channel->capacity = config->number;
  channel->length = 0;
  channel->reg = config->parent->base.reg;

  LPC_SDMMC_Type * const reg = channel->reg;

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

  reg->FIFOTH = FIFOTH_DMA_MTS(config->burst)
      | FIFOTH_TX_WMARK(FIFO_SIZE / 2)
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
static enum Result channelEnable(void *object)
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
static enum Result channelResidue(const void *object, size_t *count)
{
  const struct DmaSdmmc * const channel = object;
  const LPC_SDMMC_Type * const reg = channel->reg;
  const struct DmaSdmmcEntry * const current =
      (const struct DmaSdmmcEntry *)reg->DSCADDR;

  if (current != NULL)
  {
    *count = channel->length - (current - channel->list);
    return E_OK;
  }
  else
    return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum Result channelStatus(const void *object)
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

  assert(destination != NULL || source != NULL);
  assert(destination == NULL || source == NULL);
  assert(size);
  assert(size <= channel->capacity * DESC_SIZE_MAX);

  const uintptr_t address = destination != NULL ?
      (uintptr_t)destination : (uintptr_t)source;

  /* Address and size must be aligned along 4-byte boundary */
  assert(address % sizeof(uint32_t) == 0);
  assert(size % sizeof(uint32_t) == 0);

  /* Reset DMA */
  reg->CTRL |= CTRL_DMA_RESET | CTRL_FIFO_RESET;
  while (reg->CTRL & (CTRL_DMA_RESET | CTRL_FIFO_RESET));

  channel->length = 0;

  size_t offset = 0;

  while (offset < size)
  {
    const size_t chunkLength = MIN(size - offset, DESC_SIZE_MAX);

    appendItem(channel, address + offset, chunkLength);
    offset += chunkLength;
  }
}
/*----------------------------------------------------------------------------*/
static size_t channelQueued(const void *object)
{
  const struct DmaSdmmc * const channel = object;
  return channel->length;
}
