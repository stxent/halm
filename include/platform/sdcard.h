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
enum cardType
{
  CARD_SD,
  CARD_SDHC,
  CARD_SDXC
};
/*----------------------------------------------------------------------------*/
struct SdCardConfig
{
  /** Mandatory: low-level interface. */
  struct Interface *interface;
};
/*----------------------------------------------------------------------------*/
struct SdCard
{
  struct Interface parent;

  /* Parent SPI interface */
  struct Interface *interface;
  /* Current position in internal memory space */
  uint64_t position;
  /* Type of the memory card */
  enum cardType type;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_SDCARD_H_ */
