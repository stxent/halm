/*
 * msc.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <irq.h>
#include <memory.h>
#include <usb/composite_device.h>
#include <usb/msc.h>
#include <usb/msc_defs.h>
#include <usb/usb_trace.h>
/*----------------------------------------------------------------------------*/
enum state
{
  STATE_IDLE,
  STATE_TEST_UNIT_READY,
  STATE_REQUEST_SENSE,
  STATE_INQUIRY,
  STATE_MODE_SENSE,
  STATE_READ_CAPACITIES,
  STATE_READ_CAPACITY,
  STATE_READ_SETUP,
  STATE_READ,
  STATE_READ_WAIT,
  STATE_WRITE_SETUP,
  STATE_WRITE,
  STATE_WRITE_WAIT,
  STATE_VERIFY,
  STATE_COMPLETED,
  STATE_FAILURE,
  STATE_ERROR,
  STATE_SUSPEND
};
/*----------------------------------------------------------------------------*/
struct MscControlRequest
{
  struct UsbRequest base;

  uint8_t payload[MSC_DATA_EP_SIZE];
};
/*----------------------------------------------------------------------------*/
struct PrivateData
{
  struct UsbRequest rxRequests[RX_QUEUE_SIZE];
  struct UsbRequest txRequests[TX_QUEUE_SIZE];
  struct MscControlRequest controlRequests[CONTROL_QUEUE_SIZE];

  struct UsbEndpointDescriptor endpointDescriptors[2];

#ifdef CONFIG_USB_DEVICE_COMPOSITE
  struct UsbInterfaceAssociationDescriptor associationDescriptor;
  struct UsbInterfaceDescriptor interfaceDescriptor;
#endif

  struct
  {
    /* Current position in the storage */
    uint64_t position;
    /* Bytes left */
    uint32_t left;

    /* Chunks in the transfer queue */
    size_t queuedChunks;
    /* Total chunks in the transfer */
    size_t totalChunks;
    /* Current position in the buffer */
    uintptr_t bufferPosition;

    /* Pending command */
    struct CBW cbw;
  } context;
};
/*----------------------------------------------------------------------------*/
struct StateEntry
{
  enum state (*enter)(struct Msc *);
  enum state (*run)(struct Msc *);
};
/*----------------------------------------------------------------------------*/
static enum state stateIdleEnter(struct Msc *);
static enum state stateIdleRun(struct Msc *);
static enum state stateTestUnitReadyEnter(struct Msc *);
static enum state stateRequestSenseEnter(struct Msc *);
static enum state stateInquiryEnter(struct Msc *);
static enum state stateModeSenseEnter(struct Msc *);
static enum state stateReadCapacitiesEnter(struct Msc *);
static enum state stateReadCapacityEnter(struct Msc *);
static enum state stateReadSetupEnter(struct Msc *);
static enum state stateReadEnter(struct Msc *);
static enum state stateReadRun(struct Msc *);
static enum state stateReadWaitRun(struct Msc *);
static enum state stateWriteSetupEnter(struct Msc *);
static enum state stateWriteEnter(struct Msc *);
static enum state stateWriteRun(struct Msc *);
static enum state stateWriteWaitRun(struct Msc *);
static enum state stateVerifyEnter(struct Msc *);
static enum state stateVerifyRun(struct Msc *);
static enum state stateCompletedEnter(struct Msc *);
static enum state stateCompletedRun(struct Msc *);
static enum state stateFailureEnter(struct Msc *);
static enum state stateFailureRun(struct Msc *);
static enum state stateErrorEnter(struct Msc *);
static enum state stateErrorRun(struct Msc *);
static enum state stateSuspendEnter(struct Msc *);

static void runStateMachine(struct Msc *);
/*----------------------------------------------------------------------------*/
static void mscCommandReceived(void *, struct UsbRequest *,
    enum usbRequestStatus);
static void mscDataReceived(void *, struct UsbRequest *, enum usbRequestStatus);
static void mscDataReceivedQuietly(void *, struct UsbRequest *,
    enum usbRequestStatus);
static void mscDataSent(void *, struct UsbRequest *, enum usbRequestStatus);
static void mscDataSentQuietly(void *, struct UsbRequest *,
    enum usbRequestStatus);
static void mscStatusSent(void *, struct UsbRequest *, enum usbRequestStatus);
/*----------------------------------------------------------------------------*/
static inline size_t calcNextTransferLength(const struct Msc *, uint32_t);
static enum result enqueueControlRequest(struct Msc *);
static enum result enqueueDataRx(struct Msc *, void *, size_t);
static enum result enqueueDataTx(struct Msc *, const void *, size_t);
static bool isInputDataValid(const struct CBW *);
static void prepareDataRx(struct Msc *, struct UsbRequest *, bool);
static void prepareDataTx(struct Msc *, struct UsbRequest *, bool);
static enum result sendBuffer(struct Msc *, const void *, size_t, bool);
static enum result sendResponse(struct Msc *, const struct CBW *,
    const void *, size_t);
static enum result sendStatus(struct Msc *, uint32_t, uint32_t, uint8_t);
static enum result storageRead(struct Msc *);
static enum result storageWrite(struct Msc *);

static void storageCallback(void *);
/*----------------------------------------------------------------------------*/
static enum result buildDescriptors(struct Msc *, struct PrivateData *,
    const struct MscConfig *);
static enum result descriptorEraseWrapper(void *, const void *);
static enum result handleRequest(struct Msc *,
    const struct UsbSetupPacket *, const uint8_t *, uint16_t, uint8_t *,
    uint16_t *, uint16_t);
static enum result initPrivateData(struct Msc *, const struct MscConfig *);
static enum result initRequestQueues(struct Msc *, struct PrivateData *);
static enum result iterateOverDescriptors(const struct Msc *,
    struct PrivateData *, enum result (*)(void *, const void *));
static void freePrivateData(struct Msc *);
static void resetBuffers(struct Msc *);
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *, const void *);
static void driverDeinit(void *);
static enum result driverConfigure(void *, const struct UsbSetupPacket *,
    const uint8_t *, uint16_t, uint8_t *, uint16_t *, uint16_t);
