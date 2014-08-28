/*
 * platform/nxp/adc_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_ADC_BASE_H_
#define PLATFORM_NXP_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <entity.h>
#include <irq.h>
#include <pin.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/PLATFORM/adc_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const AdcUnitBase;
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
#endif /* PLATFORM_NXP_ADC_BASE_H_ */
