/*
 * usb/requests.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef USB_REQUESTS_H_
#define USB_REQUESTS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <bits.h>
/*----------------------------------------------------------------------------*/
struct UsbSetupPacket
{
  uint8_t requestType; /* Characteristics of the specific request */
  uint8_t request;     /* Specific request */
  uint16_t value;      /* Request specific parameter */
  uint16_t index;      /* Request specific parameter */
  uint16_t length;     /* Length of data transfered in data phase */
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
enum requestDirection
{
  REQUEST_DIRECTION_TO_DEVICE = 0,
  REQUEST_DIRECTION_TO_HOST   = 1
};

enum requestType
{
  REQUEST_TYPE_STANDARD = 0,
  REQUEST_TYPE_CLASS    = 1,
  REQUEST_TYPE_VENDOR   = 2,
  REQUEST_TYPE_RESERVED = 3
};

enum requestRecipient
{
  REQUEST_RECIPIENT_DEVICE    = 0,
  REQUEST_RECIPIENT_INTERFACE = 1,
  REQUEST_RECIPIENT_ENDPOINT  = 2,
  REQUEST_RECIPIENT_OTHER     = 3
};
/*----------------------------------------------------------------------------*/
#define REQUEST_DIRECTION_MASK          BIT_FIELD(MASK(1), 7)
#define REQUEST_DIRECTION_VALUE(reg) \
    FIELD_VALUE((reg), REQUEST_DIRECTION_MASK, 7)
#define REQUEST_TYPE_MASK               BIT_FIELD(MASK(2), 5)
#define REQUEST_TYPE_VALUE(reg) \
    FIELD_VALUE((reg), REQUEST_TYPE_MASK, 5)
#define REQUEST_RECIPIENT_MASK          BIT_FIELD(MASK(5), 0)
#define REQUEST_RECIPIENT_VALUE(reg) \
    FIELD_VALUE((reg), REQUEST_RECIPIENT_MASK, 0)
/*----------------------------------------------------------------------------*/
enum standardRequest
{
  REQUEST_GET_STATUS        = 0x00,
  REQUEST_CLEAR_FEATURE     = 0x01,
  REQUEST_SET_FEATURE       = 0x03,
  REQUEST_SET_ADDRESS       = 0x05,
  REQUEST_GET_DESCRIPTOR    = 0x06,
  REQUEST_SET_DESCRIPTOR    = 0x07,
  REQUEST_GET_CONFIGURATION = 0x08,
  REQUEST_SET_CONFIGURATION = 0x09,
  REQUEST_GET_INTERFACE     = 0x0A,
  REQUEST_SET_INTERFACE     = 0x0B,
  REQUEST_SYNCH_FRAME       = 0x0C
};

enum hidRequest
{
  HID_GET_REPORT    = 0x01,
  HID_GET_IDLE      = 0x02,
  HID_GET_PROTOCOL  = 0x03,
  HID_SET_REPORT    = 0x09,
  HID_SET_IDLE      = 0x0A,
  HID_SET_PROTOCOL  = 0x0B
};

enum featureSelector
{
  FEA_ENDPOINT_HALT = 0x00,
  FEA_REMOTE_WAKEUP = 0x01,
  FEA_TEST_MODE     = 0x02
};
/*----------------------------------------------------------------------------*/
struct UsbDescriptorHeader
{
  uint8_t length;
  uint8_t type;
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
#define DESC_DEVICE           1
#define DESC_CONFIGURATION    2
#define DESC_STRING           3
#define DESC_INTERFACE        4
#define DESC_ENDPOINT         5
#define DESC_DEVICE_QUALIFIER 6
#define DESC_OTHER_SPEED      7
#define DESC_INTERFACE_POWER  8

#define DESC_HID_HID        0x21
#define DESC_HID_REPORT     0x22
#define DESC_HID_PHYSICAL   0x23

#define GET_DESC_TYPE(x)    (((x)>>8)&0xFF)
#define GET_DESC_INDEX(x)   ((x)&0xFF)
/*----------------------------------------------------------------------------*/
#endif /* USB_REQUESTS_H_ */
