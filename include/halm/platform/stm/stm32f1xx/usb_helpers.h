/*
 * halm/platform/stm/stm32f1xx/usb_helpers.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_STM_STM32F1XX_USB_HELPERS_H_
#define HALM_PLATFORM_STM_STM32F1XX_USB_HELPERS_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
#include <halm/platform/stm/stm32f1xx/usb_defs.h>
/*----------------------------------------------------------------------------*/
static inline uint32_t *calcRxEpAddr(const STM_USB_Type *reg, unsigned int ep)
{
  return (uint32_t *)(STM_CAN_USB_SRAM_BASE + (reg->BTABLE + ep * 8 + 4) * 2);
}

static inline void *calcRxEpBuffer(const STM_USB_Type *reg, unsigned int ep)
{
  return (void *)(STM_CAN_USB_SRAM_BASE + *calcRxEpAddr(reg, ep) * 2);
}

static inline uint32_t *calcRxEpCount(const STM_USB_Type *reg, unsigned int ep)
{
  return (uint32_t *)(STM_CAN_USB_SRAM_BASE + (reg->BTABLE + ep * 8 + 6) * 2);
}

static inline uint32_t *calcTxEpAddr(const STM_USB_Type *reg, unsigned int ep)
{
  return (uint32_t *)(STM_CAN_USB_SRAM_BASE + (reg->BTABLE + ep * 8 + 0) * 2);
}

static inline void *calcTxEpBuffer(const STM_USB_Type *reg, unsigned int ep)
{
  return (void *)(STM_CAN_USB_SRAM_BASE + *calcTxEpAddr(reg, ep) * 2);
}

static inline uint32_t *calcTxEpCount(const STM_USB_Type *reg, unsigned int ep)
{
  return (uint32_t *)(STM_CAN_USB_SRAM_BASE + (reg->BTABLE + ep * 8 + 2) * 2);
}

static inline void changeRxStat(volatile uint32_t *reg, uint32_t stat)
{
  *reg = ((*reg & (EPR_TOGGLE_MASK | EPR_STAT_RX_MASK)) ^ EPR_STAT_RX(stat))
      | (EPR_CTR_RX | EPR_CTR_TX);
}

static inline void changeTxStat(volatile uint32_t *reg, uint32_t stat)
{
  *reg = ((*reg & (EPR_TOGGLE_MASK | EPR_STAT_TX_MASK)) ^ EPR_STAT_TX(stat))
      | (EPR_CTR_RX | EPR_CTR_TX);
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM_STM32F1XX_USB_HELPERS_H_ */
