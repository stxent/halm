/*
 * platform/nxp/adc_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef ADC_BASE_TOP_H_
#define ADC_BASE_TOP_H_
/*----------------------------------------------------------------------------*/
#include <entity.h>
#include <gpio.h>
#include <irq.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/PLATFORM/adc_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct EntityClass *AdcUnitBase;
/*----------------------------------------------------------------------------*/
struct AdcUnitBaseConfig
{
  uint8_t channel; /* Mandatory: peripheral identifier */
};
/*----------------------------------------------------------------------------*/
struct AdcUnitBase
{
  struct Entity parent;

  void *reg;
  void (*handler)(void *);
  irq_t irq;

  /* Unique peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
#endif /* ADC_BASE_TOP_H_ */
