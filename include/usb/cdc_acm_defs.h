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
