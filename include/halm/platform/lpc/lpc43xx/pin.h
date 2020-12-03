/*
 * halm/platform/lpc/lpc43xx/pin.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LPC_LPC43XX_PIN_H_
#define HALM_PLATFORM_LPC_LPC43XX_PIN_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
#include <assert.h>
#include <stdbool.h>
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
  PORT_ADC,
  PORT_USB
};

enum
{
  PIN_I2C0_SCL,
  PIN_I2C0_SDA,
  PIN_USB0_DM,
  PIN_USB0_DP,
  PIN_USB0_ID,
  PIN_USB0_VBUS,
  PIN_USB1_DM,
  PIN_USB1_DP
};
/*----------------------------------------------------------------------------*/
struct Pin
{
  void *reg;
  uint16_t index;
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
void pinSetSlewRate(struct Pin, enum PinSlewRate);

END_DECLS
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline bool pinRead(struct Pin pin)
{
  return (bool)LPC_GPIO->B[pin.index];
}

static inline void pinReset(struct Pin pin)
{
  LPC_GPIO->B[pin.index] = 0;
}

static inline void pinSet(struct Pin pin)
{
  LPC_GPIO->B[pin.index] = 1;
}

static inline void pinToggle(struct Pin pin)
{
  LPC_GPIO->NOT[pin.port] = 1UL << pin.number;
}

static inline bool pinValid(struct Pin pin)
{
  return pin.port != 0xFF;
}

static inline void pinWrite(struct Pin pin, bool value)
{
  /* Only 0 and 1 are allowed */
  LPC_GPIO->B[pin.index] = value;
}

static inline void pinSetType(struct Pin pin __attribute__((unused)),
    enum PinType type __attribute__((unused)))
{
  /* Pin type control is not supported on these devices */
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_PIN_H_ */