static void driverEvent(void *, unsigned int);
/*----------------------------------------------------------------------------*/
static const struct UsbDriverClass driverTable = {
    .size = sizeof(struct Msc),
    .init = driverInit,
    .deinit = driverDeinit,

    .configure = driverConfigure,
    .event = driverEvent
};
/*----------------------------------------------------------------------------*/
const struct UsbDriverClass * const Msc = &driverTable;
/*----------------------------------------------------------------------------*/
static const struct StateEntry stateTable[] = {
    [STATE_IDLE]            = {stateIdleEnter, stateIdleRun},
    [STATE_TEST_UNIT_READY] = {stateTestUnitReadyEnter, 0},
    [STATE_REQUEST_SENSE]   = {stateRequestSenseEnter, 0},
    [STATE_INQUIRY]         = {stateInquiryEnter, 0},
    [STATE_MODE_SENSE]      = {stateModeSenseEnter, 0},
    [STATE_READ_CAPACITIES] = {stateReadCapacitiesEnter, 0},
    [STATE_READ_CAPACITY]   = {stateReadCapacityEnter, 0},
    [STATE_READ_SETUP]      = {stateReadSetupEnter, 0},
    [STATE_READ]            = {stateReadEnter, stateReadRun},
    [STATE_READ_WAIT]       = {0, stateReadWaitRun},
    [STATE_WRITE_SETUP]     = {stateWriteSetupEnter, 0},
    [STATE_WRITE]           = {stateWriteEnter, stateWriteRun},
    [STATE_WRITE_WAIT]      = {0, stateWriteWaitRun},
    [STATE_VERIFY]          = {stateVerifyEnter, stateVerifyRun},
    [STATE_COMPLETED]       = {stateCompletedEnter, stateCompletedRun},
    [STATE_FAILURE]         = {stateFailureEnter, stateFailureRun},
    [STATE_ERROR]           = {stateErrorEnter, stateErrorRun},
    [STATE_SUSPEND]         = {stateSuspendEnter, 0}
};
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_USB_DEVICE_COMPOSITE
static const struct UsbDeviceDescriptor deviceDescriptor = {
    .length             = sizeof(struct UsbDeviceDescriptor),
    .descriptorType     = DESCRIPTOR_TYPE_DEVICE,
    .usb                = TO_LITTLE_ENDIAN_16(0x0200),
    .deviceClass        = USB_CLASS_PER_INTERFACE,
    .deviceSubClass     = 0x00, /* Reserved value */
    .deviceProtocol     = 0x00, /* No class specific protocol */
    .maxPacketSize      = MSC_CONTROL_EP_SIZE,
    .idVendor           = TO_LITTLE_ENDIAN_16(CONFIG_USB_DEVICE_VENDOR_ID),
    .idProduct          = TO_LITTLE_ENDIAN_16(CONFIG_USB_DEVICE_PRODUCT_ID),
    .device             = TO_LITTLE_ENDIAN_16(0x0100),
    .manufacturer       = 0,
    .product            = 0,
    .serialNumber       = 0,
    .numConfigurations  = 1
};

static const struct UsbConfigurationDescriptor configDescriptor = {
    .length             = sizeof(struct UsbConfigurationDescriptor),
    .descriptorType     = DESCRIPTOR_TYPE_CONFIGURATION,
    .totalLength        = TO_LITTLE_ENDIAN_16(
        sizeof(struct UsbConfigurationDescriptor)
        + sizeof(struct UsbInterfaceDescriptor)
        + sizeof(struct UsbEndpointDescriptor) * 2),
    .numInterfaces      = 1,
    .configurationValue = 1,
    .configuration      = 0, /* No configuration name */
    .attributes         = CONFIGURATION_DESCRIPTOR_DEFAULT
        | CONFIGURATION_DESCRIPTOR_SELF_POWERED,
    .maxPower           = ((CONFIG_USB_DEVICE_CURRENT + 1) >> 1)
};

static const struct UsbInterfaceDescriptor interfaceDescriptor = {
    .length             = sizeof(struct UsbInterfaceDescriptor),
    .descriptorType     = DESCRIPTOR_TYPE_INTERFACE,
    .interfaceNumber    = 0,
    .alternateSettings  = 0,
    .numEndpoints       = 2,
    .interfaceClass     = USB_CLASS_MASS_STORAGE,
    .interfaceSubClass  = MSC_SUBCLASS_SCSI,
    .interfaceProtocol  = MSC_PROTOCOL_BBB,
    .interface          = 0 /* No interface name */
};
#endif
/*----------------------------------------------------------------------------*/
static enum state stateIdleEnter(struct Msc *driver)
{
  struct PrivateData * const privateData = driver->privateData;

  if (enqueueControlRequest(driver) == E_OK)
  {
    memset(&privateData->context.cbw, 0, sizeof(privateData->context.cbw));
    return STATE_IDLE;
  }
  else
    return STATE_SUSPEND;
}
/*----------------------------------------------------------------------------*/
static enum state stateIdleRun(struct Msc *driver)
{
  struct PrivateData * const privateData = driver->privateData;
  const struct CBW * const cbw = &privateData->context.cbw;

  if (cbw->signature != CBW_SIGNATURE)
    return STATE_IDLE;

  if (cbw->lun != 0 || !cbw->cbLength || cbw->cbLength > 16) //FIXME LUN
    return STATE_ERROR;

  privateData->context.left = cbw->dataTransferLength;

  switch (cbw->cb[0])
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

    case SCSI_READ_FORMAT_CAPACITIES:
      return STATE_READ_CAPACITIES;

    case SCSI_READ_CAPACITY:
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
      usbTrace("msc: unsupported command 0x%02X", cbw->cb[0]);
      return STATE_FAILURE;
  }
}
/*----------------------------------------------------------------------------*/
static enum state stateTestUnitReadyEnter(struct Msc *driver)
{
  const struct PrivateData * const privateData = driver->privateData;
  const struct CBW * const cbw = &privateData->context.cbw;

  if (!cbw->dataTransferLength)
  {
    usbTrace("msc: test unit ready");
    return STATE_COMPLETED;
  }
  else
    return STATE_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum state stateRequestSenseEnter(struct Msc *driver)
{
  struct PrivateData * const privateData = driver->privateData;
  const struct CBW * const cbw = &privateData->context.cbw;

  if (!isInputDataValid(cbw))
    return STATE_ERROR;

  struct RequestSenseData * const buffer =
      (struct RequestSenseData *)driver->buffer;

  memset(buffer, 0, sizeof(*buffer));
  buffer->responseCode = 0x70;
  buffer->flags = 0x02;
  buffer->additionalSenseLength = 0x0A;
  buffer->additionalSenseCode = 0x30;
  buffer->additionalSenseCodeQualifier = 0x01;

  usbTrace("msc: request sense");

  if (sendResponse(driver, cbw, buffer, sizeof(*buffer)) == E_OK)
    return STATE_COMPLETED;
  else
    return STATE_SUSPEND;
}
/*----------------------------------------------------------------------------*/
static enum state stateInquiryEnter(struct Msc *driver)
{
  struct PrivateData * const privateData = driver->privateData;
  const struct CBW * const cbw = &privateData->context.cbw;

