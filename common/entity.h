/*
 * entity.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef ENTITY_H_
#define ENTITY_H_
/*----------------------------------------------------------------------------*/
enum result
{
  E_OK = 0,
  E_ERROR,
  E_IO,
  E_MEM
};
/*----------------------------------------------------------------------------*/
typedef unsigned int entitySize;
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct EntityType
{
  entitySize size;
  /* Create object, arguments: pointer to object, constructor parameters */
  int (*init)(void *, const void *);
  /* Delete object, arguments: pointer to object */
  void (*deinit)(void *);
};
/*----------------------------------------------------------------------------*/
/* Base class */
struct Entity
{
  const struct EntityType *type;
};
/*----------------------------------------------------------------------------*/
void *init(const struct EntityType *, const void *);
void deinit(struct Entity *);
/*----------------------------------------------------------------------------*/
#endif /* ENTITY_H_ */
