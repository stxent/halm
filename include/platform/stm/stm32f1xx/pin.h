/*
 * platform/stm/stm32f1xx/pin.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_STM_STM32F1XX_PIN_H_
#define HALM_PLATFORM_STM_STM32F1XX_PIN_H_
/*----------------------------------------------------------------------------*/
#include <platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
enum
{
  PORT_A = 0x00,
  PORT_B = 0x01,
  PORT_C = 0x02,
  PORT_D = 0x03,
  PORT_E = 0x04,
  PORT_F = 0x05,
  PORT_G = 0x06
};
/*----------------------------------------------------------------------------*/
void *pinAddress(struct Pin);
struct Pin pinInit(pinNumber);
void pinInput(struct Pin);
void pinOutput(struct Pin, uint8_t);
void pinSetFunction(struct Pin, uint8_t);
void pinSetPull(struct Pin, enum pinPull);
void pinSetType(struct Pin, enum pinType);
void pinSetSlewRate(struct Pin, enum pinSlewRate);
/*----------------------------------------------------------------------------*/
static inline uint8_t pinRead(struct Pin pin)
{
  return (((const STM_GPIO_Type *)pin.reg)->IDR & (1 << pin.data.offset)) != 0;
}
/*----------------------------------------------------------------------------*/
static inline void pinReset(struct Pin pin)
{
  ((STM_GPIO_Type *)pin.reg)->BRR = 1 << pin.data.offset;
}
/*----------------------------------------------------------------------------*/
static inline void pinSet(struct Pin pin)
{
  ((STM_GPIO_Type *)pin.reg)->BSRR = 1 << pin.data.offset;
}
/*----------------------------------------------------------------------------*/
static inline void pinWrite(struct Pin pin, uint8_t value)
{
  ((STM_GPIO_Type *)pin.reg)->BSRR = (1 << pin.data.offset) << (value ? 0 : 16);
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM_STM32F1XX_PIN_H_ */
