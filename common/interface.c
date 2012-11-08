/*
 * interface.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
/*----------------------------------------------------------------------------*/
#include "interface.h"
/*----------------------------------------------------------------------------*/
struct Interface *ifInit(const struct InterfaceClass *type, const void *args)
{
  /* Actually entity is an instance of another class */
  struct Interface *iface;

  iface = (struct Interface *)malloc(type->size);
  if (!iface)
    return 0;
  if (type->init && type->init(iface, args) != E_OK)
  {
    free(iface);
    return 0;
  }
  iface->type = type;
  return iface;
}
/*----------------------------------------------------------------------------*/
void ifDeinit(struct Interface *iface)
{
  if (iface->type->deinit)
    iface->type->deinit(iface);
  free(iface);
}
/*----------------------------------------------------------------------------*/
enum result ifStart(struct Interface *iface, uint8_t *address)
{
  const struct InterfaceClass *type = iface->type;
  if (type->start)
    return type->start(iface, address);
  else
    return E_ERROR;
}
/*----------------------------------------------------------------------------*/
void ifStop(struct Interface *iface)
{
  const struct InterfaceClass *type = iface->type;
  if (type->stop)
    type->stop(iface);
}
/*----------------------------------------------------------------------------*/
unsigned int ifRead(struct Interface *iface, uint8_t *buffer, unsigned int length)
{
  const struct InterfaceClass *type = iface->type;
  if (type->read)
    return type->read(iface, buffer, length);
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
unsigned int ifWrite(struct Interface *iface, const uint8_t *buffer,
    unsigned int length)
{
  const struct InterfaceClass *type = iface->type;
  if (type->write)
    return type->write(iface, buffer, length);
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
enum result ifGetOpt(struct Interface *iface, enum ifOption option, void *data)
{
  const struct InterfaceClass *type = iface->type;
  if (type->getopt)
    return type->getopt(iface, option, data);
  else
    return E_ERROR;
}
/*----------------------------------------------------------------------------*/
enum result ifSetOpt(struct Interface *iface, enum ifOption option,
    const void *data)
{
  const struct InterfaceClass *type = iface->type;
  if (type->setopt)
    return type->setopt(iface, option, data);
  else
    return E_ERROR;
}
