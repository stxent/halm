/*
 * halm/usb/msc_defs.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_USB_MSC_DEFS_H_
#define HALM_USB_MSC_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
#include <xcore/memory.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#define MSC_CONTROL_EP_SIZE 64
#define MSC_DATA_EP_SIZE    64
#define MSC_DATA_EP_SIZE_HS 512

#define MSC_BLOCK_SIZE      512
#define DATA_QUEUE_SIZE     (MSC_BLOCK_SIZE / MSC_DATA_EP_SIZE + 1)
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
  SCSI_MEDIUM_REMOVAL         = 0x1E,
  SCSI_READ_FORMAT_CAPACITIES = 0x23,
  SCSI_READ_CAPACITY10        = 0x25,
  SCSI_READ10                 = 0x28,
  SCSI_WRITE10                = 0x2A,
  SCSI_VERIFY10               = 0x2F,
  SCSI_MODE_SELECT10          = 0x55,
  SCSI_MODE_SENSE10           = 0x5A,
  SCSI_READ12                 = 0xA8,
  SCSI_WRITE12                = 0xAA
};

/* Sense Key */
enum
{
  SCSI_SK_NO_SENSE            = 0x00,
  SCSI_SK_RECOVERED_ERROR     = 0x01,
  SCSI_SK_NOT_READY           = 0x02,
  SCSI_SK_MEDIUM_ERROR        = 0x03,
  SCSI_SK_HARDWARE_ERROR      = 0x04,
  SCSI_SK_ILLEGAL_REQUEST     = 0x05,
  SCSI_SK_UNIT_ATTENTION      = 0x06,
  SCSI_SK_DATA_PROTECT        = 0x07,
  SCSI_SK_BLANK_CHECK         = 0x08,
  SCSI_SK_VENDOR_SPECIFIC     = 0x09,
  SCSI_SK_COPY_ABORTED        = 0x0A,
  SCSI_SK_ABORTED_COMMAND     = 0x0B,
  SCSI_SK_VOLUME_OVERFLOW     = 0x0D,
  SCSI_SK_MISCOMPARE          = 0x0E,
  SCSI_SK_COMPLETED           = 0x0F
};

/* Sense Key 0x00 */
#define SCSI_ASC_NOSENSE                0x0000

/* Sense Key 0x02 */
#define SCSI_ASC_NR_MEDIUMNOTPRESENT    0x3A00

/* Sense Key 0x03 */
#define SCSI_ASC_ME_WRITEFAULT          0x0300
#define SCSI_ASC_ME_READERROR           0x1100

/* Sense Key 0x05 */
#define SCSI_ASC_IR_INVALIDCOMMAND      0x2000
#define SCSI_ASC_IR_LBAOUTOFRANGE       0x2100
#define SCSI_ASC_IR_INVALIDFIELDINCBA   0x2400
#define SCSI_ASC_IR_INVALIDLUN          0x2500

/* Sense Key 0x06 */
#define SCSI_ASC_UA_READYTRANSITION     0x2800

/* Sense Key 0x07 */
#define SCSI_ASC_WP_COMMANDNOTALLOWED   0x2700
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
  MSC_PROTOCOL_CBI_INT                  = 0x00,
  MSC_PROTOCOL_CBI                      = 0x01,
  MSC_PROTOCOL_BBB                      = 0x50,
  MSC_PROTOCOL_UAS                      = 0x62,
  MSC_PROTOCOL_VENDOR_SPECIFIC          = 0xFF
};
/*----------------------------------------------------------------------------*/
enum
{
  MSC_REQUEST_RESET                     = 0xFF,
  MSC_REQUEST_GET_MAX_LUN               = 0xFE
};
/*----------------------------------------------------------------------------*/
/*
 * Possible values of the Prevent field in a
 * Prevent/Allow Medium Removal command.
 */
enum
{
  PAMR_UNLOCK                           = 0x00,
  PAMR_LOCK                             = 0x01
};

/*
 * Possible values of the Peripheral Device Type field in a response to a
 * Inquiry command.
 */
