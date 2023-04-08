/*
 * halm/platform/stm32/gen_2/pin.h
 * Copyright (C) 2016, 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_PIN_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_GEN_2_PIN_H_
#define HALM_PLATFORM_STM32_GEN_2_PIN_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
#include <xcore/helpers.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/pin.h>
#include HEADER_PATH
/*----------------------------------------------------------------------------*/
struct Pin
{
  void *reg;
  uint16_t mask;
  uint8_t number;
  uint8_t port;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void *pinAddress(struct Pin);
struct Pin pinInit(PinNumber);
void pinInput(struct Pin);
void pinOutput(struct Pin, bool);
void pinSetFunction(struct Pin, uint8_t);
void pinSetPull(struct Pin, enum PinPull);
void pinSetType(struct Pin, enum PinType);
void pinSetSlewRate(struct Pin, enum PinSlewRate);

END_DECLS
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline bool pinRead(struct Pin pin)
{
  return (((STM_GPIO_Type *)pin.reg)->IDR & pin.mask) != 0;
}

static inline void pinReset(struct Pin pin)
{
  ((STM_GPIO_Type *)pin.reg)->BSRR = pin.mask << 16;
}

static inline void pinSet(struct Pin pin)
{
  ((STM_GPIO_Type *)pin.reg)->BSRR = pin.mask;
}

static inline void pinToggle(struct Pin pin)
{
  STM_GPIO_Type * const reg = (STM_GPIO_Type *)pin.reg;

  if (reg->ODR & pin.mask)
    reg->BSRR = pin.mask << 16;
  else
    reg->BSRR = pin.mask;
}

static inline bool pinValid(struct Pin pin)
{
  return pin.port != 0xFF;
}

static inline void pinWrite(struct Pin pin, bool value)
{
  STM_GPIO_Type * const reg = (STM_GPIO_Type *)pin.reg;

  if (value)
    reg->BSRR = pin.mask;
  else
    reg->BSRR = pin.mask << 16;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GEN_2_PIN_H_ */
