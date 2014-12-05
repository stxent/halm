/*
 * dma_sdmmc.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
#include <platform/platform_defs.h>
#include <platform/nxp/dma_sdmmc.h>
#include <platform/nxp/sdmmc_defs.h>
/*----------------------------------------------------------------------------*/
static enum result appendItem(void *, uint32_t, uint32_t);
/*----------------------------------------------------------------------------*/
static enum result controllerInit(void *, const void *);
static void controllerDeinit(void *);
static void controllerCallback(void *, void (*)(void *), void *);
static uint32_t controllerIndex(const void *);
static enum result controllerStart(void *, void *, const void *, uint32_t);
static enum result controllerStatus(const void *);
static void controllerStop(void *);
/*----------------------------------------------------------------------------*/
static const struct DmaClass controllerTable = {
    .size = sizeof(struct DmaSdmmc),
    .init = controllerInit,
    .deinit = controllerDeinit,

    .callback = controllerCallback,
    .index = controllerIndex,
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
  struct DmaSdmmcEntry * const entry = controller->list + controller->number;
  uint32_t control = DESC_CONTROL_OWN | DESC_CONTROL_CH | DESC_CONTROL_LD;

  if (controller->number++)
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

  if (!config->number)
    return E_VALUE;

  /* Allocation should produce memory chunks aligned along 4-byte boundary */
  controller->list = malloc(sizeof(struct DmaSdmmcEntry) * config->number);
  if (!controller->list)
    return E_MEMORY;

  controller->capacity = config->number;
  controller->circular = config->circular;
  controller->number = 0;
  controller->reg = config->parent->parent.reg;

  LPC_SDMMC_Type * const reg = controller->reg;

  /* Control register is initialized in parent class */

  /* Reset DMA controller */
  reg->BMOD = BMOD_SWR;
  /* Reset DMA interface */
  reg->CTRL |= CTRL_DMA_RESET;
  while (reg->CTRL & CTRL_DMA_RESET);

  /* Use internal DMA */
  reg->CTRL |= CTRL_USE_INTERNAL_DMAC;
  /* Enable internal DMA */
  reg->BMOD = BMOD_DE;
  /* Disable all DMA interrupts */
  reg->IDINTEN = 0;

  //TODO Configure DSL
  //TODO Configure burst

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void controllerDeinit(void *object)
{
  struct DmaSdmmc * const controller = object;

  free(controller->list);
}
/*----------------------------------------------------------------------------*/
static void controllerCallback(void *object __attribute__((unused)),
    void (*callback)(void *) __attribute__((unused)),
    void *argument __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static uint32_t controllerIndex(const void *object __attribute__((unused)))
{
  //TODO
  return 0;
}
/*----------------------------------------------------------------------------*/
static enum result controllerStart(void *object, void *destination,
    const void *source, uint32_t size)
{
  struct DmaSdmmc * const controller = object;
  LPC_SDMMC_Type * const reg = controller->reg;

  if (size > (uint32_t)(controller->capacity * DESC_SIZE_MAX))
    return E_VALUE;
  if ((!destination ^ !source) == 0)
    return E_VALUE;

  const uint32_t address = destination ?
      (uint32_t)destination : (uint32_t)source;

  /* Reset DMA */
  reg->CTRL |= CTRL_DMA_RESET | CTRL_FIFO_RESET;
  while (reg->CTRL & (CTRL_DMA_RESET | CTRL_FIFO_RESET));

  uint32_t offset = 0;

  controller->number = 0;
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
  //TODO
}
