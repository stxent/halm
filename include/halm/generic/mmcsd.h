/*
 * halm/generic/mmcsd.h
 * Copyright (C) 2014, 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_MMCSD_H_
#define HALM_GENERIC_MMCSD_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const MMCSD;

enum MMCSDParameter
{
  /** Size of the erase group in bytes. Parameter type is \a uint32_t. */
  IF_MMCSD_ERASE_GROUP_SIZE = IF_PARAMETER_END,
  /** Erase sector using 32-bit address. Parameter type is \a uint32_t. */
  IF_MMCSD_ERASE,
  /** Erase sector using 64-bit address. Parameter type is \a uint64_t. */
  IF_MMCSD_ERASE_64
};

struct MMCSDConfig
{
  /** Mandatory: hardware interface. */
  void *interface;
  /** Optional: enable integrity checking for all transfers. */
  bool crc;
};

struct MMCSD
{
  struct Interface base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Hardware interface */
  struct Interface *interface;
  /* Subclass of the hardware interface */
  uint8_t mode;
  /* Enable blocking mode */
  bool blocking;
  /* Enable integrity checking */
  bool crc;

  struct
  {
    /* Number of sectors on the card */
    uint32_t sectorCount;
    /* Relative card address */
    uint16_t cardAddress;
    /* Number of sectors in the erase group */
    uint16_t eraseGroupSize;
    /* Capacity type: standard capacity or high capacity */
    uint8_t capacityType;
    /* Memory card type */
    uint8_t cardType;
  } info;

  struct
  {
    /* Address of the user-space buffer */
    uintptr_t buffer;
    /* Length of the current transfer */
    size_t length;

    /* Current position in the internal memory space */
    uint64_t position;
    /* Command argument */
    uint32_t argument;
    /* Command code */
    uint32_t command;
    /* Transfer state */
    uint8_t state;
    /* Send stop command after current data transfer */
    bool autostop;
  } transfer;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_MMCSD_H_ */
