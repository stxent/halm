/*
 * pin.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
struct PinDescriptor
{
  uint8_t number;
  uint8_t port;
};
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
  struct PinDescriptor pin;

  pin.port = PIN_TO_PORT(key);
  pin.number = PIN_TO_OFFSET(key);

  while (list->begin && list->end)
  {
    if (list->channel == channel)
    {
      const struct PinDescriptor begin = {
          .number = PIN_TO_OFFSET(list->begin),
          .port = PIN_TO_PORT(list->begin)
      };
      const struct PinDescriptor end = {
          .number = PIN_TO_OFFSET(list->end),
          .port = PIN_TO_PORT(list->end)
      };
      const bool found = pin.port >= begin.port && pin.port <= end.port
          && pin.number >= begin.number && pin.number <= end.number;

      if (found)
        return list;
    }

    ++list;
  }

  return 0;
}
