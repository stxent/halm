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
#define RX 8
#define TX 8

//struct MscUsbRequest
//{
//  struct UsbRequest base;
//
//  uint8_t payload[64]; //FIXME
//};

struct PrivateData
{
  /* Should be aligned along 4-byte boundary */
  uint8_t rxBuffer[RX * 64]; //FIXME
  uint8_t txBuffer[TX * 64]; //FIXME

  struct UsbRequest rxRequests[RX];
  struct UsbRequest txRequests[RX];

  struct UsbEndpointDescriptor endpointDescriptors[2];

#ifdef CONFIG_USB_DEVICE_COMPOSITE
  struct UsbInterfaceAssociationDescriptor associationDescriptor;
  struct UsbInterfaceDescriptor interfaceDescriptor;
#endif

  uint8_t txBuffersUsed;
};

struct MsdCbw
{
  uint32_t signature;
  uint32_t tag;
  uint32_t dataTransferLength;
  uint8_t flags;
  uint8_t lun;
  uint8_t cbLength;
  uint8_t cb[16];
} __attribute__((packed));

struct MsdCsw
{
  uint32_t signature;
  uint32_t tag;
  uint32_t dataResidue;
  uint8_t status;
} __attribute__((packed));

enum
{
  MSC_BS_CBW,                 /* Command Block Wrapper */
  MSC_BS_DATA_OUT,            /* Data Out Phase */
  MSC_BS_DATA_IN,             /* Data In Phase */
  MSC_BS_DATA_IN_LAST,        /* Data In Last Phase */
  MSC_BS_DATA_IN_LAST_STALL,  /* Data In Last Phase with Stall */
  MSC_BS_CSW,                 /* Command Status Wrapper */
  MSC_BS_ERROR,               /* Error */
};

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

#define MSC_CBW_Signature               0x43425355
#define MSC_CSW_Signature               0x53425355


/* CSW Status Definitions */
enum
{
  CSW_CMD_PASSED  = 0x00,
  CSW_CMD_FAILED  = 0x01,
  CSW_PHASE_ERROR = 0x02
};
/*----------------------------------------------------------------------------*/
static enum result buildDescriptors(struct Msc *, const struct MscConfig *);
static enum result descriptorEraseWrapper(void *, const void *);
static enum result iterateOverDescriptors(struct Msc *,
    enum result (*)(void *, const void *));
static enum result handleRequest(struct Msc *,
    const struct UsbSetupPacket *, const uint8_t *, uint16_t, uint8_t *,
    uint16_t *, uint16_t);
/*----------------------------------------------------------------------------*/
static void mscDataReceived(void *, struct UsbRequest *, enum usbRequestStatus);
static void mscDataSent(void *, struct UsbRequest *, enum usbRequestStatus);
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
#ifndef CONFIG_USB_DEVICE_COMPOSITE
static const struct UsbDeviceDescriptor deviceDescriptor = {
    .length             = sizeof(struct UsbDeviceDescriptor),
    .descriptorType     = DESCRIPTOR_TYPE_DEVICE,
    .usb                = TO_LITTLE_ENDIAN_16(0x0200),
    .deviceClass        = USB_CLASS_PER_INTERFACE,
    .deviceSubClass     = 0x00, /* Reserved value */
    .deviceProtocol     = 0x00, /* No class specific protocol */
    .maxPacketSize      = 64, //FIXME
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
    .interfaceSubClass  = 0x06, /* FIXME */
    .interfaceProtocol  = 0x50, /* Mass Storage Device bulk-only transport FIXME */
    .interface          = 0 /* No interface name */
};
#endif
/*----------------------------------------------------------------------------*/
struct MsdCbw CBW;
struct MsdCsw CSW;
int mscCounter = 0;

void MSC_SetCSW(struct Msc *driver, struct MsdCsw *csw)
{
  struct PrivateData * const privateData = driver->privateData;
  struct UsbRequest * const request = &privateData->txRequests[0];

  ++privateData->txBuffersUsed;
  csw->signature = MSC_CSW_Signature;
  usbRequestInit(request, csw, sizeof(*csw), mscDataSent, driver);
  request->length = sizeof(*csw);

  if (usbEpEnqueue(driver->txEp, request) != E_OK)
  {
    usbTrace("msc: FAIL");
    return;
  }
  mscCounter++;

  driver->stage = MSC_BS_CSW;
}

