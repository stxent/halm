/*
 * halm/platform/imxrt/imxrt106x/edma_base.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_IMXRT_EDMA_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_IMXRT_IMXRT106X_EDMA_BASE_H_
#define HALM_PLATFORM_IMXRT_IMXRT106X_EDMA_BASE_H_
/*----------------------------------------------------------------------------*/
/** EDMA Request Source Selection. */
enum [[gnu::packed]] EdmaEvent
{
  EDMA_FLEXIO1                          = 0,
  EDMA_FLEXIO2                          = 1,

  EDMA_LPUART1_TX                       = 2,
  EDMA_LPUART1_RX                       = 3,
  EDMA_LPUART3_TX                       = 4,
  EDMA_LPUART3_RX                       = 5,
  EDMA_LPUART5_TX                       = 6,
  EDMA_LPUART5_RX                       = 7,
  EDMA_LPUART7_TX                       = 8,
  EDMA_LPUART7_RX                       = 9,

  EDMA_FLEXCAN3                         = 11,
  EDMA_CSI                              = 12,

  EDMA_LPSPI1_RX                        = 13,
  EDMA_LPSPI1_TX                        = 14,
  EDMA_LPSPI3_RX                        = 15,
  EDMA_LPSPI3_TX                        = 16,

  EDMA_LPI2C1                           = 17,
  EDMA_LPI2C3                           = 18,

  EDMA_SAI1_RX                          = 19,
  EDMA_SAI1_TX                          = 20,
  EDMA_SAI2_RX                          = 21,
  EDMA_SAI2_TX                          = 22,

  EDMA_ADC_ETC                          = 23,
  EDMA_ADC1                             = 24,
  EDMA_ACMP1                            = 25,
  EDMA_ACMP3                            = 26,

  EDMA_FLEXSPI1_RX                      = 28,
  EDMA_FLEXSPI1_TX                      = 29,

  EDMA_XBAR1_REQ0                       = 30,
  EDMA_XBAR1_REQ1                       = 31,

  EDMA_FLEXPWM1_CAP_PWM0                = 32,
  EDMA_FLEXPWM1_CAP_PWM1                = 33,
  EDMA_FLEXPWM1_CAP_PWM2                = 34,
  EDMA_FLEXPWM1_CAP_PWM3                = 35,
  EDMA_FLEXPWM1_REG_PWM0                = 36,
  EDMA_FLEXPWM1_REG_PWM1                = 37,
  EDMA_FLEXPWM1_REG_PWM2                = 38,
  EDMA_FLEXPWM1_REG_PWM3                = 39,

  EDMA_FLEXPWM3_CAP_PWM0                = 40,
  EDMA_FLEXPWM3_CAP_PWM1                = 41,
  EDMA_FLEXPWM3_CAP_PWM2                = 42,
  EDMA_FLEXPWM3_CAP_PWM3                = 43,
  EDMA_FLEXPWM3_REG_PWM0                = 44,
  EDMA_FLEXPWM3_REG_PWM1                = 45,
  EDMA_FLEXPWM3_REG_PWM2                = 46,
  EDMA_FLEXPWM3_REG_PWM3                = 47,

  EDMA_QTIMER1_CAP_TIM0                 = 48,
  EDMA_QTIMER1_CAP_TIM1                 = 49,
  EDMA_QTIMER1_CAP_TIM2                 = 50,
  EDMA_QTIMER1_CAP_TIM3                 = 51,
  EDMA_QTIMER1_CMP1_TIM0_CMP2_TIM1      = 52,
  EDMA_QTIMER1_CMP1_TIM1_CMP2_TIM0      = 53,
  EDMA_QTIMER1_CMP1_TIM2_CMP2_TIM3      = 54,
  EDMA_QTIMER1_CMP1_TIM3_CMP2_TIM2      = 55,

  EDMA_QTIMER3_CAP_CMP1_TIM0_CMP2_TIM1  = 56,
  EDMA_QTIMER3_CAP_CMP1_TIM1_CMP2_TIM0  = 57,
  EDMA_QTIMER3_CAP_CMP1_TIM2_CMP2_TIM3  = 58,
  EDMA_QTIMER3_CAP_CMP1_TIM3_CMP2_TIM2  = 59,

  EDMA_FLEXSPI2_RX                      = 60,
  EDMA_FLEXSPI2_TX                      = 61,

  EDMA_FLEXIO3                          = 64,
  EDMA_FLEXIO4                          = 65,

  EDMA_LPUART2_TX                       = 66,
  EDMA_LPUART2_RX                       = 67,
  EDMA_LPUART4_TX                       = 68,
  EDMA_LPUART4_RX                       = 69,
  EDMA_LPUART6_TX                       = 70,
  EDMA_LPUART6_RX                       = 71,
  EDMA_LPUART8_TX                       = 72,
  EDMA_LPUART8_RX                       = 73,

