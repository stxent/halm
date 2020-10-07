/*
 * halm/usb/dfu_defs.h
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_USB_DFU_DEFS_H_
#define HALM_USB_DFU_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
#include <xcore/memory.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#define DFU_CONTROL_EP_SIZE 64
/*----------------------------------------------------------------------------*/
enum
{
  APP_SPEC_PROTOCOL_DFU_RUNTIME = 0x01,
  APP_SPEC_PROTOCOL_DFU_MODE    = 0x02
};
/*----------------------------------------------------------------------------*/
/* Class-specific descriptor types */
enum
{
  DESCRIPTOR_TYPE_DFU_FUNCTIONAL = 0x21
};
/*----------------------------------------------------------------------------*/
enum
{
  DFU_REQUEST_DETACH    = 0,
  DFU_REQUEST_DNLOAD    = 1,
  DFU_REQUEST_UPLOAD    = 2,
  DFU_REQUEST_GETSTATUS = 3,
  DFU_REQUEST_CLRSTATUS = 4,
  DFU_REQUEST_GETSTATE  = 5,
  DFU_REQUEST_ABORT     = 6
};
/*----------------------------------------------------------------------------*/
enum DfuStatus
{
  DFU_STATUS_OK               = 0x00,
  DFU_STATUS_ERR_TARGET       = 0x01,
  DFU_STATUS_ERR_FILE         = 0x02,
  DFU_STATUS_ERR_WRITE        = 0x03,
  DFU_STATUS_ERR_ERASE        = 0x04,
  DFU_STATUS_ERR_CHECK_ERASED = 0x05,
  DFU_STATUS_ERR_PROG         = 0x06,
  DFU_STATUS_ERR_VERIFY       = 0x07,
  DFU_STATUS_ERR_ADDRESS      = 0x08,
  DFU_STATUS_ERR_NOTDONE      = 0x09,
  DFU_STATUS_ERR_FIRMWARE     = 0x0A,
  DFU_STATUS_ERR_VENDOR       = 0x0B,
  DFU_STATUS_ERR_USBR         = 0x0C,
  DFU_STATUS_ERR_POR          = 0x0D,
  DFU_STATUS_ERR_UNKNOWN      = 0x0E,
  DFU_STATUS_ERR_STALLEDPKT   = 0x0F
};

enum DfuState
{
  STATE_APP_IDLE                = 0,
  STATE_APP_DETACH              = 1,
  STATE_DFU_IDLE                = 2,
  STATE_DFU_DNLOAD_SYNC         = 3,
  STATE_DFU_DNBUSY              = 4,
  STATE_DFU_DNLOAD_IDLE         = 5,
  STATE_DFU_MANIFEST_SYNC       = 6,
  STATE_DFU_MANIFEST            = 7,
  STATE_DFU_MANIFEST_WAIT_RESET = 8,
  STATE_DFU_UPLOAD_IDLE         = 9,
  STATE_DFU_ERROR               = 10
};
/*----------------------------------------------------------------------------*/
enum
{
  DFU_CAN_DNLOAD              = 0x01,
  DFU_CAN_UPLOAD              = 0x02,
  DFU_MANIFESTATION_TOLERANT  = 0x04,
  DFU_WILL_DETACH             = 0x08
};

struct DfuFunctionalDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t attributes;
  uint16_t detachTimeout;
  uint16_t transferSize;
  uint16_t dfuVersion;
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
struct DfuGetStatusResponse
{
  uint8_t status;
  uint8_t pollTimeout[3];
  uint8_t state;
  uint8_t string;
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_DFU_DEFS_H_ */
