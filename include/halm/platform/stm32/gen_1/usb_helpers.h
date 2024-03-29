/*
 * halm/platform/stm32/gen_1/usb_helpers.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_GEN_1_USB_HELPERS_H_
#define HALM_PLATFORM_STM32_GEN_1_USB_HELPERS_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
#include <halm/platform/stm32/gen_1/usb_defs.h>
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcEpAddr(const STM_USB_Type *reg,
    unsigned int entry)
{
  return &STM_CAN_USB_SRAM[(reg->BTABLE >> 1) + entry * 2];
}

static inline volatile void *calcEpBuffer(const STM_USB_Type *reg,
    unsigned int entry)
{
  return &STM_CAN_USB_SRAM[*calcEpAddr(reg, entry) >> 1];
}

static inline volatile uint32_t *calcEpCount(const STM_USB_Type *reg,
    unsigned int entry)
{
  return calcEpAddr(reg, entry) + 1;
}

static inline unsigned int calcDbEpEntry(unsigned int ep)
{
  return ep * 2;
}

static inline unsigned int calcRxEpEntry(unsigned int ep)
{
  return ep * 2 + 1;
}

static inline unsigned int calcTxEpEntry(unsigned int ep)
{
  return ep * 2;
}

static inline uint32_t eprMakeRxStat(uint32_t epr, uint32_t stat)
{
  return ((epr & (EPR_TOGGLE_MASK | EPR_STAT_RX_MASK)) ^ EPR_STAT_RX(stat))
      | EPR_CTR_MASK;
}

static inline uint32_t eprMakeTxStat(uint32_t epr, uint32_t stat)
{
  return ((epr & (EPR_TOGGLE_MASK | EPR_STAT_TX_MASK)) ^ EPR_STAT_TX(stat))
      | EPR_CTR_MASK;
}

static inline uint32_t eprMakeDtog(uint32_t epr, uint32_t set, uint32_t reset,
    uint32_t toggle)
{
  return ((epr ^ set) & (EPR_TOGGLE_MASK | set | reset)) | toggle
      | EPR_CTR_MASK;
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GEN_1_USB_HELPERS_H_ */
