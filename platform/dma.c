/*
 * dma.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "platform/dma.h"
/*----------------------------------------------------------------------------*/
/**
 * Start DMA transfer.
 * @param controller Pointer to Dma object.
 * @param dest Destination memory address.
 * @param src Source memory address.
 * @param size Size of the transfer.
 * @return @b E_OK on success.
 */
enum result dmaStart(void *controller, void *dest, const void *src,
    uint32_t size)
{
  return ((struct DmaClass *)CLASS(controller))->start(controller,
      dest, src, size);
}
/*----------------------------------------------------------------------------*/
/**
 * Start scatter-gather DMA transfer.
 * @param controller Pointer to Dma object.
 * @param first Pointer to the first descriptor in linked list.
 * @return @b E_OK on success.
 */
enum result dmaStartList(void *controller, const void *first)
{
  return ((struct DmaClass *)CLASS(controller))->startList(controller, first);
}
/*----------------------------------------------------------------------------*/
/**
 * Disable a channel and lose data in the FIFO.
 * @param controller Pointer to Dma object.
 */
void dmaStop(void *controller)
{
  ((struct DmaClass *)CLASS(controller))->stop(controller);
}
/*----------------------------------------------------------------------------*/
/**
 * Disable a channel without losing data in the FIFO.
 * @param controller Pointer to Dma object.
 */
void dmaHalt(void *controller)
{
  ((struct DmaClass *)CLASS(controller))->halt(controller);
}
/*----------------------------------------------------------------------------*/
/**
 * Link next element to the linked list for scatter-gather transfers.
 * @param controller Pointer to Dma object.
 * @param current Pointer to current linked list item.
 * @param next Pointer to next linked list item or zero on list end.
 * @param dest Destination memory address.
 * @param src Source memory address.
 * @param size Size of the transfer.
 * @return @b E_OK on success.
 */
void dmaLinkItem(void *controller, void *current, void *next,
    void *dest, const void *src, uint16_t size)
{
  ((struct DmaClass *)CLASS(controller))->linkItem(controller, current, next,
      dest, src, size);
}
/*----------------------------------------------------------------------------*/
/**
 * Check whether the channel is enabled or not.
 * @param controller Pointer to Dma object.
 * @return @b true when the transmission is active or @b false otherwise.
 */
bool dmaIsActive(const struct Dma *controller)
{
  return controller->active;
}
