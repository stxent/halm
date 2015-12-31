/*
 * usb/hid_defs.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef USB_HID_DEFS_H_
#define USB_HID_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <bits.h>
/*----------------------------------------------------------------------------*/
#define HID_CONTROL_EP_SIZE 64
/*----------------------------------------------------------------------------*/
/* Descriptor types */
enum
{
  DESCRIPTOR_TYPE_HID           = 0x21,
  DESCRIPTOR_TYPE_HID_REPORT    = 0x22,
  DESCRIPTOR_TYPE_HID_PHYSICAL  = 0x23
};
/*----------------------------------------------------------------------------*/
enum
{
  HID_REQUEST_GET_REPORT    = 0x01,
  HID_REQUEST_GET_IDLE      = 0x02,
  HID_REQUEST_GET_PROTOCOL  = 0x03,
  HID_REQUEST_SET_REPORT    = 0x09,
  HID_REQUEST_SET_IDLE      = 0x0A,
  HID_REQUEST_SET_PROTOCOL  = 0x0B
};

enum
{
  HID_SUBCLASS_NONE = 0,
  HID_SUBCLASS_BOOT = 1
};

enum
{
  HID_PROTOCOL_NONE     = 0,
  HID_PROTOCOL_KEYBOARD = 1,
  HID_PROTOCOL_MOUSE    = 2
};

enum
{
  HID_REPORT_INPUT    = 0x01,
  HID_REPORT_OUTPUT   = 0x02,
  HID_REPORT_FEATURE  = 0x03
};
/*----------------------------------------------------------------------------*/
enum
{
  HID_USAGE_PAGE_UNDEFINED    = 0x00,
  HID_USAGE_PAGE_GENERIC      = 0x01,
  HID_USAGE_PAGE_SIMULATION   = 0x02,
  HID_USAGE_PAGE_VR           = 0x03,
  HID_USAGE_PAGE_SPORT        = 0x04,
  HID_USAGE_PAGE_GAME         = 0x05,
  HID_USAGE_PAGE_DEV_CONTROLS = 0x06,
  HID_USAGE_PAGE_KEYBOARD     = 0x07,
  HID_USAGE_PAGE_LED          = 0x08,
  HID_USAGE_PAGE_BUTTON       = 0x09,
  HID_USAGE_PAGE_ORDINAL      = 0x0A,
  HID_USAGE_PAGE_TELEPHONY    = 0x0B,
  HID_USAGE_PAGE_CONSUMER     = 0x0C,
  HID_USAGE_PAGE_DIGITIZER    = 0x0D,
  HID_USAGE_PAGE_UNICODE      = 0x10,
  HID_USAGE_PAGE_ALPHANUMERIC = 0x14
};