enum
{
  PDT_DIRECT_ACCESS_BLOCK_DEVICE        = 0x00,
  PDT_SEQUENTIAL_ACCESS_DEVICE          = 0x01,
  PDT_PRINTER_DEVICE                    = 0x02,
  PDT_PROCESSOR_DEVICE                  = 0x03,
  PDT_WRITE_ONCE_DEVICE                 = 0x04,
  PDT_CD_DVD_DEVICE                     = 0x05,
  PDT_OPTICAL_MEMORY_DEVICE             = 0x07,
  PDT_MEDIA_CHANGER_DEVICE              = 0x08,
  PDT_STORAGE_ARRAY_CONTROLLER_DEVICE   = 0x0C,
  PDT_ENCLOSURE_SERVICES_DEVICE         = 0x0D,
  PDT_SIMPLIFIED_DIRECT_ACCESS_DEVICE   = 0x0E,
  PDT_OPTICAL_CARD_RW_DEVICE            = 0x0F,
  PDT_BRIDGE_CONTROLLER_COMMANDS        = 0x10,
  PDT_OBJECT_BASED_STORAGE_DEVICE       = 0x11,
  PDT_AUTOMATION_DRIVE_INTERFACE        = 0x12,
  PDT_SECURITY_MANAGER_DEVICE           = 0x13,
  PDT_WELL_KNOWN_LOGICAL_UNIT           = 0x1E,
  PDT_UNKNOWN_DEVICE                    = 0x1F
};

/* Enable Vital Product Data */
#define INQUIRY_EVPD                    BIT(0)
/* Removable Medium Bit set to one indicates that the medium is removable */
#define INQUIRY_FLAGS_0_RMB             BIT(7)
/* Response Data Format */
#define INQUIRY_FLAGS_1_RDF(value)      BIT_FIELD((value), 0)
/* Hierarchical Support */
#define INQUIRY_FLAGS_1_HISUP           BIT(4)
/* Normal ACA bit: support of the ACA task attribute */
#define INQUIRY_FLAGS_1_NORMACA         BIT(5)
/* PROTECT bit: the logical unit supports type 1, 2 or 3 protection */
#define INQUIRY_FLAGS_2_PROTECT         BIT(0)
/* Third-Party Copy: the device supports third-party copy commands */
#define INQUIRY_FLAGS_2_3PC             BIT(3)
/* Target Port Group Support: the support for asymmetric logcal unit access */
#define INQUIRY_FLAGS_2_TPGS(value)     BIT_FIELD((value), 4)
/*
 * Access Controls Coordinator: the device contains an access controls
 * coordinator, that may be addressed through this logical unit.
 */
#define INQUIRY_FLAGS_2_ACC             BIT(6)
/* SCC Supported bit: the device contains an embedded storage controller */
#define INQUIRY_FLAGS_2_SCCS            BIT(7)
/*
 * Multi Port: the device contains two or more ports and conforms
 * to the multi-port requirements.
 */
#define INQUIRY_FLAGS_3_MULTIP          BIT(4)
/* Enclosure Services: the device contains an embedded enclosure services */
#define INQUIRY_FLAGS_3_ENCSERV         BIT(6)
/* Command Queuing */
#define INQUIRY_FLAGS_4_CMDQUE          BIT(1)

#define MEDIUMREMOVAL_FLAGS_PREVENT_MASK \
    BIT_FIELD(MASK(2), 0)
#define MEDIUMREMOVAL_FLAGS_PREVENT_VALUE(value) \
    FIELD_VALUE((value), MEDIUMREMOVAL_FLAGS_PREVENT_MASK, 0)

#define READ6_LBA_MASK                  MASK(21)
#define READ10_RARC                     BIT(2)
#define READ10_FUA                      BIT(3)
#define READ10_DPO                      BIT(4)
#define READ10_RDPROTECT_MASK           BIT_FIELD(MASK(3), 5)
#define READ12_GROUP_NUMBER_MASK        BIT_FIELD(MASK(5), 0)

#define READCAPACITY10_FLAGS_0_RELADR   BIT(0)
#define READCAPACITY10_FLAGS_1_PMI      BIT(0)

#define SENSE_FLAGS_ILI                 BIT(5)
#define SENSE_FLAGS_EOM                 BIT(6)
#define SENSE_FLAGS_FILEMARK            BIT(7)

#define VERIFY10_BYTCHK_MASK            BIT_FIELD(MASK(2), 1)
#define VERIFY10_DPO                    BIT(4)
#define VERIFY10_VRPROTECT_MASK         BIT_FIELD(MASK(3), 5)
#define VERIFY10_GROUP_NUMBER_MASK      BIT_FIELD(MASK(5), 0)
/*----------------------------------------------------------------------------*/
#define CBW_FLAG_DIRECTION_TO_HOST      BIT(7)
#define CBW_SIGNATURE                   TO_LITTLE_ENDIAN_32(0x43425355)
#define CSW_SIGNATURE                   TO_LITTLE_ENDIAN_32(0x53425355)

enum
{
  CSW_CMD_PASSED  = 0x00,
  CSW_CMD_FAILED  = 0x01,
  CSW_PHASE_ERROR = 0x02
};

