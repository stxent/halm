/*
 * halm/platform/stm32/dcmi_defs.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_DCMI_DEFS_H_
#define HALM_PLATFORM_STM32_DCMI_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Control Register------------------------------------------*/
/* Continuous capture */
#define CR_CAPTURE                      BIT(0)
/* Capture mode */
#define CR_CM                           BIT(1)
/* Crop */
#define CR_CROP                         BIT(2)
/* JPEG mode */
#define CR_JPEG                         BIT(3)
/* Embedded synchronization mode */
#define CR_ESS                          BIT(4)
/* Pixel clock polarity */
#define CR_PCKPOL                       BIT(5)
/* Horizontal synchronization polarity */
#define CR_HSPOL                        BIT(6)
/* Vertical synchronization polarity */
#define CR_VSPOL                        BIT(7)

/* Frame Capture Rate control */
#define CR_FCRC_MASK                    BIT_FIELD(MASK(2), 8)
#define CR_FCRC(value)                  BIT_FIELD(value, 8)
#define CR_FCRC_VALUE(reg)              FIELD_VALUE(reg, CR_FCRC_MASK, 8)

/* Extended data mode */
#define CR_EDM_MASK                     BIT_FIELD(MASK(2), 10)
#define CR_EDM(value)                   BIT_FIELD(value, 10)
#define CR_EDM_VALUE(reg)               FIELD_VALUE(reg, CR_EDM_MASK, 10)

/* CRC enable */
#define CR_CRE                          BIT(12)
/* Enable */
#define CR_ENABLE                       BIT(14)
/*------------------Status Register-------------------------------------------*/
/* Horizontal synchronization */
#define SR_HSYNC                        BIT(0)
/* Vertical synchronization */
#define SR_VSYNC                        BIT(1)
/* FIFO not empty */
#define SR_FNE                          BIT(2)
/*------------------Raw Interrupt Status Register-----------------------------*/
/* Frame capture complete raw interrupt status */
#define RISR_FRAME_RIS                  BIT(0)
/* Overrun raw interrupt status */
#define RISR_OVR_RIS                    BIT(1)
/* Synchronization error raw interrupt status */
#define RISR_ERR_RIS                    BIT(2)
/* VSYNC raw interrupt status */
#define RISR_VSYNC_RIS                  BIT(3)
/* Line raw interrupt status */
#define RISR_LINE_RIS                   BIT(4)
/*------------------Interrupt Enable Register---------------------------------*/
/* Frame capture complete interrupt enable */
#define IER_FRAME_IE                    BIT(0)
/* Overrun interrupt enable */
#define IER_OVR_IE                      BIT(1)
/* Synchronization error interrupt enable */
#define IER_ERR_IE                      BIT(2)
/* VSYNC interrupt enable */
#define IER_VSYNC_IE                    BIT(3)
/* Line interrupt enable */
#define IER_LINE_IE                     BIT(4)
/*------------------Masked Interrupt Status Register--------------------------*/
/* Frame capture complete masked interrupt status */
#define MISR_FRAME_MIS                  BIT(0)
/* Overrun masked interrupt status */
#define MISR_OVR_MIS                    BIT(1)
/* Synchronization error masked interrupt status */
#define MISR_ERR_MIS                    BIT(2)
/* VSYNC masked interrupt status */
#define MISR_VSYNC_MIS                  BIT(3)
/* Line masked interrupt status */
#define MISR_LINE_MIS                   BIT(4)
/*------------------Interrupt Clear Register----------------------------------*/
/* Frame capture complete interrupt status clear */
#define ICR_FRAME_ISC                   BIT(0)
/* Overrun interrupt status clear */
#define ICR_OVR_ISC                     BIT(1)
/* Synchronization error interrupt status clear */
#define ICR_ERR_ISC                     BIT(2)
/* VSYNC interrupt status clear */
#define ICR_VSYNC_ISC                   BIT(3)
/* Line interrupt status clear */
#define ICR_LINE_ISC                    BIT(4)

/* Combined interrupt clear mask */
#define ICR_MASK                        BIT_FIELD(MASK(5), 0)
/*------------------Embedded Synchronization Code Register--------------------*/
/* Frame start code */
#define ESCR_FSC_MASK                   BIT_FIELD(MASK(8), 0)
#define ESCR_FSC(value)                 BIT_FIELD(value, 0)
#define ESCR_FSC_VALUE(reg)             FIELD_VALUE(reg, ESCR_FSC_MASK, 0)

/* Line start code */
#define ESCR_LSC_MASK                   BIT_FIELD(MASK(8), 8)
#define ESCR_LSC(value)                 BIT_FIELD(value, 8)
#define ESCR_LSC_VALUE(reg)             FIELD_VALUE(reg, ESCR_LSC_MASK, 8)

