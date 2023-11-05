/*
 * halm/platform/stm32/gptimer_defs.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_GPTIMER_DEFS_H_
#define HALM_PLATFORM_STM32_GPTIMER_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
#include <stdbool.h>
#include <stdint.h>
/*------------------Channel packing-------------------------------------------*/
enum
{
  FUNC_CH1_POS,
  FUNC_CH1_NEG,
  FUNC_CH2_POS,
  FUNC_CH2_NEG,
  FUNC_CH3_POS,
  FUNC_CH3_NEG,
  FUNC_CH4_POS,
  FUNC_ETR,
  FUNC_BKIN,

  CHANNEL_COUNT
};

#define CHANNEL_CH1(channel)          ((channel) * CHANNEL_COUNT + FUNC_CH1_POS)
#define CHANNEL_CH1N(channel)         ((channel) * CHANNEL_COUNT + FUNC_CH1_NEG)
#define CHANNEL_CH2(channel)          ((channel) * CHANNEL_COUNT + FUNC_CH2_POS)
#define CHANNEL_CH2N(channel)         ((channel) * CHANNEL_COUNT + FUNC_CH2_NEG)
#define CHANNEL_CH3(channel)          ((channel) * CHANNEL_COUNT + FUNC_CH3_POS)
#define CHANNEL_CH3N(channel)         ((channel) * CHANNEL_COUNT + FUNC_CH3_NEG)
#define CHANNEL_CH4(channel)          ((channel) * CHANNEL_COUNT + FUNC_CH4_POS)
#define CHANNEL_ETR(channel)          ((channel) * CHANNEL_COUNT + FUNC_ETR)
#define CHANNEL_BKIN(channel)         ((channel) * CHANNEL_COUNT + FUNC_BKIN)
#define PACK_CHANNEL(channel, index)  ((channel) * CHANNEL_COUNT + (index))
#define UNPACK_CHANNEL(value)         ((value) & (CHANNEL_COUNT - 1))
/*------------------Timer channel identifiers---------------------------------*/
enum TimerChannel
{
  TIMER_CH1,
  TIMER_CH2,
  TIMER_CH3,
  TIMER_CH4
};
/*------------------Filter settings-------------------------------------------*/
enum TimerFilter
{
  FILTER_DTS_DIV_1_N_1    = 0,
  FILTER_CK_INT_DIV_1_N_2 = 1,
  FILTER_CK_INT_DIV_1_N_4 = 2,
  FILTER_CK_INT_DIV_1_N_8 = 3,
  FILTER_DTS_DIV_2_N_6    = 4,
  FILTER_DTS_DIV_2_N_8    = 5,
  FILTER_DTS_DIV_4_N_6    = 6,
  FILTER_DTS_DIV_4_N_8    = 7,
  FILTER_DTS_DIV_8_N_6    = 8,
  FILTER_DTS_DIV_8_N_8    = 9,
  FILTER_DTS_DIV_16_N_5   = 10,
  FILTER_DTS_DIV_16_N_6   = 11,
  FILTER_DTS_DIV_16_N_8   = 12,
  FILTER_DTS_DIV_32_N_5   = 13,
  FILTER_DTS_DIV_32_N_6   = 14,
  FILTER_DTS_DIV_32_N_8   = 15
};
/*------------------Timer capabilities----------------------------------------*/
enum TimerFlags
{
  TIMER_FLAG_32_BIT   = 0x01,
  TIMER_FLAG_DMA      = 0x02,
  TIMER_FLAG_UPDOWN   = 0x04,
  TIMER_FLAG_INVERSE  = 0x08
};
/*------------------Control Register 1----------------------------------------*/
/* Counter enable */
#define CR1_CEN                         BIT(0)
/* Update disable */
#define CR1_UDIS                        BIT(1)
/* Update request source */
#define CR1_URS                         BIT(2)
/* One pulse mode */
#define CR1_OPM                         BIT(3)
/* Direction, 0 for upcounter and 1 for downcounter */
#define CR1_DIR                         BIT(4)

/* Center-aligned mode selection */
enum
{
  CMS_EDGE_ALIGNED_MODE     = 0,
  CMS_CENTER_ALIGNED_MODE_1 = 1,
  CMS_CENTER_ALIGNED_MODE_2 = 2,
  CMS_CENTER_ALIGNED_MODE_3 = 3
};

