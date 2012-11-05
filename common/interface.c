/*
 * interface.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "interface.h"
/*----------------------------------------------------------------------------*/
enum result ifInit(void *, const void *);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass tableInterface = {
    .entity = {
        .size = sizeof(struct Interface),
        .init = ifInit,
        .deinit = 0
    },
    .start = 0,
    .stop = 0,
    .read = 0,
    .write = 0,
    .getopt = 0,
    .setopt = 0
};
/*----------------------------------------------------------------------------*/
const struct EntityClass *Interface =
    (const struct EntityClass *)&tableInterface;
/*----------------------------------------------------------------------------*/
enum result ifInit(void *entity, const void *cdata)
{
  struct Interface *iface = (struct Interface *)entity;
  mutexInit(&iface->lock);
}
/*----------------------------------------------------------------------------*/
enum result ifStart(struct Interface *iface, uint8_t *address)
{
  const struct InterfaceClass *type =
      (const struct InterfaceClass *)((struct Entity *)iface)->type;
  if (type->start)
    return type->start(iface, address);
  else
    return E_ERROR;
}
/*----------------------------------------------------------------------------*/
void ifStop(struct Interface *iface)
{
  const struct InterfaceClass *type =
      (const struct InterfaceClass *)((struct Entity *)iface)->type;
  if (type->stop)
    type->stop(iface);
}
/*----------------------------------------------------------------------------*/
unsigned int ifRead(struct Interface *iface, uint8_t *buffer, unsigned int length)
{
  const struct InterfaceClass *type =
      (const struct InterfaceClass *)((struct Entity *)iface)->type;
  if (type->read)
    return type->read(iface, buffer, length);
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
unsigned int ifWrite(struct Interface *iface, const uint8_t *buffer,
    unsigned int length)
{
  const struct InterfaceClass *type =
      (const struct InterfaceClass *)((struct Entity *)iface)->type;
  if (type->write)
    return type->write(iface, buffer, length);
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
enum result ifGetOpt(struct Interface *iface, enum ifOption option, void *data)
{
  const struct InterfaceClass *type =
      (const struct InterfaceClass *)((struct Entity *)iface)->type;
  if (type->getopt)
    return type->getopt(iface, option, data);
  else
    return E_ERROR;
}
/*----------------------------------------------------------------------------*/
enum result ifSetOpt(struct Interface *iface, enum ifOption option,
    const void *data)
{
  const struct InterfaceClass *type =
      (const struct InterfaceClass *)((struct Entity *)iface)->type;
  if (type->setopt)
    return type->setopt(iface, option, data);
  else
    return E_ERROR;
}
