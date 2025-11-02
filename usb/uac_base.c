/*
 * uac.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/usb/uac.h>
#include <halm/usb/uac_base.h>
#include <halm/usb/uac_defs.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_trace.h>
#include <xcore/memory.h>
#include <assert.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
enum
{
  UNIT_CLOCK_SOURCE         = 0x01,
  UNIT_IN_INPUT_TERMINAL    = 0x10,
  UNIT_IN_FEATURE_UNIT      = 0x11,
  UNIT_IN_OUTPUT_TERMINAL   = 0x12,
  UNIT_OUT_INPUT_TERMINAL   = 0x20,
  UNIT_OUT_FEATURE_UNIT     = 0x21,
  UNIT_OUT_OUTPUT_TERMINAL  = 0x22
};
/*----------------------------------------------------------------------------*/
struct UacBase
{
  struct Entity base;

  /* Upper-half driver */
  struct Uac *owner;
  /* USB peripheral */
  struct UsbDevice *device;

  /* Maximum packet size in bytes for 1 kHz frame rate */
  uint16_t maxPacketSize;

  /* Addresses of endpoints */
  struct
  {
    uint8_t fb;
    uint8_t rx;
    uint8_t tx;
  } endpoints;

  /* Number of the first interface in the device */
  uint8_t controlInterfaceIndex;
  /* Speed of the USB interface */
  enum UsbSpeed speed;

  /* Alternate Setting 1 is activated for input path */
  bool rxActive;
  /* Alternate Setting 1 is activated for output path */
  bool txActive;

  /* Enable input path */
  bool in;
  /* Enable output path */
  bool out;

  UsbDescriptorFunctor table[26];
};
/*----------------------------------------------------------------------------*/
static void buildDescriptorTable(struct UacBase *);
/*----------------------------------------------------------------------------*/
static void interfaceAssociationDescriptor(const void *, struct UsbDescriptor *,
    void *);
static void deviceDescriptor(const void *, struct UsbDescriptor *, void *);
static void configDescriptor(const void *, struct UsbDescriptor *, void *);

static void controlInterfaceDescriptor(const void *, struct UsbDescriptor *,
    void *);
static void controlInterfaceCSDescriptor(const void *, struct UsbDescriptor *,
    void *);

static void clockSourceDescriptor(const void *, struct UsbDescriptor *, void *);
static void inInputTerminalDescriptor(const void *, struct UsbDescriptor *,
    void *);
static void inFeatureUnitDescriptor(const void *, struct UsbDescriptor *,
    void *);
static void inOutputTerminalDescriptor(const void *, struct UsbDescriptor *,
    void *);
static void outInputTerminalDescriptor(const void *, struct UsbDescriptor *,
    void *);
static void outFeatureUnitDescriptor(const void *, struct UsbDescriptor *,
    void *);
static void outOutputTerminalDescriptor(const void *, struct UsbDescriptor *,
    void *);

static void inStreamInterfaceAlt0Descriptor(const void *,
    struct UsbDescriptor *, void *);
static void inStreamInterfaceAlt1Descriptor(const void *,
    struct UsbDescriptor *, void *);
static void inAudioStreamingInterfaceDescriptor(const void *,
    struct UsbDescriptor *, void *);
static void inAudioFormatDescriptor(const void *, struct UsbDescriptor *,
    void *);
static void inStreamEndpointDescriptor(const void *, struct UsbDescriptor *,
    void *);
static void inStreamEndpointCSDescriptor(const void *, struct UsbDescriptor *,
    void *);

static void outStreamInterfaceAlt0Descriptor(const void *,
    struct UsbDescriptor *, void *);
static void outStreamInterfaceAlt1Descriptor(const void *,
    struct UsbDescriptor *, void *);
static void outAudioStreamingInterfaceDescriptor(const void *,
    struct UsbDescriptor *, void *);
static void outAudioFormatDescriptor(const void *, struct UsbDescriptor *,
    void *);
static void outStreamEndpointDescriptor(const void *, struct UsbDescriptor *,
    void *);
static void outStreamEndpointCSDescriptor(const void *, struct UsbDescriptor *,
    void *);
static void outStreamEndpointDescriptorFB(const void *, struct UsbDescriptor *,
    void *);
/*----------------------------------------------------------------------------*/
static enum Result handleCommonInterfaceRequest(struct UacBase *,
    const struct UsbSetupPacket *, void *, uint16_t *);
static enum Result handleInInterfaceRequest(struct UacBase *,
    const struct UsbSetupPacket *, void *, uint16_t *);
static enum Result handleOutInterfaceRequest(struct UacBase *,
    const struct UsbSetupPacket *, void *, uint16_t *);
/*----------------------------------------------------------------------------*/
static enum Result driverInit(void *, const void *);
static void driverDeinit(void *);
static enum Result driverControl(void *, const struct UsbSetupPacket *,
    void *, uint16_t *, uint16_t);