void MSC_TestUnitReady(struct Msc *driver, const struct MsdCbw *cbw)
{
  if (cbw->dataTransferLength != 0)
  {
    if ((cbw->flags & 0x80) != 0)
      usbEpSetStalled(driver->txEp, true);
    else
      usbEpSetStalled(driver->rxEp, true); //FIXME Optimize
  }

  CSW.status = CSW_CMD_PASSED;
  MSC_SetCSW(driver, &CSW);

  usbTrace("msc: test unit ready");
}

bool DataInFormat(struct Msc *driver, const struct MsdCbw *cbw)
{
  if (cbw->dataTransferLength == 0)
  {
    CSW.status = CSW_PHASE_ERROR;
    MSC_SetCSW(driver, &CSW);
    return false;
  }
  else if ((cbw->flags & 0x80) == 0)
  {
    usbEpSetStalled(driver->rxEp, true);
    CSW.status = CSW_PHASE_ERROR;
    MSC_SetCSW(driver, &CSW);
    return false;
  }
  else
  {
    return true;
  }
}

void DataInTransfer(struct Msc *driver, const struct MsdCbw *cbw,
    struct MsdCsw *csw, const uint8_t *buffer, size_t length)
{
  struct PrivateData * const privateData = driver->privateData;
  struct UsbRequest *request = privateData->txRequests;

  while (length)
  {
    size_t chunkLength;

    if (cbw)
      chunkLength = length > cbw->dataTransferLength ?
        cbw->dataTransferLength : length;
    else
      chunkLength = 64;

    usbRequestInit(request, buffer, chunkLength, mscDataSent, driver);
    request->length = chunkLength;

    if (usbEpEnqueue(driver->txEp, request) != E_OK)
    {
      usbTrace("msc: FAIL");
      break;
    }

    mscCounter++;

    length -= chunkLength;
    buffer += chunkLength;
    ++privateData->txBuffersUsed;
    ++request;
  }

  driver->stage = MSC_BS_DATA_IN_LAST_STALL;
  csw->dataResidue = 0;
  csw->status = CSW_CMD_PASSED;
}

void MSC_RequestSense(struct Msc *driver, const struct MsdCbw *cbw)
{
  if (!DataInFormat(driver, cbw))
    return;

  uint8_t BulkBuf[18];

  BulkBuf[0] = 0x70; /* Response Code */
  BulkBuf[1] = 0x00;
  BulkBuf[2] = 0x02; /* Sense Key */
  BulkBuf[3] = 0x00;
  BulkBuf[4] = 0x00;
  BulkBuf[5] = 0x00;
  BulkBuf[6] = 0x00;
  BulkBuf[7] = 0x0A; /* Additional Length */
  BulkBuf[8] = 0x00;
  BulkBuf[9] = 0x00;
  BulkBuf[10] = 0x00;
  BulkBuf[11] = 0x00;
  BulkBuf[12] = 0x30; /* ASC */
  BulkBuf[13] = 0x01; /* ASCQ */
  BulkBuf[14] = 0x00;
  BulkBuf[15] = 0x00;
  BulkBuf[16] = 0x00;
  BulkBuf[17] = 0x00;

  usbTrace("msc: request sense");

  DataInTransfer(driver, cbw, &CSW, BulkBuf, sizeof(BulkBuf));
}

