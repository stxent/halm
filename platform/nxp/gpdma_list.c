/*
 * gpdma_list.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <platform/nxp/gpdma.h>
#include <platform/nxp/gpdma_defs.h>
#include <platform/nxp/gpdma_list.h>
/*----------------------------------------------------------------------------*/
static enum result bufferListInit(void *, const void *);
static void bufferListDeinit(void *);
static enum result bufferListAppend(void *, void *, const void *, uint32_t);
static void bufferListClear(void *);
/*----------------------------------------------------------------------------*/
static const struct DmaListClass bufferListTable = {
    .size = sizeof(struct GpDmaList),
    .init = bufferListInit,
    .deinit = bufferListDeinit,

    .append = bufferListAppend,
    .clear = bufferListClear
};
/*----------------------------------------------------------------------------*/
const struct DmaListClass *GpDmaList = &bufferListTable;
/*----------------------------------------------------------------------------*/
static enum result bufferListInit(void *object, const void *configPtr)
{
  const struct GpDmaListConfig * const config = configPtr;
  struct GpDmaList *list = object;

  if (!config->size)
    return E_VALUE;

  /* Allocation should produce memory chunks aligned along 4-byte boundary */
  list->first = malloc(sizeof(struct GpDmaListItem) * config->size);
  if (!list->first) //FIXME Rename
    return E_MEMORY;

  list->circular = config->circular;
  list->capacity = config->size;
  list->size = 0;

  //FIXME Operations with parent
  for (uint16_t index = 0; index < list->capacity; ++index)
    list->first[index].control = ((struct GpDma *)config->parent)->control;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void bufferListDeinit(void *object)
{
  struct GpDmaList *list = object;

  free(list->first);
}
/*----------------------------------------------------------------------------*/
static enum result bufferListAppend(void *object, void *destination,
    const void *source, uint32_t size)
{
  struct GpDmaList *list = object;
  struct GpDmaListItem *item = list->first + list->size;

  if (size > GPDMA_MAX_TRANSFER)
    return E_VALUE;

  if (list->size >= list->capacity)
    return E_FAULT;

  /* Append current element to the previous one */
  if (list->size++)
    (item - 1)->next = (uint32_t)item;

  item->next = list->circular ? (uint32_t)list->first : 0;

  item->source = (uint32_t)source;
  item->destination = (uint32_t)destination;
  item->control = (item->control & ~CONTROL_SIZE_MASK) | CONTROL_SIZE(size);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void bufferListClear(void *object)
{
  ((struct GpDmaList *)object)->size = 0;
}
