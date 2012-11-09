/*
 * entity.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef ENTITY_H_
#define ENTITY_H_
/*----------------------------------------------------------------------------*/
#define CLASS_GENERATOR(name) \
  unsigned int size;\
  enum result (*init)(struct name *, const void *);\
  void (*deinit)(struct name *);
/*----------------------------------------------------------------------------*/
enum result
{
  E_OK = 0,
  E_ERROR,
  E_MEMORY,
  E_DEVICE,
  E_INTERFACE
};
/*----------------------------------------------------------------------------*/
void *init(const void *, const void *);
void deinit(void *);
/*----------------------------------------------------------------------------*/
#endif /* ENTITY_H_ */