void MSC_Inquiry(struct Msc *driver, const struct MsdCbw *cbw)
{
  if (!DataInFormat(driver, cbw))
    return;

  uint8_t BulkBuf[36];

  BulkBuf[0] = 0x00; /* Direct Access Device */
  BulkBuf[1] = 0x80; /* RMB = 1: Removable Medium */
  BulkBuf[2] = 0x00; /* Version: No conformance claim to standard */
  BulkBuf[3] = 0x01;

  BulkBuf[4] = 36 - 4; /* Additional Length */
  BulkBuf[5] = 0x80; /* SCCS = 1: Storage Controller Component */
  BulkBuf[6] = 0x00;
  BulkBuf[7] = 0x00;

  BulkBuf[8] = 'S'; /* Vendor Identification */
  BulkBuf[9] = 't';
  BulkBuf[10] = 'X';
  BulkBuf[11] = 'e';
  BulkBuf[12] = 'n';
  BulkBuf[13] = 't';
  BulkBuf[14] = ' ';
  BulkBuf[15] = ' ';

  BulkBuf[16] = 'H'; /* Product Identification */
  BulkBuf[17] = 'A';
  BulkBuf[18] = 'L';
  BulkBuf[19] = 'M';
  BulkBuf[20] = ' ';
  BulkBuf[21] = 'D';
  BulkBuf[22] = 'i';
  BulkBuf[23] = 's';
  BulkBuf[24] = 'k';
  BulkBuf[25] = ' ';
  BulkBuf[26] = ' ';
  BulkBuf[27] = ' ';
  BulkBuf[28] = ' ';
  BulkBuf[29] = ' ';
  BulkBuf[30] = ' ';
  BulkBuf[31] = ' ';

  BulkBuf[32] = '1'; /* Product Revision Level */
  BulkBuf[33] = '.';
  BulkBuf[34] = '0';
  BulkBuf[35] = ' ';

  usbTrace("msc: inquiry");

  DataInTransfer(driver, cbw, &CSW, BulkBuf, sizeof(BulkBuf));
}

void MSC_ModeSense6(struct Msc *driver, const struct MsdCbw *cbw)
{
  if (!DataInFormat(driver, cbw))
    return;

  uint8_t BulkBuf[4];

  BulkBuf[0] = 0x03;
  BulkBuf[1] = 0x00;
  BulkBuf[2] = 0x00;
  BulkBuf[3] = 0x00;

  usbTrace("msc: mode sense 6");

  DataInTransfer(driver, cbw, &CSW, BulkBuf, sizeof(BulkBuf));
}

void MSC_ModeSense10(struct Msc *driver, const struct MsdCbw *cbw)
{
  if (!DataInFormat(driver, cbw))
    return;

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

  DataInTransfer(driver, cbw, &CSW, BulkBuf, sizeof(BulkBuf));
}

void MSC_ReadFormatCapacity(struct Msc *driver, const struct MsdCbw *cbw)
{
  if (!DataInFormat(driver, cbw))
    return;

  uint64_t cardSize;
  ifGet(driver->storage, IF_SIZE, &cardSize);
  const uint32_t MSC_BlockSize = 512; //TODO
  const uint32_t MSC_BlockCount = cardSize / MSC_BlockSize;

  uint8_t BulkBuf[12];

  BulkBuf[0] = 0x00;
  BulkBuf[1] = 0x00;
  BulkBuf[2] = 0x00;
  BulkBuf[3] = 0x08; /* Capacity List Length */

  /* Block Count */ //XXX
  BulkBuf[4] = (MSC_BlockCount >> 24) & 0xFF;
  BulkBuf[5] = (MSC_BlockCount >> 16) & 0xFF;
  BulkBuf[6] = (MSC_BlockCount >> 8) & 0xFF;
  BulkBuf[7] = (MSC_BlockCount >> 0) & 0xFF;

  /* Block Length */
  BulkBuf[8] = 0x02; /* Descriptor Code: Formatted Media */
  BulkBuf[9] = (MSC_BlockSize >> 16) & 0xFF;
  BulkBuf[10] = (MSC_BlockSize >> 8) & 0xFF;
  BulkBuf[11] = (MSC_BlockSize >> 0) & 0xFF;

  usbTrace("msc: read format capacity");

  DataInTransfer(driver, cbw, &CSW, BulkBuf, sizeof(BulkBuf));
}

