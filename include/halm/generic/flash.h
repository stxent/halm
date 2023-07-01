/*
 * halm/generic/flash.h
 * Copyright (C) 2019 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_FLASH_H_
#define HALM_GENERIC_FLASH_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
#include <stdint.h>
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

struct FlashGeometry
{
  /** Sector count in a flash region. */
  size_t count;
  /** Size of each sector in a region. */
  size_t size;
  /** Sector erase time in milliseconds. */
  uint32_t time;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Common functions */
const struct FlashGeometry *flashFindRegion(const struct FlashGeometry *,
    size_t, uint32_t);

/* Platform-specific functions */
size_t flashGetGeometry(const void *, struct FlashGeometry *, size_t);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_FLASH_H_ */
