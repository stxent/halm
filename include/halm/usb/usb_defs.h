/*
 * halm/usb/usb_defs.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_USB_USB_DEFS_H_
#define HALM_USB_USB_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
enum
{
  /* Descriptor types */
  DESCRIPTOR_TYPE_DEVICE                  = 0x01,
  DESCRIPTOR_TYPE_CONFIGURATION           = 0x02,
  DESCRIPTOR_TYPE_STRING                  = 0x03,
  DESCRIPTOR_TYPE_INTERFACE               = 0x04,
  DESCRIPTOR_TYPE_ENDPOINT                = 0x05,
  DESCRIPTOR_TYPE_DEVICE_QUALIFIER        = 0x06,
  DESCRIPTOR_TYPE_OTHER_SPEED             = 0x07,
  DESCRIPTOR_TYPE_INTERFACE_POWER         = 0x08,
  DESCRIPTOR_TYPE_OTG                     = 0x09,
  DESCRIPTOR_TYPE_DEBUG                   = 0x0A,
  DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION   = 0x0B,
  DESCRIPTOR_TYPE_SECURITY                = 0x0C,
  DESCRIPTOR_TYPE_KEY                     = 0x0D,
  DESCRIPTOR_TYPE_ENCRYPTION_TYPE         = 0x0E,
  DESCRIPTOR_TYPE_BOS                     = 0x0F,
  DESCRIPTOR_TYPE_DEVICE_CAPABILITY       = 0x10,
  DESCRIPTOR_TYPE_WIRELESS_ENDPOINT_COMP  = 0x11,

  /* Conventional codes for class-specific descriptors */
  DESCRIPTOR_TYPE_CS_DEVICE               = 0x21,
  DESCRIPTOR_TYPE_CS_CONFIG               = 0x22,
  DESCRIPTOR_TYPE_CS_STRING               = 0x23,
  DESCRIPTOR_TYPE_CS_INTERFACE            = 0x24,
  DESCRIPTOR_TYPE_CS_ENDPOINT             = 0x25
};

enum
{
  ENDPOINT_TYPE_CONTROL     = 0,
  ENDPOINT_TYPE_ISOCHRONOUS = 1,
  ENDPOINT_TYPE_BULK        = 2,
  ENDPOINT_TYPE_INTERRUPT   = 3
};
/*----------------------------------------------------------------------------*/
enum
{
  USB_CLASS_PER_INTERFACE       = 0x00,
  USB_CLASS_AUDIO               = 0x01,
  USB_CLASS_CDC                 = 0x02,
  USB_CLASS_HID                 = 0x03,
  USB_CLASS_PHYSICAL            = 0x05,
  USB_CLASS_STILL_IMAGE         = 0x06,
  USB_CLASS_PRINTER             = 0x07,
  USB_CLASS_MASS_STORAGE        = 0x08,
  USB_CLASS_HUB                 = 0x09,
  USB_CLASS_CDC_DATA            = 0x0A,
  USB_CLASS_CSCID               = 0x0B,
  USB_CLASS_CONTENT_SEC         = 0x0D,
  USB_CLASS_VIDEO               = 0x0E,
  USB_CLASS_WIRELESS_CONTROLLER = 0xE0,
  USB_CLASS_MISCELLANEOUS       = 0xEF,
  USB_CLASS_APP_SPEC            = 0xFE,
  USB_CLASS_VENDOR_SPEC         = 0xFF
};
/*----------------------------------------------------------------------------*/
enum
{
  FEATURE_ENDPOINT_HALT = 0x00,
  FEATURE_REMOTE_WAKEUP = 0x01,
  FEATURE_TEST_MODE     = 0x02
};

enum
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

enum
{
  REQUEST_DIRECTION_TO_DEVICE = 0,
  REQUEST_DIRECTION_TO_HOST   = 1
};

enum
{
  REQUEST_RECIPIENT_DEVICE    = 0,
  REQUEST_RECIPIENT_INTERFACE = 1,
  REQUEST_RECIPIENT_ENDPOINT  = 2,
  REQUEST_RECIPIENT_OTHER     = 3
};

