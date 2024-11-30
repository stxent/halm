/*
 * halm/usb/uac_defs.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_USB_UAC_DEFS_H_
#define HALM_USB_UAC_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <halm/usb/usb_defs.h>
/*----------------------------------------------------------------------------*/
#define RATE_FB_MUL           17
#define RATE_FB_OFFSET        14
#define RATE_FB_OFFSET_HS     16

#define UAC_CONTROL_EP_SIZE   64
#define UAC_FEEDBACK_EP_SIZE  4
/*----------------------------------------------------------------------------*/
/* Audio Interface Subclass Codes */
enum
{
  AUDIO_SUBCLASS_UNDEFINED      = 0x00,
  AUDIO_SUBCLASS_AUDIOCONTROL   = 0x01,
  AUDIO_SUBCLASS_AUDIOSTREAMING = 0x02,
  AUDIO_SUBCLASS_MIDISTREAMING  = 0x03
};

/* Audio Interface Protocol Codes */
enum
{
  AUDIO_PROTOCOL_UNDEFINED        = 0x00,
  AUDIO_PROTOCOL_IP_VERSION_02_00 = 0x20
};

/* Audio Control Interface Descriptor Subtypes */
enum
{
  AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_UNDEFINED             = 0x00,
  AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_HEADER                = 0x01,
  AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_INPUT_TERMINAL        = 0x02,
  AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_OUTPUT_TERMINAL       = 0x03,
  AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_MIXER_UNIT            = 0x04,
  AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_SELECTOR_UNIT         = 0x05,
  AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_FEATURE_UNIT          = 0x06,
  AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_EFFEC_UNIT            = 0x07,
  AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_PROCESSING_UNIT       = 0x08,
  AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_EXTENSION_UNIT        = 0x09,
  AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_CLOCK_SOURCE          = 0x0A,
  AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_CLOCK_SELECTOR        = 0x0B,
  AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_CLOCK_MULTIPLIER      = 0x0C,
  AUDIO_CS_CONTROL_DESCRIPTOR_SUBTYPE_SAMPLE_RATE_CONVERTER = 0x0D
};

/* Audio Streaming Interface Descriptor Subtypes */
enum
{
  AUDIO_CS_STREAMING_DESCRIPTOR_SUBTYPE_UNDEFINED    = 0x00,
  AUDIO_CS_STREAMING_DESCRIPTOR_SUBTYPE_GENERAL      = 0x01,
  AUDIO_CS_STREAMING_DESCRIPTOR_SUBTYPE_FORMAT_TYPE  = 0x02,
  AUDIO_CS_STREAMING_DESCRIPTOR_SUBTYPE_ENCODER      = 0x03,
  AUDIO_CS_STREAMING_DESCRIPTOR_SUBTYPE_DECODER      = 0x04
};

/* Audio Endpoint Descriptor Subtypes */
enum
{
  AUDIO_CS_ENDPOINT_DESCRIPTOR_SUBTYPE_UNDEFINED  = 0x00,
  AUDIO_CS_ENDPOINT_DESCRIPTOR_SUBTYPE_GENERAL    = 0x01
};

/* Audio Function Category Codes */
enum
{
  AUDIO_CATEGORY_UNDEFINED            = 0x00,
  AUDIO_CATEGORY_DESKTOP_SPEAKER      = 0x01,
  AUDIO_CATEGORY_HOME_THEATER         = 0x02,
  AUDIO_CATEGORY_MICROPHONE           = 0x03,
  AUDIO_CATEGORY_HEADSET              = 0x04,
  AUDIO_CATEGORY_TELEPHONE            = 0x05,
  AUDIO_CATEGORY_CONVERTER            = 0x06,
  AUDIO_CATEGORY_VOICE_SOUND_RECORDER = 0x07,
  AUDIO_CATEGORY_IO_BOX               = 0x08,
  AUDIO_CATEGORY_MUSICAL_INSTRUMENT   = 0x09,
  AUDIO_CATEGORY_PRO_AUDIO            = 0x0A,
  AUDIO_CATEGORY_AUDIO_VIDEO          = 0x0B,
  AUDIO_CATEGORY_CONTROL_PANEL        = 0x0C,
  AUDIO_CATEGORY_OTHER                = 0xFF
};

