/*
 * halm/platform/generic/posix/pin.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
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

static inline struct Pin pinInit(PinNumber id __attribute__((unused)))
{
  return (struct Pin){-1};
}

static inline void pinInput(struct Pin pin __attribute__((unused)))
{
}

static inline void pinOutput(struct Pin pin __attribute__((unused)),
    bool value __attribute__((unused)))
{
}

static inline bool pinRead(struct Pin pin __attribute__((unused)))
{
  return false;
}

static inline void pinReset(struct Pin pin __attribute__((unused)))
{
}

static inline void pinSet(struct Pin pin __attribute__((unused)))
{
}

static inline void pinToggle(struct Pin pin __attribute__((unused)))
{
}

static inline bool pinValid(struct Pin pin __attribute__((unused)))
{
  return false;
}

static inline void pinWrite(struct Pin pin __attribute__((unused)),
    bool value __attribute__((unused)))
{
}

static inline void pinSetFunction(struct Pin pin __attribute__((unused)),
    uint8_t function __attribute__((unused)))
{
}

static inline void pinSetPull(struct Pin pin __attribute__((unused)),
    enum PinPull pull __attribute__((unused)))
{
}

static inline void pinSetSlewRate(struct Pin pin __attribute__((unused)),
    enum PinSlewRate rate __attribute__((unused)))
{
}

static inline void pinSetType(struct Pin pin __attribute__((unused)),
    enum PinType type __attribute__((unused)))
{
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_GENERIC_POSIX_PIN_H_ */
