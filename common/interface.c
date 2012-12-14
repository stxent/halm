/*
 * interface.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "interface.h"
/*----------------------------------------------------------------------------*/
uint32_t ifRead(struct Interface *interface, uint8_t *buffer, uint32_t length)
{
  return ((struct InterfaceClass *)CLASS(interface))->read(interface, buffer,
      length);
}
/*----------------------------------------------------------------------------*/
uint32_t ifWrite(struct Interface *interface, const uint8_t *buffer,
    uint32_t length)
{
  return ((struct InterfaceClass *)CLASS(interface))->write(interface, buffer,
      length);
}
/*----------------------------------------------------------------------------*/
enum result ifGetOpt(struct Interface *interface, enum ifOption option,
    void *data)
{
  return ((struct InterfaceClass *)CLASS(interface))->getopt(interface, option,
      data);
}
/*----------------------------------------------------------------------------*/
enum result ifSetOpt(struct Interface *interface, enum ifOption option,
    const void *data)
{
  return ((struct InterfaceClass *)CLASS(interface))->setopt(interface, option,
      data);
}
