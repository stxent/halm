/*
 * halm/platform/lpc/gen_2/adc_defs.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GEN_2_ADC_DEFS_H_
#define HALM_PLATFORM_LPC_GEN_2_ADC_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Control register------------------------------------------*/
#define CTRL_CLKDIV(value)              BIT_FIELD((value), 0)
#define CTRL_CLKDIV_MASK                BIT_FIELD(MASK(8), 0)
#define CTRL_CLKDIV_VALUE(reg)          FIELD_VALUE((reg), CTRL_CLKDIV_MASK, 0)
#define CTRL_CLKDIV_MAX                 255

#define CTRL_ASYNCMODE                  BIT(8)
#define CTRL_MODE10BIT                  BIT(9)
#define CTRL_LPWRMODE                   BIT(10)
#define CTRL_CALMODE                    BIT(30)
/*------------------Sequence Control registers--------------------------------*/
#define SEQ_CTRL_CHANNEL(channel)       BIT_FIELD(BIT(channel), 0)
#define SEQ_CTRL_CHANNELS(value)        BIT_FIELD((value), 0)
#define SEQ_CTRL_CHANNELS_MASK          BIT_FIELD(MASK(12), 0)
#define SEQ_CTRL_CHANNELS_VALUE(reg) \
    FIELD_VALUE((reg), SEQ_CTRL_CHANNELS_MASK, 0)

#define SEQ_CTRL_TRIGGER(value)         BIT_FIELD((value), 12)
#define SEQ_CTRL_TRIGGER_MASK           BIT_FIELD(MASK(3), 12)
#define SEQ_CTRL_TRIGGER_VALUE(reg) \
    FIELD_VALUE((reg), SEQ_CTRL_TRIGGER_MASK, 12)

#define SEQ_CTRL_TRIGPOL                BIT(18)
#define SEQ_CTRL_SYNCBYPASS             BIT(19)
#define SEQ_CTRL_START                  BIT(26)
#define SEQ_CTRL_BURST                  BIT(27)
#define SEQ_CTRL_SINGLESTEP             BIT(28)
#define SEQ_CTRL_LOWPRIO                BIT(29)
#define SEQ_CTRL_MODE                   BIT(30)
#define SEQ_CTRL_SEQ_ENA                BIT(31)
/*------------------Sequence Global Data register-----------------------------*/
#define SEQ_GDAT_RESULT_MASK            BIT_FIELD(MASK(16), 0)
#define SEQ_GDAT_RESULT_VALUE(reg) \
    FIELD_VALUE((reg), SEQ_GDAT_RESULT_MASK, 0)

#define SEQ_GDAT_THCMPRANGE_MASK        BIT_FIELD(MASK(2), 16)
#define SEQ_GDAT_THCMPRANGE_VALUE(reg) \
    FIELD_VALUE((reg), SEQ_GDAT_THCMPRANGE_MASK, 16)

#define SEQ_GDAT_THCMPCROSS_MASK        BIT_FIELD(MASK(2), 18)
#define SEQ_GDAT_THCMPCROSS_VALUE(reg) \
    FIELD_VALUE((reg), SEQ_GDAT_THCMPCROSS_MASK, 18)

#define SEQ_GDAT_CHN_MASK               BIT_FIELD(MASK(4), 26)
#define SEQ_GDAT_CHN_VALUE(reg) \
    FIELD_VALUE((reg), SEQ_GDAT_CHN_MASK, 26)

#define SEQ_GDAT_OVERRUN                BIT(30)
#define SEQ_GDAT_DATAVALID              BIT(31)
/*------------------Data Registers--------------------------------------------*/
#define DAT_RESULT_MASK                 BIT_FIELD(MASK(16), 0)
#define DAT_RESULT_VALUE(reg)           FIELD_VALUE((reg), DAT_RESULT_MASK, 0)

#define DAT_THCMPRANGE_MASK             BIT_FIELD(MASK(2), 16)
#define DAT_THCMPRANGE_VALUE(reg) \
    FIELD_VALUE((reg), DAT_THCMPRANGE_MASK, 16)

#define DAT_THCMPCROSS_MASK             BIT_FIELD(MASK(2), 18)
#define DAT_THCMPCROSS_VALUE(reg) \
    FIELD_VALUE((reg), DAT_THCMPCROSS_MASK, 18)

#define DAT_CHANNEL_MASK                BIT_FIELD(MASK(4), 26)
#define DAT_CHANNEL_VALUE(reg)          FIELD_VALUE((reg), DAT_CHANNEL_MASK, 26)

#define DAT_OVERRUN                     BIT(30)
#define DAT_DATAVALID                   BIT(31)
/*------------------Interrupt Enable register---------------------------------*/
enum
{
  ADCMPINTEN_DISABLED           = 0,
  ADCMPINTEN_OUTSIDE_THRESHOLD  = 1,
  ADCMPINTEN_CROSSING_THRESHOLD = 2
};

#define INTEN_SEQA_INTEN                BIT(0)
#define INTEN_SEQB_INTEN                BIT(1)
#define INTEN_OVR_INTEN                 BIT(2)

#define INTEN_ADCMPINTEN(channel, value) \
    BIT_FIELD((value), (channel) * 2 + 3)
#define INTEN_ADCMPINTEN_MASK(channel) \
    BIT_FIELD(MASK(2), (channel) * 2 + 3)
#define INTEN_ADCMPINTEN_MASK_ALL       BIT_FIELD(MASK(24), 3)
/*------------------Flags register--------------------------------------------*/
#define FLAGS_THCMP(channel)            BIT((channel) + 0)
#define FLAGS_THCMP_MASK                BIT_FIELD(MASK(12), 0)

#define FLAGS_OVERRUN(channel)          BIT((channel) + 12)
#define FLAGS_OVERRUN_MASK              BIT_FIELD(MASK(12), 12)

#define FLAGS_SEQA_OVR                  BIT(24)
#define FLAGS_SEQB_OVR                  BIT(25)
#define FLAGS_SEQA_INT                  BIT(28)
#define FLAGS_SEQB_INT                  BIT(29)
#define FLAGS_THCMP_INT                 BIT(30)
#define FLAGS_OVR_INT                   BIT(31)
/*------------------Trim register---------------------------------------------*/
#define TRM_VRANGE                      BIT(5) /* Enable low voltage range */
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_ADC_DEFS_H_ */