enum
{
  REQUEST_TYPE_STANDARD = 0,
  REQUEST_TYPE_CLASS    = 1,
  REQUEST_TYPE_VENDOR   = 2,
  REQUEST_TYPE_RESERVED = 3
};
/*----------------------------------------------------------------------------*/
enum
{
  LANGID_FRENCH     = 0x040C,
  LANGID_GERMAN     = 0x0407,
  LANGID_JAPANESE   = 0x0411,
  LANGID_RUSSIAN    = 0x0419,
  LANGID_CHINESE    = 0x0804,
  LANGID_ENGLISH_UK = 0x0809
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
#define DESCRIPTOR_INDEX(value) FIELD_VALUE((value), BIT_FIELD(MASK(8), 0), 0)
#define DESCRIPTOR_TYPE(value)  FIELD_VALUE((value), BIT_FIELD(MASK(8), 8), 8)
/* Prefix for string descriptors where length is zero and type is 3 */
#define USB_STRING_PREFIX       "\x00\x03"
/*----------------------------------------------------------------------------*/
/* Configuration attributes */
#define CONFIGURATION_DESCRIPTOR_DEFAULT        0x80
#define CONFIGURATION_DESCRIPTOR_SELF_POWERED   0x40
#define CONFIGURATION_DESCRIPTOR_REMOTE_WAKEUP  0x20

/* Common endpoint attributes */
#define ENDPOINT_DESCRIPTOR_TYPE(value)         BIT_FIELD((value), 0)
#define ENDPOINT_DESCRIPTOR_TYPE_MASK           BIT_FIELD(MASK(2), 0)
#define ENDPOINT_DESCRIPTOR_TYPE_VALUE(attribute) \
    FIELD_VALUE((attribute), ENDPOINT_DESCRIPTOR_TYPE_MASK, 0)

/* Attributes of isochronous endpoints */
#define ENDPOINT_DESCRIPTOR_NO_SYNC             BIT_FIELD(0, 2)
#define ENDPOINT_DESCRIPTOR_ASYNC               BIT_FIELD(1, 2)
#define ENDPOINT_DESCRIPTOR_ADAPTIVE            BIT_FIELD(2, 2)
#define ENDPOINT_DESCRIPTOR_SYNC                BIT_FIELD(3, 2)

#define ENDPOINT_DESCRIPTOR_DATA                BIT_FIELD(0, 4)
#define ENDPOINT_DESCRIPTOR_FEEDBACK            BIT_FIELD(1, 4)
#define ENDPOINT_DESCRIPTOR_EXPLICIT_FEEDBACK   BIT_FIELD(2, 4)
/*----------------------------------------------------------------------------*/
struct UsbSetupPacket
{
  uint8_t requestType;
  uint8_t request;
  uint16_t value;
  uint16_t index;
  uint16_t length;
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
struct UsbDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t data[];
} __attribute__((packed));

struct UsbConfigurationDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint16_t totalLength;
  uint8_t numInterfaces;
  uint8_t configurationValue;
  uint8_t configuration;
  uint8_t attributes;
  uint8_t maxPower;
} __attribute__((packed));

struct UsbDeviceDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint16_t usb;
  uint8_t deviceClass;
  uint8_t deviceSubClass;
  uint8_t deviceProtocol;
  uint8_t maxPacketSize;
  uint16_t idVendor;
  uint16_t idProduct;
  uint16_t device;
  uint8_t manufacturer;
  uint8_t product;
  uint8_t serialNumber;
  uint8_t numConfigurations;
} __attribute__((packed));

struct UsbDeviceQualifierDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint16_t usb;
  uint8_t deviceClass;
  uint8_t deviceSubClass;
  uint8_t deviceProtocol;
  uint8_t maxPacketSize;
  uint8_t numConfigurations;
  uint8_t reserved;
} __attribute__((packed));

struct UsbEndpointDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t endpointAddress;
  uint8_t attributes;
  uint16_t maxPacketSize;
  uint8_t interval;
} __attribute__((packed));

struct UsbInterfaceDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t interfaceNumber;
  uint8_t alternateSettings;
  uint8_t numEndpoints;
  uint8_t interfaceClass;
  uint8_t interfaceSubClass;
  uint8_t interfaceProtocol;
  uint8_t interface;
} __attribute__((packed));

struct UsbInterfaceAssociationDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t firstInterface;
  uint8_t interfaceCount;
  uint8_t functionClass;
  uint8_t functionSubClass;
  uint8_t functionProtocol;
  uint8_t function;
} __attribute__((packed));

struct UsbStringDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint16_t langid;
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_USB_DEFS_H_ */
