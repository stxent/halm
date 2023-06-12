/*
 * halm/platform/numicro/m03x/pdma_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_PDMA_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_M03X_PDMA_BASE_H_
#define HALM_PLATFORM_NUMICRO_M03X_PDMA_BASE_H_
/*----------------------------------------------------------------------------*/
/** Symbolic names for the DMA channels */
enum
{
  DMA0_CHANNEL0 = 0,
  DMA0_CHANNEL1 = 1,
  DMA0_CHANNEL2 = 2,
  DMA0_CHANNEL3 = 3,
  DMA0_CHANNEL4 = 4,
  DMA0_CHANNEL5 = 5,
  DMA0_CHANNEL6 = 6,
  DMA0_CHANNEL7 = 7,
  DMA0_CHANNEL8 = 8
};

/** PDMA Request Source Selection. */
enum PdmaEvent
{
  PDMA_DISABLE    = 0,
  PDMA_UART0_TX   = 4,
  PDMA_UART0_RX   = 5,
  PDMA_UART1_TX   = 6,
  PDMA_UART1_RX   = 7,
  PDMA_UART2_TX   = 8,
  PDMA_UART2_RX   = 9,
  PDMA_USCI0_TX   = 10,
  PDMA_USCI0_RX   = 11,
  PDMA_USCI1_TX   = 12,
  PDMA_USCI1_RX   = 13,
  PDMA_QSPI0_TX   = 16,
  PDMA_QSPI0_RX   = 17,
  PDMA_SPI0_TX    = 18,
  PDMA_SPI0_RX    = 19,
  PDMA_ADC_RX     = 20,
  PDMA_PWM0_P1_RX = 21,
  PDMA_PWM0_P2_RX = 22,
  PDMA_PWM0_P3_RX = 23,
  PDMA_PWM1_P1_RX = 24,
  PDMA_PWM1_P2_RX = 25,
  PDMA_PWM1_P3_RX = 26,
  PDMA_I2C0_TX    = 28,
  PDMA_I2C0_RX    = 29,
  PDMA_I2C1_TX    = 30,
  PDMA_I2C1_RX    = 31,
  PDMA_TMR0       = 32,
  PDMA_TMR1       = 33,
  PDMA_TMR2       = 34,
  PDMA_TMR3       = 35,
  PDMA_UART3_TX   = 36,
  PDMA_UART3_RX   = 37,
  PDMA_UART4_TX   = 38,
  PDMA_UART4_RX   = 39,
  PDMA_UART5_TX   = 40,
  PDMA_UART5_RX   = 41,
  PDMA_UART6_TX   = 42,
  PDMA_UART6_RX   = 43,
  PDMA_UART7_TX   = 44,
  PDMA_UART7_RX   = 45,
  PDMA_MEMORY,
  PDMA_EVENT_END
} __attribute__((packed));

struct PdmaMuxConfig
{
  uint32_t mask;
  uint32_t value;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline enum PdmaEvent pdmaGetEventAdc(uint8_t channel
    __attribute__((unused)))
{
  return PDMA_ADC_RX;
}

static inline enum PdmaEvent pdmaGetEventI2CRx(uint8_t channel)
{
  return PDMA_I2C0_RX + channel * 2;
}

static inline enum PdmaEvent pdmaGetEventI2CTx(uint8_t channel)
{
  return PDMA_I2C0_TX + channel * 2;
}

static inline enum PdmaEvent pdmaGetEventSpiRx(uint8_t channel
    __attribute__((unused)))
{
  return PDMA_SPI0_RX;
}

static inline enum PdmaEvent pdmaGetEventSpiTx(uint8_t channel
    __attribute__((unused)))
{
  return PDMA_SPI0_TX;
}

static inline enum PdmaEvent pdmaGetEventQspiRx(uint8_t channel
    __attribute__((unused)))
{
  return PDMA_QSPI0_RX;
}

static inline enum PdmaEvent pdmaGetEventQspiTx(uint8_t channel
    __attribute__((unused)))
{
  return PDMA_QSPI0_TX;
}

static inline enum PdmaEvent pdmaGetEventUartRx(uint8_t channel)
{
  if (channel < 3)
    return PDMA_UART0_RX + channel * 2;
  else
    return PDMA_UART3_RX + (channel - 3) * 2;
}

static inline enum PdmaEvent pdmaGetEventUartTx(uint8_t channel)
{
  if (channel < 3)
    return PDMA_UART0_TX + channel * 2;
  else
    return PDMA_UART3_TX + (channel - 3) * 2;
}

static inline bool pdmaIsTOCAvailable(uint8_t channel)
{
  return channel < 2;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M03X_PDMA_BASE_H_ */