  if (!isInputDataValid(cbw))
    return STATE_ERROR;

  struct InquiryData * const buffer = (struct InquiryData *)driver->buffer;

  memset(buffer, 0, sizeof(*buffer));
  buffer->peripheralDeviceType = PDT_DIRECT_ACCESS_BLOCK_DEVICE;
  buffer->flags = 0x80; /* FIXME Removable Medium */
  buffer->responseDataFormat = 0x01;

  buffer->additionalLength = 31;
  buffer->version = 0;
  memset(buffer->vendorInformation, ' ', sizeof(buffer->vendorInformation));
  memset(buffer->productIdentification, ' ',
      sizeof(buffer->productIdentification));
  memcpy(buffer->productRevisionLevel,
      (const uint8_t []){'1', '.', '0', '0'}, 4);

  usbTrace("msc: inquiry");

  if (sendResponse(driver, cbw, buffer, sizeof(*buffer)) == E_OK)
    return STATE_COMPLETED;
  else
    return STATE_SUSPEND;
}
/*----------------------------------------------------------------------------*/
static enum state stateModeSenseEnter(struct Msc *driver)
{
  struct PrivateData * const privateData = driver->privateData;
  const struct CBW * const cbw = &privateData->context.cbw;

  if (!isInputDataValid(cbw))
    return STATE_ERROR;

  if (cbw->cb[0] == SCSI_MODE_SENSE6)
  {
    uint8_t BulkBuf[4];

    BulkBuf[0] = 0x03;
    BulkBuf[1] = 0x00;
    BulkBuf[2] = 0x00;
    BulkBuf[3] = 0x00;

    usbTrace("msc: mode sense 6");

    memcpy(driver->buffer, BulkBuf, sizeof(BulkBuf));

    if (sendResponse(driver, cbw, driver->buffer, sizeof(BulkBuf)) == E_OK)
      return STATE_COMPLETED;
    else
      return STATE_SUSPEND;
  }
  else
  {
    uint8_t BulkBuf[8];

    BulkBuf[0] = 0x00;
    BulkBuf[1] = 0x06;
    BulkBuf[2] = 0x00;
    BulkBuf[3] = 0x00;
    BulkBuf[4] = 0x00;
    BulkBuf[5] = 0x00;
    BulkBuf[6] = 0x00;
    BulkBuf[7] = 0x00;

    usbTrace("msc: mode sense 10");

    memcpy(driver->buffer, BulkBuf, sizeof(BulkBuf));

    if (sendResponse(driver, cbw, driver->buffer, sizeof(BulkBuf)) == E_OK)
      return STATE_COMPLETED;
    else
      return STATE_SUSPEND;
  }
}
/*----------------------------------------------------------------------------*/
static enum state stateReadCapacitiesEnter(struct Msc *driver)
{
  struct PrivateData * const privateData = driver->privateData;
  const struct CBW * const cbw = &privateData->context.cbw;

  if (!isInputDataValid(cbw))
    return STATE_ERROR;

  uint8_t BulkBuf[12];

  BulkBuf[0] = 0x00;
  BulkBuf[1] = 0x00;
  BulkBuf[2] = 0x00;
  BulkBuf[3] = 0x08; /* Capacity List Length */

  /* Block Count */ //XXX
  BulkBuf[4] = (driver->blockCount >> 24) & 0xFF;
  BulkBuf[5] = (driver->blockCount >> 16) & 0xFF;
  BulkBuf[6] = (driver->blockCount >> 8) & 0xFF;
  BulkBuf[7] = (driver->blockCount >> 0) & 0xFF;

  /* Block Length */
  BulkBuf[8] = 0x02; /* Descriptor Code: Formatted Media */
  BulkBuf[9] = (driver->blockLength >> 16) & 0xFF;
  BulkBuf[10] = (driver->blockLength >> 8) & 0xFF;
  BulkBuf[11] = (driver->blockLength >> 0) & 0xFF;

  usbTrace("msc: read format capacity");

  memcpy(driver->buffer, BulkBuf, sizeof(BulkBuf));

  if (sendResponse(driver, cbw, driver->buffer, sizeof(BulkBuf)) == E_OK)
    return STATE_COMPLETED;
  else
    return STATE_SUSPEND;
}
/*----------------------------------------------------------------------------*/
static enum state stateReadCapacityEnter(struct Msc *driver)
{
  struct PrivateData * const privateData = driver->privateData;
  const struct CBW * const cbw = &privateData->context.cbw;

  if (!isInputDataValid(cbw))
    return STATE_ERROR;

  struct ReadCapacityData * const buffer =
      (struct ReadCapacityData *)driver->buffer;

  buffer->lastLogicalBlockAddress = toBigEndian32(driver->blockCount - 1);
  buffer->blockLength = toBigEndian32(driver->blockLength);

  usbTrace("msc: read capacity, %"PRIu32" blocks, block length %"PRIu32,
      driver->blockCount, driver->blockLength);

  if (sendResponse(driver, cbw, buffer, sizeof(*buffer)) == E_OK)
    return STATE_COMPLETED;
  else
    return STATE_SUSPEND;
}
/*----------------------------------------------------------------------------*/
static enum state stateReadSetupEnter(struct Msc *driver)
{
  struct PrivateData * const privateData = driver->privateData;
  const struct CBW * const cbw = &privateData->context.cbw;

  uint32_t logicalBlockAddress = 0;
  uint32_t numberOfBlocks = 0;