void MSC_ReadCapacity(struct Msc *driver, const struct MsdCbw *cbw)
{
  if (!DataInFormat(driver, cbw))
    return;

  uint64_t cardSize;
  ifGet(driver->storage, IF_SIZE, &cardSize);
  const uint32_t MSC_BlockSize = 512; //TODO
  const uint32_t MSC_BlockCount = cardSize / MSC_BlockSize;

  uint8_t BulkBuf[8];

  /* Last Logical Block */ //XXX
  BulkBuf[0] = ((MSC_BlockCount - 1) >> 24) & 0xFF;
  BulkBuf[1] = ((MSC_BlockCount - 1) >> 16) & 0xFF;
  BulkBuf[2] = ((MSC_BlockCount - 1) >> 8) & 0xFF;
  BulkBuf[3] = ((MSC_BlockCount - 1) >> 0) & 0xFF;

  /* Block Length */
  BulkBuf[4] = (MSC_BlockSize >> 24) & 0xFF;
  BulkBuf[5] = (MSC_BlockSize >> 16) & 0xFF;
  BulkBuf[6] = (MSC_BlockSize >> 8) & 0xFF;
  BulkBuf[7] = (MSC_BlockSize >> 0) & 0xFF;

  usbTrace("msc: read capacity");

  DataInTransfer(driver, cbw, &CSW, BulkBuf, sizeof(BulkBuf));
}

bool MSC_RWSetup(struct Msc *driver, const struct MsdCbw *cbw)
{
  uint64_t cardSize;
  ifGet(driver->storage, IF_SIZE, &cardSize);
  const uint32_t MSC_BlockSize = 512; //TODO
//  const uint32_t MSC_BlockCount = cardSize / MSC_BlockSize;

  uint32_t n;

  /* Logical Block Address of First Block */
  n = (cbw->cb[2] << 24) |
      (cbw->cb[3] << 16) |
      (cbw->cb[4] << 8) |
      (cbw->cb[5] << 0);

  driver->position = n * MSC_BlockSize;

  /* Number of Blocks to transfer */
  switch (cbw->cb[0])
  {
    case SCSI_READ10:
    case SCSI_WRITE10:
    case SCSI_VERIFY10:
      n = (cbw->cb[7] << 8) | (cbw->cb[8] << 0);
      break;

    case SCSI_READ12:
    case SCSI_WRITE12:
      n = (cbw->cb[6] << 24) |
          (cbw->cb[7] << 16) |
          (cbw->cb[8] << 8) |
          (cbw->cb[9] << 0);
      break;
  }

  driver->length = n * MSC_BlockSize;
  driver->chunk = 0;

  if (cbw->dataTransferLength == 0)
  {
    /* Host requests no data */
    CSW.status = CSW_CMD_FAILED;
    MSC_SetCSW(driver, &CSW);
    return false;
  }

  if (cbw->dataTransferLength != driver->length)
  {
    if ((cbw->flags & 0x80) != 0)
    {
      /* Stall appropriate EP */
      usbEpSetStalled(driver->txEp, true);
    }
    else
    {
      usbEpSetStalled(driver->rxEp, true);
    }

    CSW.status = CSW_CMD_FAILED;
    MSC_SetCSW(driver, &CSW);

    return false;
  }

  usbTrace("msc: rw setup %16"PRIX64" %u", driver->position, driver->length);
  return true;
}

void readCb(void *argument)
{
  struct Msc * const driver = argument;
  struct PrivateData * const privateData = driver->privateData;

  const irqState state = irqSave();

//  if ((Offset + n) > MSC_MemorySize)
//  {
//    n = MSC_MemorySize - Offset;
//    BulkStage = MSC_BS_DATA_IN_LAST_STALL;//FIXME
//  }

  DataInTransfer(driver, 0, &CSW, privateData->txBuffer, driver->chunk);
  CSW.dataResidue = driver->length;

  if (!driver->length)
  {
    driver->stage = MSC_BS_DATA_IN_LAST;
  }
  else
  {
    driver->stage = MSC_BS_DATA_IN;
  }

  if (driver->stage != MSC_BS_DATA_IN)
    CSW.status = CSW_CMD_PASSED;
  irqRestore(state);
}

void MSC_MemoryRead(struct Msc *driver)
{
  struct PrivateData * const privateData = driver->privateData;
  const size_t chunkLength = driver->length > sizeof(privateData->txBuffer) ?
      sizeof(privateData->txBuffer) : driver->length;

  usbTrace("msc: read from %16"PRIX64", length %zu",
      driver->position, chunkLength);

  ifCallback(driver->storage, readCb, driver);
  driver->length -= chunkLength;
  driver->position += chunkLength;
  driver->chunk = chunkLength;
  ifSet(driver->storage, IF_POSITION, &driver->position);
  ifRead(driver->storage, privateData->txBuffer, chunkLength);
}

