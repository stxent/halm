/*
 * dma_sdmmc.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <platform/nxp/dma_sdmmc.h>
#include <platform/nxp/sdmmc_defs.h>
#include <platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
static enum result appendItem(void *, uint32_t, uint32_t);
/*----------------------------------------------------------------------------*/
static enum result controllerInit(void *, const void *);
static void controllerDeinit(void *);
static void controllerCallback(void *, void (*)(void *), void *);
static uint32_t controllerCount(const void *);
static enum result controllerReconfigure(void *, const void *);
static enum result controllerStart(void *, void *, const void *, uint32_t);
static enum result controllerStatus(const void *);
static void controllerStop(void *);
/*----------------------------------------------------------------------------*/
static const struct DmaClass controllerTable = {
    .size = sizeof(struct DmaSdmmc),
    .init = controllerInit,
    .deinit = controllerDeinit,

    .callback = controllerCallback,
    .count = controllerCount,
    .reconfigure = controllerReconfigure,
    .start = controllerStart,
    .status = controllerStatus,
    .stop = controllerStop
};
/*----------------------------------------------------------------------------*/
const struct DmaClass * const DmaSdmmc = &controllerTable;
/*----------------------------------------------------------------------------*/
static enum result appendItem(void *object, uint32_t buffer, uint32_t size)
{
  struct DmaSdmmc * const controller = object;
  struct DmaSdmmcEntry * const entry = controller->list + controller->length;
  uint32_t control = DESC_CONTROL_OWN | DESC_CONTROL_CH | DESC_CONTROL_LD;

  if (controller->length++)
  {
    struct DmaSdmmcEntry *previous = entry - 1;

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
  entry->buffer1 = buffer;
  entry->buffer2 = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result controllerInit(void *object, const void *configBase)
{
  const struct DmaSdmmcConfig * const config = configBase;
  struct DmaSdmmc * const controller = object;

  assert(config->number);
  assert(config->burst <= DMA_BURST_256 && config->burst != DMA_BURST_2);

  /* Allocation should produce memory chunks aligned along 4-byte boundary */
  controller->list = malloc(sizeof(struct DmaSdmmcEntry) * config->number);
  if (!controller->list)
    return E_MEMORY;

  controller->capacity = config->number;
  controller->length = 0;
  controller->reg = config->parent->base.reg;

  LPC_SDMMC_Type * const reg = controller->reg;
  const uint8_t burst = config->burst >= DMA_BURST_4 ?
      config->burst - 1 : config->burst;

  /* Control register is originally initialized in parent class */

  /* Reset DMA controller */
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
static void controllerDeinit(void *object)
{
  struct DmaSdmmc * const controller = object;
  LPC_SDMMC_Type * const reg = controller->reg;

  reg->CTRL &= ~CTRL_USE_INTERNAL_DMAC;
  reg->BMOD = 0;

  free(controller->list);
}
/*----------------------------------------------------------------------------*/
static void controllerCallback(void *object __attribute__((unused)),
    void (*callback)(void *) __attribute__((unused)),
    void *argument __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static uint32_t controllerCount(const void *object __attribute__((unused)))
{
  const struct DmaSdmmc * const controller = object;
  const LPC_SDMMC_Type * const reg = controller->reg;
  const struct DmaSdmmcEntry * const current =
      (const struct DmaSdmmcEntry *)reg->DSCADDR;

  if (!current)
    return 0;

  return (uint32_t)(controller->length - (current - controller->list));
}
/*----------------------------------------------------------------------------*/
static enum result controllerReconfigure(void *object __attribute__((unused)),
    const void *configBase __attribute__((unused)))
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result controllerStart(void *object, void *destination,
    const void *source, uint32_t size)
{
  struct DmaSdmmc * const controller = object;
  LPC_SDMMC_Type * const reg = controller->reg;

  assert(size && size <= (uint32_t)(controller->capacity * DESC_SIZE_MAX));
  assert(!destination ^ !source);

  const uint32_t address = destination ?
      (uint32_t)destination : (uint32_t)source;

  /* Reset DMA */
  reg->CTRL |= CTRL_DMA_RESET | CTRL_FIFO_RESET;
  while (reg->CTRL & (CTRL_DMA_RESET | CTRL_FIFO_RESET));

  uint32_t offset = 0;

  controller->length = 0;
  while (offset < size)
  {
    const uint32_t chunk = size - offset >= DESC_SIZE_MAX ?
        DESC_SIZE_MAX : size - offset;

    appendItem(controller, address + offset, chunk);
    offset += chunk;
  }

  /* Clear status flags */
  reg->IDSTS = IDINTEN_TI | IDINTEN_RI | IDINTEN_FBE | IDINTEN_DU | IDINTEN_CES;
  /* Set DMA descriptor base address */
  reg->DBADDR = (uint32_t)controller->list;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result controllerStatus(const void *object)
{
  const struct DmaSdmmc * const controller = object;
  const LPC_SDMMC_Type * const reg = controller->reg;
  const uint32_t status = reg->IDINTEN;

  if (status & IDINTEN_AIS)
    return E_INTERFACE;
  else if (status & IDINTEN_NIS)
    return E_OK;
  else
    return E_BUSY;
}
/*----------------------------------------------------------------------------*/
static void controllerStop(void *object __attribute__((unused)))
{

}