  switch (cbw->cb[0])
  {
    case SCSI_READ6:
    {
      const struct Read6Command * const command =
          (const struct Read6Command *)cbw->cb;

      logicalBlockAddress = ((command->logicalBlockAddress[0] & 0x1F) << 16)
          | (command->logicalBlockAddress[1] << 8)
          | command->logicalBlockAddress[2];
      numberOfBlocks = !command->transferLength ? 256 : command->transferLength;
      break;
    }

    case SCSI_READ10:
    {
      const struct Read10Command * const command =
          (const struct Read10Command *)cbw->cb;

      logicalBlockAddress = fromBigEndian32(command->logicalBlockAddress);
      numberOfBlocks = fromBigEndian16(command->transferLength);
      break;
    }

    case SCSI_READ12:
    {
      const struct Read12Command * const command =
          (const struct Read12Command *)cbw->cb;

      logicalBlockAddress = fromBigEndian32(command->logicalBlockAddress);
      numberOfBlocks = fromBigEndian32(command->transferLength);
      break;
    }
  }

  const uint32_t transferLength = numberOfBlocks * driver->blockLength;

  if (cbw->dataTransferLength != transferLength || !transferLength)
    return STATE_FAILURE;
  if (logicalBlockAddress + numberOfBlocks > driver->blockCount)
    return STATE_FAILURE;

  privateData->context.position = logicalBlockAddress * driver->blockLength;

  usbTrace("msc: read command, position %012"PRIX64", length %"PRIu32,
      privateData->context.position, transferLength);

  return STATE_READ;
}
/*----------------------------------------------------------------------------*/
static enum state stateReadEnter(struct Msc *driver)
{
  if (storageRead(driver) == E_OK)
    return STATE_READ;
  else
    return STATE_FAILURE;
}
/*----------------------------------------------------------------------------*/
static enum state stateReadRun(struct Msc *driver)
{
  struct PrivateData * const privateData = driver->privateData;
  const enum result status = ifGet(driver->storage, IF_STATUS, 0);

  if (status == E_OK)
  {
    const size_t transferLength = calcNextTransferLength(driver,
        privateData->context.left);

    /* Data read successfully from the storage */
    privateData->context.left -= transferLength;

    if (enqueueDataTx(driver, driver->buffer, transferLength) == E_OK)
      return STATE_READ_WAIT;
    else
      return STATE_SUSPEND; /* Unrecoverable USB error */
  }
  else
    return STATE_FAILURE;
}
/*----------------------------------------------------------------------------*/
static enum state stateReadWaitRun(struct Msc *driver)
{
  const struct PrivateData * const privateData = driver->privateData;

  return privateData->context.left ? STATE_READ : STATE_COMPLETED;
}
/*----------------------------------------------------------------------------*/
static enum state stateWriteSetupEnter(struct Msc *driver)
{
  struct PrivateData * const privateData = driver->privateData;
  const struct CBW * const cbw = &privateData->context.cbw;

  uint32_t logicalBlockAddress = 0;
  uint32_t numberOfBlocks = 0;

  switch (cbw->cb[0])
  {
    case SCSI_WRITE6:
    {
      const struct Write6Command * const command =
          (const struct Write6Command *)cbw->cb;

      logicalBlockAddress = ((command->logicalBlockAddress[0] & 0x1F) << 16)
          | (command->logicalBlockAddress[1] << 8)
          | command->logicalBlockAddress[2];
      numberOfBlocks = !command->transferLength ? 256 : command->transferLength;
      break;
    }

    case SCSI_WRITE10:
    {
      const struct Write10Command * const command =
          (const struct Write10Command *)cbw->cb;

      logicalBlockAddress = fromBigEndian32(command->logicalBlockAddress);
      numberOfBlocks = fromBigEndian16(command->transferLength);
      break;
    }

    case SCSI_WRITE12:
    {
      const struct Write12Command * const command =
          (const struct Write12Command *)cbw->cb;

      logicalBlockAddress = fromBigEndian32(command->logicalBlockAddress);
      numberOfBlocks = fromBigEndian32(command->transferLength);
      break;
    }
  }

  const uint32_t transferLength = numberOfBlocks * driver->blockLength;

  if (cbw->dataTransferLength != transferLength || !transferLength)
    return STATE_FAILURE;
  if (logicalBlockAddress + numberOfBlocks > driver->blockCount)
    return STATE_FAILURE;

  privateData->context.position = logicalBlockAddress * driver->blockLength;

  usbTrace("msc: write command, position %012"PRIX64", length %"PRIu32,
      privateData->context.position, transferLength);

  return STATE_WRITE;
}
/*----------------------------------------------------------------------------*/
static enum state stateWriteEnter(struct Msc *driver)
{
  struct PrivateData * const privateData = driver->privateData;
  const size_t transferLength = calcNextTransferLength(driver,
      privateData->context.left);

  if (enqueueDataRx(driver, driver->buffer, transferLength) == E_OK)
    return STATE_WRITE;
  else
    return STATE_SUSPEND; /* Unrecoverable USB error */
}
/*----------------------------------------------------------------------------*/
static enum state stateWriteRun(struct Msc *driver)
{
  if (storageWrite(driver) == E_OK)
    return STATE_WRITE_WAIT;
  else
    return STATE_FAILURE;
}
/*----------------------------------------------------------------------------*/
static enum state stateWriteWaitRun(struct Msc *driver)
{
  struct PrivateData * const privateData = driver->privateData;
  const enum result status = ifGet(driver->storage, IF_STATUS, 0);

  if (status == E_OK)
  {
    privateData->context.left -= calcNextTransferLength(driver,
        privateData->context.left);

    return privateData->context.left ? STATE_WRITE : STATE_COMPLETED;
  }
  else
    return STATE_FAILURE;
}
/*----------------------------------------------------------------------------*/
static enum state stateVerifyEnter(struct Msc *driver)
{
  return STATE_IDLE;
}
/*----------------------------------------------------------------------------*/
static enum state stateVerifyRun(struct Msc *driver)
{
  return STATE_IDLE;
}
/*----------------------------------------------------------------------------*/
static enum state stateCompletedEnter(struct Msc *driver)
{
  const struct PrivateData * const privateData = driver->privateData;
  const struct CBW * const cbw = &privateData->context.cbw;
  const uint32_t left = privateData->context.left;

  if (sendStatus(driver, cbw->tag, left, CSW_CMD_PASSED) == E_OK)
    return STATE_COMPLETED;
  else
    return STATE_SUSPEND;
}
/*----------------------------------------------------------------------------*/
static enum state stateCompletedRun(struct Msc *driver)
{
  const struct PrivateData * const privateData = driver->privateData;

//  if (privateData->context.left) //FIXME
//    usbEpSetStalled(driver->txEp, true);

