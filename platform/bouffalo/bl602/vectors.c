/*
 * vectors.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <stdalign.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
[[gnu::weak]] [[gnu::interrupt]] void defaultHandler(void);
/*----------------------------------------------------------------------------*/
/* Core SiFive E24 IRQ handlers */
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CLIC_MSIP_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CLIC_MTIP_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CLIC_MEIP_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CLIC_CSIP_ISR(void);
/*----------------------------------------------------------------------------*/
/* Chip-specific IRQ handlers */
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void BMX_ERR_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void BMX_TO_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void L1C_BMX_ERR_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void L1C_BMX_TO_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SEC_BMX_ERR_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void RF_TOP_INT0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void RF_TOP_INT1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SDIO_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DMA_BMX_ERR_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SEC_GMAC_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SEC_CDET_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SEC_PKA_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SEC_TRNG_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SEC_AES_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SEC_SHA_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DMA_ALL_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void IRTX_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void IRRX_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SF_CTRL_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void GPADC_DMA_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void EFUSE_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SPI_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void UART0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void UART1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2C_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PWM_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TIMER_CH0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TIMER_CH1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TIMER_WDT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void GPIO_INT0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PDS_WAKEUP_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void HBN_OUT0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void HBN_OUT1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void BOR_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void WIFI_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void BZ_PHY_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void BLE_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void MAC_TXRX_TIMER_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void MAC_TXRX_MISC_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void MAC_RX_TRG_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void MAC_TX_TRG_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void MAC_GEN_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void MAC_PORT_TRG_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void WIFI_IPC_PUBLIC_ISR(void);
/*----------------------------------------------------------------------------*/
[[gnu::section(".vectors")]] void (* const vector_table[])(void) = {
    /* Core interrupts */
    NULL,
    NULL,
    NULL,
    CLIC_MSIP_ISR,
    NULL,
    NULL,
    NULL,
    CLIC_MTIP_ISR,
    NULL,
    NULL,
    NULL,
    CLIC_MEIP_ISR,
    CLIC_CSIP_ISR,
    NULL,
    NULL,
    NULL,

    /* Chip-specific interrupts */
    BMX_ERR_ISR,
    BMX_TO_ISR,
    L1C_BMX_ERR_ISR,
    L1C_BMX_TO_ISR,
    SEC_BMX_ERR_ISR,
    RF_TOP_INT0_ISR,
    RF_TOP_INT1_ISR,
    SDIO_ISR,
    DMA_BMX_ERR_ISR,
    SEC_GMAC_ISR,
    SEC_CDET_ISR,
    SEC_PKA_ISR,
    SEC_TRNG_ISR,
    SEC_AES_ISR,
    SEC_SHA_ISR,
    DMA_ALL_ISR,
    NULL,
    NULL,
    NULL,
    IRTX_ISR,
    IRRX_ISR,
    NULL,
    NULL,
    SF_CTRL_ISR,
    NULL,
    GPADC_DMA_ISR,
    EFUSE_ISR,
    SPI_ISR,
    NULL,
    UART0_ISR,
    UART1_ISR,
    NULL,
    I2C_ISR,
    NULL,
    PWM_ISR,
    NULL,
    TIMER_CH0_ISR,
    TIMER_CH1_ISR,
    TIMER_WDT_ISR,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    GPIO_INT0_ISR,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    PDS_WAKEUP_ISR,
    HBN_OUT0_ISR,
    HBN_OUT1_ISR,
    BOR_ISR,
    WIFI_ISR,
    BZ_PHY_ISR,
    BLE_ISR,
    MAC_TXRX_TIMER_ISR,
    MAC_TXRX_MISC_ISR,
    MAC_RX_TRG_ISR,
    MAC_TX_TRG_ISR,
    MAC_GEN_ISR,
    MAC_PORT_TRG_ISR,
    WIFI_IPC_PUBLIC_ISR
};
/*----------------------------------------------------------------------------*/
void defaultHandler(void)
{
  while (1);
}
