/*
 * msc.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/irq.h>
#include <halm/usb/msc.h>
#include <halm/usb/msc_datapath.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_trace.h>
#include <xcore/memory.h>
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
enum Flags
{
  FLAG_LOCKED     = 0x01,
  FLAG_READONLY   = 0x02,
  FLAG_ATTENTION  = 0x04,
  FLAG_FAILURE    = 0x08
};

enum State
{
  STATE_IDLE,
  STATE_TEST_UNIT_READY,
  STATE_REQUEST_SENSE,
  STATE_INQUIRY,
  STATE_MODE_SENSE,
  STATE_MEDIUM_REMOVAL,
  STATE_READ_FORMAT_CAPACITIES,
  STATE_READ_CAPACITY,
  STATE_READ_SETUP,
  STATE_READ,
  STATE_WRITE_SETUP,
  STATE_WRITE,
  STATE_VERIFY,
  STATE_ACK,
  STATE_ACK_STALL,
  STATE_COMPLETED,
  STATE_FAILURE,
  STATE_ERROR,
  STATE_SUSPEND
};

struct StateEntry
{
  enum State (*enter)(struct Msc *);
  enum State (*run)(struct Msc *);
};
/*----------------------------------------------------------------------------*/
static inline uint32_t fromBigEndian24(const uint8_t *, uint32_t);
static inline void toBigEndian24(uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static void dispatch(struct Msc *);

static enum State stateIdleEnter(struct Msc *);
static enum State stateIdleRun(struct Msc *);
static enum State stateTestUnitReadyEnter(struct Msc *);
static enum State stateRequestSenseEnter(struct Msc *);
static enum State stateInquiryEnter(struct Msc *);
static enum State stateMediumRemovalEnter(struct Msc *);
static enum State stateModeSenseEnter(struct Msc *);
static enum State stateReadWriteRun(struct Msc *);
static enum State stateReadFormatCapacitiesEnter(struct Msc *);
static enum State stateReadCapacityEnter(struct Msc *);
static enum State stateReadSetupEnter(struct Msc *);
static enum State stateReadEnter(struct Msc *);
static enum State stateVerifyEnter(struct Msc *);
static enum State stateWriteSetupEnter(struct Msc *);
static enum State stateWriteEnter(struct Msc *);
static enum State stateAckEnter(struct Msc *);
static enum State stateAckRun(struct Msc *);
static enum State stateAckStallRun(struct Msc *);
static enum State stateCompletedRun(struct Msc *);
static enum State stateFailureEnter(struct Msc *);
static enum State stateFailureRun(struct Msc *);
static enum State stateErrorEnter(struct Msc *);
static enum State stateErrorRun(struct Msc *);
static enum State stateSuspendEnter(struct Msc *);
/*----------------------------------------------------------------------------*/
static inline bool isInputDataValid(size_t, uint8_t);
static enum State sendResponse(struct Msc *, uint32_t, uint32_t,
    const void *, size_t);
/*----------------------------------------------------------------------------*/
static void deviceDescriptor(const void *, struct UsbDescriptor *, void *);
static void configDescriptor(const void *, struct UsbDescriptor *, void *);
static void interfaceDescriptor(const void *, struct UsbDescriptor *, void *);
static void rxEndpointDescriptor(const void *, struct UsbDescriptor *, void *);
static void txEndpointDescriptor(const void *, struct UsbDescriptor *, void *);
/*----------------------------------------------------------------------------*/
static enum Result handleClassRequest(struct Msc *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *);
static void resetBuffers(struct Msc *);
static void resetEndpoints(struct Msc *);
/*----------------------------------------------------------------------------*/
static enum Result driverInit(void *, const void *);
static void driverDeinit(void *);
static enum Result driverControl(void *, const struct UsbSetupPacket *,
    void *, uint16_t *, uint16_t);
static const UsbDescriptorFunctor *driverDescribe(const void *);
static void driverNotify(void *, unsigned int);
/*----------------------------------------------------------------------------*/
const struct UsbDriverClass * const Msc = &(const struct UsbDriverClass){
    .size = sizeof(struct Msc),
    .init = driverInit,
    .deinit = driverDeinit,

    .control = driverControl,
    .describe = driverDescribe,
    .notify = driverNotify
};
/*----------------------------------------------------------------------------*/
static const UsbDescriptorFunctor deviceDescriptorTable[] = {
    deviceDescriptor,
    configDescriptor,
    interfaceDescriptor,
    rxEndpointDescriptor,
    txEndpointDescriptor,
    NULL
};
/*----------------------------------------------------------------------------*/
static const struct StateEntry stateTable[] = {
    [STATE_IDLE]                    = {stateIdleEnter, stateIdleRun},
    [STATE_TEST_UNIT_READY]         = {stateTestUnitReadyEnter, NULL},
    [STATE_REQUEST_SENSE]           = {stateRequestSenseEnter, NULL},
    [STATE_INQUIRY]                 = {stateInquiryEnter, NULL},
    [STATE_MODE_SENSE]              = {stateModeSenseEnter, NULL},
    [STATE_MEDIUM_REMOVAL]          = {stateMediumRemovalEnter, NULL},
    [STATE_READ_FORMAT_CAPACITIES]  = {stateReadFormatCapacitiesEnter, NULL},
    [STATE_READ_CAPACITY]           = {stateReadCapacityEnter, NULL},
    [STATE_READ_SETUP]              = {stateReadSetupEnter, NULL},
    [STATE_READ]                    = {stateReadEnter, stateReadWriteRun},
    [STATE_WRITE_SETUP]             = {stateWriteSetupEnter, NULL},
    [STATE_WRITE]                   = {stateWriteEnter, stateReadWriteRun},
    [STATE_VERIFY]                  = {stateVerifyEnter, NULL},
    [STATE_ACK]                     = {stateAckEnter, stateAckRun},
    [STATE_ACK_STALL]               = {NULL, stateAckStallRun},
    [STATE_COMPLETED]               = {NULL, stateCompletedRun},
    [STATE_FAILURE]                 = {stateFailureEnter, stateFailureRun},
    [STATE_ERROR]                   = {stateErrorEnter, stateErrorRun},
    [STATE_SUSPEND]                 = {stateSuspendEnter, NULL}
};
/*----------------------------------------------------------------------------*/
static inline uint32_t fromBigEndian24(const uint8_t *input, uint32_t mask)
{
  return ((input[0] << 16) | (input[1] << 8) | input[2]) & mask;
}
/*----------------------------------------------------------------------------*/
static inline void toBigEndian24(uint8_t *output, uint32_t input)
{
  output[0] = (uint8_t)(input >> 16);
  output[1] = (uint8_t)(input >> 8);
  output[2] = (uint8_t)input;
}
/*----------------------------------------------------------------------------*/
static enum State stateIdleEnter(struct Msc *driver)
{
  memset(driver->buffer, 0, sizeof(struct CBW));

  if (datapathReceiveControl(driver->datapath, driver->buffer,
      sizeof(struct CBW)))
  {
    return STATE_IDLE;
  }
  else
    return STATE_SUSPEND;
}
/*----------------------------------------------------------------------------*/
static enum State stateIdleRun(struct Msc *driver)
{
  struct CBW * const cbw = driver->buffer;

  if (cbw->signature != CBW_SIGNATURE)
    return STATE_IDLE;
  if (!cbw->cbLength || cbw->cbLength > 16)
    return STATE_ERROR;

  driver->context.cbw.length = fromLittleEndian32(cbw->dataTransferLength);
  driver->context.cbw.tag = cbw->tag;
  driver->context.cbw.flags = cbw->flags;
  driver->context.cbw.lun = cbw->lun;
  memcpy(driver->context.cbw.cb, cbw->cb, sizeof(driver->context.cbw.cb));

  driver->context.left = cbw->dataTransferLength;

  if (driver->context.cbw.lun >= ARRAY_SIZE(driver->lun))
    return STATE_FAILURE;

  switch (driver->context.cbw.cb[0])
  {
    case SCSI_TEST_UNIT_READY:
      return STATE_TEST_UNIT_READY;

    case SCSI_REQUEST_SENSE:
      return STATE_REQUEST_SENSE;

    case SCSI_INQUIRY:
      return STATE_INQUIRY;

    case SCSI_MODE_SENSE6:
      return STATE_MODE_SENSE;

    case SCSI_MODE_SENSE10:
      return STATE_MODE_SENSE;

    case SCSI_MEDIUM_REMOVAL:
      return STATE_MEDIUM_REMOVAL;

    case SCSI_READ_FORMAT_CAPACITIES:
      return STATE_READ_FORMAT_CAPACITIES;

    case SCSI_READ_CAPACITY10:
      return STATE_READ_CAPACITY;

    case SCSI_READ6:
    case SCSI_READ10:
    case SCSI_READ12:
      return STATE_READ_SETUP;

    case SCSI_WRITE6:
    case SCSI_WRITE10:
    case SCSI_WRITE12:
      return STATE_WRITE_SETUP;

    case SCSI_VERIFY10:
      return STATE_VERIFY;

    default:
      driver->lun[driver->context.cbw.lun].sense = SCSI_SK_ILLEGAL_REQUEST;
      driver->lun[driver->context.cbw.lun].asc = SCSI_ASC_IR_INVALIDCOMMAND;

      usbTrace("msc: unsupported command 0x%02X", driver->context.cbw.cb[0]);
      return STATE_FAILURE;
  }
}
/*----------------------------------------------------------------------------*/
static enum State stateTestUnitReadyEnter(struct Msc *driver)
{
  if (!driver->context.cbw.length)
  {
    const size_t index = driver->context.cbw.lun;

    usbTrace("msc: test unit ready");

    if (!driver->lun[index].interface)
    {
      driver->lun[index].sense = SCSI_SK_NOT_READY;
      driver->lun[index].asc = SCSI_ASC_NR_MEDIUMNOTPRESENT;
      return STATE_FAILURE;
    }
    else if (driver->lun[index].flags & FLAG_ATTENTION)
    {
      driver->lun[index].flags &= ~FLAG_ATTENTION;

      driver->lun[index].sense = SCSI_SK_UNIT_ATTENTION;
      driver->lun[index].asc = SCSI_ASC_UA_READYTRANSITION;
      return STATE_FAILURE;
    }
    else
      return STATE_ACK;
  }
  else
    return STATE_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum State stateRequestSenseEnter(struct Msc *driver)
{
  if (!isInputDataValid(driver->context.cbw.length, driver->context.cbw.flags))
    return STATE_ERROR;

  struct RequestSenseData * const buffer = driver->buffer;
  const size_t index = driver->context.cbw.lun;

  memset(buffer, 0, sizeof(struct RequestSenseData));
  buffer->responseCode = 0x70;
  buffer->additionalSenseLength = sizeof(struct RequestSenseData) - 8;

  if (index >= ARRAY_SIZE(driver->lun))
  {
    buffer->flags = SCSI_SK_ILLEGAL_REQUEST;
    buffer->additionalSenseCode = (uint8_t)(SCSI_ASC_IR_INVALIDLUN >> 8);
    buffer->additionalSenseCodeQualifier = (uint8_t)SCSI_ASC_IR_INVALIDLUN;
  }
  else
  {
    buffer->flags = driver->lun[index].sense;
    buffer->additionalSenseCode = (uint8_t)(driver->lun[index].asc >> 8);
    buffer->additionalSenseCodeQualifier = (uint8_t)driver->lun[index].asc;

    /* Clear error */
    driver->lun[index].sense = SCSI_SK_NO_SENSE;
    driver->lun[index].asc = SCSI_ASC_NOSENSE;
  }

  usbTrace("msc: request sense 0x%06X", buffer->flags << 16
      | buffer->additionalSenseCode << 8
      | buffer->additionalSenseCodeQualifier);

  return sendResponse(driver, driver->context.cbw.tag,
      driver->context.cbw.length, buffer, sizeof(struct RequestSenseData));
}
/*----------------------------------------------------------------------------*/
static enum State stateInquiryEnter(struct Msc *driver)
{
  if (!isInputDataValid(driver->context.cbw.length, driver->context.cbw.flags))
    return STATE_ERROR;

  static const char version[] = "1.00";
  struct InquiryData * const buffer = driver->buffer;

  memset(buffer, 0, sizeof(struct InquiryData));

  buffer->peripheralDeviceType = PDT_DIRECT_ACCESS_BLOCK_DEVICE;
  buffer->version = 0;
  buffer->additionalLength = 32;

  buffer->flags0 = INQUIRY_FLAGS_0_RMB;
  /* Response Data Format should be set to a fixed value of 2 */
  buffer->flags1 = INQUIRY_FLAGS_1_RDF(2);
  buffer->flags2 = INQUIRY_FLAGS_2_SCCS;
  buffer->flags3 = 0x00;
  buffer->flags4 = 0x00;

  memset(buffer->vendorIdentification, ' ',
      sizeof(buffer->vendorIdentification));
  memset(buffer->productIdentification, ' ',
      sizeof(buffer->productIdentification));
  memcpy(buffer->productRevisionLevel, version,
      sizeof(buffer->productRevisionLevel));

  usbTrace("msc: inquiry");

  return sendResponse(driver, driver->context.cbw.tag,
      driver->context.cbw.length, buffer, sizeof(struct InquiryData));
}
/*----------------------------------------------------------------------------*/
static enum State stateMediumRemovalEnter(struct Msc *driver)
{
  const struct PreventAllowMediumRemovalCommand * const command =
      (const struct PreventAllowMediumRemovalCommand *)driver->context.cbw.cb;
  const size_t index = driver->context.cbw.lun;

  if (driver->lun[index].interface)
  {
    uint8_t flags = driver->lun[index].flags;

    if (MEDIUMREMOVAL_FLAGS_PREVENT_VALUE(command->flags1) == PAMR_LOCK)
      flags |= FLAG_LOCKED;
    else
      flags &= ~FLAG_LOCKED;

    if (flags != driver->lun[index].flags)
    {
      driver->lun[index].flags = flags;

      if (driver->callback != NULL)
        driver->callback(driver->callbackArgument);
    }

    usbTrace("msc: medium removal state %u", (flags & FLAG_LOCKED) != 0);
    return STATE_ACK;
  }
  else
  {
    driver->lun[index].sense = SCSI_SK_NOT_READY;
    driver->lun[index].asc = SCSI_ASC_NR_MEDIUMNOTPRESENT;
    return STATE_FAILURE;
  }
}
/*----------------------------------------------------------------------------*/
static enum State stateModeSenseEnter(struct Msc *driver)
{
  if (!isInputDataValid(driver->context.cbw.length, driver->context.cbw.flags))
    return STATE_ERROR;

  size_t length;

  if (driver->context.cbw.cb[0] == SCSI_MODE_SENSE6)
  {
    struct ModeParameterHeader6 * const buffer = driver->buffer;

    length = sizeof(struct ModeParameterHeader6);

    buffer->modeDataLength = 3;
    buffer->mediumType = 0;
    buffer->deviceSpecificParameter = 0;
    buffer->blockDescriptorLength = 0;

    usbTrace("msc: mode sense 6");
  }
  else
  {
    struct ModeParameterHeader10 * const buffer = driver->buffer;

    length = sizeof(struct ModeParameterHeader10);
    memset(buffer, 0, length); /* Clear reserved fields */

    buffer->modeDataLength = TO_BIG_ENDIAN_16(6);
    buffer->mediumType = 0;
    buffer->deviceSpecificParameter = 0;
    buffer->flags = 0;
    buffer->blockDescriptorLength = TO_BIG_ENDIAN_16(0);

    usbTrace("msc: mode sense 10");
  }

  return sendResponse(driver, driver->context.cbw.tag,
      driver->context.cbw.length, driver->buffer, length);
}
/*----------------------------------------------------------------------------*/
static enum State stateReadWriteRun(struct Msc *driver)
{
  /* Verify completion of the transfer */
  const enum Result status = datapathStatus(driver->datapath);

  switch (status)
  {
    case E_OK:
      return STATE_ACK;

    case E_ERROR:
      return STATE_SUSPEND;

    default:
    {
      const size_t index = driver->context.cbw.lun;

      if (driver->context.state == STATE_WRITE)
      {
        driver->lun[index].sense = SCSI_SK_MEDIUM_ERROR;
        driver->lun[index].asc = SCSI_ASC_ME_WRITEFAULT;
      }
      else
      {
        driver->lun[index].sense = SCSI_SK_MEDIUM_ERROR;
        driver->lun[index].asc = SCSI_ASC_ME_READERROR;
      }

      driver->lun[index].flags |= FLAG_FAILURE;
      if (driver->callback != NULL)
        driver->callback(driver->callbackArgument);

      return STATE_FAILURE;
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum State stateReadFormatCapacitiesEnter(struct Msc *driver)
{
  if (!isInputDataValid(driver->context.cbw.length, driver->context.cbw.flags))
    return STATE_ERROR;

  const size_t index = driver->context.cbw.lun;

  if (driver->lun[index].interface)
  {
    /* TODO MSC constants */
    struct ReadFormatCapacitiesData * const buffer = driver->buffer;

    memset(buffer->header.reserved, 0, sizeof(buffer->header.reserved));
    buffer->header.capacityListLength = 8;

    buffer->descriptors[0].numberOfBlocks =
        toBigEndian32(driver->lun[index].blocks);
    buffer->descriptors[0].flags = 0x02; /* Descriptor Type: Formatted Medium */
    toBigEndian24(buffer->descriptors[0].blockLength, driver->blockSize);

    usbTrace("msc: read format capacity, %"PRIu32" blocks",
        driver->lun[index].blocks);

    return sendResponse(driver,  driver->context.cbw.tag,
        driver->context.cbw.length, buffer,
        sizeof(struct ReadFormatCapacitiesData));
  }
  else
  {
    driver->lun[index].sense = SCSI_SK_NOT_READY;
    driver->lun[index].asc = SCSI_ASC_NR_MEDIUMNOTPRESENT;
    return STATE_FAILURE;
  }
}
/*----------------------------------------------------------------------------*/
static enum State stateReadCapacityEnter(struct Msc *driver)
{
  if (!isInputDataValid(driver->context.cbw.length, driver->context.cbw.flags))
    return STATE_ERROR;

  const struct ReadCapacity10Command * const command =
      (const struct ReadCapacity10Command *)driver->context.cbw.cb;
  const size_t index = driver->context.cbw.lun;

  if ((!(command->flags1 & READCAPACITY10_FLAGS_1_PMI) && command->lba)
      || (command->lba > driver->lun[index].blocks - 1))
  {
    driver->lun[index].sense = SCSI_SK_ILLEGAL_REQUEST;
    driver->lun[index].asc = SCSI_ASC_IR_INVALIDFIELDINCBA;
    return STATE_FAILURE;
  }

  if (driver->lun[index].interface)
  {
    struct ReadCapacityData * const buffer = driver->buffer;

    buffer->lastLogicalBlockAddress =
        toBigEndian32(driver->lun[index].blocks - 1);
    buffer->blockLength = toBigEndian32(driver->blockSize);

    usbTrace("msc: read capacity, %"PRIu32" blocks",
        driver->lun[index].blocks);

    return sendResponse(driver, driver->context.cbw.tag,
        driver->context.cbw.length, buffer,
        sizeof(struct ReadCapacityData));
  }
  else
  {
    driver->lun[index].sense = SCSI_SK_NOT_READY;
    driver->lun[index].asc = SCSI_ASC_NR_MEDIUMNOTPRESENT;
    return STATE_FAILURE;
  }
}
/*----------------------------------------------------------------------------*/
static enum State stateReadSetupEnter(struct Msc *driver)
{
  const size_t index = driver->context.cbw.lun;

  if (!driver->lun[index].interface)
  {
    driver->lun[index].sense = SCSI_SK_NOT_READY;
    driver->lun[index].asc = SCSI_ASC_NR_MEDIUMNOTPRESENT;
    return STATE_FAILURE;
  }

  uint32_t logicalBlockAddress = 0;
  uint32_t numberOfBlocks = 0;

  switch (driver->context.cbw.cb[0])
  {
    case SCSI_READ6:
    {
      const struct Read6Command * const command =
          (const struct Read6Command *)driver->context.cbw.cb;

      logicalBlockAddress =
          fromBigEndian24(command->logicalBlockAddress, READ6_LBA_MASK);
      numberOfBlocks = !command->transferLength ? 256 : command->transferLength;
      break;
    }

    case SCSI_READ10:
    {
      const struct Read10Command * const command =
          (const struct Read10Command *)driver->context.cbw.cb;

      logicalBlockAddress = fromBigEndian32(command->logicalBlockAddress);
      numberOfBlocks = fromBigEndian16(command->transferLength);
      break;
    }

    case SCSI_READ12:
    {
      const struct Read12Command * const command =
          (const struct Read12Command *)driver->context.cbw.cb;

      logicalBlockAddress = fromBigEndian32(command->logicalBlockAddress);
      numberOfBlocks = fromBigEndian32(command->transferLength);
      break;
    }
  }

  const uint32_t transferLength = numberOfBlocks * driver->blockSize;

  if (driver->context.cbw.length != transferLength || !transferLength)
  {
    driver->lun[index].sense = SCSI_SK_ILLEGAL_REQUEST;
    driver->lun[index].asc = SCSI_ASC_IR_INVALIDFIELDINCBA;
    return STATE_FAILURE;
  }
  if (logicalBlockAddress + numberOfBlocks > driver->lun[index].blocks)
  {
    driver->lun[index].sense = SCSI_SK_ILLEGAL_REQUEST;
    driver->lun[index].asc = SCSI_ASC_IR_LBAOUTOFRANGE;
    return STATE_FAILURE;
  }

  driver->context.position =
      (uint64_t)logicalBlockAddress * driver->blockSize;

  usbTrace("msc: read command, start block %"PRIu32", count %"PRIu32,
      (uint32_t)(driver->context.position / driver->blockSize),
      transferLength / driver->blockSize);

  return STATE_READ;
}
/*----------------------------------------------------------------------------*/
static enum State stateReadEnter(struct Msc *driver)
{
  const bool queued = datapathReadAndSendData(driver->datapath,
      driver->buffer, driver->bufferSize,
      driver->context.position, driver->context.left);

  if (!queued)
  {
    const size_t index = driver->context.cbw.lun;

    driver->lun[index].sense = SCSI_SK_MEDIUM_ERROR;
    driver->lun[index].asc = SCSI_ASC_ME_READERROR;

    driver->lun[index].flags |= FLAG_FAILURE;
    if (driver->callback != NULL)
      driver->callback(driver->callbackArgument);

    return STATE_FAILURE;
  }
  else
    return STATE_READ;
}
/*----------------------------------------------------------------------------*/
static enum State stateVerifyEnter(struct Msc *driver)
{
  const size_t index = driver->context.cbw.lun;

  if (!driver->lun[index].interface)
  {
    driver->lun[index].sense = SCSI_SK_NOT_READY;
    driver->lun[index].asc = SCSI_ASC_NR_MEDIUMNOTPRESENT;
    return STATE_FAILURE;
  }

  const struct Verify10Command * const command =
      (const struct Verify10Command *)driver->context.cbw.cb;

  if ((command->flags & VERIFY10_BYTCHK_MASK) == 0)
  {
    const uint32_t logicalBlockAddress =
        fromBigEndian32(command->logicalBlockAddress);
    const uint16_t numberOfBlocks =
        fromBigEndian16(command->verificationLength);

    usbTrace("msc: verify command, start block %"PRIu32", count %"PRIu16,
        logicalBlockAddress, numberOfBlocks);

    if (logicalBlockAddress + numberOfBlocks > driver->lun[index].blocks)
    {
      driver->lun[index].sense = SCSI_SK_ILLEGAL_REQUEST;
      driver->lun[index].asc = SCSI_ASC_IR_INVALIDCOMMAND;
      return STATE_FAILURE;
    }
    else
      return STATE_ACK;
  }
  else
  {
    usbTrace("msc: incorrect verify command flags 0x%02X", command->flags);
    return STATE_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum State stateWriteSetupEnter(struct Msc *driver)
{
  const size_t index = driver->context.cbw.lun;

  if (!driver->lun[index].interface)
  {
    driver->lun[index].sense = SCSI_SK_NOT_READY;
    driver->lun[index].asc = SCSI_ASC_NR_MEDIUMNOTPRESENT;
    return STATE_FAILURE;
  }

  uint32_t logicalBlockAddress = 0;
  uint32_t numberOfBlocks = 0;

  switch (driver->context.cbw.cb[0])
  {
    case SCSI_WRITE6:
    {
      const struct Write6Command * const command =
          (const struct Write6Command *)driver->context.cbw.cb;

      logicalBlockAddress =
          fromBigEndian24(command->logicalBlockAddress, 0x1FFFFF);
      numberOfBlocks = !command->transferLength ? 256 : command->transferLength;
      break;
    }

    case SCSI_WRITE10:
    {
      const struct Write10Command * const command =
          (const struct Write10Command *)driver->context.cbw.cb;

      logicalBlockAddress = fromBigEndian32(command->logicalBlockAddress);
      numberOfBlocks = fromBigEndian16(command->transferLength);
      break;
    }

    case SCSI_WRITE12:
    {
      const struct Write12Command * const command =
          (const struct Write12Command *)driver->context.cbw.cb;

      logicalBlockAddress = fromBigEndian32(command->logicalBlockAddress);
      numberOfBlocks = fromBigEndian32(command->transferLength);
      break;
    }
  }

  const uint32_t transferLength = numberOfBlocks * driver->blockSize;

  if (driver->context.cbw.length != transferLength || !transferLength)
  {
    driver->lun[index].sense = SCSI_SK_ILLEGAL_REQUEST;
    driver->lun[index].asc = SCSI_ASC_IR_INVALIDFIELDINCBA;
    return STATE_FAILURE;
  }
  if (logicalBlockAddress + numberOfBlocks > driver->lun[index].blocks)
  {
    driver->lun[index].sense = SCSI_SK_ILLEGAL_REQUEST;
    driver->lun[index].asc = SCSI_ASC_IR_INVALIDCOMMAND;
    return STATE_FAILURE;
  }

  driver->context.position =
      (uint64_t)logicalBlockAddress * driver->blockSize;

  usbTrace("msc: write command, start block %"PRIu32", count %"PRIu32,
      (uint32_t)(driver->context.position / driver->blockSize),
      transferLength / driver->blockSize);

  return STATE_WRITE;
}
/*----------------------------------------------------------------------------*/
static enum State stateWriteEnter(struct Msc *driver)
{
  const bool queued = datapathReceiveAndWriteData(driver->datapath,
      driver->buffer, driver->bufferSize,
      driver->context.position, driver->context.left);

  if (!queued)
  {
    const size_t index = driver->context.cbw.lun;

    driver->lun[index].sense = SCSI_SK_MEDIUM_ERROR;
    driver->lun[index].asc = SCSI_ASC_ME_WRITEFAULT;

    driver->lun[index].flags |= FLAG_FAILURE;
    if (driver->callback != NULL)
      driver->callback(driver->callbackArgument);

    return STATE_FAILURE;
  }
  else
    return STATE_WRITE;
}
/*----------------------------------------------------------------------------*/
static enum State stateAckEnter(struct Msc *driver)
{
  if (datapathSendStatus(driver->datapath, driver->context.cbw.tag,
      0, CSW_CMD_PASSED))
  {
    return STATE_ACK;
  }
  else
    return STATE_SUSPEND;
}
/*----------------------------------------------------------------------------*/
static enum State stateAckRun(struct Msc *driver)
{
  /* Verify completion of the transfer */
  return datapathStatus(driver->datapath) == E_OK ? STATE_IDLE : STATE_SUSPEND;
}
/*----------------------------------------------------------------------------*/
static enum State stateAckStallRun(struct Msc *driver)
{
  /* Verify completion of the transfer */
  if (datapathStatus(driver->datapath) != E_OK)
    return STATE_SUSPEND;

  usbTrace("msc: IN stalled, sent %"PRIu32", requested %"PRIu32,
      driver->context.cbw.length - driver->context.left,
      driver->context.cbw.length);

  usbEpSetStalled(driver->txEp, true);

  if (datapathSendStatus(driver->datapath, driver->context.cbw.tag,
      driver->context.left, CSW_CMD_PASSED))
  {
    return STATE_COMPLETED;
  }
  else
    return STATE_SUSPEND;
}
/*----------------------------------------------------------------------------*/
static enum State stateCompletedRun(struct Msc *driver)
{
  /* Verify completion of the transfer */
  return datapathStatus(driver->datapath) == E_OK ? STATE_IDLE : STATE_SUSPEND;
}
/*----------------------------------------------------------------------------*/
static enum State stateFailureEnter(struct Msc *driver)
{
  usbEpSetStalled(driver->txEp, true);

  if (datapathSendStatus(driver->datapath, driver->context.cbw.tag,
      driver->context.left, CSW_CMD_FAILED))
  {
    return STATE_FAILURE;
  }
  else
    return STATE_SUSPEND;
}
/*----------------------------------------------------------------------------*/
static enum State stateFailureRun(struct Msc *driver)
{
  /* Verify completion of the transfer */
  return datapathStatus(driver->datapath) == E_OK ? STATE_IDLE : STATE_SUSPEND;
}
/*----------------------------------------------------------------------------*/
static enum State stateErrorEnter(struct Msc *driver)
{
  if (datapathSendStatus(driver->datapath, driver->context.cbw.tag,
      driver->context.cbw.length, CSW_PHASE_ERROR))
  {
    return STATE_ERROR;
  }
  else
    return STATE_SUSPEND;
}
/*----------------------------------------------------------------------------*/
static enum State stateErrorRun(struct Msc *)
{
  return STATE_SUSPEND;
}
/*----------------------------------------------------------------------------*/
static enum State stateSuspendEnter(struct Msc *driver)
{
  usbTrace("msc: suspended");

  usbEpSetStalled(driver->rxEp, true);
  usbEpSetStalled(driver->txEp, true);
  return STATE_SUSPEND;
}
/*----------------------------------------------------------------------------*/
static void dispatch(struct Msc *driver)
{
  enum State current = driver->context.state;
  enum State previous = current;

  if (stateTable[current].run != NULL)
    current = stateTable[current].run(driver);

  while (current != previous)
  {
    if (current == STATE_FAILURE)
      usbTrace("msc: failure in state %u", previous);
    if (current == STATE_ERROR)
      usbTrace("msc: critical error in state %u", previous);

    previous = current;

    if (stateTable[current].enter != NULL)
      current = stateTable[current].enter(driver);
  }

  driver->context.state = current;
}
/*----------------------------------------------------------------------------*/
static inline bool isInputDataValid(size_t length, uint8_t flags)
{
  return length && (flags & CBW_FLAG_DIRECTION_TO_HOST);
}
/*----------------------------------------------------------------------------*/
static enum State sendResponse(struct Msc *driver, uint32_t tag,
    uint32_t residue, const void *buffer, size_t length)
{
  assert(buffer != NULL);
  assert(length);

  const size_t dataLength = MIN(length, residue);
  const bool dataFit = dataLength == length;

  if (dataFit)
  {
    if (datapathSendResponseAndStatus(driver->datapath, buffer, length,
        tag, 0, CSW_CMD_PASSED))
    {
      return STATE_COMPLETED;
    }
    else
      return STATE_SUSPEND;
  }
  else
  {
    if (datapathSendResponse(driver->datapath, buffer, length))
      return STATE_ACK_STALL;
    else
      return STATE_SUSPEND;
  }
}
/*----------------------------------------------------------------------------*/
static void deviceDescriptor(const void *, struct UsbDescriptor *header,
    void *payload)
{
  header->length = sizeof(struct UsbDeviceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_DEVICE;

  if (payload != NULL)
  {
    struct UsbDeviceDescriptor * const descriptor = payload;

    descriptor->usb = TO_LITTLE_ENDIAN_16(0x0200);
    descriptor->deviceClass = USB_CLASS_PER_INTERFACE;
    descriptor->maxPacketSize = TO_LITTLE_ENDIAN_16(MSC_CONTROL_EP_SIZE);
    descriptor->device = TO_LITTLE_ENDIAN_16(0x0100);
    descriptor->numConfigurations = 1;
  }
}
/*----------------------------------------------------------------------------*/
static void configDescriptor(const void *, struct UsbDescriptor *header,
    void *payload)
{
  header->length = sizeof(struct UsbConfigurationDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CONFIGURATION;

  if (payload != NULL)
  {
    struct UsbConfigurationDescriptor * const descriptor = payload;

    descriptor->totalLength = TO_LITTLE_ENDIAN_16(
        sizeof(struct UsbConfigurationDescriptor)
        + sizeof(struct UsbInterfaceDescriptor)
        + sizeof(struct UsbEndpointDescriptor) * 2);
    descriptor->numInterfaces = 1;
    descriptor->configurationValue = 1;
  }
}
/*----------------------------------------------------------------------------*/
static void interfaceDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct Msc * const driver = object;

  header->length = sizeof(struct UsbInterfaceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_INTERFACE;

  if (payload != NULL)
  {
    struct UsbInterfaceDescriptor * const descriptor = payload;

    descriptor->interfaceNumber = driver->interfaceIndex;
    descriptor->numEndpoints = 2;
    descriptor->interfaceClass = USB_CLASS_MASS_STORAGE;
    descriptor->interfaceSubClass = MSC_SUBCLASS_SCSI;
    descriptor->interfaceProtocol = MSC_PROTOCOL_BBB;
  }
}
/*----------------------------------------------------------------------------*/
static void rxEndpointDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct Msc * const driver = object;

  header->length = sizeof(struct UsbEndpointDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_ENDPOINT;

  if (payload != NULL)
  {
    struct UsbEndpointDescriptor * const descriptor = payload;

    descriptor->endpointAddress = driver->endpoints.rx;
    descriptor->attributes = ENDPOINT_DESCRIPTOR_TYPE(ENDPOINT_TYPE_BULK);
    descriptor->maxPacketSize = toLittleEndian16(driver->packetSize);
    descriptor->interval = 0;
  }
}
/*----------------------------------------------------------------------------*/
static void txEndpointDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct Msc * const driver = object;

  header->length = sizeof(struct UsbEndpointDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_ENDPOINT;

  if (payload != NULL)
  {
    struct UsbEndpointDescriptor * const descriptor = payload;

    descriptor->endpointAddress = driver->endpoints.tx;
    descriptor->attributes = ENDPOINT_DESCRIPTOR_TYPE(ENDPOINT_TYPE_BULK);
    descriptor->maxPacketSize = toLittleEndian16(driver->packetSize);
    descriptor->interval = 0;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result handleClassRequest(struct Msc *driver,
    const struct UsbSetupPacket *packet, uint8_t *response,
    uint16_t *responseLength)
{
  if (packet->index != driver->interfaceIndex)
    return E_INVALID;

  switch (packet->request)
  {
    case MSC_REQUEST_RESET:
      usbTrace("msc at %u: reset requested", driver->interfaceIndex);

      resetBuffers(driver);
      return E_OK;

    case MSC_REQUEST_GET_MAX_LUN:
      usbTrace("msc at %u: max LUN requested", driver->interfaceIndex);

      response[0] = ARRAY_SIZE(driver->lun) - 1;
      *responseLength = 1;
      return E_OK;

    default:
      usbTrace("msc at %u: unknown request %02X",
          driver->interfaceIndex, packet->request);
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static void resetBuffers(struct Msc *driver)
{
  /* Return queued requests to pools */
  usbEpClear(driver->rxEp);
  usbEpClear(driver->txEp);

  /* Enqueue buffer for first CBW and reset state machine */
  memset(driver->buffer, 0, sizeof(struct CBW));

  if (datapathReceiveControl(driver->datapath, driver->buffer,
      sizeof(struct CBW)))
  {
    driver->context.state = STATE_IDLE;
  }
  else
    driver->context.state = STATE_SUSPEND; /* Unrecoverable error */
}
/*----------------------------------------------------------------------------*/
static void resetEndpoints(struct Msc *driver)
{
  usbEpEnable(driver->rxEp, ENDPOINT_TYPE_BULK, driver->packetSize);
  usbEpEnable(driver->txEp, ENDPOINT_TYPE_BULK, driver->packetSize);
}
/*----------------------------------------------------------------------------*/
static enum Result driverInit(void *object, const void *configBase)
{
  const struct MscConfig * const config = configBase;
  assert(config != NULL);
  assert(config->device != NULL);
  assert(config->size && !(config->size & (MSC_BLOCK_SIZE - 1)));

  struct Msc * const driver = object;

  driver->callback = NULL;
  driver->bufferSize = config->size;
  driver->device = config->device;
  driver->blockSize = MSC_BLOCK_SIZE;
  driver->packetSize = MSC_DATA_EP_SIZE;
  driver->endpoints.rx = config->endpoints.rx;
  driver->endpoints.tx = config->endpoints.tx;

  if (config->arena == NULL)
  {
    driver->buffer = malloc(driver->bufferSize);
    if (driver->buffer == NULL)
      return E_MEMORY;
    driver->preallocated = false;
  }
  else
  {
    driver->buffer = config->arena;
    driver->preallocated = true;
  }

  for (size_t index = 0; index < ARRAY_SIZE(driver->lun); ++index)
    mscDetachUnit(driver, index);

  /* Initialize context, suspend state machine */
  memset(&driver->context.cbw, 0, sizeof(driver->context.cbw));
  driver->context.state = STATE_SUSPEND;

  driver->rxEp = usbDevCreateEndpoint(config->device, config->endpoints.rx);
  if (driver->rxEp == NULL)
    return E_ERROR;
  driver->txEp = usbDevCreateEndpoint(config->device, config->endpoints.tx);
  if (driver->txEp == NULL)
    return E_ERROR;

  driver->datapath = malloc(sizeof(struct MscQueryHandler));
  if (driver->datapath == NULL)
    return E_MEMORY;

  const enum Result res = datapathInit(driver->datapath, driver, dispatch);
  if (res != E_OK)
    return res;

  driver->interfaceIndex = usbDevGetInterface(driver->device);
  return usbDevBind(driver->device, driver);
}
/*----------------------------------------------------------------------------*/
static void driverDeinit(void *object)
{
  struct Msc * const driver = object;

  usbDevUnbind(driver->device, driver);

  /* Clear endpoint queues */
  usbEpClear(driver->txEp);
  usbEpClear(driver->rxEp);

  /* Delete query handler */
  datapathDeinit(driver->datapath);

  /* Delete endpoints */
  deinit(driver->txEp);
  deinit(driver->rxEp);

  if (!driver->preallocated)
    free(driver->buffer);
}
/*----------------------------------------------------------------------------*/
static enum Result driverControl(void *object,
    const struct UsbSetupPacket *packet, void *buffer, uint16_t *responseLength,
    uint16_t)
{
  if (REQUEST_TYPE_VALUE(packet->requestType) == REQUEST_TYPE_CLASS)
    return handleClassRequest(object, packet, buffer, responseLength);
  else
    return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static const UsbDescriptorFunctor *driverDescribe(const void *)
{
  return deviceDescriptorTable;
}
/*----------------------------------------------------------------------------*/
static void driverNotify(void *object, unsigned int event)
{
  struct Msc * const driver = object;

#ifdef CONFIG_USB_DEVICE_HS
  if (event == USB_DEVICE_EVENT_PORT_CHANGE)
  {
    const enum UsbSpeed speed = usbDevGetSpeed(driver->device);

    if (speed == USB_HS)
      driver->packetSize = TO_LITTLE_ENDIAN_16(MSC_DATA_EP_SIZE_HS);
    else
      driver->packetSize = TO_LITTLE_ENDIAN_16(MSC_DATA_EP_SIZE);

    usbTrace("msc: current speed is %s", speed == USB_HS ? "HS" : "FS");
  }
#endif

  if (event == USB_DEVICE_EVENT_RESET)
  {
    resetEndpoints(driver);
    resetBuffers(driver);

    usbTrace("msc: reset completed");
  }
}
/*----------------------------------------------------------------------------*/
enum Result mscAttachUnit(struct Msc *driver, uint8_t index, void *interface)
{
  assert(index < ARRAY_SIZE(driver->lun));
  assert(interface != NULL);

  uint64_t capacity;
  const enum Result res = ifGetParam(interface, IF_SIZE_64, &capacity);

  if (res == E_OK)
  {
    const IrqState state = irqSave();

    driver->lun[index].interface = interface;
    driver->lun[index].blocks = capacity / driver->blockSize;
    driver->lun[index].sense = SCSI_SK_NO_SENSE;
    driver->lun[index].asc = SCSI_ASC_NOSENSE;
    driver->lun[index].flags = FLAG_ATTENTION;

    irqRestore(state);
  }

  return res;
}
/*----------------------------------------------------------------------------*/
void mscDetachUnit(struct Msc *driver, uint8_t index)
{
  assert(index < ARRAY_SIZE(driver->lun));

  const IrqState state = irqSave();

  driver->lun[index].interface = NULL;
  driver->lun[index].blocks = 0;
  driver->lun[index].sense = SCSI_SK_NO_SENSE;
  driver->lun[index].asc = SCSI_ASC_NOSENSE;
  driver->lun[index].flags = 0;

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
bool mscIsUnitFailed(const struct Msc *driver, uint8_t index)
{
  assert(index < ARRAY_SIZE(driver->lun));
  return (driver->lun[index].flags & FLAG_FAILURE) != 0;
}
/*----------------------------------------------------------------------------*/
bool mscIsUnitLocked(const struct Msc *driver, uint8_t index)
{
  assert(index < ARRAY_SIZE(driver->lun));
  return (driver->lun[index].flags & FLAG_LOCKED) != 0;
}
/*----------------------------------------------------------------------------*/
void mscSetCallback(struct Msc *driver, void (*callback)(void *),
    void *argument)
{
  driver->callbackArgument = argument;
  driver->callback = callback;
}