  EDMA_PXP                              = 75,
  EDMA_LCDIF                            = 76,

  EDMA_LPSPI2_RX                        = 77,
  EDMA_LPSPI2_TX                        = 78,
  EDMA_LPSPI4_RX                        = 79,
  EDMA_LPSPI4_TX                        = 80,
  EDMA_LPI2C2                           = 81,
  EDMA_LPI2C4                           = 82,

  EDMA_SAI3_RX                          = 83,
  EDMA_SAI3_TX                          = 84,
  EDMA_SPDIF_RX                         = 85,
  EDMA_SPDIF_TX                         = 86,

  EDMA_ADC2                             = 88,
  EDMA_ACMP2                            = 89,
  EDMA_ACMP4                            = 90,

  EDMA_ENET1_REQ0                       = 92,
  EDMA_ENET1_REQ1                       = 93,

  EDMA_XBAR1_REQ2                       = 94,
  EDMA_XBAR1_REQ3                       = 95,

  EDMA_FLEXPWM2_CAP_PWM0                = 96,
  EDMA_FLEXPWM2_CAP_PWM1                = 97,
  EDMA_FLEXPWM2_CAP_PWM2                = 98,
  EDMA_FLEXPWM2_CAP_PWM3                = 99,
  EDMA_FLEXPWM2_REG_PWM0                = 100,
  EDMA_FLEXPWM2_REG_PWM1                = 101,
  EDMA_FLEXPWM2_REG_PWM2                = 102,
  EDMA_FLEXPWM2_REG_PWM3                = 103,

  EDMA_FLEXPWM4_CAP_PWM0                = 104,
  EDMA_FLEXPWM4_CAP_PWM1                = 105,
  EDMA_FLEXPWM4_CAP_PWM2                = 106,
  EDMA_FLEXPWM4_CAP_PWM3                = 107,
  EDMA_FLEXPWM4_REG_PWM0                = 108,
  EDMA_FLEXPWM4_REG_PWM1                = 109,
  EDMA_FLEXPWM4_REG_PWM2                = 110,
  EDMA_FLEXPWM4_REG_PWM3                = 111,

  EDMA_QTIMER2_CAP_TIM0                 = 112,
  EDMA_QTIMER2_CAP_TIM1                 = 113,
  EDMA_QTIMER2_CAP_TIM2                 = 114,
  EDMA_QTIMER2_CAP_TIM3                 = 115,
  EDMA_QTIMER2_CMP1_TIM0_CMP2_TIM1      = 116,
  EDMA_QTIMER2_CMP1_TIM1_CMP2_TIM0      = 117,
  EDMA_QTIMER2_CMP1_TIM2_CMP2_TIM3      = 118,
  EDMA_QTIMER2_CMP1_TIM3_CMP2_TIM2      = 119,

  EDMA_QTIMER4_CAP_CMP1_TIM0_CMP2_TIM1  = 120,
  EDMA_QTIMER4_CAP_CMP1_TIM1_CMP2_TIM0  = 121,
  EDMA_QTIMER4_CAP_CMP1_TIM2_CMP2_TIM3  = 122,
  EDMA_QTIMER4_CAP_CMP1_TIM3_CMP2_TIM2  = 123,

  EDMA_ENET2_REQ0                       = 124,
  EDMA_ENET2_REQ1                       = 125,

  EDMA_MEMORY,
  EDMA_EVENT_END
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline enum EdmaEvent edmaGetEventLpSpiRx(uint8_t channel)
{
  if (channel & 1)
    return EDMA_LPSPI2_RX + (channel ^ 1);
  else
    return EDMA_LPSPI1_RX + channel;
}

static inline enum EdmaEvent edmaGetEventLpSpiTx(uint8_t channel)
{
  if (channel & 1)
    return EDMA_LPSPI2_TX + (channel ^ 1);
  else
    return EDMA_LPSPI1_TX + channel;
}

static inline enum EdmaEvent edmaGetEventLpUartRx(uint8_t channel)
{
  if (channel & 1)
    return EDMA_LPUART2_RX + (channel ^ 1);
  else
    return EDMA_LPUART1_RX + channel;
}

static inline enum EdmaEvent edmaGetEventLpUartTx(uint8_t channel)
{
  if (channel & 1)
    return EDMA_LPUART2_TX + (channel ^ 1);
  else
    return EDMA_LPUART1_TX + channel;
}

static inline bool edmaIsTriggerAvailable(uint8_t channel)
{
  return channel < 4;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_IMXRT_IMXRT106X_EDMA_BASE_H_ */