/* LED Page */
enum
{
  HID_USAGE_LED_NUM_LOCK              = 0x01,
  HID_USAGE_LED_CAPS_LOCK             = 0x02,
  HID_USAGE_LED_SCROLL_LOCK           = 0x03,
  HID_USAGE_LED_COMPOSE               = 0x04,
  HID_USAGE_LED_KANA                  = 0x05,
  HID_USAGE_LED_POWER                 = 0x06,
  HID_USAGE_LED_SHIFT                 = 0x07,
  HID_USAGE_LED_DO_NOT_DISTURB        = 0x08,
  HID_USAGE_LED_MUTE                  = 0x09,
  HID_USAGE_LED_TONE_ENABLE           = 0x0A,
  HID_USAGE_LED_HIGH_CUT_FILTER       = 0x0B,
  HID_USAGE_LED_LOW_CUT_FILTER        = 0x0C,
  HID_USAGE_LED_EQUALIZER_ENABLE      = 0x0D,
  HID_USAGE_LED_SOUND_FIELD_ON        = 0x0E,
  HID_USAGE_LED_SURROUND_FIELD_ON     = 0x0F,
  HID_USAGE_LED_REPEAT                = 0x10,
  HID_USAGE_LED_STEREO                = 0x11,
  HID_USAGE_LED_SAMPLING_RATE_DETECT  = 0x12,
  HID_USAGE_LED_SPINNING              = 0x13,
  HID_USAGE_LED_CAV                   = 0x14,
  HID_USAGE_LED_CLV                   = 0x15,
  HID_USAGE_LED_RECORDING_FORMAT_DET  = 0x16,
  HID_USAGE_LED_OFF_HOOK              = 0x17,
  HID_USAGE_LED_RING                  = 0x18,
  HID_USAGE_LED_MESSAGE_WAITING       = 0x19,
  HID_USAGE_LED_DATA_MODE             = 0x1A,
  HID_USAGE_LED_BATTERY_OPERATION     = 0x1B,
  HID_USAGE_LED_BATTERY_OK            = 0x1C,
  HID_USAGE_LED_BATTERY_LOW           = 0x1D,
  HID_USAGE_LED_SPEAKER               = 0x1E,
  HID_USAGE_LED_HEAD_SET              = 0x1F,
  HID_USAGE_LED_HOLD                  = 0x20,
  HID_USAGE_LED_MICROPHONE            = 0x21,
  HID_USAGE_LED_COVERAGE              = 0x22,
  HID_USAGE_LED_NIGHT_MODE            = 0x23,
  HID_USAGE_LED_SEND_CALLS            = 0x24,
  HID_USAGE_LED_CALL_PICKUP           = 0x25,
  HID_USAGE_LED_CONFERENCE            = 0x26,
  HID_USAGE_LED_STAND_BY              = 0x27,
  HID_USAGE_LED_CAMERA_ON             = 0x28,
  HID_USAGE_LED_CAMERA_OFF            = 0x29,
  HID_USAGE_LED_ON_LINE               = 0x2A,
  HID_USAGE_LED_OFF_LINE              = 0x2B,
  HID_USAGE_LED_BUSY                  = 0x2C,
  HID_USAGE_LED_READY                 = 0x2D,
  HID_USAGE_LED_PAPER_OUT             = 0x2E,
  HID_USAGE_LED_PAPER_JAM             = 0x2F,
  HID_USAGE_LED_REMOTE                = 0x30,
  HID_USAGE_LED_FORWARD               = 0x31,
  HID_USAGE_LED_REVERSE               = 0x32,
  HID_USAGE_LED_STOP                  = 0x33,
  HID_USAGE_LED_REWIND                = 0x34,
  HID_USAGE_LED_FAST_FORWARD          = 0x35,
  HID_USAGE_LED_PLAY                  = 0x36,
  HID_USAGE_LED_PAUSE                 = 0x37,
  HID_USAGE_LED_RECORD                = 0x38,
  HID_USAGE_LED_ERROR                 = 0x39,
  HID_USAGE_LED_SELECTED_INDICATOR    = 0x3A,
  HID_USAGE_LED_IN_USE_INDICATOR      = 0x3B,
  HID_USAGE_LED_MULTI_MODE_INDICATOR  = 0x3C,
  HID_USAGE_LED_INDICATOR_ON          = 0x3D,
  HID_USAGE_LED_INDICATOR_FLASH       = 0x3E,
  HID_USAGE_LED_INDICATOR_SLOW_BLINK  = 0x3F,
  HID_USAGE_LED_INDICATOR_FAST_BLINK  = 0x40,
  HID_USAGE_LED_INDICATOR_OFF         = 0x41,
  HID_USAGE_LED_FLASH_ON_TIME         = 0x42,
  HID_USAGE_LED_SLOW_BLINK_ON_TIME    = 0x43,
  HID_USAGE_LED_SLOW_BLINK_OFF_TIME   = 0x44,
  HID_USAGE_LED_FAST_BLINK_ON_TIME    = 0x45,
  HID_USAGE_LED_FAST_BLINK_OFF_TIME   = 0x46,
  HID_USAGE_LED_INDICATOR_COLOR       = 0x47,
  HID_USAGE_LED_RED                   = 0x48,
  HID_USAGE_LED_GREEN                 = 0x49,
  HID_USAGE_LED_AMBER                 = 0x4A,
  HID_USAGE_LED_GENERIC_INDICATOR     = 0x4B
};

/* Collection items */
enum
{
  HID_PHYSICAL        = 0x00,
  HID_APPLICATION     = 0x01,
  HID_LOGICAL         = 0x02,
  HID_REPORT          = 0x03,
  HID_NAMED_ARRAY     = 0x04,
  HID_USAGE_SWITCH    = 0x05,
  HID_USAGE_MODIFIER  = 0x06
};

