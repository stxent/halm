/*
 * halm/generic/flash.h
 * Copyright (C) 2019 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_FLASH_H_
#define HALM_GENERIC_FLASH_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
enum FlashParameter
{
  /** Get page size. Parameter type is \a uint32_t. */
  IF_FLASH_PAGE_SIZE = IF_PARAMETER_END,
  /** Erase sector at a relative address. Parameter type is \a uint32_t. */
  IF_FLASH_ERASE_SECTOR,
  /** Erase page at a relative address. Parameter type is \a uint32_t. */
  IF_FLASH_ERASE_PAGE
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_FLASH_H_ */
