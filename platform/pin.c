/*
 * pin.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
/* Returns 0 when no descriptor associated with pin found */
const struct PinEntry *pinFind(const struct PinEntry *list, PinNumber key,
    uint8_t channel)
{
  while (list->key && (list->key != key || list->channel != channel))
    ++list;

  return list->key ? list : 0;
}
/*----------------------------------------------------------------------------*/
const struct PinGroupEntry *pinGroupFind(const struct PinGroupEntry *list,
    PinNumber key, uint8_t channel)
{
  struct PinData pin;

  pin.port = PIN_TO_PORT(key);
  pin.offset = PIN_TO_OFFSET(key);

  while (list->begin && list->end)
  {
    if (list->channel == channel)
    {
      const struct PinData begin = {
          .offset = PIN_TO_OFFSET(list->begin),
          .port = PIN_TO_PORT(list->begin)
      };
      const struct PinData end = {
          .offset = PIN_TO_OFFSET(list->end),
          .port = PIN_TO_PORT(list->end)
      };
      const bool found = pin.port >= begin.port && pin.port <= end.port
          && pin.offset >= begin.offset && pin.offset <= end.offset;

      if (found)
        return list;
    }

    ++list;
  }

  return 0;
}
