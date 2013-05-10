/*
 * interface.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "interface.h"
/*----------------------------------------------------------------------------*/
/**
 * Receive data.
 * @param interface Pointer to an Interface object.
 * @param buffer Pointer to a buffer with a size of at least @b length bytes.
 * @param length Number of bytes to be read.
 * @return The total number of elements successfully read is returned.
 */
uint32_t ifRead(void *interface, uint8_t *buffer, uint32_t length)
{
  return ((struct InterfaceClass *)CLASS(interface))->read(interface, buffer,
      length);
}
/*----------------------------------------------------------------------------*/
/**
 * Send data.
 * @param interface Pointer to an Interface object.
 * @param buffer Pointer to a buffer with a size of at least @b length bytes.
 * @param length Number of bytes to be written.
 * @return The total number of elements successfully written is returned.
 */
uint32_t ifWrite(void *interface, const uint8_t *buffer, uint32_t length)
{
  return ((struct InterfaceClass *)CLASS(interface))->write(interface, buffer,
      length);
}
/*----------------------------------------------------------------------------*/
/**
 * Retrieve value of an option.
 * @param interface Pointer to an Interface object.
 * @param option Option to be read.
 * @param data Pointer to a variable where a value of the option will be stored.
 * @return E_OK on success.
 */
enum result ifGet(void *interface, enum ifOption option, void *data)
{
  return ((struct InterfaceClass *)CLASS(interface))->get(interface, option,
      data);
}
/*----------------------------------------------------------------------------*/
/**
 * Set value of an option.
 * @param interface Pointer to an Interface object.
 * @param option Option to be set.
 * @param data Pointer to a new value of the option.
 * @return E_OK on success.
 */
enum result ifSet(void *interface, enum ifOption option, const void *data)
{
  return ((struct InterfaceClass *)CLASS(interface))->set(interface, option,
      data);
}
