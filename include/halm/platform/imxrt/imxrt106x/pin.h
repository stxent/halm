/*
 * halm/platform/imxrt/imxrt106x/pin.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_IMXRT_PIN_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_IMXRT_IMXRT106X_PIN_H_
#define HALM_PLATFORM_IMXRT_IMXRT106X_PIN_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
#include <stdbool.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
enum
{
  PORT_AD_B0,
  PORT_AD_B1,
  PORT_B0,
  PORT_B1,
  PORT_EMC,
  PORT_SD_B0,
  PORT_SD_B1,
  PORT_SNVS,
  PORT_USB
};

enum
{
  PIN_SNVS_WAKEUP = 0,
  PIN_SNVS_ON_REQ,
  PIN_SNVS_STANDBY_REQ,
  PIN_SNVS_TEST_MODE,
  PIN_SNVS_POR_B,
  PIN_SNVS_ONOFF,

  PIN_USB0_DN = 0,
  PIN_USB0_DP,
  PIN_USB0_ID,
  PIN_USB0_VBUS,
  PIN_USB1_DN,
  PIN_USB1_DP,
  PIN_USB1_VBUS
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
void pinSetType(struct Pin, enum PinType);

END_DECLS
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline bool pinRead(struct Pin pin)
{
  return (((const IMX_GPIO_Type *)pin.reg)->PSR & (1UL << pin.number)) != 0;
}

static inline void pinReset(struct Pin pin)
{
  ((IMX_GPIO_Type *)pin.reg)->DR_CLEAR = 1UL << pin.number;
}

static inline void pinSet(struct Pin pin)
{
  ((IMX_GPIO_Type *)pin.reg)->DR_SET = 1UL << pin.number;
}

static inline struct Pin pinStub(void)
{
  return (struct Pin){NULL, 0xFFFF, 0xFF, 0xFF};
}

static inline void pinToggle(struct Pin pin)
{
  ((IMX_GPIO_Type *)pin.reg)->DR_TOGGLE = 1UL << pin.number;
}

static inline bool pinValid(struct Pin pin)
{
  return pin.port != 0xFF;
}

static inline void pinWrite(struct Pin pin, bool value)
{
  if (value)
    ((IMX_GPIO_Type *)pin.reg)->DR_SET = 1UL << pin.number;
  else
    ((IMX_GPIO_Type *)pin.reg)->DR_CLEAR = 1UL << pin.number;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_IMXRT_IMXRT106X_PIN_H_ */