void writeCb(void *argument)
{
  struct Msc * const driver = argument;
  struct PrivateData * const privateData = driver->privateData;

  const irqState state = irqSave();

  while (!queueEmpty(&driver->rxRequestQueue))
  {
    struct UsbRequest *request;

    queuePop(&driver->rxRequestQueue, &request);
    request->length = 0;

    if (usbEpEnqueue(driver->rxEp, request) != E_OK)
    {
      queuePush(&driver->rxRequestQueue, &request);
      break;
    }
  }

  if (!driver->length || driver->stage == MSC_BS_CSW)
  {
    CSW.status = CSW_CMD_PASSED;
    MSC_SetCSW(driver, &CSW);
  }

  irqRestore(state);
}

void MSC_MemoryWrite(struct Msc *driver, struct UsbRequest *request,
    const struct MsdCbw *cbw)
{
  struct PrivateData * const privateData = driver->privateData;

//  if ((driver->position + driver->length) > MSC_MemorySize)
//  {
//  }

  memcpy(privateData->txBuffer + driver->chunk, request->buffer,
      request->length);
  driver->chunk += request->length;

  ifCallback(driver->storage, writeCb, driver);
  driver->position += request->length;
  driver->length -= request->length;

  CSW.dataResidue -= request->length;

  if (!driver->length || driver->chunk >= sizeof(privateData->txBuffer))
  {
    usbTrace("msc: write to from %16"PRIX64", length %zu",
        driver->position, driver->chunk);

    ifCallback(driver->storage, writeCb, driver);
    ifSet(driver->storage, IF_POSITION, &driver->position);
    ifWrite(driver->storage, privateData->txBuffer, driver->chunk);

    driver->chunk = 0;
  }
}

void MSC_MemoryVerify(struct Msc *driver, const struct MsdCbw *cbw)
{
  usbTrace("msc: memory verify");
//  uint32_t n;
//
//  if ((Offset + BulkLen) > MSC_MemorySize)
//  {
//    BulkLen = MSC_MemorySize - Offset;
//    BulkStage = MSC_BS_CSW;
//    MSC_SetStallEP(MSC_EP_OUT);
//  }
//
//  Offset += BulkLen;
//  Length -= BulkLen;
//
//  CSW.dataResidue -= BulkLen;
//
//  if ((Length == 0) || (BulkStage == MSC_BS_CSW))
//  {
//    CSW.bStatus = (MemOK) ? CSW_CMD_PASSED : CSW_CMD_FAILED;
//    MSC_SetCSW();
//  }
//
//  CSW.status = CSW_CMD_PASSED;
//  MSC_SetCSW(driver, &csw);
}

