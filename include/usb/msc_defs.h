/*
 * usb/msc_defs.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_USB_MSC_DEFS_H_
#define HALM_USB_MSC_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <bits.h>
#include <memory.h>
/*----------------------------------------------------------------------------*/
#define MSC_CONTROL_EP_SIZE     64
#define MSC_DATA_EP_SIZE        64
#define MSC_DATA_EP_SIZE_HS     512

#define MSC_BLOCK_SIZE          512
#define MSC_BUFFER_SIZE         512

#define RX_QUEUE_SIZE           (MSC_BUFFER_SIZE / MSC_DATA_EP_SIZE)
#define TX_QUEUE_SIZE           RX_QUEUE_SIZE
#define CONTROL_QUEUE_SIZE      2
/*----------------------------------------------------------------------------*/
enum
{
  SCSI_TEST_UNIT_READY        = 0x00,
  SCSI_REQUEST_SENSE          = 0x03,
  SCSI_FORMAT_UNIT            = 0x04,
  SCSI_INQUIRY                = 0x12,
  SCSI_MODE_SELECT6           = 0x15,
  SCSI_MODE_SENSE6            = 0x1A,
  SCSI_START_STOP_UNIT        = 0x1B,
  SCSI_MEDIA_REMOVAL          = 0x1E,
  SCSI_READ_FORMAT_CAPACITIES = 0x23,
  SCSI_READ_CAPACITY          = 0x25,
  SCSI_READ10                 = 0x28,
  SCSI_WRITE10                = 0x2A,
  SCSI_VERIFY10               = 0x2F,
  SCSI_READ12                 = 0xA8,
  SCSI_WRITE12                = 0xAA,
  SCSI_MODE_SELECT10          = 0x55,
  SCSI_MODE_SENSE10           = 0x5A
};
/*----------------------------------------------------------------------------*/
enum
{
  MSC_REQUEST_RESET       = 0xFF,
  MSC_REQUEST_GET_MAX_LUN = 0xFE
};
/*----------------------------------------------------------------------------*/
#define CBW_FLAG_DIRECTION_TO_HOST  BIT(7)
#define CBW_SIGNATURE               TO_LITTLE_ENDIAN_32(0x43425355)
#define CSW_SIGNATURE               TO_LITTLE_ENDIAN_32(0x53425355)

enum
{
  CSW_CMD_PASSED  = 0x00,
  CSW_CMD_FAILED  = 0x01,
  CSW_PHASE_ERROR = 0x02
};

struct CBW
{
  uint32_t signature;
  uint32_t tag;
  uint32_t dataTransferLength;
  uint8_t flags;
  uint8_t lun;
  uint8_t cbLength;
  uint8_t cb[16];
} __attribute__((packed));

struct CSW
{
  uint32_t signature;
  uint32_t tag;
  uint32_t dataResidue;
  uint8_t status;
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
struct InquiryData
{
  uint8_t peripheralDeviceType;
  uint8_t rmb; /* Removable Media Bit */
  uint8_t version; /* ISO Version, ECMA Version, ANSI Version */
  uint8_t responseDataFormat;
  uint8_t additionalLength;
  uint8_t reserved0[2];
  uint8_t vendorInformation[8];
  uint8_t productIdentification[16];
  uint8_t productRevisionLevel[4];
} __attribute__((packed));

struct RequestSenseData
{
  uint8_t flags; /* Valid bit and Error Code field */
  uint8_t reserved0;
  uint8_t senseKey;
  uint32_t information;
  uint8_t additionalSenseLength;
  uint8_t reserved1[4];
  uint8_t additionalSenseCode;
  uint8_t additionalSenseCodeQualifier;
  uint8_t reserved2[4];
} __attribute__((packed));

struct ReadCapacityData
{
  uint32_t lastLogicalBlockAddress;
  uint32_t blockLength;
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_MSC_DEFS_H_ */
