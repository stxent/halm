/*
 * halm/pin.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PIN_H_
#define HALM_PIN_H_
/*----------------------------------------------------------------------------*/
#include <halm/target.h>
#include <xcore/helpers.h>
#include <stdbool.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
typedef uint16_t PinNumber;
/*----------------------------------------------------------------------------*/
/*
 * External pin id consist of port number and offset in 1's complement form.
 * Unused pins should be initialized with zero.
 * This representation supports up to 2^7 ports and 2^8 pins on each port.
 */
#define PIN(port, offset) \
    ((PinNumber)~((((port) & 0xFF) << 8) | ((offset) & 0xFF)))

#define PIN_TO_OFFSET(key)  (~(key) & 0xFF)
#define PIN_TO_PORT(key)    ((~(key) >> 8) & 0xFF)
/*----------------------------------------------------------------------------*/
/* Special function values */
enum
{
  PIN_DEFAULT = 0x7F,
  PIN_ANALOG  = 0x7E
};

enum PinDirection
{
  PIN_INPUT,
  PIN_OUTPUT
} __attribute__((packed));

enum PinEvent
{
  PIN_RISING,
  PIN_FALLING,
  PIN_TOGGLE
} __attribute__((packed));

enum PinPull
{
  PIN_NOPULL,
  PIN_PULLUP,
  PIN_PULLDOWN
} __attribute__((packed));

enum PinSlewRate
{
  PIN_SLEW_FAST,
  PIN_SLEW_NORMAL,
  PIN_SLEW_SLOW
} __attribute__((packed));

enum PinType
{
  PIN_PUSHPULL,
  PIN_OPENDRAIN
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
struct PinEntry
{
  PinNumber key;
  uint8_t channel;
  uint8_t value;
};

struct PinGroupEntry
{
  PinNumber begin;
  PinNumber end;
  uint8_t channel;
  uint8_t value;
};
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/pin.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

const struct PinEntry *pinFind(const struct PinEntry *, PinNumber, uint8_t);
const struct PinGroupEntry *pinGroupFind(const struct PinGroupEntry *,
    PinNumber, uint8_t);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PIN_H_ */
