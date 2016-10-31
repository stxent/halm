/*
 * halm/platform/stm/stm32f1xx/pin.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_STM_STM32F1XX_PIN_H_
#define HALM_PLATFORM_STM_STM32F1XX_PIN_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
enum
{
  PORT_A,
  PORT_B,
  PORT_C,
  PORT_D,
  PORT_E,
  PORT_F,
  PORT_G
};
/*----------------------------------------------------------------------------*/
void *pinAddress(struct Pin);
struct Pin pinInit(pinNumber);
void pinInput(struct Pin);
void pinOutput(struct Pin, bool);
void pinSetFunction(struct Pin, uint8_t);
void pinSetPull(struct Pin, enum pinPull);
void pinSetType(struct Pin, enum pinType);
void pinSetSlewRate(struct Pin, enum pinSlewRate);
/*----------------------------------------------------------------------------*/
static inline bool pinRead(struct Pin pin)
{
  const STM_GPIO_Type * const reg = pin.reg;

  return (reg->IDR & (1UL << pin.data.offset)) != 0;
}
/*----------------------------------------------------------------------------*/
static inline void pinReset(struct Pin pin)
{
  ((STM_GPIO_Type *)pin.reg)->BRR = 1UL << pin.data.offset;
}
/*----------------------------------------------------------------------------*/
static inline void pinSet(struct Pin pin)
{
  ((STM_GPIO_Type *)pin.reg)->BSRR = 1UL << pin.data.offset;
}
/*----------------------------------------------------------------------------*/
static inline void pinToggle(struct Pin pin)
{
  STM_GPIO_Type * const reg = pin.reg;
  const uint32_t mask = 1UL << pin.data.offset;

  if (reg->ODR & mask)
    reg->BRR = mask;
  else
    reg->BSRR = mask;
}
/*----------------------------------------------------------------------------*/
static inline void pinWrite(struct Pin pin, bool value)
{
  STM_GPIO_Type * const reg = pin.reg;
  const uint32_t mask = 1UL << pin.data.offset;

  if (value)
    reg->BSRR = mask;
  else
    reg->BRR = mask;
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM_STM32F1XX_PIN_H_ */
