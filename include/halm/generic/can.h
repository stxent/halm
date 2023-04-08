/*
 * halm/generic/can.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_CAN_H_
#define HALM_GENERIC_CAN_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
struct CANFilter
{
  uint32_t id;
  uint32_t mask;
};

enum CANFlags
{
  CAN_EXT_ID  = 0x01,
  CAN_RTR     = 0x02,
  CAN_FD      = 0x04,
  CAN_SELF_RX = 0x08
};

enum CANParameter
{
  /** Enable active mode. Data pointer should be set to zero. */
  IF_CAN_ACTIVE = IF_PARAMETER_END,
  /**
   * Enable listener mode. This mode is set by default.
   * Data pointer should be set to zero.
   */
  IF_CAN_LISTENER,
  /** Enable loopback mode. Data pointer should be set to zero. */
  IF_CAN_LOOPBACK,

  /**
   * Add acceptance filter for standard frames.
   * Parameter type is \a struct \a CANFilter.
   */
  IF_CAN_FILTER_ADD_STD,
  /**
   * Remove acceptance filter for standard frames.
   * Parameter type is \a struct \a CANFilter.
   */
  IF_CAN_FILTER_REMOVE_STD,
  /**
   * Add acceptance filter for extended frames.
   * Parameter type is \a struct \a CANFilter.
   */
  IF_CAN_FILTER_ADD_EXT,
  /**
   * Remove acceptance filter for extended frames.
   * Parameter type is \a struct \a CANFilter.
   */
  IF_CAN_FILTER_REMOVE_EXT,
  /**
   * Add acceptance filter for flexible data-rate frames.
   * Parameter type is \a struct \a CANFilter.
   */
  IF_CAN_FILTER_ADD_FD,
  /**
   * Remove acceptance filter for flexible data-rate frames.
   * Parameter type is \a struct \a CANFilter.
   */
  IF_CAN_FILTER_REMOVE_FD,

  /** Total number of bus errors. Parameter type is \a uint32_t. */
  IF_CAN_ERRORS,
  /** Number of received frame overruns. Parameter type is \a uint32_t. */
  IF_CAN_OVERRUNS,
  /** Number of received frames. Parameter type is \a uint32_t. */
  IF_CAN_RX_COUNT,
  /** Number of transmitted frames. Parameter type is \a uint32_t. */
  IF_CAN_TX_COUNT
};
/*----------------------------------------------------------------------------*/
struct CANMessage
{
  uint32_t timestamp;
  uint32_t id;
  uint8_t flags;
  uint8_t length;
  uint8_t data[];
};

struct CANStandardMessage
{
  uint32_t timestamp;
  uint32_t id;
  uint8_t flags;
  uint8_t length;
  uint8_t data[8];
};

struct CANFlexibleDataMessage
{
  uint32_t timestamp;
  uint32_t id;
  uint8_t flags;
  uint8_t length;
  uint8_t data[64];
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_CAN_H_ */
