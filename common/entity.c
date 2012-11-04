/*
 * entity.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
/*----------------------------------------------------------------------------*/
#include "entity.h"
/*----------------------------------------------------------------------------*/
void *init(const struct EntityClass *type, const void *args)
{
  /* Actually entity is an instance of another class */
  struct Entity *entity;

  if (!type)
    return 0;
  entity = malloc(type->size);
  if (!entity)
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
void deinit(struct Entity *entity)
{
  if (entity->type->deinit)
    entity->type->deinit(entity);
  free(entity);
}
