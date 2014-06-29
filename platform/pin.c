/*
 * pin.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <pin.h>
/*----------------------------------------------------------------------------*/
/* Returns 0 when no descriptor associated with pin found */
const struct PinEntry *pinFind(const struct PinEntry *list, pin_t key,
    uint8_t channel)
{
  while (list->key && (list->key != key || list->channel != channel))
    ++list;

  return list->key ? list : 0;
}