#define CR1_CMS(value)                  BIT_FIELD((value), 5)
#define CR1_CMS_MASK                    BIT_FIELD(MASK(2), 5)
#define CR1_CMS_VALUE(reg)              FIELD_VALUE((reg), CR1_CMS_MASK, 5)

/* Auto-reload preload enable */
#define CR1_ARPE                        BIT(7)

/* Clock division for dead-time generator and digital filters */
enum
{
  CKD_CK_INT,
  CKD_CK_INT_MUL_2,
  CKD_CK_INT_MUL_4
};

#define CR1_CKD(value)                  BIT_FIELD((value), 8)
#define CR1_CKD_MASK                    BIT_FIELD(MASK(2), 8)
#define CR1_CKD_VALUE(reg)              FIELD_VALUE((reg), CR1_CKD_MASK, 8)
/*------------------Control Register 2----------------------------------------*/
/* Capture/compare preloaded control */
#define CR2_CCPC                        BIT(0)
/* Capture/compare control update selection */
#define CR2_CCUS                        BIT(2)
/* Capture/compare DMA selection */
#define CR2_CCDS                        BIT(3)

/* Master mode selection */
enum
{
  MMS_RESET          = 0,
  MMS_ENABLE         = 1,
  MMS_UPDATE         = 2,
  MMS_COMPARE_PULSE  = 3,
  MMS_COMPARE_OC1REF = 4,
  MMS_COMPARE_OC2REF = 5,
  MMS_COMPARE_OC3REF = 6,
  MMS_COMPARE_OC4REF = 7
};

#define CR2_MMS(value)                  BIT_FIELD((value), 4)
#define CR2_MMS_MASK                    BIT_FIELD(MASK(3), 4)
#define CR2_MMS_VALUE(reg)              FIELD_VALUE((reg), CR2_MMS_MASK, 4)

/* TI1 selection  */
#define CR2_TI1S                        BIT(7)
/* Output idle state (OCx output) */
#define CR2_OIS(channel)                BIT(8 + (channel) * 2)
/* Output idle state (OCxN output, except channel 4) */
#define CR2_OISN(channel)               BIT(9 + (channel) * 2)
/*------------------Slave Mode Control Register-------------------------------*/
/* Slave mode selection */
enum
{
  SMS_DISABLED            = 0,
  SMS_ENCODER_MODE_1      = 1,
  SMS_ENCODER_MODE_2      = 2,
  SMS_ENCODER_MODE_3      = 3,
  SMS_RESET_MODE          = 4,
  SMS_GATED_MODE          = 5,
  SMS_TRIGGER_MODE        = 6,
  SMS_EXTERNAL_CLOCK_MODE = 7
};

#define SMCR_SMS(value)                 BIT_FIELD((value), 0)
#define SMCR_SMS_MASK                   BIT_FIELD(MASK(3), 0)
#define SMCR_SMS_VALUE(reg)             FIELD_VALUE((reg), SMCR_SMS_MASK, 0)

/* Trigger selection */
enum
{
  TS_ITR0    = 0,
  TS_ITR1    = 1,
  TS_ITR2    = 2,
  TS_ITR3    = 3,
  TS_TI1F_ED = 4,
  TS_TI1FP1  = 5,
  TS_TI2FP2  = 6,
  TS_ETRF    = 7
};

#define SMCR_TS(value)                  BIT_FIELD((value), 4)
#define SMCR_TS_MASK                    BIT_FIELD(MASK(3), 4)
#define SMCR_TS_VALUE(reg)              FIELD_VALUE((reg), SMCR_TS_MASK, 4)

/* Master/slave mode */
#define SMCR_MSM                        BIT(7)

/* External trigger filter, uses TimerFilter enumeration */
#define SMCR_ETF(value)                 BIT_FIELD((value), 8)
#define SMCR_ETF_MASK                   BIT_FIELD(MASK(4), 8)
#define SMCR_ETF_VALUE(reg)             FIELD_VALUE((reg), SMCR_ETF_MASK, 8)

/* External trigger prescaler */
enum
{
  ETPS_OFF    = 0,
  ETPS_DIV_2  = 0,
  ETPS_DIV_4  = 0,
  ETPS_DIV_8  = 0,
};

#define SMCR_ETPS(value)                BIT_FIELD((value), 12)
#define SMCR_ETPS_MASK                  BIT_FIELD(MASK(2), 12)
#define SMCR_ETPS_VALUE(reg)            FIELD_VALUE((reg), SMCR_ETPS_MASK, 12)

