/*
 * interface.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
/*----------------------------------------------------------------------------*/
#include "interface.h"
/*----------------------------------------------------------------------------*/
enum result ifStart(struct Interface *iface, uint8_t *address)
{
  return iface->type->start ? iface->type->start(iface, address) : E_ERROR;
}
/*----------------------------------------------------------------------------*/
void ifStop(struct Interface *iface)
{
  if (iface->type->stop)
    iface->type->stop(iface);
}
/*----------------------------------------------------------------------------*/
unsigned int ifRead(struct Interface *iface, uint8_t *buffer,
    unsigned int length)
{
  return iface->type->read ? iface->type->read(iface, buffer, length) : 0;
}
/*----------------------------------------------------------------------------*/
unsigned int ifWrite(struct Interface *iface, const uint8_t *buffer,
    unsigned int length)
{
  return iface->type->write ? iface->type->write(iface, buffer, length) : 0;
}
/*----------------------------------------------------------------------------*/
enum result ifGetOpt(struct Interface *iface, enum ifOption option, void *data)
{
  return iface->type->getopt ?
      iface->type->getopt(iface, option, data) : E_ERROR;
}
/*----------------------------------------------------------------------------*/
enum result ifSetOpt(struct Interface *iface, enum ifOption option,
    const void *data)
{
  return iface->type->setopt ?
      iface->type->setopt(iface, option, data) : E_ERROR;
}
