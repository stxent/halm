/*
 * platform/sdcard.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_SDCARD_H_
#define PLATFORM_SDCARD_H_
/*----------------------------------------------------------------------------*/
#include <interface.h>
#include <pin.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const SdCard;
/*----------------------------------------------------------------------------*/
enum sdCardCapacity
{
  SDCARD_SDSC,
  SDCARD_SDHC,
  SDCARD_SDXC
};
/*----------------------------------------------------------------------------*/
enum sdCardType
{
  SDCARD_1_0,
  SDCARD_1_1,
  SDCARD_2_0,
  SDCARD_3_0,
  SDCARD_4_0,
  SDCARD_4_1
};
/*----------------------------------------------------------------------------*/
struct SdCardConfig
{
  /** Mandatory: low-level interface. */
  struct Interface *interface;
  /** Optional: enable integrity checking for all transfers. */
  bool crc;
};
/*----------------------------------------------------------------------------*/
struct SdCard
{
  struct Interface base;

  /* Parent interface */
  struct Interface *interface;
  /* Current position in internal memory space */
  uint64_t position;
  /* Relative card address */
  uint32_t blockCount;
  /* Relative card address */
  uint16_t address;
  /* Memory card capacity */
  enum sdCardCapacity capacity;
  /* Type of the memory card */
  enum sdCardType type;
  /* Integrity check option */
  bool crc;
  /* Type of the underlying interface */
  bool native;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_SDCARD_H_ */
