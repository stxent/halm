/*
 * interface.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "interface.h"
/*----------------------------------------------------------------------------*/
uint32_t ifRead(void *interface, uint8_t *buffer, uint32_t length)
{
  return ((struct InterfaceClass *)CLASS(interface))->read(interface, buffer,
      length);
}
/*----------------------------------------------------------------------------*/
uint32_t ifWrite(void *interface, const uint8_t *buffer, uint32_t length)
{
  return ((struct InterfaceClass *)CLASS(interface))->write(interface, buffer,
      length);
}
/*----------------------------------------------------------------------------*/
enum result ifGetOpt(void *interface, enum ifOption option, void *data)
{
  return ((struct InterfaceClass *)CLASS(interface))->getopt(interface, option,
      data);
}
/*----------------------------------------------------------------------------*/
enum result ifSetOpt(void *interface, enum ifOption option,
    const void *data)
{
  return ((struct InterfaceClass *)CLASS(interface))->setopt(interface, option,
      data);
}
