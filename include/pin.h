/*
 * pin.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PIN_H_
#define PIN_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <libhalm/target.h>
/*----------------------------------------------------------------------------*/
typedef uint16_t pinNumber;
/*----------------------------------------------------------------------------*/
/*
 * External pin id consist of port number and offset in 1's complement form.
 * Unused pins should be initialized with zero.
 * This representation supports up to 2^7 ports and 2^8 pins on each port.
 */
#define PIN(port, offset) ((pinNumber)(~(((unsigned long)(port) << 8 & 0xFF00)\
    | ((unsigned long)(offset) & 0x00FF))))
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
  PIN_SLEW_SLOW,
};
/*----------------------------------------------------------------------------*/
enum pinType
{
  PIN_PUSHPULL,
  PIN_OPENDRAIN
};
/*----------------------------------------------------------------------------*/
union PinData
{
  pinNumber key;
  struct
  {
    uint8_t offset;
    uint8_t port;
  };
};
/*----------------------------------------------------------------------------*/
struct PinEntry
{
  pinNumber key;
  uint8_t channel;
  uint8_t value;
};
/*----------------------------------------------------------------------------*/
struct PinGroupEntry
{
  pinNumber begin;
  pinNumber end;
  uint8_t channel;
  uint8_t value;
};
/*----------------------------------------------------------------------------*/
struct Pin
{
  void *reg;
  union PinData data;
};
/*----------------------------------------------------------------------------*/
const struct PinEntry *pinFind(const struct PinEntry *, pinNumber, uint8_t);
const struct PinGroupEntry *pinGroupFind(const struct PinGroupEntry *,
    pinNumber, uint8_t);
/*----------------------------------------------------------------------------*/
static inline bool pinValid(struct Pin pin)
{
  return pin.data.key != (pinNumber)~0 && pin.reg != 0;
}
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/pin.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
#endif /* PIN_H_ */
