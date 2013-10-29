/*
 * platform/nxp/ssp_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SSP_BASE_H_
#define SSP_BASE_H_
/*----------------------------------------------------------------------------*/
#include <gpio.h>
#include <interface.h>
#include <irq.h>
#include "platform_defs.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *SspBase;
/*----------------------------------------------------------------------------*/
/* TODO Add master/slave select */
struct SspBaseConfig
{
  gpio_t cs; /* Optional: chip select for slave mode */
  gpio_t miso, mosi, sck; /* Mandatory: interface pins */
  uint8_t channel; /* Mandatory: peripheral number */
};
/*----------------------------------------------------------------------------*/
struct SspBase
{
  struct Interface parent;

  /* Pointer to the SSP register block */
  void *reg;
  /* Pointer to the interrupt handler */
  void (*handler)(void *);
  /* Interrupt identifier */
  irq_t irq;

  /* Peripheral block identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
uint32_t sspGetRate(struct SspBase *);
void sspSetRate(struct SspBase *, uint32_t);
/*----------------------------------------------------------------------------*/
uint32_t sspGetClock(struct SspBase *);
enum result sspSetupPins(struct SspBase *, const struct SspBaseConfig *);
/*----------------------------------------------------------------------------*/
#endif /* SSP_BASE_H_ */
