/*
 * entity.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
#include <entity.h>
/*----------------------------------------------------------------------------*/
void *init(const void *descriptor, const void *arguments)
{
  const struct EntityClass * const base = descriptor;
  struct Entity *entity;

  if (!base || !base->size || !(entity = malloc(base->size)))
    return 0;

  if (base->init)
  {
    enum result res;

    if ((res = base->init(entity, arguments)) != E_OK)
    {
      free(entity);
      return 0;
    }
  }

  entity->descriptor = base;
  return entity;
}
/*----------------------------------------------------------------------------*/
void deinit(void *entity)
{
  const struct EntityClass * const base = CLASS(entity);

  if (base->deinit)
    base->deinit(entity);

  free(entity);
}
