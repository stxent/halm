/*
 * error.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef ERROR_H_
#define ERROR_H_
/*----------------------------------------------------------------------------*/
enum result
{
  E_OK = 0,
  E_ERROR, /* Generic error */
  E_MEMORY, /* Out of memory */
  E_ENTRY, /* No such file or directory */
  E_ACCESS, /* Permission denied */
  E_FAULT, /* Bad address */
  E_BUSY, /* Device or resource busy */
  E_DEVICE, /* Hardware device error */
  E_INTERFACE, /* Interface error */
};
/*----------------------------------------------------------------------------*/
#endif /* ERROR_H_ */
