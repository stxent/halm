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
  /** Enable active mode. The data pointer must be set to NULL. */
  IF_CAN_ACTIVE = IF_PARAMETER_END,

  /**
   * Enable listener mode. This mode is set by default. The data pointer
   * must be set to NULL.
   */
  IF_CAN_LISTENER,

  /** Enable loopback mode. The data pointer must be set to NULL. */
  IF_CAN_LOOPBACK,

  /**
   * Enable or disable automatic frame retransmission.
   * Parameter type is \p uint8_t. Valid values: 0 disables retransmission,
   * 1 enables automatic retransmission.
   */
  IF_CAN_RETRANSMISSION,

  /**
   * Add an acceptance filter for standard frames.
   * Parameter type is \p struct \p CANFilter.
   */
  IF_CAN_FILTER_ADD_STD,

  /**
   * Remove an acceptance filter for standard frames.
   * Parameter type is \p struct \p CANFilter.
   */
  IF_CAN_FILTER_REMOVE_STD,

  /**
   * Add an acceptance filter for extended frames.
   * Parameter type is \p struct \p CANFilter.
   */
  IF_CAN_FILTER_ADD_EXT,

  /**
   * Remove an acceptance filter for extended frames.
   * Parameter type is \p struct \p CANFilter.
   */
  IF_CAN_FILTER_REMOVE_EXT,

  /**
   * Add an acceptance filter for flexible data-rate (CAN FD) frames.
   * Parameter type is \p struct \p CANFilter.
   */
  IF_CAN_FILTER_ADD_FD,

  /**
   * Remove an acceptance filter for flexible data-rate (CAN FD) frames.
   * Parameter type is \p struct \p CANFilter.
   */
  IF_CAN_FILTER_REMOVE_FD,

  /**
   * Total number of bus errors. Parameter is read-only.
   * Parameter type is \p uint32_t.
   */
  IF_CAN_ERRORS,

  /**
   * Number of received frame overruns. Parameter is read-only.
   * Parameter type is \p uint32_t.
   */
  IF_CAN_OVERRUNS,

  /**
   * Total number of received frames. Parameter is read-only.
   * Parameter type is \p uint32_t.
   */
  IF_CAN_RX_COUNT,

  /**
   * Total number of transmitted frames. Parameter is read-only.
   * Parameter type is \p uint32_t.
   */
  IF_CAN_TX_COUNT,

  /** End of the CAN parameter list. */
  IF_CAN_PARAMETER_END
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
