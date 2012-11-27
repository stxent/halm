/*
 * interface.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "interface.h"
/*----------------------------------------------------------------------------*/
unsigned int ifRead(struct Interface *iface, uint8_t *buffer,
    unsigned int length)
{
  return iface->type->read(iface, buffer, length);
}
/*----------------------------------------------------------------------------*/
unsigned int ifWrite(struct Interface *iface, const uint8_t *buffer,
    unsigned int length)
{
  return iface->type->write(iface, buffer, length);
}
/*----------------------------------------------------------------------------*/
enum result ifGetOpt(struct Interface *iface, enum ifOption option, void *data)
{
  return iface->type->getopt(iface, option, data);
}
/*----------------------------------------------------------------------------*/
enum result ifSetOpt(struct Interface *iface, enum ifOption option,
    const void *data)
{
  return iface->type->setopt(iface, option, data);
}
