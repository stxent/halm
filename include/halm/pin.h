/*
 * halm/pin.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PIN_H_
#define HALM_PIN_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <halm/target.h>
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
/*----------------------------------------------------------------------------*/
enum pinDirection
{
  PIN_INPUT,
  PIN_OUTPUT
};
/*----------------------------------------------------------------------------*/
enum pinEvent
{
  PIN_RISING,
  PIN_FALLING,
  PIN_TOGGLE
};
/*----------------------------------------------------------------------------*/
enum pinPull
{
  PIN_NOPULL,
  PIN_PULLUP,
  PIN_PULLDOWN
};
/*----------------------------------------------------------------------------*/
enum pinSlewRate
{
  PIN_SLEW_FAST,
  PIN_SLEW_NORMAL,
  PIN_SLEW_SLOW
};
/*----------------------------------------------------------------------------*/
enum pinType
{
  PIN_PUSHPULL,
  PIN_OPENDRAIN
};
/*----------------------------------------------------------------------------*/
struct PinData
{
  uint8_t offset;
  uint8_t port;
};
/*----------------------------------------------------------------------------*/
struct PinEntry
{
  PinNumber key;
  uint8_t channel;
  uint8_t value;
};
/*----------------------------------------------------------------------------*/
struct PinGroupEntry
{
  PinNumber begin;
  PinNumber end;
  uint8_t channel;
  uint8_t value;
};
/*----------------------------------------------------------------------------*/
struct Pin
{
  void *reg;
  struct PinData data;
};
/*----------------------------------------------------------------------------*/
const struct PinEntry *pinFind(const struct PinEntry *, PinNumber, uint8_t);
const struct PinGroupEntry *pinGroupFind(const struct PinGroupEntry *,
    PinNumber, uint8_t);
/*----------------------------------------------------------------------------*/
static inline bool pinValid(struct Pin pin)
{
  return pin.data.port != 0xFF && pin.data.offset != 0xFF && pin.reg != 0;
}
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/pin.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
#endif /* HALM_PIN_H_ */