static const UsbDescriptorFunctor *driverDescribe(const void *);
static void driverNotify(void *, unsigned int);
/*----------------------------------------------------------------------------*/
const struct UsbDriverClass * const UacBase = &(const struct UsbDriverClass){
    .size = sizeof(struct UacBase),
    .init = driverInit,
    .deinit = driverDeinit,

    .control = driverControl,
    .describe = driverDescribe,
    .notify = driverNotify
};
/*----------------------------------------------------------------------------*/
static void buildDescriptorTable(struct UacBase *driver)
{
  UsbDescriptorFunctor *entry = driver->table;

  *entry++ = deviceDescriptor;
  *entry++ = configDescriptor;
  *entry++ = interfaceAssociationDescriptor;
  *entry++ = controlInterfaceDescriptor;
  *entry++ = controlInterfaceCSDescriptor;
  *entry++ = clockSourceDescriptor;

  if (driver->in)
  {
    *entry++ = inInputTerminalDescriptor;
    *entry++ = inFeatureUnitDescriptor;
    *entry++ = inOutputTerminalDescriptor;
  }
  if (driver->out)
  {
    *entry++ = outInputTerminalDescriptor;
    *entry++ = outFeatureUnitDescriptor;
    *entry++ = outOutputTerminalDescriptor;
  }

  if (driver->in)
  {
    *entry++ = inStreamInterfaceAlt0Descriptor;
    *entry++ = inStreamInterfaceAlt1Descriptor;
    *entry++ = inAudioStreamingInterfaceDescriptor;
    *entry++ = inAudioFormatDescriptor;
    *entry++ = inStreamEndpointDescriptor;
    *entry++ = inStreamEndpointCSDescriptor;
  }

  if (driver->out)
  {
    *entry++ = outStreamInterfaceAlt0Descriptor;
    *entry++ = outStreamInterfaceAlt1Descriptor;
    *entry++ = outAudioStreamingInterfaceDescriptor;
    *entry++ = outAudioFormatDescriptor;
    *entry++ = outStreamEndpointDescriptor;
    *entry++ = outStreamEndpointCSDescriptor;

    if (driver->endpoints.fb)
      *entry++ = outStreamEndpointDescriptorFB;
  }

  assert((size_t)(entry - driver->table) < ARRAY_SIZE(driver->table));
  *entry = NULL;
}
/*----------------------------------------------------------------------------*/
static void interfaceAssociationDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct UacBase * const driver = object;

  header->length = sizeof(struct UsbInterfaceAssociationDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION;

  if (payload)
  {
    struct UsbInterfaceAssociationDescriptor descriptor = {
        .length = sizeof(struct UsbInterfaceAssociationDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION,
        .firstInterface = driver->controlInterfaceIndex,
        .interfaceCount = 1,
        .functionClass = USB_CLASS_AUDIO,
        .functionSubClass = AUDIO_SUBCLASS_UNDEFINED,
        .functionProtocol = AUDIO_PROTOCOL_IP_VERSION_02_00,
        .function = 0
    };

    if (driver->in)
      ++descriptor.interfaceCount;
    if (driver->out)
      ++descriptor.interfaceCount;

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void deviceDescriptor(const void *, struct UsbDescriptor *header,
    void *payload)
{
  header->length = sizeof(struct UsbDeviceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_DEVICE;

  if (payload)
  {
    static const struct UsbDeviceDescriptor descriptor = {
        .length = sizeof(struct UsbDeviceDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_DEVICE,
        .usb = TO_LITTLE_ENDIAN_16(0x0200),
        .deviceClass = USB_CLASS_PER_INTERFACE,
        .deviceSubClass = 0,
        .deviceProtocol = 0,
        .maxPacketSize = TO_LITTLE_ENDIAN_16(UAC_CONTROL_EP_SIZE),
        .idVendor = 0,
        .idProduct = 0,
        .device = TO_LITTLE_ENDIAN_16(0x0100),
        .manufacturer = 0,
        .product = 0,
        .serialNumber = 0,
        .numConfigurations = 1
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void configDescriptor(const void *object, struct UsbDescriptor *header,
    void *payload)
{
  const struct UacBase * const driver = object;

  header->length = sizeof(struct UsbConfigurationDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CONFIGURATION;

  if (payload)
  {
    struct UsbConfigurationDescriptor descriptor = {
        .length = sizeof(struct UsbConfigurationDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_CONFIGURATION,
        .totalLength = sizeof(struct UsbConfigurationDescriptor)
            /* interfaceAssociationDescriptor */
            + sizeof(struct UsbInterfaceAssociationDescriptor)
            /* controlInterfaceDescriptor */
            + sizeof(struct UsbInterfaceDescriptor)
            /* controlInterfaceCSDescriptor */
            + sizeof(struct AudioControlInterface)
            /* clockSourceDescriptor */
            + sizeof(struct AudioClockSourceDescriptor),

        .numInterfaces = 1,
        .configurationValue = 1,
        .configuration = 0,
        .attributes = 0,
        .maxPower = 0
    };

    if (driver->in)
    {
      ++descriptor.numInterfaces;

      descriptor.totalLength +=
          /* inInputTerminalDescriptor */
          sizeof(struct AudioInputTerminalDescriptor)
          /* inFeatureUnitDescriptor */
          + sizeof(struct AudioFeatureUnitDescriptorCH0)
          /* inOutputTerminalDescriptor */
          + sizeof(struct AudioOutputTerminalDescriptor)
          /* inStreamInterfaceAlt0Descriptor */
          + sizeof(struct UsbInterfaceDescriptor)
          /* inStreamInterfaceAlt1Descriptor */
          + sizeof(struct UsbInterfaceDescriptor)
          /* inAudioStreamingInterfaceDescriptor */
          + sizeof(struct AudioStreamingInterfaceDescriptor)
          /* inAudioFormatDescriptor */
          + sizeof(struct AudioTypeIFormatDescriptor)
          /* inStreamEndpointDescriptor */
          + sizeof(struct UsbEndpointDescriptor)
          /* inStreamEndpointCSDescriptor */
          + sizeof(struct ClassSpecificAudioDataEndpointDescriptor);
    }
    if (driver->out)
    {
      ++descriptor.numInterfaces;

      descriptor.totalLength +=
          /* outInputTerminalDescriptor */
          sizeof(struct AudioInputTerminalDescriptor)
          /* outFeatureUnitDescriptor */
          + sizeof(struct AudioFeatureUnitDescriptorCH0)
          /* outOutputTerminalDescriptor */
          + sizeof(struct AudioOutputTerminalDescriptor)
          /* outStreamInterfaceAlt0Descriptor */
          + sizeof(struct UsbInterfaceDescriptor)
          /* outStreamInterfaceAlt1Descriptor */
          + sizeof(struct UsbInterfaceDescriptor)
          /* outAudioStreamingInterfaceDescriptor */
          + sizeof(struct AudioStreamingInterfaceDescriptor)
          /* outAudioFormatDescriptor */
          + sizeof(struct AudioTypeIFormatDescriptor)
          /* outStreamEndpointDescriptor */
          + sizeof(struct UsbEndpointDescriptor)
          /* outStreamEndpointCSDescriptor */
          + sizeof(struct ClassSpecificAudioDataEndpointDescriptor);

      if (driver->endpoints.fb)
      {
        /* outStreamEndpointDescriptorFB */
        descriptor.totalLength += sizeof(struct UsbEndpointDescriptor);
      }
    }

    descriptor.totalLength = toLittleEndian16(descriptor.totalLength);
    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void controlInterfaceDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct UacBase * const driver = object;

  header->length = sizeof(struct UsbInterfaceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_INTERFACE;

  if (payload)
  {
    const struct UsbInterfaceDescriptor descriptor = {
        .length = sizeof(struct UsbInterfaceDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_INTERFACE,
        .interfaceNumber = driver->controlInterfaceIndex,
        .alternateSettings = 0,
        .numEndpoints = 0,
        .interfaceClass = USB_CLASS_AUDIO,
        .interfaceSubClass = AUDIO_SUBCLASS_AUDIOCONTROL,
        .interfaceProtocol = AUDIO_PROTOCOL_IP_VERSION_02_00,
        .interface = 0
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void controlInterfaceCSDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct UacBase * const driver = object;

  header->length = sizeof(struct AudioControlInterface);
  header->descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE;

  if (payload)
  {
    struct AudioControlInterface descriptor = {
        .length = sizeof(struct AudioControlInterface),
        .descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE,
        .descriptorSubType = AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_HEADER,

        .adc = TO_LITTLE_ENDIAN_16(0x0200),
        .category = AUDIO_CATEGORY_OTHER,
        .totalLength =
            /* controlInterfaceCSDescriptor */
            sizeof(struct AudioControlInterface)
            /* clockSourceDescriptor */
            + sizeof(struct AudioClockSourceDescriptor),
        .controls = 0
    };

    if (driver->in)
    {
      descriptor.totalLength +=
          /* inInputTerminalDescriptor */
          sizeof(struct AudioInputTerminalDescriptor)
          /* inFeatureUnitDescriptor */
          + sizeof(struct AudioFeatureUnitDescriptorCH0)
          /* inOutputTerminalDescriptor */
          + sizeof(struct AudioOutputTerminalDescriptor);
    }
    if (driver->out)
    {
      descriptor.totalLength +=
          /* outInputTerminalDescriptor */
          sizeof(struct AudioInputTerminalDescriptor)
          /* outFeatureUnitDescriptor */
          + sizeof(struct AudioFeatureUnitDescriptorCH0)
          /* outOutputTerminalDescriptor */
          + sizeof(struct AudioOutputTerminalDescriptor);
    }

    descriptor.totalLength = toLittleEndian16(descriptor.totalLength);
    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void clockSourceDescriptor(const void *, struct UsbDescriptor *header,
    void *payload)
{
  header->length = sizeof(struct AudioClockSourceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE;

  if (payload)
  {
    const struct AudioClockSourceDescriptor descriptor = {
        .length = sizeof(struct AudioClockSourceDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE,
        .descriptorSubType = AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_CLOCK_SOURCE,
        .clockID = UNIT_CLOCK_SOURCE,
        .attributes = 1, // TODO Internal fixed clock
        .controls = 7, // TODO Host programmable, host read only
        .assocTerminal = 0,
        .clockSource = 1
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void inInputTerminalDescriptor(const void *,
    struct UsbDescriptor *header, void *payload)
{
  header->length = sizeof(struct AudioInputTerminalDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE;

  if (payload)
  {
    static const struct AudioInputTerminalDescriptor descriptor = {
        .length = sizeof(struct AudioInputTerminalDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE,
        .descriptorSubType = AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_INPUT_TERMINAL,

        .terminalID = UNIT_IN_INPUT_TERMINAL,
        .terminalType = TO_LITTLE_ENDIAN_16(AUDIO_TERMINAL_MICROPHONE),
        .assocTerminal = 0,
        .cSourceID = UNIT_CLOCK_SOURCE,
        .nrChannels = 2,
        .channelConfig = TO_LITTLE_ENDIAN_32(AUDIO_CHANNEL_L | AUDIO_CHANNEL_R),
        .channelNames = 0,
        .controls = 0,
        .terminal = 0
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void inFeatureUnitDescriptor(const void *, struct UsbDescriptor *header,
    void *payload)
{
  header->length = sizeof(struct AudioFeatureUnitDescriptorCH0);
  header->descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE;

  if (payload)
  {
    const struct AudioFeatureUnitDescriptorCH0 descriptor = {
        .length = sizeof(struct AudioFeatureUnitDescriptorCH0),
        .descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE,
        .descriptorSubType = AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_FEATURE_UNIT,
        .unitId = UNIT_IN_FEATURE_UNIT,
        .sourceId = UNIT_IN_INPUT_TERMINAL,
        /* Read-write Mute and Volume controls */
        // TODO Verify
        .controls = TO_LITTLE_ENDIAN_32(FU_CONTROL_MUTE | FU_CONTROL_VOLUME),
        .feature = 0
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void inOutputTerminalDescriptor(const void *,
    struct UsbDescriptor *header, void *payload)
{
  header->length = sizeof(struct AudioOutputTerminalDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE;

  if (payload)
  {
    const struct AudioOutputTerminalDescriptor descriptor = {
        .length = sizeof(struct AudioOutputTerminalDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE,
        .descriptorSubType =
            AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_OUTPUT_TERMINAL,

        .terminalID = UNIT_IN_OUTPUT_TERMINAL,
        .terminalType = TO_LITTLE_ENDIAN_16(AUDIO_TERMINAL_USB_STREAMING),
        .assocTerminal = 0,
        .sourceID = UNIT_IN_FEATURE_UNIT,
        .cSourceID = UNIT_CLOCK_SOURCE,
        .controls = 0,
        .terminal = 0
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void outInputTerminalDescriptor(const void *,
    struct UsbDescriptor *header, void *payload)
{
  header->length = sizeof(struct AudioInputTerminalDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE;

  if (payload)
  {
    static const struct AudioInputTerminalDescriptor descriptor = {
        .length = sizeof(struct AudioInputTerminalDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE,
        .descriptorSubType = AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_INPUT_TERMINAL,

        .terminalID = UNIT_OUT_INPUT_TERMINAL,
        .terminalType = TO_LITTLE_ENDIAN_16(AUDIO_TERMINAL_USB_STREAMING),
        .assocTerminal = 0,
        .cSourceID = UNIT_CLOCK_SOURCE,
        .nrChannels = 2,
        .channelConfig = TO_LITTLE_ENDIAN_32(AUDIO_CHANNEL_L | AUDIO_CHANNEL_R),
        .channelNames = 0,
        .controls = 0,
        .terminal = 0
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void outFeatureUnitDescriptor(const void *, struct UsbDescriptor *header,
    void *payload)
{
  header->length = sizeof(struct AudioFeatureUnitDescriptorCH0);
  header->descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE;

  if (payload)
  {
    const struct AudioFeatureUnitDescriptorCH0 descriptor = {
        .length = sizeof(struct AudioFeatureUnitDescriptorCH0),
        .descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE,
        .descriptorSubType = AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_FEATURE_UNIT,
        .unitId = UNIT_OUT_FEATURE_UNIT,
        .sourceId = UNIT_OUT_INPUT_TERMINAL,
        /* Read-write Mute and Volume controls */
        // TODO Verify
        .controls = TO_LITTLE_ENDIAN_32(FU_CONTROL_MUTE | FU_CONTROL_VOLUME),
        .feature = 0
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void outOutputTerminalDescriptor(const void *,
    struct UsbDescriptor *header, void *payload)
{
  header->length = sizeof(struct AudioOutputTerminalDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE;

  if (payload)
  {
    const struct AudioOutputTerminalDescriptor descriptor = {
        .length = sizeof(struct AudioOutputTerminalDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE,
        .descriptorSubType =
            AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_OUTPUT_TERMINAL,

        .terminalID = UNIT_OUT_OUTPUT_TERMINAL,
        .terminalType = TO_LITTLE_ENDIAN_16(AUDIO_TERMINAL_SPEAKER),
        .assocTerminal = 0,
        .sourceID = UNIT_OUT_FEATURE_UNIT,
        .cSourceID = UNIT_CLOCK_SOURCE,
        .controls = 0,
        .terminal = 0
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void inStreamInterfaceAlt0Descriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct UacBase * const driver = object;

  header->length = sizeof(struct UsbInterfaceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_INTERFACE;

  if (payload)
  {
    const struct UsbInterfaceDescriptor descriptor = {
        .length = sizeof(struct UsbInterfaceDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_INTERFACE,
        .interfaceNumber = driver->controlInterfaceIndex + 1,
        .alternateSettings = 0,
        .numEndpoints = 0,
        .interfaceClass = USB_CLASS_AUDIO,
        .interfaceSubClass = AUDIO_SUBCLASS_AUDIOSTREAMING,
        .interfaceProtocol = AUDIO_PROTOCOL_IP_VERSION_02_00,
        .interface = 0
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void inStreamInterfaceAlt1Descriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct UacBase * const driver = object;

  header->length = sizeof(struct UsbInterfaceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_INTERFACE;

  if (payload)
  {
    const struct UsbInterfaceDescriptor descriptor = {
        .length = sizeof(struct UsbInterfaceDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_INTERFACE,
        .interfaceNumber = driver->controlInterfaceIndex + 1,
        .alternateSettings = 1,
        .numEndpoints = 1,
        .interfaceClass = USB_CLASS_AUDIO,
        .interfaceSubClass = AUDIO_SUBCLASS_AUDIOSTREAMING,
        .interfaceProtocol = AUDIO_PROTOCOL_IP_VERSION_02_00,
        .interface = 0
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void inAudioStreamingInterfaceDescriptor(const void *,
    struct UsbDescriptor *header, void *payload)
{
  header->length = sizeof(struct AudioStreamingInterfaceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE;

  if (payload)
  {
    const struct AudioStreamingInterfaceDescriptor descriptor = {
        .length = sizeof(struct AudioStreamingInterfaceDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE,
        .descriptorSubType = AUDIO_CS_STREAMING_DESCRIPTOR_SUBTYPE_GENERAL,

        .terminalLink = UNIT_IN_OUTPUT_TERMINAL,
        .controls = 0,
        .formatType = AUDIO_FORMAT_TYPE_I,
        .formats = TO_LITTLE_ENDIAN_32(AUDIO_FORMAT_TYPE_I_PCM),
        .nrChannels = 2,
        .channelConfig = TO_LITTLE_ENDIAN_32(AUDIO_CHANNEL_L | AUDIO_CHANNEL_R),
        .channelNames = 0
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void inAudioFormatDescriptor(const void *, struct UsbDescriptor *header,
    void *payload)
{
  header->length = sizeof(struct AudioTypeIFormatDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE;

  if (payload)
  {
    const struct AudioTypeIFormatDescriptor descriptor = {
        .length = sizeof(struct AudioTypeIFormatDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE,
        .descriptorSubType = AUDIO_CS_STREAMING_DESCRIPTOR_SUBTYPE_FORMAT_TYPE,

        .formatType = AUDIO_FORMAT_TYPE_I,
        .subSlotSize = 2, // TODO From bit resolution
        .bitResolution = 16
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void inStreamEndpointDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct UacBase * const driver = object;

  header->length = sizeof(struct UsbEndpointDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_ENDPOINT;

  if (payload)
  {
    const struct UsbEndpointDescriptor descriptor = {
        .length = sizeof(struct UsbEndpointDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_ENDPOINT,
        .endpointAddress = driver->endpoints.tx,
        .attributes = ENDPOINT_DESCRIPTOR_TYPE(ENDPOINT_TYPE_ISOCHRONOUS)
            | (driver->endpoints.fb ?
                ENDPOINT_DESCRIPTOR_ASYNC : ENDPOINT_DESCRIPTOR_SYNC),
        .maxPacketSize = toLittleEndian16(driver->maxPacketSize),
        /* Interval is 2 ^ (4 - 1) microframes for HS devices */
        .interval = usbDevGetSpeed(driver->device) == USB_HS ? 4 : 1
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void inStreamEndpointCSDescriptor(const void *,
    struct UsbDescriptor *header, void *payload)
{
  header->length = sizeof(struct ClassSpecificAudioDataEndpointDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CS_ENDPOINT;

  if (payload)
  {
    const struct ClassSpecificAudioDataEndpointDescriptor descriptor = {
        .length = sizeof(struct ClassSpecificAudioDataEndpointDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_CS_ENDPOINT,
        .descriptorSubType = AUDIO_CS_ENDPOINT_DESCRIPTOR_SUBTYPE_GENERAL,
        .attributes = 0,
        .controls = 0,
        .lockDelayUnits = 0,
        .lockDelay = 0
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void outStreamInterfaceAlt0Descriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct UacBase * const driver = object;

  header->length = sizeof(struct UsbInterfaceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_INTERFACE;

  if (payload)
  {
    const struct UsbInterfaceDescriptor descriptor = {
        .length = sizeof(struct UsbInterfaceDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_INTERFACE,
        .interfaceNumber = driver->controlInterfaceIndex + (driver->in ? 2 : 1),
        .alternateSettings = 0,
        .numEndpoints = 0,
        .interfaceClass = USB_CLASS_AUDIO,
        .interfaceSubClass = AUDIO_SUBCLASS_AUDIOSTREAMING,
        .interfaceProtocol = AUDIO_PROTOCOL_IP_VERSION_02_00,
        .interface = 0
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void outStreamInterfaceAlt1Descriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct UacBase * const driver = object;

  header->length = sizeof(struct UsbInterfaceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_INTERFACE;

  if (payload)
  {
    struct UsbInterfaceDescriptor descriptor = {
        .length = sizeof(struct UsbInterfaceDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_INTERFACE,
        .interfaceNumber = driver->controlInterfaceIndex + (driver->in ? 2 : 1),
        .alternateSettings = 1,
        .numEndpoints = 1,
        .interfaceClass = USB_CLASS_AUDIO,
        .interfaceSubClass = AUDIO_SUBCLASS_AUDIOSTREAMING,
        .interfaceProtocol = AUDIO_PROTOCOL_IP_VERSION_02_00,
        .interface = 0
    };

    if (driver->endpoints.fb)
      ++descriptor.numEndpoints;

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void outAudioStreamingInterfaceDescriptor(const void *,
    struct UsbDescriptor *header, void *payload)
{
  header->length = sizeof(struct AudioStreamingInterfaceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE;

  if (payload)
  {
    const struct AudioStreamingInterfaceDescriptor descriptor = {
        .length = sizeof(struct AudioStreamingInterfaceDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE,
        .descriptorSubType = AUDIO_CS_STREAMING_DESCRIPTOR_SUBTYPE_GENERAL,

        .terminalLink = UNIT_OUT_INPUT_TERMINAL,
        .controls = 0,
        .formatType = AUDIO_FORMAT_TYPE_I,
        .formats = TO_LITTLE_ENDIAN_32(AUDIO_FORMAT_TYPE_I_PCM),
        .nrChannels = 2,
        .channelConfig = TO_LITTLE_ENDIAN_32(AUDIO_CHANNEL_L | AUDIO_CHANNEL_R),
        .channelNames = 0
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void outAudioFormatDescriptor(const void *, struct UsbDescriptor *header,
    void *payload)
{
  header->length = sizeof(struct AudioTypeIFormatDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE;

  if (payload)
  {
    const struct AudioTypeIFormatDescriptor descriptor = {
        .length = sizeof(struct AudioTypeIFormatDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE,
        .descriptorSubType = AUDIO_CS_STREAMING_DESCRIPTOR_SUBTYPE_FORMAT_TYPE,

        .formatType = AUDIO_FORMAT_TYPE_I,
        .subSlotSize = 2, // TODO From bit resolution
        .bitResolution = 16
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void outStreamEndpointDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct UacBase * const driver = object;

  header->length = sizeof(struct UsbEndpointDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_ENDPOINT;

  if (payload)
  {
    const struct UsbEndpointDescriptor descriptor = {
        .length = sizeof(struct UsbEndpointDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_ENDPOINT,
        .endpointAddress = driver->endpoints.rx,
        .attributes = ENDPOINT_DESCRIPTOR_TYPE(ENDPOINT_TYPE_ISOCHRONOUS)
            | (driver->endpoints.fb ?
                ENDPOINT_DESCRIPTOR_ASYNC : ENDPOINT_DESCRIPTOR_SYNC),
        .maxPacketSize = toLittleEndian16(driver->maxPacketSize),
        /* Interval is 2 ^ (4 - 1) microframes for HS devices */
        .interval = usbDevGetSpeed(driver->device) == USB_HS ? 4 : 1
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void outStreamEndpointCSDescriptor(const void *,
    struct UsbDescriptor *header, void *payload)
{
  header->length = sizeof(struct ClassSpecificAudioDataEndpointDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CS_ENDPOINT;

  if (payload)
  {
    const struct ClassSpecificAudioDataEndpointDescriptor descriptor = {
        .length = sizeof(struct ClassSpecificAudioDataEndpointDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_CS_ENDPOINT,
        .descriptorSubType = AUDIO_CS_ENDPOINT_DESCRIPTOR_SUBTYPE_GENERAL,
        .attributes = 0,
        .controls = 0,
        .lockDelayUnits = 0,
        .lockDelay = 0
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void outStreamEndpointDescriptorFB(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct UacBase * const driver = object;

  header->length = sizeof(struct UsbEndpointDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_ENDPOINT;

  if (payload)
  {
    const struct UsbEndpointDescriptor descriptor = {
        .length = sizeof(struct UsbEndpointDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_ENDPOINT,
        .endpointAddress = driver->endpoints.fb,
        .attributes = ENDPOINT_DESCRIPTOR_TYPE(ENDPOINT_TYPE_ISOCHRONOUS)
            | ENDPOINT_DESCRIPTOR_FEEDBACK,
        .maxPacketSize = toLittleEndian16(UAC_FEEDBACK_EP_SIZE),
        /* Interval is 2 ^ (4 - 1) microframes for HS devices */
        .interval = usbDevGetSpeed(driver->device) == USB_HS ? 4 : 1
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static enum Result handleCommonInterfaceRequest(struct UacBase *driver,
    const struct UsbSetupPacket *packet, void *buffer,
    uint16_t *responseLength)
{
  const uint8_t selector = (uint8_t)(packet->value >> 8);
  const uint8_t entity = (uint8_t)(packet->index >> 8);
  enum Result res = E_INVALID;

  if (REQUEST_DIRECTION_VALUE(packet->requestType) == REQUEST_DIRECTION_TO_HOST)
  {
    /* GET */
    if (entity == UNIT_CLOCK_SOURCE)
    {
      switch (selector)
      {
        case CS_CONTROL_SELECTOR_SAM_FREQ:
          if (packet->request == 1)
          {
            const uint32_t response = toLittleEndian32(
                uacOnSampleRateGetCurrent(driver->owner));

            memcpy(buffer, &response, sizeof(response));
            *responseLength = sizeof(response);
            res = E_OK;
          }
          else if (packet->request == 2)
          {
            uint8_t *position = (uint8_t *)buffer + sizeof(uint16_t);
            uint16_t index = 0;
            uint32_t value;

            while ((value = uacOnSampleRateGet(driver->owner, index)))
            {
              uint32_t tmp = toLittleEndian32(value);

              /* Fill min field */
              memcpy(position, &tmp, sizeof(tmp));
              position += sizeof(tmp);

              /* Fill max field */
              memcpy(position, &tmp, sizeof(tmp));
              position += sizeof(tmp);

              /* Fill res field */
              tmp = TO_LITTLE_ENDIAN_32(1);
              memcpy(position, &tmp, sizeof(tmp));
              position += sizeof(tmp);

              ++index;
            }

            /* Fill numSubRanges field */
            index = toLittleEndian16(index);
            memcpy(buffer, &index, sizeof(index));

            *responseLength = (size_t)(position - (uint8_t *)buffer);
            res = E_OK;
          }
          break;

        case CS_CONTROL_SELECTOR_CLOCK_VALID:
          if (packet->request == 1)
          {
            const uint8_t respo = 1;
            memcpy(buffer, &respo, sizeof(respo));
            *responseLength = sizeof(respo);
            res = E_OK;
          }
          break;

        default:
          break;
      }
    }
    else if (entity == UNIT_IN_FEATURE_UNIT)
    {
      switch (selector)
      {
        case FU_CONTROL_SELECTOR_MUTE:
          if (packet->request == 1)
          {
            const uint8_t respo = 0;
            memcpy(buffer, &respo, sizeof(respo));
            *responseLength = sizeof(respo);
            res = E_OK;
          }
          break;

        case FU_CONTROL_SELECTOR_VOLUME:
          break;

        default:
          break;
      }
    }
    else if (entity == UNIT_OUT_FEATURE_UNIT)
    {
      switch (selector)
      {
        case FU_CONTROL_SELECTOR_MUTE:
          if (packet->request == 1)
          {
            const uint8_t respo = 0;
            memcpy(buffer, &respo, sizeof(respo));
            *responseLength = sizeof(respo);
            res = E_OK;
          }
          break;

        case FU_CONTROL_SELECTOR_VOLUME:
          break;

        default:
          break;
      }
    }

    if (res != E_OK)
    {
      usbTrace("uac: unknown GET request with index %04X, value %04X",
         packet->index, packet->value);
    }
  }
  else
  {
    /* SET */
    if (entity == UNIT_CLOCK_SOURCE)
    {
      switch (selector)
      {
        case CS_CONTROL_SELECTOR_SAM_FREQ:
          if (packet->request == 1)
          {
            uint32_t value;

            memcpy(&value, buffer, sizeof(value));
            value = fromLittleEndian32(value);

            if (uacOnSampleRateSet(driver->owner, value))
              res = E_OK;

            usbTrace("uac: set sample rate %u", value);
          }
          break;

        default:
          break;
      }
    }
    else if (entity == UNIT_IN_FEATURE_UNIT)
    {
      switch (selector)
      {
        case FU_CONTROL_SELECTOR_MUTE:
          if (packet->request == 1)
          {
            // TODO Mute and volume control
            res = E_OK;
          }
          break;

        default:
          break;
      }
    }
    else if (entity == UNIT_OUT_FEATURE_UNIT)
    {
      switch (selector)
      {
        case FU_CONTROL_SELECTOR_MUTE:
          if (packet->request == 1)
          {
            // TODO Mute and volume control
            res = E_OK;
          }
          break;

        default:
          break;
      }
    }
  
    if (res != E_OK)
    {
      usbTrace("uac: unknown SET request with index %04X, value %04X",
          packet->index, packet->value);
    }
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result handleInInterfaceRequest(struct UacBase *driver,
    const struct UsbSetupPacket *packet, void *buffer,
    uint16_t *responseLength)
{
  enum Result res = E_OK;

  switch (packet->request)
  {
    case REQUEST_GET_INTERFACE:
      *(uint8_t *)buffer = (driver->txActive ? 1 : 0);
      *responseLength = 1;
      break;

    case REQUEST_SET_INTERFACE:
      usbTrace("uac: set interface %u", packet->value);

      /* Two alternative interfaces are supported */
      if (packet->value < 2)
        driver->txActive = (packet->value == 1);
      else
        res = E_VALUE;
      break;

    default:
      res = E_INVALID;
      break;
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result handleOutInterfaceRequest(struct UacBase *driver,
    const struct UsbSetupPacket *packet, void *buffer,
    uint16_t *responseLength)
{
  enum Result res = E_OK;

  switch (packet->request)
  {
    case REQUEST_GET_INTERFACE:
      *(uint8_t *)buffer = (driver->rxActive ? 1 : 0);
      *responseLength = 1;
      break;

    case REQUEST_SET_INTERFACE:
      usbTrace("uac: set interface %u", packet->value);

      /* Two alternative interfaces are supported */
      if (packet->value < 2)
        driver->rxActive = (packet->value == 1);
      else
        res = E_VALUE;
      break;

    default:
      res = E_INVALID;
      break;
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result driverInit(void *object, const void *configBase)
{
  const struct UacBaseConfig * const config = configBase;
  assert(config->owner != NULL);

  struct UacBase * const driver = object;

  driver->owner = config->owner;
  driver->device = config->device;
  driver->maxPacketSize = config->packet;
  driver->endpoints.fb = config->endpoints.fb;
  driver->endpoints.tx = config->endpoints.tx;
  driver->endpoints.rx = config->endpoints.rx;
  driver->controlInterfaceIndex = usbDevGetInterface(driver->device);
  driver->speed = USB_FS;

  driver->rxActive = false;
  driver->txActive = false;
  driver->in = driver->endpoints.tx != 0;
  driver->out = driver->endpoints.rx != 0;

  buildDescriptorTable(driver);
  return usbDevBind(driver->device, driver);
}
/*----------------------------------------------------------------------------*/
static void driverDeinit(void *object)
{
  struct UacBase * const driver = object;
  usbDevUnbind(driver->device, driver);
}
/*----------------------------------------------------------------------------*/
static enum Result driverControl(void *object,
    const struct UsbSetupPacket *packet, void *buffer, uint16_t *responseLength,
    uint16_t)
{
  struct UacBase * const driver = object;
  const uint8_t recipient = REQUEST_RECIPIENT_VALUE(packet->requestType);
  const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);
  enum Result res = E_INVALID;

  if (type == DESCRIPTOR_TYPE_DEVICE
      && recipient == REQUEST_RECIPIENT_ENDPOINT)
  {
    usbTrace("uac: unknown endpoint request with index %04X, value %04X",
        packet->index, packet->value);
  }

  if (type == DESCRIPTOR_TYPE_DEVICE
      && recipient == REQUEST_RECIPIENT_INTERFACE)
  {
    res = handleCommonInterfaceRequest(driver, packet, buffer, responseLength);
  }

  if (type == REQUEST_TYPE_STANDARD
      && recipient == REQUEST_RECIPIENT_INTERFACE)
  {
    const uint8_t inInterfaceIndex = driver->in ?
        (driver->controlInterfaceIndex + 1) : 0;
    const uint8_t outInterfaceIndex = driver->out ?
        (driver->controlInterfaceIndex + (driver->in ? 2 : 1)) : 0;

    if (inInterfaceIndex && (uint8_t)packet->index == inInterfaceIndex)
      res = handleInInterfaceRequest(driver, packet, buffer, responseLength);
    if (outInterfaceIndex && (uint8_t)packet->index == outInterfaceIndex)
      res = handleOutInterfaceRequest(driver, packet, buffer, responseLength);
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static const UsbDescriptorFunctor *driverDescribe(const void *object)
{
  const struct UacBase * const driver = object;
  return driver->table;
}
/*----------------------------------------------------------------------------*/
static void driverNotify(void *object, unsigned int event)
{
  struct UacBase * const driver = object;

#ifdef CONFIG_USB_DEVICE_HS
  if (event == USB_DEVICE_EVENT_PORT_CHANGE)
  {
    driver->speed = usbDevGetSpeed(driver->device);

    usbTrace("uac: current speed is %s",
        driver->speed == USB_HS ? "HS" : "FS");
  }
#endif

  switch ((enum UsbDeviceEvent)event)
  {
    case USB_DEVICE_EVENT_FRAME:
      uacOnEvent(driver->owner, USB_DEVICE_EVENT_FRAME);
      break;

    case USB_DEVICE_EVENT_RESET:
    case USB_DEVICE_EVENT_SUSPEND:
    case USB_DEVICE_EVENT_RESUME:
      driver->rxActive = false;
      driver->txActive = false;
      uacOnEvent(driver->owner, event);
      break;

    default:
      break;
  }
}
/*----------------------------------------------------------------------------*/
uint8_t uacBaseGetInterfaceIndex(const struct UacBase *driver)
{
  return driver->controlInterfaceIndex;
}
/*----------------------------------------------------------------------------*/
size_t uacBaseGetPacketSize(const struct UacBase *driver)
{
  return driver->maxPacketSize;
}
/*----------------------------------------------------------------------------*/
enum UsbSpeed uacBaseGetUsbSpeed(const struct UacBase *driver)
{
  return driver->speed;
}
/*----------------------------------------------------------------------------*/
bool uacBaseIsRxActive(const struct UacBase *driver)
{
  return driver->rxActive;
}
/*----------------------------------------------------------------------------*/
bool uacBaseIsTxActive(const struct UacBase *driver)
{
  return driver->txActive;
}
