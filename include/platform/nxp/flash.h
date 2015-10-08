/*
 * platform/nxp/flash.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_FLASH_H_
#define PLATFORM_NXP_FLASH_H_
/*----------------------------------------------------------------------------*/
#include <interface.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Flash;
/*----------------------------------------------------------------------------*/
enum flashOption
{
  /** Erase sector. */
  IF_FLASH_ERASE_SECTOR = IF_OPTION_END,
  /** Erase page in sector. Not supported on all interfaces. */
  IF_FLASH_ERASE_PAGE,
  /**
   * Check page erase capability. Return @b @_OK, when this feature
   * is supported.
   */
  IF_FLASH_PAGE_ERASABLE,
  /**
   * Check byte erase capability. Return @b @_OK, when this feature
   * is supported.
   */
  IF_FLASH_BYTE_ERASABLE,
  /** Sector size. */
  IF_FLASH_SECTOR_SIZE,
  /** Page size. */
  IF_FLASH_PAGE_SIZE
};
/*----------------------------------------------------------------------------*/
struct Flash
{
  struct Interface parent;

  void (*callback)(void *);
  void *callbackArgument;

  /* Current address */
  uint32_t position;
  /* Size of the Flash memory */
  uint32_t size;
};
/*----------------------------------------------------------------------------*/
enum result flashBlankCheckSector(uint32_t);
enum result flashErasePage(uint32_t);
enum result flashEraseSector(uint32_t);
uint32_t flashReadId();
enum result flashWriteBuffer(uint32_t, const uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_FLASH_H_ */