/* Audio Format Types */
enum
{
  AUDIO_FORMAT_TYPE_UNDEFINED = 0x00,
  AUDIO_FORMAT_TYPE_I         = 0x01,
  AUDIO_FORMAT_TYPE_II        = 0x02,
  AUDIO_FORMAT_TYPE_III       = 0x03
};

/* Audio Data Format Type I Codes */
enum
{
  AUDIO_FORMAT_TYPE_I_UNDEFINED   = 0x0000,
  AUDIO_FORMAT_TYPE_I_PCM         = 0x0001,
  AUDIO_FORMAT_TYPE_I_PCM8        = 0x0002,
  AUDIO_FORMAT_TYPE_I_IEEE_FLOAT  = 0x0003,
  AUDIO_FORMAT_TYPE_I_ALAW        = 0x0004,
  AUDIO_FORMAT_TYPE_I_MULAW       = 0x0005
};

/* Audio Channel Configuration Bits */
enum
{
  AUDIO_CHANNEL_M   = 0x0000, /* Mono */
  AUDIO_CHANNEL_L   = 0x0001, /* Left Front */
  AUDIO_CHANNEL_R   = 0x0002, /* Right Front */
  AUDIO_CHANNEL_C   = 0x0004, /* Center Front */
  AUDIO_CHANNEL_LFE = 0x0008, /* Low Frequency Enhancement */
  AUDIO_CHANNEL_LS  = 0x0010, /* Left Surround */
  AUDIO_CHANNEL_RS  = 0x0020, /* Right Surround */
  AUDIO_CHANNEL_LC  = 0x0040, /* Left of Center */
  AUDIO_CHANNEL_RC  = 0x0080, /* Right of Center */
  AUDIO_CHANNEL_S   = 0x0100, /* Surround */
  AUDIO_CHANNEL_SL  = 0x0200, /* Side Left */
  AUDIO_CHANNEL_SR  = 0x0400, /* Side Right */
  AUDIO_CHANNEL_T   = 0x0800  /* Top */
};

/* Clock Source Control Selectors */
enum
{
  CS_CONTROL_SELECTOR_SAM_FREQ    = 0x01,
  CS_CONTROL_SELECTOR_CLOCK_VALID = 0x02
};

/* Feature Unit Control Bits */
enum
{
  FU_CONTROL_MUTE               = 0x0001,
  FU_CONTROL_VOLUME             = 0x0002,
  FU_CONTROL_BASS               = 0x0004,
  FU_CONTROL_MID                = 0x0008,
  FU_CONTROL_TREBLE             = 0x0010,
  FU_CONTROL_GRAPHIC_EQUALIZER  = 0x0020,
  FU_CONTROL_AUTOMATIC_GAIN     = 0x0040,
  FU_CONTROL_DEALY              = 0x0080,
  FU_CONTROL_BASS_BOOST         = 0x0100,
  FU_CONTROL_LOUDNESS           = 0x0200
};

/* Feature Unit Control Selectors */
enum
{
  FU_CONTROL_SELECTOR_MUTE              = 0x01,
  FU_CONTROL_SELECTOR_VOLUME            = 0x02,
  FU_CONTROL_SELECTOR_BASS              = 0x03,
  FU_CONTROL_SELECTOR_MID               = 0x04,
  FU_CONTROL_SELECTOR_TREBLE            = 0x05,
  FU_CONTROL_SELECTOR_GRAPHIC_EQUALIZER = 0x06,
  FU_CONTROL_SELECTOR_AUTOMATIC_GAIN    = 0x07,
  FU_CONTROL_SELECTOR_DELAY             = 0x08,
  FU_CONTROL_SELECTOR_BASS_BOOST        = 0x09,
  FU_CONTROL_SELECTOR_LOUDNESS          = 0x0A
};