  return STATE_IDLE;
}
/*----------------------------------------------------------------------------*/
static enum state stateFailureEnter(struct Msc *driver)
{
  const struct PrivateData * const privateData = driver->privateData;
  const struct CBW * const cbw = &privateData->context.cbw;
  const uint32_t left = privateData->context.left;

  if (sendStatus(driver, cbw->tag, left, CSW_CMD_FAILED) == E_OK)
    return STATE_FAILURE;
  else
    return STATE_SUSPEND;
}
/*----------------------------------------------------------------------------*/
static enum state stateFailureRun(struct Msc *driver)
{
  const struct PrivateData * const privateData = driver->privateData;
  const struct CBW * const cbw = &privateData->context.cbw;

  if (cbw->dataTransferLength)
  {
    if (cbw->flags & CBW_FLAG_DIRECTION_TO_HOST)
      usbEpSetStalled(driver->txEp, true);
    else
      usbEpSetStalled(driver->rxEp, true);
  }

  return STATE_IDLE;
}
/*----------------------------------------------------------------------------*/
static enum state stateErrorEnter(struct Msc *driver)
{
  const struct PrivateData * const privateData = driver->privateData;
  const struct CBW * const cbw = &privateData->context.cbw;
  const uint32_t left = cbw->dataTransferLength;

  if (sendStatus(driver, cbw->tag, left, CSW_PHASE_ERROR) == E_OK)
    return STATE_ERROR;
  else
    return STATE_SUSPEND;
}
/*----------------------------------------------------------------------------*/
static enum state stateErrorRun(struct Msc *driver __attribute__((unused)))
{
  return STATE_SUSPEND;
}
/*----------------------------------------------------------------------------*/
static enum state stateSuspendEnter(struct Msc *driver)
{
  usbTrace("msc: suspended");

  usbEpSetStalled(driver->rxEp, true);
  usbEpSetStalled(driver->txEp, true);
  return STATE_SUSPEND;
}
/*----------------------------------------------------------------------------*/
static void runStateMachine(struct Msc *driver)
{
  enum state current = driver->state;
  enum state previous = current;

  if (stateTable[current].run)
    current = stateTable[current].run(driver);

  while (current != previous)
  {
    if (current == STATE_FAILURE)
      usbTrace("msc: failure in state %u", (unsigned int)previous);
    if (current == STATE_ERROR)
      usbTrace("msc: critical error in state %u", (unsigned int)previous);

    previous = current;

    if (stateTable[current].enter)
      current = stateTable[current].enter(driver);
  }

  driver->state = current;
}
/*----------------------------------------------------------------------------*/
static void mscCommandReceived(void *argument, struct UsbRequest *request,
    enum usbRequestStatus status)
{
  struct Msc * const driver = argument;
  struct PrivateData * const privateData = driver->privateData;

  if (status == USB_REQUEST_COMPLETED)
  {
    if (request->length == sizeof(struct CBW))
    {
      memcpy(&privateData->context.cbw, request->buffer,
          sizeof(privateData->context.cbw));
      privateData->context.cbw.dataTransferLength =
          fromLittleEndian32(privateData->context.cbw.dataTransferLength);

      queuePush(&driver->controlQueue, &request);
      runStateMachine(driver);
    }
    else
    {
      //TODO Suspend
      queuePush(&driver->controlQueue, &request);
    }
  }
  else
  {
    queuePush(&driver->controlQueue, &request);
  }
}
/*----------------------------------------------------------------------------*/
static void mscDataReceived(void *argument, struct UsbRequest *request,
    enum usbRequestStatus status)
{
  struct Msc * const driver = argument;

  queuePush(&driver->rxQueue, &request);

  if (status == USB_REQUEST_COMPLETED)
  {
    runStateMachine(driver);
  }
  else if (status != USB_REQUEST_CANCELLED)
  {
    //TODO Suspend
  }
}
/*----------------------------------------------------------------------------*/
static void mscDataReceivedQuietly(void *argument, struct UsbRequest *request,
    enum usbRequestStatus status)
{
  struct Msc * const driver = argument;
  struct PrivateData * const privateData = driver->privateData;
  bool returnToQueue = true;

  if (status == USB_REQUEST_COMPLETED)
  {
    const size_t chunksLeft = privateData->context.totalChunks
        - privateData->context.queuedChunks;

    if (chunksLeft)
    {
      prepareDataRx(driver, request, chunksLeft == 1);

      if (usbEpEnqueue(driver->rxEp, request) == E_OK)
      {
        ++privateData->context.queuedChunks;
        returnToQueue = false;
      }
      else
      {
        //TODO Suspend
      }
    }
  }
  else if (status != USB_REQUEST_CANCELLED)
  {
    //TODO Suspend
  }

  if (returnToQueue)
    queuePush(&driver->rxQueue, &request);
}
/*----------------------------------------------------------------------------*/
static void mscDataSent(void *argument, struct UsbRequest *request,
    enum usbRequestStatus status)
{
  struct Msc * const driver = argument;

  queuePush(&driver->txQueue, &request);

  if (status == USB_REQUEST_COMPLETED)
  {
    runStateMachine(driver);
  }
  else if (status != USB_REQUEST_CANCELLED)
  {
    //TODO Suspend
  }
}
/*----------------------------------------------------------------------------*/
static void mscDataSentQuietly(void *argument, struct UsbRequest *request,
    enum usbRequestStatus status)
{
  struct Msc * const driver = argument;
  struct PrivateData * const privateData = driver->privateData;
  bool returnToQueue = true;

  if (status == USB_REQUEST_COMPLETED)
  {
    const size_t chunksLeft = privateData->context.totalChunks
        - privateData->context.queuedChunks;

    if (chunksLeft)
    {
      prepareDataTx(driver, request, chunksLeft == 1);

      if (usbEpEnqueue(driver->txEp, request) == E_OK)
      {
        ++privateData->context.queuedChunks;
        returnToQueue = false;
      }
      else
      {
        //TODO Suspend
      }
    }
  }
  else if (status != USB_REQUEST_CANCELLED)
  {
    //TODO Suspend
  }

  if (returnToQueue)
    queuePush(&driver->txQueue, &request);
}
/*----------------------------------------------------------------------------*/
static void mscStatusSent(void *argument, struct UsbRequest *request,
    enum usbRequestStatus status)
{
  struct Msc * const driver = argument;