/* External clock enable */
#define SMCR_ECE                        BIT(14)
/* External trigger polarity */
#define SMCR_ETP                        BIT(15)
/*------------------DMA/Interrupt Enable Register-----------------------------*/
/* Update interrupt enable */
#define DIER_UIE                        BIT(0)
/* Capture/Compare interrupt enable */
#define DIER_CCIE_MASK                  BIT_FIELD(MASK(4), 1)
#define DIER_CCIE(channel)              BIT(1 + (channel))
/* COM interrupt enable */
#define DIER_COMIE                      BIT(5)
/* Trigger interrupt enable */
#define DIER_TIE                        BIT(6)
/* Break interrupt enable */
#define DIER_BIE                        BIT(7)
/* Update DMA request enable */
#define DIER_UDE                        BIT(8)
/* Capture/Compare DMA request enable */
#define DIER_CCDE_MASK                  BIT_FIELD(MASK(4), 9)
#define DIER_CCDE(channel)              BIT(9 + (channel))
/* COM DMA request enable */
#define DIER_COMDE                      BIT(13)
/* Trigger DMA request enable */
#define DIER_TDE                        BIT(14)
/*------------------Status Register-------------------------------------------*/
/* Update interrupt flag */
#define SR_UIF                          BIT(0)
/* Capture/Compare interrupt flag */
#define SR_CCIF_MASK                    BIT_FIELD(MASK(4), 1)
#define SR_CCIF(channel)                BIT(1 + (channel))
/* COM interrupt flag */
#define SR_COMIF                        BIT(5)
/* Trigger interrupt flag */
#define SR_TIF                          BIT(6)
/* Break interrupt flag */
#define SR_BIF                          BIT(7)
/* Capture/Compare overcapture flag */
#define SR_CCOF_MASK                    BIT_FIELD(MASK(4), 9)
#define SR_CCOF(channel)                BIT(9 + (channel))
/*------------------Event Generation Register---------------------------------*/
/* Update generation */
#define EGR_UG                          BIT(0)
/* Capture/Compare generation */
#define EGR_CCG_MASK                    BIT_FIELD(MASK(4), 1)
#define EGR_CCG(channel)                BIT(1 + (channel))
/* Capture/Compare control update generation */
#define EGR_COMG                        BIT(5)
/* Trigger generation */
#define EGR_TG                          BIT(6)
/* Break generation */
#define EGR_BG                          BIT(7)
/*------------------Capture/Compare Mode Registers----------------------------*/
/* Capture/Compare selection */
enum
{
  CCS_OUTPUT,
  CCS_INPUT_TRC,
  /* ICx is mapped on TIx */
  CCS_INPUT_DIRECT,
  /* IC1 is mapped on TI2, IC2 on TI1, IC3 on TI4 or IC4 on TI3 */
  CCS_INPUT_SWAPPED,
};

#define CCMR_CCS(channel, value) \
    BIT_FIELD((value), ((channel) & 1) * 8)
#define CCMR_CCS_MASK(channel) \
    BIT_FIELD(MASK(2), ((channel) & 1) * 8)
#define CCMR_CCS_VALUE(channel, reg) \
    FIELD_VALUE((reg), CCMR_CCS_MASK(channel), ((channel) & 1) * 8)

/* Output compare fast enable */
#define CCMR_OCFE(channel)              BIT(2 + (channel) * 8)
/* Output compare preload enable */
#define CCMR_OCPE(channel)              BIT(3 + (channel) * 8)

/* Output compare mode */
enum
{
  OCM_FROZEN        = 0,
  OCM_HIGH_ON_MATCH = 1,
  OCM_LOW_ON_MATCH  = 2,
  OCM_TOGGLE        = 3,
  OCM_FORCE_LOW     = 4,
  OCM_FORCE_HIGH    = 5,
  OCM_PWM_MODE_1    = 6,
  OCM_PWM_MODE_2    = 7
};

#define CCMR_OCM(channel, value) \
    BIT_FIELD((value), 4 + ((channel) & 1) * 8)
#define CCMR_OCM_MASK(channel) \
    BIT_FIELD(MASK(3), 4 + ((channel) & 1) * 8)