/* Global Items */
#define REPORT_USAGE_PAGE(x)            0x05, x
#define REPORT_USAGE_PAGE_VENDOR(x)     0x06, x, 0xFF
#define REPORT_LOGICAL_MIN(x)           0x15, x
#define REPORT_LOGICAL_MIN_SHORT(x) \
    0x16, x & 0xFF, (x >> 8) & 0xFF
#define REPORT_LOGICAL_MIN_LONG(x) \
    0x17, x & 0xFF, (x >> 8) & 0xFF, (x >> 16) & 0xFF, (x >> 24) & 0xFF
#define REPORT_LOGICAL_MAX(x)           0x25, x
#define REPORT_LOGICAL_MAX_SHORT(x) \
    0x26, x & 0xFF, (x >> 8) & 0xFF
#define REPORT_LOGICAL_MAX_LONG(x) \
    0x27, x & 0xFF, (x >> 8) & 0xFF, (x >> 16) & 0xFF, (x >> 24) & 0xFF
#define REPORT_PHYSICAL_MIN(x)          0x35, x
#define REPORT_PHYSICAL_MIN_SHORT(x) \
    0x36, x & 0xFF, (x >> 8) & 0xFF
#define REPORT_PHYSICAL_MIN_LONG(x) \
    0x37, x & 0xFF, (x >> 8) & 0xFF, (x >> 16) & 0xFF, (x >> 24) & 0xFF
#define REPORT_PHYSICAL_MAX(x)          0x45, x
#define REPORT_PHYSICAL_MAX_SHORT(x) \
    0x46, x & 0xFF, (x >> 8) & 0xFF
#define REPORT_PHYSICAL_MAX_LONG(x) \
    0x47, x & 0xFF, (x >> 8) & 0xFF, (x >> 16) & 0xFF, (x >> 24) & 0xFF
#define REPORT_UNIT_EXPONENT(x)         0x55, x
#define REPORT_UNIT(x)                  0x65, x
#define REPORT_UNIT_SHORT(x) \
    0x66, x & 0xFF, (x >> 8) & 0xFF
#define REPORT_UNIT_LONG(x) \
    0x67, x & 0xFF, (x >> 8) & 0xFF, (x >> 16) & 0xFF, (x >> 24) & 0xFF
#define REPORT_SIZE(x)                  0x75, x
#define REPORT_ID(x)                    0x85, x
#define REPORT_COUNT(x)                 0x95, x
#define REPORT_PUSH                     0xA0
#define REPORT_POP                      0xB0

/* Main Items */
#define REPORT_INPUT(x)                 0x81, x
#define REPORT_OUTPUT(x)                0x91, x
#define REPORT_FEATURE(x)               0xB1, x
#define REPORT_COLLECTION(x)            0xA1, x
#define REPORT_END_COLLECTION           0xC0

#define REPORT_USAGE(x)                 0x09, x
#define REPORT_USAGE_MIN(x)             0x19, x
#define REPORT_USAGE_MAX(x)             0x29, x

/* Input, Output and Feature items */
#define HID_DATA               0
#define HID_CONSTANT           BIT(0)
#define HID_ARRAY              0
#define HID_VARIABLE           BIT(1)
#define HID_ABSOLUTE           0
#define HID_RELATIVE           BIT(2)
#define HID_NO_WRAP            0
#define HID_WRAP               BIT(3)
#define HID_LINEAR             0
#define HID_NONLINEAR          BIT(4)
#define HID_PREFERRED_STATE    0
#define HID_NO_PREFERRED       BIT(5)
#define HID_NO_NULL_POSITION   0
#define HID_NULL_STATE         BIT(6)
#define HID_NONVOLATILE        0
#define HID_VOLATILE           BIT(7)
/*----------------------------------------------------------------------------*/
struct HidDescriptorBase
{
  uint8_t length;
  uint8_t descriptorType;
  uint16_t hid;
  uint8_t countryCode;
  uint8_t numDescriptors;
} __attribute__((packed));

struct HidDescriptorEntry
{
  uint8_t type;
  uint16_t length;
} __attribute__((packed));

struct AbstractHidDescriptor
{
  struct HidDescriptorBase base;
  struct HidDescriptorEntry entries[];
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
#endif /* USB_HID_DEFS_H_ */