  queuePush(&driver->controlQueue, &request);

  if (status == USB_REQUEST_COMPLETED)
  {
    runStateMachine(driver);
  }
  else if (status != USB_REQUEST_CANCELLED)
  {
    //TODO Suspend
  }
}
/*----------------------------------------------------------------------------*/
static inline size_t calcNextTransferLength(const struct Msc *driver,
    uint32_t left)
{
  return left >= driver->bufferSize ? driver->bufferSize : left;
}
/*----------------------------------------------------------------------------*/
static enum result enqueueControlRequest(struct Msc *driver)
{
  struct UsbRequest *request;

  queuePop(&driver->controlQueue, &request);
  request->callback = mscCommandReceived;
  request->callbackArgument = driver;
  request->length = 0;

  if (usbEpEnqueue(driver->rxEp, request) != E_OK)
  {
    queuePush(&driver->controlQueue, &request);
    return E_ERROR;
  }
  else
    return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result enqueueDataRx(struct Msc *driver, void *buffer,
    size_t length)
{
  const size_t maxChunks = queueSize(&driver->rxQueue);
  struct PrivateData * const privateData = driver->privateData;
  size_t chunks = length / driver->packetSize;

  usbTrace("msc: receive %zu chunks, total length %zu",
      chunks, length);

  privateData->context.bufferPosition = (uintptr_t)buffer;
  privateData->context.queuedChunks = 0;
  privateData->context.totalChunks = chunks;

  const bool moreChunksNeeded = chunks > maxChunks;
  enum result res = E_OK;

  if (moreChunksNeeded)
    chunks = maxChunks;

  while (chunks--)
  {
    struct UsbRequest *request;
    queuePop(&driver->rxQueue, &request);

    prepareDataRx(driver, request, !(moreChunksNeeded || chunks));

    if ((res = usbEpEnqueue(driver->rxEp, request)) == E_OK)
    {
      ++privateData->context.queuedChunks;
    }
    else
    {
      queuePush(&driver->rxQueue, &request);
      break;
    }
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static enum result enqueueDataTx(struct Msc *driver, const void *buffer,
    size_t length)
{
  const size_t maxChunks = queueSize(&driver->txQueue) - 1;
  struct PrivateData * const privateData = driver->privateData;
  size_t chunks = length / driver->packetSize;

  usbTrace("msc: send %zu chunks, total length %zu",
      chunks, length);

  privateData->context.bufferPosition = (uintptr_t)buffer;
  privateData->context.queuedChunks = 0;
  privateData->context.totalChunks = chunks;

  const bool moreChunksNeeded = chunks > maxChunks;
  enum result res = E_OK;

  if (moreChunksNeeded)
    chunks = maxChunks;

  while (chunks--)
  {
    struct UsbRequest *request;
    queuePop(&driver->txQueue, &request);

    prepareDataTx(driver, request, !(moreChunksNeeded || chunks));

    if ((res = usbEpEnqueue(driver->txEp, request)) == E_OK)
    {
      ++privateData->context.queuedChunks;
    }
    else
    {
      queuePush(&driver->txQueue, &request);
      break;
    }
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static bool isInputDataValid(const struct CBW *cbw)
{
  if (!cbw->dataTransferLength)
    return false;
  else if ((cbw->flags & CBW_FLAG_DIRECTION_TO_HOST) == 0)
    return false;
  else
    return true;
}
/*----------------------------------------------------------------------------*/
static void prepareDataRx(struct Msc *driver, struct UsbRequest *request,
    bool lastRequest)
{
  struct PrivateData * const privateData = driver->privateData;
  const uintptr_t bufferPosition = privateData->context.bufferPosition
      + privateData->context.queuedChunks * driver->packetSize;

  request->buffer = (uint8_t *)bufferPosition;
  request->capacity = driver->packetSize;
  request->length = 0;
  request->callbackArgument = driver;
  request->callback = lastRequest ? mscDataReceived : mscDataReceivedQuietly;
}
/*----------------------------------------------------------------------------*/
static void prepareDataTx(struct Msc *driver, struct UsbRequest *request,
    bool lastRequest)
{
  struct PrivateData * const privateData = driver->privateData;
  const uintptr_t bufferPosition = privateData->context.bufferPosition
      + privateData->context.queuedChunks * driver->packetSize;

  request->buffer = (uint8_t *)bufferPosition;
  request->capacity = request->length = driver->packetSize;
  request->callbackArgument = driver;
  request->callback = lastRequest ? mscDataSent : mscDataSentQuietly;
}
/*----------------------------------------------------------------------------*/
static enum result sendBuffer(struct Msc *driver, const void *buffer,
    size_t length, bool notify)
{
  struct PrivateData * const privateData = driver->privateData;
  uintptr_t bufferPosition = (uintptr_t)buffer;

  privateData->context.queuedChunks = 0;
  privateData->context.totalChunks = 0;

  while (length)
  {
    struct UsbRequest *request;
    queuePop(&driver->txQueue, &request);

    const size_t chunkLength = length > driver->packetSize ?
        driver->packetSize : length;

    request->buffer = (uint8_t *)bufferPosition;
    request->capacity = request->length = chunkLength;
    request->callbackArgument = driver;
    request->callback = (length == chunkLength && notify) ?
        mscDataSent : mscDataSentQuietly;

    if (usbEpEnqueue(driver->txEp, request) != E_OK)
    {
      queuePush(&driver->txQueue, &request);
      return E_ERROR;
    }

    length -= chunkLength;
    bufferPosition += chunkLength;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result sendResponse(struct Msc *driver, const struct CBW *cbw,
    const void *buffer, size_t length)
{
  struct PrivateData * const privateData = driver->privateData;

  if (buffer && length)
  {
    const size_t chunkLength = length > cbw->dataTransferLength ?
        cbw->dataTransferLength : length;
    const enum result res = sendBuffer(driver, buffer, chunkLength, false);

    if (res != E_OK)
      return res;

    privateData->context.left -= chunkLength;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result sendStatus(struct Msc *driver, uint32_t tag,
    uint32_t dataResidue, uint8_t status)
{
  const struct CSW csw = {
      .signature = CSW_SIGNATURE,
      .tag = tag,
      .dataResidue = toLittleEndian32(dataResidue),
      .status = status
  };
  struct UsbRequest *request;

  queuePop(&driver->controlQueue, &request);
  request->callback = mscStatusSent;
  request->callbackArgument = driver;
  request->length = sizeof(csw);
  memcpy(request->buffer, &csw, sizeof(csw));

  if (usbEpEnqueue(driver->txEp, request) != E_OK)
  {
    queuePush(&driver->controlQueue, &request);
    return E_ERROR;
  }
  else
    return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result storageRead(struct Msc *driver)
{
  struct PrivateData * const privateData = driver->privateData;
  const size_t transferLength = calcNextTransferLength(driver,
      privateData->context.left);
  enum result res;

  usbTrace("msc: read from %012"PRIX64", length %zu",
      privateData->context.position, transferLength);

  ifCallback(driver->storage, storageCallback, driver);

  res = ifSet(driver->storage, IF_POSITION, &privateData->context.position);
  if (res != E_OK)
    return res;

  res = ifRead(driver->storage, driver->buffer, transferLength);
  if (res != E_OK)
    return res;

  privateData->context.position += transferLength;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
//static enum result storageVerify(struct Msc *driver, struct UsbRequest *request)
//{
//  usbTrace("msc: memory verify");
//}
/*----------------------------------------------------------------------------*/
static enum result storageWrite(struct Msc *driver)
{
  struct PrivateData * const privateData = driver->privateData;
  const size_t transferLength = calcNextTransferLength(driver,
      privateData->context.left);
  enum result res;

  usbTrace("msc: write to %012"PRIX64", length %zu",
      privateData->context.position, transferLength);

  ifCallback(driver->storage, storageCallback, driver);

  res = ifSet(driver->storage, IF_POSITION, &privateData->context.position);
  if (res != E_OK)
    return res;

  res = ifWrite(driver->storage, driver->buffer, transferLength);
  if (res != E_OK)
    return res;

  privateData->context.position += transferLength;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void storageCallback(void *argument)
{
  const irqState state = irqSave();
  runStateMachine(argument);
  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static enum result buildDescriptors(struct Msc *driver,
    struct PrivateData *privateData, const struct MscConfig *config)
{
#ifdef CONFIG_USB_DEVICE_COMPOSITE
  const uint8_t firstInterface = usbCompositeDevIndex(driver->device);

  driver->controlInterfaceIndex = firstInterface;
  //TODO
#endif

  privateData->endpointDescriptors[0].length =
      sizeof(struct UsbEndpointDescriptor);
  privateData->endpointDescriptors[0].descriptorType = DESCRIPTOR_TYPE_ENDPOINT;
  privateData->endpointDescriptors[0].endpointAddress = config->endpoint.tx;
  privateData->endpointDescriptors[0].attributes =
      ENDPOINT_DESCRIPTOR_TYPE(ENDPOINT_TYPE_BULK);
  privateData->endpointDescriptors[0].maxPacketSize =
      TO_LITTLE_ENDIAN_16(MSC_DATA_EP_SIZE);
  privateData->endpointDescriptors[0].interval = 0;

  privateData->endpointDescriptors[1].length =
      sizeof(struct UsbEndpointDescriptor);
  privateData->endpointDescriptors[1].descriptorType = DESCRIPTOR_TYPE_ENDPOINT;
  privateData->endpointDescriptors[1].endpointAddress = config->endpoint.rx;
  privateData->endpointDescriptors[1].attributes =
      ENDPOINT_DESCRIPTOR_TYPE(ENDPOINT_TYPE_BULK);
  privateData->endpointDescriptors[1].maxPacketSize =
      TO_LITTLE_ENDIAN_16(MSC_DATA_EP_SIZE);
  privateData->endpointDescriptors[1].interval = 0;

  return iterateOverDescriptors(driver, privateData, usbDevAppendDescriptor);
}
/*----------------------------------------------------------------------------*/
static enum result descriptorEraseWrapper(void *device, const void *descriptor)
{
  usbDevEraseDescriptor(device, descriptor);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result handleRequest(struct Msc *driver,
    const struct UsbSetupPacket *packet,
    const uint8_t *payload __attribute__((unused)),
    uint16_t payloadLength __attribute__((unused)), uint8_t *response,
    uint16_t *responseLength, uint16_t maxResponseLength)
{
  switch (packet->request)
  {
    case MSC_REQUEST_RESET:
      resetBuffers(driver);
      *responseLength = 0;

      usbTrace("msc at %u: reset requested", packet->index);
      break;

    case MSC_REQUEST_GET_MAX_LUN:
      if (maxResponseLength < 1)
        return E_VALUE;

      response[0] = 0; //FIXME
      *responseLength = 1;

      usbTrace("msc at %u: max LUN requested", packet->index);
      break;

    default:
      usbTrace("msc at %u: unknown request %02X",
          packet->index, packet->request);
      return E_INVALID;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result initPrivateData(struct Msc *driver,
    const struct MscConfig *config)
{
  struct PrivateData * const privateData = malloc(sizeof(struct PrivateData));
  enum result res;

  if (!privateData)
    return E_MEMORY;
  driver->privateData = privateData;

  memset(&privateData->context.cbw, 0, sizeof(privateData->context.cbw));

  if ((res = buildDescriptors(driver, privateData, config)) != E_OK)
    return res;

  if ((res = initRequestQueues(driver, privateData)) != E_OK)
    return res;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result initRequestQueues(struct Msc *driver,
    struct PrivateData *privateData)
{
  enum result res;

  res = queueInit(&driver->rxQueue, sizeof(struct UsbRequest *), RX_QUEUE_SIZE);
  if (res != E_OK)
    return res;
  res = queueInit(&driver->txQueue, sizeof(struct UsbRequest *), TX_QUEUE_SIZE);
  if (res != E_OK)
    return res;
  res = queueInit(&driver->controlQueue, sizeof(struct UsbRequest *),
      CONTROL_QUEUE_SIZE);
  if (res != E_OK)
    return res;

  /* Add requests to queues */
  for (size_t index = 0; index < RX_QUEUE_SIZE; ++index)
  {
    struct UsbRequest * const request = privateData->rxRequests + index;

    usbRequestInit(request, 0, 0, 0, 0);
    queuePush(&driver->rxQueue, &request);
  }

  for (size_t index = 0; index < TX_QUEUE_SIZE; ++index)
  {
    struct UsbRequest * const request = privateData->txRequests + index;

    usbRequestInit(request, 0, 0, 0, 0);
    queuePush(&driver->txQueue, &request);
  }

  for (size_t index = 0; index < CONTROL_QUEUE_SIZE; ++index)
  {
    struct MscControlRequest * const request =
        privateData->controlRequests + index;

    usbRequestInit((struct UsbRequest *)request, request->payload,
        sizeof(request->payload), 0, 0);
    queuePush(&driver->controlQueue, &request);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result iterateOverDescriptors(const struct Msc *driver,
    struct PrivateData *privateData,
    enum result (*action)(void *, const void *))
{
#ifdef CONFIG_USB_DEVICE_COMPOSITE
  const void * const descriptors[] = {
      &privateData->associationDescriptor,
      &privateData->interfaceDescriptor,
      &privateData->endpointDescriptors[0],
      &privateData->endpointDescriptors[1]
  };
#else
  const void * const descriptors[] = {
      &deviceDescriptor,
      &configDescriptor,
      &interfaceDescriptor,
      &privateData->endpointDescriptors[0],
      &privateData->endpointDescriptors[1]
  };
#endif

  for (unsigned int i = 0; i < ARRAY_SIZE(descriptors); ++i)
  {
    const enum result res = action(driver->device, descriptors[i]);

    if (res != E_OK)
      return res;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void freePrivateData(struct Msc *driver)
{
  assert(queueFull(&driver->controlQueue));
  assert(queueFull(&driver->txQueue));
  assert(queueFull(&driver->rxQueue));

  queueDeinit(&driver->controlQueue);
  queueDeinit(&driver->txQueue);
  queueDeinit(&driver->rxQueue);

  iterateOverDescriptors(driver, driver->privateData, descriptorEraseWrapper);

  free(driver->privateData);
}
/*----------------------------------------------------------------------------*/
static void resetBuffers(struct Msc *driver)
{
  struct PrivateData * const privateData = driver->privateData;

  /* Return queued requests to pools */
  usbEpClear(driver->rxEp);
  usbEpClear(driver->txEp);

  /* Enqueue buffer for first CBW and reset state machine */
  memset(&privateData->context.cbw, 0, sizeof(privateData->context.cbw));

  if (enqueueControlRequest(driver) == E_OK)
    driver->state = STATE_IDLE;
  else
    driver->state = STATE_SUSPEND; /* Unrecoverable error */
}
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *object, const void *configBase)
{
  const struct MscConfig * const config = configBase;
  struct Msc * const driver = object;
  enum result res;

  if (!config->device || !config->storage)
    return E_VALUE;

  driver->device = config->device;
  driver->storage = config->storage;
  driver->controlInterfaceIndex = 0;

  /* Setup temporary buffer */
  if (!config->size || config->size & (MSC_BLOCK_SIZE - 1))
    return E_VALUE;

  driver->bufferSize = config->size;

  if (!config->buffer)
  {
    driver->buffer = malloc(config->size);

    if (!driver->buffer)
      return E_MEMORY;
  }
  else
    driver->buffer = config->buffer;

  /* Calculate storage geometry */
  uint64_t storageSize;

  if ((res = ifGet(driver->storage, IF_SIZE, &storageSize)) != E_OK)
    return res;

  driver->blockLength = MSC_BLOCK_SIZE;
  driver->blockCount = storageSize / driver->blockLength;
  driver->packetSize = MSC_DATA_EP_SIZE;

  /* Suspend state machine */
  driver->state = STATE_SUSPEND;

  driver->rxEp = usbDevCreateEndpoint(config->device, config->endpoint.rx);
  if (!driver->rxEp)
    return E_ERROR;
  driver->txEp = usbDevCreateEndpoint(config->device, config->endpoint.tx);
  if (!driver->txEp)
    return E_ERROR;

  if ((res = initPrivateData(driver, config)) != E_OK)
    return res;

  /* Bind driver and connect device */
  if ((res = usbDevBind(driver->device, driver)) != E_OK)
    return res;

#ifndef CONFIG_USB_DEVICE_COMPOSITE
  usbDevSetConnected(driver->device, true);
#endif

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void driverDeinit(void *object)
{
  struct Msc * const driver = object;

  /* Disconnect device */
#ifndef CONFIG_USB_DEVICE_COMPOSITE
  usbDevSetConnected(driver->device, false);
#endif

  usbDevUnbind(driver->device, driver);

  /* Clear endpoint queues */
  usbEpClear(driver->txEp);
  usbEpClear(driver->rxEp);

  /* Delete private data and queues */
  freePrivateData(driver);

  /* Delete endpoints */
  deinit(driver->txEp);
  deinit(driver->rxEp);
}
/*----------------------------------------------------------------------------*/
static enum result driverConfigure(void *object,
    const struct UsbSetupPacket *packet, const uint8_t *payload,
    uint16_t payloadLength, uint8_t *response, uint16_t *responseLength,
    uint16_t maxResponseLength)
{
  struct Msc * const driver = object;

  if (REQUEST_TYPE_VALUE(packet->requestType) != REQUEST_TYPE_CLASS)
    return E_INVALID;

  if (packet->index != driver->controlInterfaceIndex)
    return E_INVALID;

  return handleRequest(driver, packet, payload, payloadLength, response,
      responseLength, maxResponseLength);
}
/*----------------------------------------------------------------------------*/
static void driverEvent(void *object, unsigned int event)
{
  struct Msc * const driver = object;

#ifdef CONFIG_USB_DEVICE_HS
  if (event == USB_DEVICE_EVENT_PORT_CHANGE)
  {
    const enum usbSpeed speed = usbDevGetSpeed(driver->device);
    struct PrivateData * const privateData = driver->privateData;
    uint16_t maxPacketSize;

    if (speed == USB_HS)
      maxPacketSize = TO_LITTLE_ENDIAN_16(MSC_DATA_EP_SIZE_HS);
    else
      maxPacketSize = TO_LITTLE_ENDIAN_16(MSC_DATA_EP_SIZE);

    privateData->endpointDescriptors[0].maxPacketSize = maxPacketSize;
    privateData->endpointDescriptors[1].maxPacketSize = maxPacketSize;
    driver->packetSize = maxPacketSize;

    usbTrace("msc: current speed is %s", speed == USB_HS ? "HS" : "FS");
  }
#endif

  if (event == USB_DEVICE_EVENT_RESET)
  {
    resetBuffers(driver);

    usbTrace("msc: reset completed");
  }
}
