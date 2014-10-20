/*
 * platform/nxp/lpc43xx/pin.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_LPC43XX_PIN_H_
#define PLATFORM_NXP_LPC43XX_PIN_H_
/*----------------------------------------------------------------------------*/
#include <assert.h>
#include <platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
enum
{
  PORT_0 = 0x00,
  PORT_1 = 0x01,
  PORT_2 = 0x02,
  PORT_3 = 0x03,
  PORT_4 = 0x04,
  PORT_5 = 0x05,
  PORT_6 = 0x06,
  PORT_7 = 0x07,
  PORT_8 = 0x08,
  PORT_9 = 0x09,
  PORT_A = 0x0A,
  PORT_B = 0x0B,
  PORT_C = 0x0C,
  PORT_D = 0x0D,
  PORT_E = 0x0E,
  PORT_F = 0x0F,
  PORT_CLK,
  PORT_I2C,
  PORT_ADC
};
/*----------------------------------------------------------------------------*/
enum
{
  PIN_I2C0_SCL,
  PIN_I2C0_SDA
};
/*----------------------------------------------------------------------------*/
struct Pin pinInit(pin_t);
void pinInput(struct Pin);
void pinOutput(struct Pin, uint8_t);
void pinSetFunction(struct Pin, uint8_t);
void pinSetPull(struct Pin, enum pinPull);
void pinSetSlewRate(struct Pin, enum pinSlewRate);
/*----------------------------------------------------------------------------*/
static inline bool pinGpioValid(struct Pin pin)
{
  /* Device family has eight GPIO ports */
  return pin.data.port < 8;
}
/*----------------------------------------------------------------------------*/
static inline uint8_t pinRead(struct Pin pin)
{
  assert(pinGpioValid(pin));

  return LPC_GPIO->B[(pin.data.port << 5) + pin.data.offset];
}
/*----------------------------------------------------------------------------*/
static inline void pinReset(struct Pin pin)
{
  assert(pinGpioValid(pin));

  LPC_GPIO->B[(pin.data.port << 5) + pin.data.offset] = 0;
}
/*----------------------------------------------------------------------------*/
static inline void pinSet(struct Pin pin)
{
  assert(pinGpioValid(pin));

  LPC_GPIO->B[(pin.data.port << 5) + pin.data.offset] = 1;
}
/*----------------------------------------------------------------------------*/
static inline void pinWrite(struct Pin pin, uint8_t value)
{
  assert(pinGpioValid(pin));

  /* Only 0 and 1 are allowed */
  LPC_GPIO->B[(pin.data.port << 5) + pin.data.offset] = value;
}
/*----------------------------------------------------------------------------*/
static inline void pinSetType(struct Pin pin __attribute__((unused)),
    enum pinType type __attribute__((unused)))
{
  /* Pin type control is not supported on these devices */
}
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC43XX_PIN_H_ */
