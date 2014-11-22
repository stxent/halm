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
  E_OK,
  /** Generic error. */
  E_ERROR,
  /** Out of memory. */
  E_MEMORY,
  /** Permission denied. */
  E_ACCESS,
  /** Bad address. */
  E_ADDRESS,
  /** Device or resource busy. */
  E_BUSY,
  /** Device error. */
  E_DEVICE,
  /** Device or resource is in idle state. */
  E_IDLE,
  /** Interface error. */
  E_INTERFACE,
  /** Unsupported operation. */
  E_INVALID,
  /** Operation timed out. */
  E_TIMEOUT,
  /** Argument contains wrong value. */
  E_VALUE,
  /** No such file. */
  E_ENTRY,
  /** File exists. */
  E_EXIST,
  /** No data available. */
  E_EMPTY,
  /** No space left. */
  E_OVERFLOW
};
/*----------------------------------------------------------------------------*/
#endif /* ERROR_H_ */
