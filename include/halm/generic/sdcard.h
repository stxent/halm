/*
 * halm/generic/sdcard.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_GENERIC_SDCARD_H_
#define HALM_GENERIC_SDCARD_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
#include <halm/pin.h>
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
  /** Mandatory: hardware interface. */
  struct Interface *interface;
  /** Optional: enable integrity checking for all transfers. */
  bool crc;
};
/*----------------------------------------------------------------------------*/
struct SdCard
{
  struct Interface base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Hardware interface */
  struct Interface *interface;
  /* Current position in the internal memory space */
  uint64_t position;

  /* Number of blocks on the card */
  uint32_t blockCount;
  /* Relative card address */
  uint16_t address;
  /* Memory card capacity, possible values are SC, HC or XC */
  uint8_t capacity;
  /* Subclass of the hardware interface */
  uint8_t mode;
  /* Type of the memory card */
  uint8_t type;
  /* Integrity check flag */
  bool crc;

  /* Command argument */
  uint32_t argument;
  /* Interface command */
  uint32_t command;
  /* Address of the user-space buffer */
  uintptr_t buffer;
  /* Length of the current transfer */
  size_t length;
  /* Transfer state */
  uint8_t state;
  /* Blocking mode flag */
  bool blocking;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_SDCARD_H_ */
