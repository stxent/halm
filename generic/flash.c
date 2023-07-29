/*
 * flash.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/flash.h>
/*----------------------------------------------------------------------------*/
const struct FlashGeometry *flashFindRegion(const struct FlashGeometry *layout,
    size_t count, uint32_t address)
{
  uint32_t offset = 0;

  for (size_t index = 0; index < count; ++index)
  {
    const uint32_t total = layout->count * layout->size;

    if (address >= offset && address < offset + total)
      return layout;

    offset += total;
    ++layout;
  }

  return NULL;
}
