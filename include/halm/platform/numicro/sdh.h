/*
 * halm/platform/numicro/sdh.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_SDH_H_
#define HALM_PLATFORM_NUMICRO_SDH_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/numicro/sdh_base.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Sdh;

struct Interrupt;
struct Timer;

struct SdhConfig
{
  /** Optional: timer for data timeout calculation. */
  void *timer;
  /** Mandatory: data rate. */
  uint32_t rate;
  /** Mandatory: clock line. */
  PinNumber clk;
  /** Mandatory: command line. */
  PinNumber cmd;
  /** Mandatory: data line 0. */
  PinNumber dat0;
  /** Optional: data line 1. */
  PinNumber dat1;
  /** Optional: data line 2. */
  PinNumber dat2;
  /** Optional: data line 3. */
  PinNumber dat3;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /* Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct Sdh
{
  struct SdhBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* External interrupt on data line 0 */
  struct Interrupt *finalizer;
  /* Timer for data timeout calculation */
  struct Timer *timer;

  /* Argument for the most recent command */
  uint32_t argument;
  /* Interface command */
  uint32_t command;
  /* Data rate */
  uint32_t rate;
  /* Status of the last command */
  enum Result status;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_SDH_H_ */
