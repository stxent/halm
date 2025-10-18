/*
 * halm/platform/lpc/lpc82x/sdma_base.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SDMA_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC82X_SDMA_BASE_H_
#define HALM_PLATFORM_LPC_LPC82X_SDMA_BASE_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
/** Direct Memory Access controller requests. */
enum [[gnu::packed]] SdmaRequest
{
  SDMA_REQUEST_USART0_RX  = 0,
  SDMA_REQUEST_USART0_TX  = 1,
  SDMA_REQUEST_USART1_RX  = 2,
  SDMA_REQUEST_USART1_TX  = 3,
  SDMA_REQUEST_USART2_RX  = 4,
  SDMA_REQUEST_USART2_TX  = 5,

  SDMA_REQUEST_SPI0_RX    = 6,
  SDMA_REQUEST_SPI0_TX    = 7,
  SDMA_REQUEST_SPI1_RX    = 8,
  SDMA_REQUEST_SPI1_TX    = 9,

  SDMA_REQUEST_I2C0_SLV   = 10,
  SDMA_REQUEST_I2C0_MST   = 11,
  SDMA_REQUEST_I2C1_SLV   = 12,
  SDMA_REQUEST_I2C1_MST   = 13,
  SDMA_REQUEST_I2C2_SLV   = 14,
  SDMA_REQUEST_I2C2_MST   = 15,
  SDMA_REQUEST_I2C3_SLV   = 16,
  SDMA_REQUEST_I2C3_MST   = 17,

  SDMA_REQUEST_NONE,
  SDMA_REQUEST_END
};

/** Direct Memory Access controller triggers. */
enum [[gnu::packed]] SdmaTrigger
{
  SDMA_TRIGGER_ADC_SEQA   = 0,
  SDMA_TRIGGER_ADC_SEQB   = 1,
  SDMA_TRIGGER_SCT_DMA0   = 2,
  SDMA_TRIGGER_SCT_DMA1   = 3,
  SDMA_TRIGGER_ACMP_O     = 4,
  SDMA_TRIGGER_PININT0    = 5,
  SDMA_TRIGGER_PININT1    = 6,
  SDMA_TRIGGER_DMA_INMUX0 = 7,
  SDMA_TRIGGER_DMA_INMUX1 = 8,

  SDMA_TRIGGER_NONE,
  SDMA_TRIGGER_END
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline enum SdmaTrigger sdmaGetTriggerAdc(uint8_t channel)
{
  return SDMA_TRIGGER_ADC_SEQA + channel;
}

static inline enum SdmaRequest sdmaGetRequestI2CMaster(uint8_t channel)
{
  return SDMA_REQUEST_I2C0_MST + channel * 2;
}

static inline enum SdmaRequest sdmaGetRequestI2CSlave(uint8_t channel)
{
  return SDMA_REQUEST_I2C0_SLV + channel * 2;
}

static inline enum SdmaRequest sdmaGetRequestSpiRx(uint8_t channel)
{
  return SDMA_REQUEST_SPI0_RX + channel * 2;
}

static inline enum SdmaRequest sdmaGetRequestSpiTx(uint8_t channel)
{
  return SDMA_REQUEST_SPI0_TX + channel * 2;
}

static inline enum SdmaRequest sdmaGetRequestUartRx(uint8_t channel)
{
  return SDMA_REQUEST_USART0_RX + channel * 2;
}

static inline enum SdmaRequest sdmaGetRequestUartTx(uint8_t channel)
{
  return SDMA_REQUEST_USART0_TX + channel * 2;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC82X_SDMA_BASE_H_ */
