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

#define RX_QUEUE_SIZE           (MSC_BLOCK_SIZE / MSC_DATA_EP_SIZE)
#define TX_QUEUE_SIZE           (MSC_BLOCK_SIZE / MSC_DATA_EP_SIZE + 1)
#define CONTROL_QUEUE_SIZE      2
/*----------------------------------------------------------------------------*/
enum
{
  MSC_SUBCLASS_COMMAND_SET_NOT_REPORTED = 0x00,
  MSC_SUBCLASS_RBC                      = 0x01,
  MSC_SUBCLASS_MMC5                     = 0x02,
  MSC_SUBCLASS_UFI                      = 0x04,
  MSC_SUBCLASS_SCSI                     = 0x06,
  MSC_SUBCLASS_LSDFS                    = 0x07,
  MSC_SUBCLASS_IEEE1667                 = 0x08
};

enum
{
  MSC_PROTOCOL_CBI_INT          = 0x00,
  MSC_PROTOCOL_CBI              = 0x01,
  MSC_PROTOCOL_BBB              = 0x50,
  MSC_PROTOCOL_UAS              = 0x62,
  MSC_PROTOCOL_VENDOR_SPECIFIC  = 0xFF
};
/*----------------------------------------------------------------------------*/
enum
{
  PDT_DIRECT_ACCESS_BLOCK_DEVICE      = 0x00,
  PDT_SEQUENTIAL_ACCESS_DEVICE        = 0x01,
  PDT_PRINTER_DEVICE                  = 0x02,
  PDT_PROCESSOR_DEVICE                = 0x03,
  PDT_WRITE_ONCE_DEVICE               = 0x04,
  PDT_CD_DVD_DEVICE                   = 0x05,
  PDT_OPTICAL_MEMORY_DEVICE           = 0x07,
  PDT_MEDIA_CHANGER_DEVICE            = 0x08,
  PDT_STORAGE_ARRAY_CONTROLLER_DEVICE = 0x0C,
  PDT_ENCLOSURE_SERVICES_DEVICE       = 0x0D,
  PDT_SIMPLIFIED_DIRECT_ACCESS_DEVICE = 0x0E,
  PDT_OPTICAL_CARD_RW_DEVICE          = 0x0F,
  PDT_BRIDGE_CONTROLLER_COMMANDS      = 0x10,
  PDT_OBJECT_BASED_STORAGE_DEVICE     = 0x11,
  PDT_AUTOMATION_DRIVE_INTERFACE      = 0x12,
  PDT_SECURITY_MANAGER_DEVICE         = 0x13,
  PDT_WELL_KNOWN_LOGICAL_UNIT         = 0x1E,
  PDT_UNKNOWN_DEVICE                  = 0x1F
};
/*----------------------------------------------------------------------------*/
enum
{
  SCSI_TEST_UNIT_READY        = 0x00,
  SCSI_REQUEST_SENSE          = 0x03,
  SCSI_FORMAT_UNIT            = 0x04,
  SCSI_READ6                  = 0x08,
  SCSI_WRITE6                 = 0x0A,
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
  SCSI_MODE_SELECT10          = 0x55,
  SCSI_MODE_SENSE10           = 0x5A,
  SCSI_READ12                 = 0xA8,
  SCSI_WRITE12                = 0xAA
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
  uint8_t flags0; /* RMB */
  uint8_t version;
  uint8_t flags1; /* NormACA, HiSup, Response Data Format */
  uint8_t additionalLength; /* N - 4 */
  uint8_t flags2; /* SCCS, ACC, TPGS, 3PC, PROPTECT */
  uint8_t flags3; /* EncServ, VS, MultiP, ADDR16 */
  uint8_t flags4; /* WBUS16, SYNC, CmdQue, VS */
  uint8_t vendorIdentification[8];
  uint8_t productIdentification[16];
  uint8_t productRevisionLevel[4];
} __attribute__((packed));

struct ModeParameterHeader6
{
  uint8_t modeDataLength;
  uint8_t mediumType;
  uint8_t deviceSpecificParameter;
  uint8_t blockDescriptorLength;
} __attribute__((packed));

struct ModeParameterHeader10
{
  uint16_t modeDataLength;
  uint8_t mediumType;
  uint8_t deviceSpecificParameter;
  uint8_t flags; /* LONGLBA */
  uint8_t reserved;
  uint16_t blockDescriptorLength;
} __attribute__((packed));

struct ModeSense6Command
{
  uint8_t operationCode;
  uint8_t flags;
  uint8_t pageCode;
  uint8_t subpageCode;
  uint8_t allocationLength;
  uint8_t control;
} __attribute__((packed));

struct ModeSense10Command
{
  uint8_t operationCode;
  uint8_t flags;
  uint8_t pageCode;
  uint8_t subpageCode;
  uint8_t reserved[3];
  uint16_t allocationLength;
  uint8_t control;
} __attribute__((packed));

struct Read6Command
{
  uint8_t operationCode;
  uint8_t logicalBlockAddress[3];
  uint8_t transferLength;
  uint8_t control;
} __attribute__((packed));

struct Read10Command
{
  uint8_t operationCode;
  uint8_t flags;
  uint32_t logicalBlockAddress;
  uint8_t groupNumber;
  uint16_t transferLength;
  uint8_t control;
} __attribute__((packed));

struct Read12Command
{
  uint8_t operationCode;
  uint8_t flags;
  uint32_t logicalBlockAddress;
  uint32_t transferLength;
  uint8_t groupNumber;
  uint8_t control;
} __attribute__((packed));

struct RequestSenseData
{
  uint8_t responseCode;
  uint8_t reserved0;
  uint8_t flags;
  uint32_t information;
  uint8_t additionalSenseLength;
  uint32_t commandInformation;
  uint8_t additionalSenseCode;
  uint8_t additionalSenseCodeQualifier;
  uint8_t fieldReplacableUnitCode;
  uint8_t senseKeyData[3];
  uint8_t senseData[];
} __attribute__((packed));

struct ReadCapacityData
{
  uint32_t lastLogicalBlockAddress;
  uint32_t blockLength;
} __attribute__((packed));

struct Verify10Command
{
  uint8_t operationCode;
  uint8_t flags;
  uint32_t logicalBlockAddress;
  uint8_t groupNumber;
  uint16_t verificationLength;
  uint8_t control;
} __attribute__((packed));

struct Write6Command
{
  uint8_t operationCode;
  uint8_t logicalBlockAddress[3];
  uint8_t transferLength;
  uint8_t control;
} __attribute__((packed));

struct Write10Command
{
  uint8_t operationCode;
  uint8_t flags;
  uint32_t logicalBlockAddress;
  uint8_t groupNumber;
  uint16_t transferLength;
  uint8_t control;
} __attribute__((packed));

struct Write12Command
{
  uint8_t operationCode;
  uint8_t flags;
  uint32_t logicalBlockAddress;
  uint32_t transferLength;
  uint8_t groupNumber;
  uint8_t control;
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
struct CapacityListHeader
{
  uint8_t reserved[3];
  uint8_t capacityListLength;
} __attribute__((packed));

struct CapacityDescriptor
{
  uint32_t numberOfBlocks;
  uint8_t flags;
  uint8_t blockLength[3];
} __attribute__((packed));

struct ReadFormatCapacitiesData
{
  struct CapacityListHeader header;
  struct CapacityDescriptor descriptors[];
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_MSC_DEFS_H_ */