#define CCMR_OCM_VALUE(channel, reg) \
    FIELD_VALUE((reg), CCMR_OCM_MASK(channel), 4 + ((channel) & 1) * 8)

/* Output compare clear enable */
#define CCMR_OCCE(channel)              BIT(7 + (channel) * 8)

/* Input capture prescaler */
enum
{
  ICPSC_OFF   = 0,
  ICPSC_DIV_2 = 1,
  ICPSC_DIV_4 = 2,
  ICPSC_DIV_8 = 3
};

#define CCMR_ICPSC(channel, value) \
    BIT_FIELD((value), 2 + ((channel) & 1) * 8)
#define CCMR_ICPSC_MASK(channel) \
    BIT_FIELD(MASK(2), 2 + ((channel) & 1) * 8)
#define CCMR_ICPSC_VALUE(channel, reg) \
    FIELD_VALUE((reg), CCMR_ICPSC_MASK(channel), 2 + ((channel) & 1) * 8)

/* Input capture filter, uses TimerFilter enumeration */
#define CCMR_ICF(channel, value) \
    BIT_FIELD((value), 4 + ((channel) & 1) * 8)
#define CCMR_ICF_MASK(channel) \
    BIT_FIELD(MASK(4), 4 + ((channel) & 1) * 8)
#define CCMR_ICF_VALUE(channel, reg) \
    FIELD_VALUE((reg), CCMR_ICF_MASK(channel), 4 + ((channel) & 1) * 8)
/*------------------Capture/Compare Enable Register---------------------------*/
/* Capture/Compare output enable */
#define CCER_CCE(channel)               BIT(0 + (channel) * 4)
/* Capture/Compare output polarity */
#define CCER_CCP(channel)               BIT(1 + (channel) * 4)
/* Capture/Compare complementary output enable (except channel 4) */
#define CCER_CCNE(channel)              BIT(2 + (channel) * 4)
/* Capture/Compare complementary output polarity (except channel 4) */
#define CCER_CCNP(channel)              BIT(3 + (channel) * 4)
/*------------------Break and Dead-Time Register------------------------------*/
/* Dead-time generator setup */
#define BDTR_DTG(value)                 BIT_FIELD((value), 0)
#define BDTR_DTG_MASK                   BIT_FIELD(MASK(8), 0)
#define BDTR_DTG_VALUE(reg)             FIELD_VALUE((reg), BDTR_DTG_MASK, 0)

/* Lock configuration */
#define BDTR_LOCK(value)                BIT_FIELD((value), 8)
#define BDTR_LOCK_MASK                  BIT_FIELD(MASK(2), 8)
#define BDTR_LOCK_VALUE(reg)            FIELD_VALUE((reg), BDTR_LOCK_MASK, 8)

/* Off-state selection for idle mode */
#define BDTR_OSSI                       BIT(10)
/* Off-state selection for run mode */
#define BDTR_OSSR                       BIT(11)
/* Break enable */
#define BDTR_BKE                        BIT(12)
/* Break polarity */
#define BDTR_BKP                        BIT(13)
/* Automatic output enable */
#define BDTR_AOE                        BIT(14)
/* Main output enable */
#define BDTR_MOE                        BIT(15)
/*------------------DMA Control Register--------------------------------------*/
/* DMA base address */
#define DCR_DBA(value)                  BIT_FIELD((value), 0)
#define DCR_DBA_MASK                    BIT_FIELD(MASK(5), 0)
#define DCR_DBA_VALUE(reg)              FIELD_VALUE((reg), DCR_DBA_MASK, 0)

/* DMA burst length */
#define DCR_DBL(value)                  BIT_FIELD((value), 8)
#define DCR_DBL_MASK                    BIT_FIELD(MASK(5), 8)
#define DCR_DBL_VALUE(reg)              FIELD_VALUE((reg), DCR_DBL_MASK, 8)
/*----------------------------------------------------------------------------*/
static inline uint32_t getMaxValue(uint8_t flags)
{
  return (flags & TIMER_FLAG_32_BIT) ? 0xFFFFFFFFUL : 0xFFFFUL;
}

static inline bool isInputChannel(uint8_t channel)
{
  return channel != FUNC_CH1_NEG
      && channel != FUNC_CH2_NEG
      && channel != FUNC_CH3_NEG;
}

static inline bool isOutputChannel(uint8_t channel)
{
  return channel <= FUNC_CH4_POS;
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GPTIMER_DEFS_H_ */
