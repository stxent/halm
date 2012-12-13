/*
 * entity.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
/*----------------------------------------------------------------------------*/
#include "entity.h"
/*----------------------------------------------------------------------------*/
void *init(const void *typeDesc, const void *args)
{
  const struct EntityClass *type = (const struct EntityClass *)typeDesc;
  struct Entity *entity;

  if (!type || !type->size || !(entity = malloc(type->size)))
    return 0;
  if (type->init && type->init(entity, args) != E_OK)
  {
    free(entity);
    return 0;
  }
  entity->type = type;
  return entity;
}
/*----------------------------------------------------------------------------*/
void deinit(void *entity)
{
  const struct EntityClass *type = CLASS(entity);

  if (type->deinit)
    type->deinit(entity);
  free(entity);
}