void MSC_GetCBW(struct Msc *driver, const struct UsbRequest *request)
{
  memcpy(&CBW, request->buffer, request->length); //FIXME Check length

  if ((request->length == sizeof(CBW)) && (CBW.signature == MSC_CBW_Signature)) //FIXME Byte order
  {
    /* Valid CBW */
    CSW.tag = CBW.tag;
    CSW.dataResidue = CBW.dataTransferLength;
    if (CBW.lun != 0 || CBW.cbLength < 1 || CBW.cbLength > 16)
    {
fail:
      CSW.status = CSW_CMD_FAILED;
      MSC_SetCSW(driver, &CSW);
    }
    else
    {
      switch (CBW.cb[0])
      {
        case SCSI_TEST_UNIT_READY:
          MSC_TestUnitReady(driver, &CBW);
          break;

        case SCSI_REQUEST_SENSE:
          MSC_RequestSense(driver, &CBW);
          break;

        case SCSI_FORMAT_UNIT:
          goto fail;

        case SCSI_INQUIRY:
          MSC_Inquiry(driver, &CBW);
          break;

        case SCSI_START_STOP_UNIT:
          goto fail;

        case SCSI_MEDIA_REMOVAL:
          goto fail;

        case SCSI_MODE_SELECT6:
          goto fail;

        case SCSI_MODE_SENSE6:
          MSC_ModeSense6(driver, &CBW);
          break;

        case SCSI_MODE_SELECT10:
          goto fail;

        case SCSI_MODE_SENSE10:
          MSC_ModeSense10(driver, &CBW);
          break;

        case SCSI_READ_FORMAT_CAPACITIES:
          MSC_ReadFormatCapacity(driver, &CBW);
          break;

        case SCSI_READ_CAPACITY:
          MSC_ReadCapacity(driver, &CBW);
          break;

        case SCSI_READ10:
        case SCSI_READ12:
          if (MSC_RWSetup(driver, &CBW))
          {
            if ((CBW.flags & 0x80) != 0)
            {
              driver->stage = MSC_BS_DATA_IN;
              MSC_MemoryRead(driver/*, &CBW*/);
            }
            else
            {
              /* Direction mismatch */
              usbEpSetStalled(driver->rxEp, true);
              CSW.status = CSW_PHASE_ERROR;
              MSC_SetCSW(driver, &CSW);
            }
          }
          break;

        case SCSI_WRITE10:
        case SCSI_WRITE12:
          if (MSC_RWSetup(driver, &CBW))
          {
            if ((CBW.flags & 0x80) == 0)
            {
              driver->stage = MSC_BS_DATA_OUT;
            }
            else
            {
              /* Direction mismatch */
              usbEpSetStalled(driver->txEp, true);
              CSW.status = CSW_PHASE_ERROR;
              MSC_SetCSW(driver, &CSW);
            }
          }
          break;

        case SCSI_VERIFY10:
          if ((CBW.cb[1] & 0x02) == 0)
          {
            // BYTCHK = 0 -> CRC Check (not implemented) //TODO
            CSW.status = CSW_CMD_PASSED;
            MSC_SetCSW(driver, &CSW);
            break;
          }
          if (MSC_RWSetup(driver, &CBW))
          {
            if ((CBW.flags & 0x80) == 0)
            {
              driver->stage = MSC_BS_DATA_OUT;
            }
            else
            {
              usbEpSetStalled(driver->txEp, true);
              CSW.status = CSW_PHASE_ERROR;
              MSC_SetCSW(driver, &CSW);
            }
          }
          break;

        default:
          goto fail;
      }
    }
  }
  else
  {
    /* Invalid CBW */
    usbEpSetStalled(driver->rxEp, true);
    usbEpSetStalled(driver->txEp, true);
    driver->stage = MSC_BS_ERROR;
  }
}

static void mscDataReceived(void *argument, struct UsbRequest *request,
    enum usbRequestStatus status)
{
  struct Msc * const driver = argument;

  if (status == REQUEST_COMPLETED)
  {
    switch (driver->stage)
    {
      case MSC_BS_CBW:
        MSC_GetCBW(driver, request);
        usbEpEnqueue(driver->rxEp, request);
        break;

      case MSC_BS_DATA_OUT:
        switch (CBW.cb[0]) {
          case SCSI_WRITE10:
          case SCSI_WRITE12:
            MSC_MemoryWrite(driver, request, &CBW);
            break;
          case SCSI_VERIFY10:
            MSC_MemoryVerify(driver, &CBW);
            break;
        }
        queuePush(&driver->rxRequestQueue, &request);
        break;

      default:
        usbEpSetStalled(driver->rxEp, true);
        CSW.status = CSW_PHASE_ERROR;
        MSC_SetCSW(driver, &CSW);
        usbEpEnqueue(driver->rxEp, request);
        break;
    }
  }
  else
  {
    queuePush(&driver->rxRequestQueue, &request);
  }
}
/*----------------------------------------------------------------------------*/
static void mscDataSent(void *argument, struct UsbRequest *request,
    enum usbRequestStatus status)
{
  struct Msc * const driver = argument;
  struct PrivateData * const privateData = driver->privateData;

  mscCounter--;

  if (--privateData->txBuffersUsed) //XXX
    return;

