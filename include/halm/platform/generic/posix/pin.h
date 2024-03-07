/*
 * halm/platform/generic/posix/pin.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_GENERIC_POSIX_PIN_H_
#define HALM_PLATFORM_GENERIC_POSIX_PIN_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
struct Pin
{
  int handle;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline struct Pin pinInit([[maybe_unused]] PinNumber id)
{
  return (struct Pin){-1};
}

static inline void pinInput([[maybe_unused]] struct Pin pin)
{
}

static inline void pinOutput([[maybe_unused]] struct Pin pin,
    [[maybe_unused]] bool value)
{
}

static inline bool pinRead([[maybe_unused]] struct Pin pin)
{
  return false;
}

static inline void pinReset([[maybe_unused]] struct Pin pin)
{
}

static inline void pinSet([[maybe_unused]] struct Pin pin)
{
}

static inline struct Pin pinStub(void)
{
  return (struct Pin){-1};
}

static inline void pinToggle([[maybe_unused]] struct Pin pin)
{
}

static inline bool pinValid([[maybe_unused]] struct Pin pin)
{
  return false;
}

static inline void pinWrite([[maybe_unused]] struct Pin pin,
    [[maybe_unused]] bool value)
{
}

static inline void pinSetFunction([[maybe_unused]] struct Pin pin,
    [[maybe_unused]] uint8_t function)
{
}

static inline void pinSetPull([[maybe_unused]] struct Pin pin,
    [[maybe_unused]] enum PinPull pull)
{
}

static inline void pinSetSlewRate([[maybe_unused]] struct Pin pin,
    [[maybe_unused]] enum PinSlewRate rate)
{
}

static inline void pinSetType([[maybe_unused]] struct Pin pin,
    [[maybe_unused]] enum PinType type)
{
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_GENERIC_POSIX_PIN_H_ */
