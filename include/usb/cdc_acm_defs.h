/*
 * usb/cdc_acm_defs.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef USB_CDC_ACM_DEFS_H_
#define USB_CDC_ACM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
enum cdcRequestType
{
  CDC_SEND_ENCAPSULATED_COMMAND = 0x0,
  CDC_GET_ENCAPSULATED_RESPONSE = 0x1,
  CDC_SET_COMM_FEATURE          = 0x2,
  CDC_GET_COMM_FEATURE          = 0x3,
  CDC_CLEAR_COMM_FEATURE        = 0x4,
  CDC_SET_LINE_CODING           = 0x20,
  CDC_GET_LINE_CODING           = 0x21,
  CDC_SET_CONTROL_LINE_STATE    = 0x22,
  CDC_SEND_BREAK                = 0x23
};

struct CdcHeaderDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t descriptorSubType;
  uint16_t cdc;
} __attribute__((packed));

struct CdcCallManagementDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t descriptorSubType;
  uint8_t capabilities;
  uint8_t dataInterface;
} __attribute__((packed));

struct CdcAcmDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t descriptorSubType;
  uint8_t capabilities;
} __attribute__((packed));

struct CdcUnionDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t descriptorSubType;
  uint8_t masterInterface0;
  uint8_t slaveInterface0;
} __attribute__((packed));

struct CdcLineCoding
{
  uint32_t dteRate;
  uint8_t charFormat;
  uint8_t parityType;
  uint8_t dataBits;
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
#endif /* USB_CDC_ACM_DEFS_H_ */
