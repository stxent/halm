/*
 * error.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Error code definitions.
 */

#ifndef ERROR_H_
#define ERROR_H_
/*----------------------------------------------------------------------------*/
enum result
{
  E_OK = 0,
  /** Generic error. */
  E_ERROR,
  /** Out of memory. */
  E_MEMORY,
  /** No such file or directory. */
  E_ENTRY,
  /** Permission denied. */
  E_ACCESS,
  /** Bad address. */
  E_FAULT,
  /** Device or resource busy. */
  E_BUSY,
  /** Hardware device error. */
  E_DEVICE,
  /** Invalid argument. */
  E_VALUE,
  /** Interface error. */
  E_INTERFACE
};
/*----------------------------------------------------------------------------*/
#endif /* ERROR_H_ */
