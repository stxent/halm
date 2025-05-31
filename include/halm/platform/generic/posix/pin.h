/*
 * halm/platform/generic/posix/pin.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_GENERIC_POSIX_PIN_H_
#define HALM_PLATFORM_GENERIC_POSIX_PIN_H_
/*----------------------------------------------------------------------------*/
struct Pin
{
  int handle;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline struct Pin pinInit(PinNumber)
{
  return (struct Pin){-1};
}

static inline void pinInput(struct Pin)
{
}

static inline void pinOutput(struct Pin, bool)
{
}

static inline bool pinRead(struct Pin)
{
  return false;
}

static inline void pinReset(struct Pin)
{
}

static inline void pinSet(struct Pin)
{
}

static inline struct Pin pinStub(void)
{
  return (struct Pin){-1};
}

static inline void pinToggle(struct Pin)
{
}

static inline bool pinValid(struct Pin)
{
  return false;
}

static inline void pinWrite(struct Pin, bool)
{
}

static inline void pinSetFunction(struct Pin, uint8_t)
{
}

static inline void pinSetPull(struct Pin, enum PinPull)
{
}

static inline void pinSetSlewRate(struct Pin, enum PinSlewRate)
{
}

static inline void pinSetType(struct Pin, enum PinType)
{
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_GENERIC_POSIX_PIN_H_ */
