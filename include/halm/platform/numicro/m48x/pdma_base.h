/*
 * halm/platform/numicro/m48x/pdma_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_PDMA_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_M48X_PDMA_BASE_H_
#define HALM_PLATFORM_NUMICRO_M48X_PDMA_BASE_H_
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
  DMA0_CHANNEL8 = 8,
  DMA0_CHANNEL9 = 9,
  DMA0_CHANNEL10 = 10,
  DMA0_CHANNEL11 = 11,
  DMA0_CHANNEL12 = 12,
  DMA0_CHANNEL13 = 13,
  DMA0_CHANNEL14 = 14,
  DMA0_CHANNEL15 = 15
};

/** PDMA Request Source Selection. */
enum PdmaEvent
{
  PDMA_DISABLE      = 0,
  PDMA_USB_TX       = 2,
  PDMA_USB_RX       = 3,
  PDMA_UART0_TX     = 4,
  PDMA_UART0_RX     = 5,
  PDMA_UART1_TX     = 6,
  PDMA_UART1_RX     = 7,
  PDMA_UART2_TX     = 8,
  PDMA_UART2_RX     = 9,
  PDMA_UART3_TX     = 10,
  PDMA_UART3_RX     = 11,
  PDMA_UART4_TX     = 12,
  PDMA_UART4_RX     = 13,
  PDMA_UART5_TX     = 14,
  PDMA_UART5_RX     = 15,
  PDMA_USCI0_TX     = 16,
  PDMA_USCI0_RX     = 17,
  PDMA_USCI1_TX     = 18,
  PDMA_USCI1_RX     = 19,
  PDMA_QSPI0_TX     = 20,
  PDMA_QSPI0_RX     = 21,
  PDMA_SPI0_TX      = 22,
  PDMA_SPI0_RX      = 23,
  PDMA_SPI1_TX      = 24,
  PDMA_SPI1_RX      = 25,
  PDMA_SPI2_TX      = 26,
  PDMA_SPI2_RX      = 27,
  PDMA_SPI3_TX      = 28,
  PDMA_SPI3_RX      = 29,
  PDMA_QSPI1_TX     = 30,
  PDMA_QSPI1_RX     = 31,
  PDMA_EPWM0_P1_RX  = 32,
  PDMA_EPWM0_P2_RX  = 33,
  PDMA_EPWM0_P3_RX  = 34,
  PDMA_EPWM1_P1_RX  = 35,
  PDMA_EPWM1_P2_RX  = 36,
  PDMA_EPWM1_P3_RX  = 37,
  PDMA_I2C0_TX      = 38,
  PDMA_I2C0_RX      = 39,
  PDMA_I2C1_TX      = 40,
  PDMA_I2C1_RX      = 41,
  PDMA_I2C2_TX      = 42,
  PDMA_I2C2_RX      = 43,
  PDMA_I2S0_TX      = 44,
  PDMA_I2S0_RX      = 45,
  PDMA_TMR0         = 46,
  PDMA_TMR1         = 47,
  PDMA_TMR2         = 48,
  PDMA_TMR3         = 49,
  PDMA_EADC0_RX     = 50,
  PDMA_DAC0_TX      = 51,
  PDMA_DAC1_TX      = 52,
  PDMA_EPWM0_CH0_TX = 53,
  PDMA_EPWM0_CH1_TX = 54,
  PDMA_EPWM0_CH2_TX = 55,
  PDMA_EPWM0_CH3_TX = 56,
  PDMA_EPWM0_CH4_TX = 57,
  PDMA_EPWM0_CH5_TX = 58,
  PDMA_EPWM1_CH0_TX = 59,
  PDMA_EPWM1_CH1_TX = 60,
  PDMA_EPWM1_CH2_TX = 61,
  PDMA_EPWM1_CH3_TX = 62,
  PDMA_EPWM1_CH4_TX = 63,
  PDMA_EPWM1_CH5_TX = 64,
  PDMA_UART6_TX     = 42,
  PDMA_UART6_RX     = 43,
  PDMA_UART7_TX     = 44,
  PDMA_UART7_RX     = 45,
  PDMA_EADC1_RX     = 70,
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

static inline enum PdmaEvent pdmaGetEventAdc(uint8_t channel)
{
  return channel ? PDMA_EADC1_RX : PDMA_EADC0_RX;
}

static inline enum PdmaEvent pdmaGetEventSpiRx(uint8_t channel)
{
  return PDMA_SPI0_RX + channel * 2;
}

static inline enum PdmaEvent pdmaGetEventSpiTx(uint8_t channel)
{
  return PDMA_SPI0_TX + channel * 2;
}

static inline enum PdmaEvent pdmaGetEventQspiRx(uint8_t channel)
{
  return channel ? PDMA_QSPI1_RX : PDMA_QSPI0_RX;
}

static inline enum PdmaEvent pdmaGetEventQspiTx(uint8_t channel)
{
  return channel ? PDMA_QSPI1_TX : PDMA_QSPI0_TX;
}

static inline enum PdmaEvent pdmaGetEventUartRx(uint8_t channel)
{
  if (channel < 6)
    return PDMA_UART0_RX + channel * 2;
  else
    return PDMA_UART6_RX + (channel - 6) * 2;
}

static inline enum PdmaEvent pdmaGetEventUartTx(uint8_t channel)
{
  if (channel < 6)
    return PDMA_UART0_TX + channel * 2;
  else
    return PDMA_UART6_TX + (channel - 6) * 2;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M48X_PDMA_BASE_H_ */
