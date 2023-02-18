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
  /** Get block size. Parameter type is \a uint32_t. */
  IF_FLASH_BLOCK_SIZE = IF_PARAMETER_END,
  /** Get sector size. Parameter type is \a uint32_t. */
  IF_FLASH_SECTOR_SIZE,
  /** Get page size. Parameter type is \a uint32_t. */
  IF_FLASH_PAGE_SIZE,
  /** Erase block at a relative address. Parameter type is \a uint32_t. */
  IF_FLASH_ERASE_BLOCK,
  /** Erase sector at a relative address. Parameter type is \a uint32_t. */
  IF_FLASH_ERASE_SECTOR,
  /** Erase page at a relative address. Parameter type is \a uint32_t. */
  IF_FLASH_ERASE_PAGE
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_FLASH_H_ */