/* Audio Terminal Types */
enum
{
  /* USB Terminal Types */
  AUDIO_TERMINAL_USB_UNDEFINED          = 0x0100,
  AUDIO_TERMINAL_USB_STREAMING          = 0x0101,
  AUDIO_TERMINAL_USB_VENDOR_SPECIFIC    = 0x01FF,

  /* Input Terminal Types */
  AUDIO_TERMINAL_INPUT_UNDEFINED        = 0x0200,
  AUDIO_TERMINAL_MICROPHONE             = 0x0201,
  AUDIO_TERMINAL_DESKTOP_MICROPHONE     = 0x0202,
  AUDIO_TERMINAL_PERSONAL_MICROPHONE    = 0x0203,
  AUDIO_TERMINAL_OMNI_DIR_MICROPHONE    = 0x0204,
  AUDIO_TERMINAL_MICROPHONE_ARRAY       = 0x0205,
  AUDIO_TERMINAL_PROCESSING_MIC_ARRAY   = 0x0206,

  /* Output Terminal Types */
  AUDIO_TERMINAL_OUTPUT_UNDEFINED       = 0x0300,
  AUDIO_TERMINAL_SPEAKER                = 0x0301,
  AUDIO_TERMINAL_HEADPHONES             = 0x0302,
  AUDIO_TERMINAL_HEAD_MOUNTED_AUDIO     = 0x0303,
  AUDIO_TERMINAL_DESKTOP_SPEAKER        = 0x0304,
  AUDIO_TERMINAL_ROOM_SPEAKER           = 0x0305,
  AUDIO_TERMINAL_COMMUNICATION_SPEAKER  = 0x0306,
  AUDIO_TERMINAL_LOW_FREQ_SPEAKER       = 0x0307
};
/*----------------------------------------------------------------------------*/
struct [[gnu::packed]] AudioControlInterface
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t descriptorSubType;
  uint16_t adc;
  uint8_t category;
  uint16_t totalLength;
  uint8_t controls;
};

struct [[gnu::packed]] AudioInputTerminalDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t descriptorSubType;

  uint8_t terminalID;
  uint16_t terminalType;
  uint8_t assocTerminal;
  uint8_t cSourceID;
  uint8_t nrChannels;
  uint32_t channelConfig;
  uint8_t channelNames;
  uint16_t controls;
  uint8_t terminal;
};

struct [[gnu::packed]] AudioOutputTerminalDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t descriptorSubType;

  uint8_t terminalID;
  uint16_t terminalType;
  uint8_t assocTerminal;
  uint8_t sourceID;
  uint8_t cSourceID;
  uint16_t controls;
  uint8_t terminal;
};

struct [[gnu::packed]] AudioStreamingInterfaceDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t descriptorSubType;

  uint8_t terminalLink;
  uint8_t controls;
  uint8_t formatType;
  uint32_t formats;
  uint8_t nrChannels;
  uint32_t channelConfig;
  uint8_t channelNames;
};

struct [[gnu::packed]] TypeIFormatSampleRate
{
  uint8_t rate[3];
};

struct [[gnu::packed]] AudioTypeIFormatDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t descriptorSubType;

  uint8_t formatType;
  uint8_t subSlotSize;
  uint8_t bitResolution;
};

struct [[gnu::packed]] ClassSpecificAudioDataEndpointDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t descriptorSubType;

  uint8_t attributes;
  uint8_t controls;
  uint8_t lockDelayUnits;
  uint16_t lockDelay;
};

struct [[gnu::packed]] AudioFeatureUnitDescriptorCH0
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t descriptorSubType;

  uint8_t unitId;
  uint8_t sourceId;
  uint32_t controls;
  uint8_t feature;
};

struct [[gnu::packed]] AudioClockSourceDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint8_t descriptorSubType;

  uint8_t clockID;
  uint8_t attributes;
  uint8_t controls;
  uint8_t assocTerminal;
  uint8_t clockSource;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_UAC_DEFS_H_ */