  if (status == REQUEST_COMPLETED)
  {
    switch (driver->stage)
    {
      case MSC_BS_DATA_IN:
        switch (CBW.cb[0])
        {
          case SCSI_READ10:
          case SCSI_READ12:
            MSC_MemoryRead(driver);
            break;
        }
        break;

      case MSC_BS_DATA_IN_LAST:
        MSC_SetCSW(driver, &CSW);
        break;

      case MSC_BS_DATA_IN_LAST_STALL:
        usbEpSetStalled(driver->txEp, true);
        MSC_SetCSW(driver, &CSW);
        break;

      case MSC_BS_CSW:
        driver->stage = MSC_BS_CBW;
        break;
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum result buildDescriptors(struct Msc *driver,
    const struct MscConfig *config)
{
  struct PrivateData * const privateData = malloc(sizeof(struct PrivateData));

  if (!privateData)
    return E_MEMORY;
  driver->privateData = privateData;

#ifdef CONFIG_USB_DEVICE_COMPOSITE
  const uint8_t firstInterface = usbCompositeDevIndex(driver->device);

  driver->controlInterfaceIndex = firstInterface;

  privateData->associationDescriptor.length =
      sizeof(struct UsbInterfaceAssociationDescriptor);
  privateData->associationDescriptor.descriptorType =
      DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION;
  privateData->associationDescriptor.firstInterface = firstInterface;
  privateData->associationDescriptor.interfaceCount = 2;
  privateData->associationDescriptor.functionClass = USB_CLASS_CDC;
  /* Abstract Control Model */
  privateData->associationDescriptor.functionSubClass = 0x02;
  /* No protocol used */
  privateData->associationDescriptor.functionProtocol = 0x00;
  /* No string description */
  privateData->associationDescriptor.function = 0;

  privateData->interfaceDescriptors[0].length =
      sizeof(struct UsbInterfaceDescriptor);
  privateData->interfaceDescriptors[0].descriptorType =
      DESCRIPTOR_TYPE_INTERFACE;
  privateData->interfaceDescriptors[0].interfaceNumber = firstInterface;
  privateData->interfaceDescriptors[0].alternateSettings = 0;
  privateData->interfaceDescriptors[0].numEndpoints = 1;
  privateData->interfaceDescriptors[0].interfaceClass = USB_CLASS_CDC;
  privateData->interfaceDescriptors[0].interfaceSubClass = 0x02; //TODO
  privateData->interfaceDescriptors[0].interfaceProtocol = 0x00;
  privateData->interfaceDescriptors[0].interface = 0;

  privateData->interfaceDescriptors[1].length =
      sizeof(struct UsbInterfaceDescriptor);
  privateData->interfaceDescriptors[1].descriptorType =
      DESCRIPTOR_TYPE_INTERFACE;
  privateData->interfaceDescriptors[1].interfaceNumber = firstInterface + 1;
  privateData->interfaceDescriptors[1].alternateSettings = 0;
  privateData->interfaceDescriptors[1].numEndpoints = 2;
  privateData->interfaceDescriptors[1].interfaceClass = USB_CLASS_CDC_DATA;
  privateData->interfaceDescriptors[1].interfaceSubClass = 0x00;
  privateData->interfaceDescriptors[1].interfaceProtocol = 0x00;
  privateData->interfaceDescriptors[1].interface = 0;

  privateData->unionDescriptor.length = sizeof(struct CdcUnionDescriptor);
  privateData->unionDescriptor.descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE;
  privateData->unionDescriptor.descriptorSubType = CDC_SUBTYPE_UNION;
  privateData->unionDescriptor.masterInterface0 = firstInterface;
  privateData->unionDescriptor.slaveInterface0 = firstInterface + 1;
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

  return iterateOverDescriptors(driver, usbDevAppendDescriptor);
}
/*----------------------------------------------------------------------------*/
static enum result descriptorEraseWrapper(void *device, const void *descriptor)
{
  usbDevEraseDescriptor(device, descriptor);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result iterateOverDescriptors(struct Msc *driver,
    enum result (*action)(void *, const void *))
{
  struct PrivateData * const privateData = driver->privateData;

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
#define MSC_REQUEST_RESET               0xFF
#define MSC_REQUEST_GET_MAX_LUN         0xFE

static enum result handleRequest(struct Msc *driver,
    const struct UsbSetupPacket *packet, const uint8_t *input,
    uint16_t inputLength, uint8_t *output, uint16_t *outputLength,
    uint16_t maxOutputLength)
{
  switch (packet->request)
  {
    case MSC_REQUEST_RESET:
    {
      usbTrace("msc at %u: request reset", packet->index);
      CSW.signature = 0;
      driver->stage = MSC_BS_CBW;
      *outputLength = 0; //FIXME
      break;
    }

    case MSC_REQUEST_GET_MAX_LUN:
    {
      usbTrace("msc at %u: get max LUN", packet->index);
      *outputLength = 1;
      output[0] = 0; //FIXME
      break;
    }

    default:
    {
      usbTrace("msc at %u: unknown request %02X",
          packet->index, packet->request);
      return E_INVALID;
    }
  }

  return E_OK;
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

  driver->stage = MSC_BS_CBW;

  if ((res = buildDescriptors(driver, config)) != E_OK)
    return res;

  if ((res = usbDevBind(driver->device, driver)) != E_OK)
    return res;

  res = queueInit(&driver->rxRequestQueue, sizeof(struct UsbRequest *),
      RX); //FIXME
  if (res != E_OK)
    return res;
  res = queueInit(&driver->txRequestQueue, sizeof(struct UsbRequest *),
      TX);
  if (res != E_OK)
    return res;

  driver->rxEp = usbDevCreateEndpoint(config->device, config->endpoint.rx);
  if (!driver->rxEp)
    return E_ERROR;
  driver->txEp = usbDevCreateEndpoint(config->device, config->endpoint.tx);
  if (!driver->txEp)
    return E_ERROR;

  /* Add requests to queues */
  struct PrivateData * const privateData = driver->privateData;

  privateData->txBuffersUsed = 0;

  struct UsbRequest *request;
  uint8_t *bufferPosition;

  request = privateData->rxRequests;
  bufferPosition = privateData->rxBuffer;
  for (size_t index = 0; index < RX; ++index)
  {
    //FIXME
    usbRequestInit(request, bufferPosition, 64, mscDataReceived, driver);
    queuePush(&driver->rxRequestQueue, &request);
    ++request;

    bufferPosition += 64;
  }

//  bufferPosition = privateData->txBuffer;
//  for (size_t index = 0; index < TX; ++index)
//  {
//    usbRequestInit(request, bufferPosition, 64, mscDataSent, driver);
//    queuePush(&driver->txRequestQueue, &request);
//    ++request;
//
//    bufferPosition += 64;
//  }

#ifndef CONFIG_USB_DEVICE_COMPOSITE
  usbDevSetConnected(driver->device, true);
#endif

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void driverDeinit(void *object)
{
  struct Msc * const driver = object;

#ifndef CONFIG_USB_DEVICE_COMPOSITE
  usbDevSetConnected(driver->device, false);
#endif

  //TODO
  usbDevUnbind(driver->device, driver);
  iterateOverDescriptors(driver, descriptorEraseWrapper);
  free(driver->privateData);
}
/*----------------------------------------------------------------------------*/
static enum result driverConfigure(void *object,
    const struct UsbSetupPacket *packet, const uint8_t *payload,
    uint16_t payloadLength, uint8_t *response, uint16_t *responseLength,
    uint16_t maxResponseLength)
{
  struct Msc * const driver = object;
  const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);

  if (type != REQUEST_TYPE_CLASS)
    return E_INVALID;

//  if (packet->index != driver->controlInterfaceIndex)
//    return E_INVALID;

  const enum result res = handleRequest(driver, packet, payload, payloadLength,
      response, responseLength, maxResponseLength);

  return res;
}
/*----------------------------------------------------------------------------*/
static void resetBuffers(struct Msc *driver) //FIXME
{
  /* Return queued requests to pools */
  usbEpClear(driver->rxEp);
  usbEpClear(driver->txEp);
}
/*----------------------------------------------------------------------------*/
static void driverEvent(void *object, unsigned int event)
{
  struct Msc * const driver = object;

  if (event == DEVICE_EVENT_RESET)
  {
    driver->stage = MSC_BS_CBW;

//    driver->suspended = true;
    resetBuffers(driver);

    while (!queueEmpty(&driver->rxRequestQueue))
    {
      struct UsbRequest *request;

      queuePop(&driver->rxRequestQueue, &request);
      request->length = 0;

      if (usbEpEnqueue(driver->rxEp, request) != E_OK)
      {
        queuePush(&driver->rxRequestQueue, &request);
        break;
      }
    }

//    driver->suspended = false;
    usbTrace("msc: reset completed");
  }
}
