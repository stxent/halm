/*
 * halm/usb/cdc_acm_defs.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_USB_CDC_ACM_DEFS_H_
#define HALM_USB_CDC_ACM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#define CDC_CONTROL_EP_SIZE       64
#define CDC_DATA_EP_SIZE          64
#define CDC_DATA_EP_SIZE_HS       512
#define CDC_NOTIFICATION_EP_SIZE  64

#define CDC_LINE_CODING_DTR       BIT(0)
#define CDC_LINE_CODING_RTS       BIT(1)

#define CDC_SERIAL_STATE_DCD      BIT(0)
#define CDC_SERIAL_STATE_DSR      BIT(1)
/*----------------------------------------------------------------------------*/
/* Class-Specific Request Codes */
enum
{
  CDC_SEND_ENCAPSULATED_COMMAND = 0x00,
  CDC_GET_ENCAPSULATED_RESPONSE = 0x01,
  CDC_SET_COMM_FEATURE          = 0x02,
  CDC_GET_COMM_FEATURE          = 0x03,
  CDC_CLEAR_COMM_FEATURE        = 0x04,
  CDC_SET_LINE_CODING           = 0x20,
  CDC_GET_LINE_CODING           = 0x21,
  CDC_SET_CONTROL_LINE_STATE    = 0x22,
  CDC_SEND_BREAK                = 0x23
};

/* Class-Specific Notification Codes */
enum
{
  CDC_NETWORK_CONNECTION        = 0x00,
  CDC_RESPONSE_AVAILABLE        = 0x01,
  CDC_AUX_JACK_HOOK_STATE       = 0x08,
  CDC_RING_DETECT               = 0x09,
  CDC_SERIAL_STATE              = 0x20,
  CDC_CALL_STATE_CHANGE         = 0x28,
  CDC_LINE_STATE_CHANGE         = 0x29,
  CDC_CONNECTION_SPEED_CHANGE   = 0x2A
};

enum
{
  CDC_SUBTYPE_HEADER            = 0x00,
  CDC_SUBTYPE_CALL_MANAGEMENT   = 0x01,
  CDC_SUBTYPE_ACM               = 0x02,
  CDC_SUBTYPE_UNION             = 0x06
};
/*----------------------------------------------------------------------------*/
struct CdcAcmDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t descriptorSubType;
  uint8_t capabilities;
} __attribute__((packed));

struct CdcCallManagementDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t descriptorSubType;
  uint8_t capabilities;
  uint8_t dataInterface;
} __attribute__((packed));

struct CdcHeaderDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t descriptorSubType;
  uint16_t cdc;
} __attribute__((packed));

struct CdcUnionDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t descriptorSubType;
  uint8_t masterInterface0;
  uint8_t slaveInterface0;
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
struct CdcLineCoding
{
  uint32_t dteRate;
  uint8_t charFormat;
  uint8_t parityType;
  uint8_t dataBits;
} __attribute__((packed));

struct CdcSerialState
{
  uint8_t requestType;
  uint8_t request;
  uint16_t value;
  uint16_t index;
  uint16_t length;
  uint16_t data;
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_CDC_ACM_DEFS_H_ */