/* Line end code */
#define ESCR_LEC_MASK                   BIT_FIELD(MASK(8), 16)
#define ESCR_LEC(value)                 BIT_FIELD(value, 16)
#define ESCR_LEC_VALUE(reg)             FIELD_VALUE(reg, ESCR_LEC_MASK, 16)

/* Frame end code */
#define ESCR_FEC_MASK                   BIT_FIELD(MASK(8), 24)
#define ESCR_FEC(value)                 BIT_FIELD(value, 24)
#define ESCR_FEC_VALUE(reg)             FIELD_VALUE(reg, ESCR_FEC_MASK, 24)
/*------------------Embedded Synchronization Unmask Register------------------*/
/* Frame start unmask */
#define ESUR_FSU_MASK                   BIT_FIELD(MASK(8), 0)
#define ESUR_FSU(value)                 BIT_FIELD(value, 0)
#define ESUR_FSU_VALUE(reg)             FIELD_VALUE(reg, ESUR_FSU_MASK, 0)

/* Line start unmask */
#define ESUR_LSU_MASK                   BIT_FIELD(MASK(8), 8)
#define ESUR_LSU(value)                 BIT_FIELD(value, 8)
#define ESUR_LSU_VALUE(reg)             FIELD_VALUE(reg, ESUR_LSU_MASK, 8)

/* Line end unmask */
#define ESUR_LEU_MASK                   BIT_FIELD(MASK(8), 16)
#define ESUR_LEU(value)                 BIT_FIELD(value, 16)
#define ESUR_LEU_VALUE(reg)             FIELD_VALUE(reg, ESUR_LEU_MASK, 16)

/* Frame end unmask */
#define ESUR_FEU_MASK                   BIT_FIELD(MASK(8), 24)
#define ESUR_FEU(value)                 BIT_FIELD(value, 24)
#define ESUR_FEU_VALUE(reg)             FIELD_VALUE(reg, ESUR_FEU_MASK, 24)
/*------------------Crop Window Start Register--------------------------------*/
/* Horizontal offset count */
#define CWSTRTR_HOFFCNT_MASK            BIT_FIELD(MASK(14), 0)
#define CWSTRTR_HOFFCNT(value)          BIT_FIELD(value, 0)
#define CWSTRTR_HOFFCNT_VALUE(reg) \
    FIELD_VALUE(reg, CWSTRTR_HOFFCNT_MASK, 0)

/* Vertical start */
#define CWSTRTR_VST_MASK                BIT_FIELD(MASK(13), 16)
#define CWSTRTR_VST(value)              BIT_FIELD(value, 16)
#define CWSTRTR_VST_VALUE(reg)          FIELD_VALUE(reg, CWSTRTR_VST_MASK, 16)
/*------------------Crop Window Size Register---------------------------------*/
/* Capture count */
#define CWSIZER_CAPCNT_MASK             BIT_FIELD(MASK(14), 0)
#define CWSIZER_CAPCNT(value)           BIT_FIELD(value, 0)
#define CWSIZER_CAPCNT_VALUE(reg)       FIELD_VALUE(reg, CWSIZER_CAPCNT_MASK, 0)

/* Vertical line count */
#define CWSIZER_VLINE_MASK              BIT_FIELD(MASK(14), 16)
#define CWSIZER_VLINE(value)            BIT_FIELD(value, 16)
#define CWSIZER_VLINE_VALUE(reg)        FIELD_VALUE(reg, CWSIZER_VLINE_MASK, 16)
/*------------------Data Register---------------------------------------------*/
/* Byte 0 of captured data */
#define DR_BYTE0_MASK                   BIT_FIELD(MASK(8), 0)
#define DR_BYTE0(value)                 BIT_FIELD(value, 0)
#define DR_BYTE0_VALUE(reg)             FIELD_VALUE(reg, DR_BYTE0_MASK, 0)

/* Byte 1 of captured data */
#define DR_BYTE1_MASK                   BIT_FIELD(MASK(8), 8)
#define DR_BYTE1(value)                 BIT_FIELD(value, 8)
#define DR_BYTE1_VALUE(reg)             FIELD_VALUE(reg, DR_BYTE1_MASK, 8)

/* Byte 2 of captured data */
#define DR_BYTE2_MASK                   BIT_FIELD(MASK(8), 16)
#define DR_BYTE2(value)                 BIT_FIELD(value, 16)
#define DR_BYTE2_VALUE(reg)             FIELD_VALUE(reg, DR_BYTE2_MASK, 16)

/* Byte 3 of captured data */
#define DR_BYTE3_MASK                   BIT_FIELD(MASK(8), 24)
#define DR_BYTE3(value)                 BIT_FIELD(value, 24)
#define DR_BYTE3_VALUE(reg)             FIELD_VALUE(reg, DR_BYTE3_MASK, 24)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_DCMI_DEFS_H_ */
