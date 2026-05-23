/*
 * halm/platform/bouffalo/bl602/vectors.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_VECTORS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_BOUFFALO_BL602_VECTORS_H_
#define HALM_PLATFORM_BOUFFALO_BL602_VECTORS_H_
/*----------------------------------------------------------------------------*/
enum
{
  /* Core interrupt sources */
  CLIC_MSIP_IRQ       = 3,
  CLIC_MTIP_IRQ       = 7,
  CLIC_MEIP_IRQ       = 11,
  CLIC_CSIP_IRQ       = 12,

  /* Chip-specific interrupt sources */
  BMX_ERR_IRQ         = 16,
  BMX_TO_IRQ          = 17,
  L1C_BMX_ERR_IRQ     = 18,
  L1C_BMX_TO_IRQ      = 19,
  SEC_BMX_ERR_IRQ     = 20,
  RF_TOP_INT0_IRQ     = 21,
  RF_TOP_INT1_IRQ     = 22,
  SDIO_IRQ            = 23,
  DMA_BMX_ERR_IRQ     = 24,
  SEC_GMAC_IRQ        = 25,
  SEC_CDET_IRQ        = 26,
  SEC_PKA_IRQ         = 27,
  SEC_TRNG_IRQ        = 28,
  SEC_AES_IRQ         = 29,
  SEC_SHA_IRQ         = 30,
  DMA_ALL_IRQ         = 31,
  /* Reserved */
  /* Reserved */
  /* Reserved */
  IRTX_IRQ            = 35,
  IRRX_IRQ            = 36,
  /* Reserved */
  /* Reserved */
  SF_CTRL_IRQ         = 39,
  /* Reserved */
  GPADC_DMA_IRQ       = 41,
  EFUSE_IRQ           = 42,
  SPI_IRQ             = 43,
  /* Reserved */
  UART0_IRQ           = 45,
  UART1_IRQ           = 46,
  /* Reserved */
  I2C_IRQ             = 48,
  /* Reserved */
  PWM_IRQ             = 50,
  /* Reserved */
  TIMER_CH0_IRQ       = 52,
  TIMER_CH1_IRQ       = 53,
  TIMER_WDT_IRQ       = 54,
  /* Reserved */
  /* Reserved */
  /* Reserved */
  /* Reserved */
  /* Reserved */
  GPIO_INT0_IRQ       = 60,
  /* Reserved */
  /* Reserved */
  /* Reserved */
  /* Reserved */
  /* Reserved */
  PDS_WAKEUP_IRQ      = 66,
  HBN_OUT0_IRQ        = 67,
  HBN_OUT1_IRQ        = 68,
  BOR_IRQ             = 69,
  WIFI_IRQ            = 70,
  BZ_PHY_IRQ          = 71,
  BLE_IRQ             = 72,
  MAC_TXRX_TIMER_IRQ  = 73,
  MAC_TXRX_MISC_IRQ   = 74,
  MAC_RX_TRG_IRQ      = 75,
  MAC_TX_TRG_IRQ      = 76,
  MAC_GEN_IRQ         = 77,
  MAC_PORT_TRG_IRQ    = 78,
  WIFI_IPC_PUBLIC_IRQ = 79,

  /* System declarations */
  IRQ_END,
  IRQ_RESERVED        = -127
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_BL602_VECTORS_H_ */