struct [[gnu::packed]] CBW
{
  uint32_t signature;
  uint32_t tag;
  uint32_t dataTransferLength;
  uint8_t flags;
  uint8_t lun;
  uint8_t cbLength;
  uint8_t cb[16];
};

struct [[gnu::packed]] CSW
{
  uint32_t signature;
  uint32_t tag;
  uint32_t dataResidue;
  uint8_t status;
};
/*----------------------------------------------------------------------------*/
struct [[gnu::packed]] InquiryData
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
};

struct [[gnu::packed]] ModeParameterHeader6
{
  uint8_t modeDataLength;
  uint8_t mediumType;
  uint8_t deviceSpecificParameter;
  uint8_t blockDescriptorLength;
};

struct [[gnu::packed]] ModeParameterHeader10
{
  uint16_t modeDataLength;
  uint8_t mediumType;
  uint8_t deviceSpecificParameter;
  uint8_t flags;
  uint8_t reserved;
  uint16_t blockDescriptorLength;
};

struct [[gnu::packed]] ModeSense6Command
{
  uint8_t operationCode;
  uint8_t flags;
  uint8_t pageCode;
  uint8_t subpageCode;
  uint8_t allocationLength;
  uint8_t control;
};

struct [[gnu::packed]] ModeSense10Command
{
  uint8_t operationCode;
  uint8_t flags;
  uint8_t pageCode;
  uint8_t subpageCode;
  uint8_t reserved[3];
  uint16_t allocationLength;
  uint8_t control;
};

struct [[gnu::packed]] PreventAllowMediumRemovalCommand
{
  uint8_t operationCode;
  uint8_t flags0;
  uint8_t reserved0[2];
  uint8_t flags1;
  uint8_t reserved1[4];
  uint8_t control;
};

struct [[gnu::packed]] Read6Command
{
  uint8_t operationCode;
  uint8_t logicalBlockAddress[3];
  uint8_t transferLength;
  uint8_t control;
};

struct [[gnu::packed]] Read10Command
{
  uint8_t operationCode;
  uint8_t flags;
  uint32_t logicalBlockAddress;
  uint8_t groupNumber;
  uint16_t transferLength;
  uint8_t control;
};

struct [[gnu::packed]] Read12Command
{
  uint8_t operationCode;
  uint8_t flags;
  uint32_t logicalBlockAddress;
  uint32_t transferLength;
  uint8_t groupNumber;
  uint8_t control;
};

struct [[gnu::packed]] ReadCapacity10Command
{
  uint8_t operationCode;
  uint8_t flags0;
  uint32_t lba;
  uint8_t reserved[2];
  uint8_t flags1;
  uint8_t control;
};

struct [[gnu::packed]] RequestSenseData
{
  uint8_t responseCode;
  uint8_t reserved;
  uint8_t flags;
  uint32_t information;
  uint8_t additionalSenseLength;
  uint32_t commandInformation;
  uint8_t additionalSenseCode;
  uint8_t additionalSenseCodeQualifier;
  uint8_t fieldReplacableUnitCode;
  uint8_t senseKeyData[3];
  uint8_t senseData[];
};

struct [[gnu::packed]] ReadCapacityData
{
  uint32_t lastLogicalBlockAddress;
  uint32_t blockLength;
};

struct [[gnu::packed]] Verify10Command
{
  uint8_t operationCode;
  uint8_t flags;
  uint32_t logicalBlockAddress;
  uint8_t groupNumber;
  uint16_t verificationLength;
  uint8_t control;
};

struct [[gnu::packed]] Write6Command
{
  uint8_t operationCode;
  uint8_t logicalBlockAddress[3];
  uint8_t transferLength;
  uint8_t control;
};

struct [[gnu::packed]] Write10Command
{
  uint8_t operationCode;
  uint8_t flags;
  uint32_t logicalBlockAddress;
  uint8_t groupNumber;
  uint16_t transferLength;
  uint8_t control;
};

struct [[gnu::packed]] Write12Command
{
  uint8_t operationCode;
  uint8_t flags;
  uint32_t logicalBlockAddress;
  uint32_t transferLength;
  uint8_t groupNumber;
  uint8_t control;
};
/*----------------------------------------------------------------------------*/
struct [[gnu::packed]] CapacityListHeader
{
  uint8_t reserved[3];
  uint8_t capacityListLength;
};

struct [[gnu::packed]] CapacityDescriptor
{
  uint32_t numberOfBlocks;
  uint8_t flags;
  uint8_t blockLength[3];
};

struct [[gnu::packed]] ReadFormatCapacitiesData
{
  struct CapacityListHeader header;
  struct CapacityDescriptor descriptors[];
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_MSC_DEFS_H_ */
