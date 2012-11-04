/*
 * interface.c
 *
 *  Created on: Oct 20, 2012
 *      Author: xen
 */

#include "interface.h"
/*----------------------------------------------------------------------------*/
/* Interface is an abstract class */
const struct EntityClass *Interface = 0;
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
