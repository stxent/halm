/*
 * halm/platform/nxp/lpc43xx/platform_defs.h
 * Based on original from NXP
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC43XX_PLATFORM_DEFS_H_
#define HALM_PLATFORM_NXP_LPC43XX_PLATFORM_DEFS_H_
/*----------------------------------------------------------------------------*/
#define NVIC_PRIORITY_SIZE 3
/*------------------Clock Generation Unit-------------------------------------*/
typedef struct
{
  __ne__ uint32_t RESERVED0[5];
  __rw__ uint32_t FREQ_MON;
  __rw__ uint32_t XTAL_OSC_CTRL;
  __ro__ uint32_t PLL0USB_STAT;

  /* Offset 0x0020 */
  __rw__ uint32_t PLL0USB_CTRL;
  __rw__ uint32_t PLL0USB_MDIV;
  __rw__ uint32_t PLL0USB_NP_DIV;
  __ro__ uint32_t PLL0AUDIO_STAT;
  __rw__ uint32_t PLL0AUDIO_CTRL;
  __rw__ uint32_t PLL0AUDIO_MDIV;
  __rw__ uint32_t PLL0AUDIO_NP_DIV;
  __rw__ uint32_t PLL0AUDIO_FRAC;

  /* Offset 0x0040 */
  __ro__ uint32_t PLL1_STAT;
  __rw__ uint32_t PLL1_CTRL;
  __rw__ uint32_t IDIVA_CTRL;
  __rw__ uint32_t IDIVB_CTRL;
  __rw__ uint32_t IDIVC_CTRL;
  __rw__ uint32_t IDIVD_CTRL;
  __rw__ uint32_t IDIVE_CTRL;
  __ro__ uint32_t BASE_SAFE_CLK;

  /* Offset 0x0060 */
  __rw__ uint32_t BASE_USB0_CLK;
  __rw__ uint32_t BASE_PERIPH_CLK;
  __rw__ uint32_t BASE_USB1_CLK;
  __rw__ uint32_t BASE_M4_CLK;
  __rw__ uint32_t BASE_SPIFI_CLK;
  __rw__ uint32_t BASE_SPI_CLK;
  __rw__ uint32_t BASE_PHY_RX_CLK;
  __rw__ uint32_t BASE_PHY_TX_CLK;

  /* Offset 0x0080 */
  __rw__ uint32_t BASE_APB1_CLK;
  __rw__ uint32_t BASE_APB3_CLK;
  __rw__ uint32_t BASE_LCD_CLK;
  __rw__ uint32_t BASE_ADCHS_CLK;
  __rw__ uint32_t BASE_SDIO_CLK;
  __rw__ uint32_t BASE_SSP0_CLK;
  __rw__ uint32_t BASE_SSP1_CLK;
  __rw__ uint32_t BASE_UART0_CLK;

  /* Offset 0x00A0 */
  __rw__ uint32_t BASE_UART1_CLK;
  __rw__ uint32_t BASE_UART2_CLK;
  __rw__ uint32_t BASE_UART3_CLK;
  __rw__ uint32_t BASE_OUT_CLK;
  __ne__ uint32_t RESERVED1[4];

  /* Offset 0x00C0 */
  __rw__ uint32_t BASE_AUDIO_CLK;
  __rw__ uint32_t BASE_CGU_OUT0_CLK;
  __rw__ uint32_t BASE_CGU_OUT1_CLK;
} LPC_CGU_Type;
/*------------------Clock Control Unit 1--------------------------------------*/
typedef struct
{
  __rw__ uint32_t PM;
  __ro__ uint32_t BASE_STAT;
  __ne__ uint32_t RESERVED0[62];

  /* Offset 0x0100 */
  __rw__ uint32_t CLK_APB3_BUS_CFG;
  __ro__ uint32_t CLK_APB3_BUS_STAT;
  __rw__ uint32_t CLK_APB3_I2C1_CFG;
  __ro__ uint32_t CLK_APB3_I2C1_STAT;
  __rw__ uint32_t CLK_APB3_DAC_CFG;
  __ro__ uint32_t CLK_APB3_DAC_STAT;
  __rw__ uint32_t CLK_APB3_ADC0_CFG;
  __ro__ uint32_t CLK_APB3_ADC0_STAT;
  __rw__ uint32_t CLK_APB3_ADC1_CFG;
  __ro__ uint32_t CLK_APB3_ADC1_STAT;
  __rw__ uint32_t CLK_APB3_CAN0_CFG;
  __ro__ uint32_t CLK_APB3_CAN0_STAT;
  __ne__ uint32_t RESERVED1[52];

  /* Offset 0x0200 */
  __rw__ uint32_t CLK_APB1_BUS_CFG;
  __ro__ uint32_t CLK_APB1_BUS_STAT;
  __rw__ uint32_t CLK_APB1_MCPWM_CFG;
  __ro__ uint32_t CLK_APB1_MCPWM_STAT;
  __rw__ uint32_t CLK_APB1_I2C0_CFG;
  __ro__ uint32_t CLK_APB1_I2C0_STAT;
  __rw__ uint32_t CLK_APB1_I2S_CFG;
  __ro__ uint32_t CLK_APB1_I2S_STAT;
  __rw__ uint32_t CLK_APB1_CAN1_CFG;
  __ro__ uint32_t CLK_APB1_CAN1_STAT;
  __ne__ uint32_t RESERVED2[54];

  /* Offset 0x0300 */
  __rw__ uint32_t CLK_SPIFI_CFG;
  __ro__ uint32_t CLK_SPIFI_STAT;
  __ne__ uint32_t RESERVED3[62];

  /* Offset 0x0400 */
  __rw__ uint32_t CLK_M4_BUS_CFG;
  __ro__ uint32_t CLK_M4_BUS_STAT;
  __rw__ uint32_t CLK_M4_SPIFI_CFG;
  __ro__ uint32_t CLK_M4_SPIFI_STAT;
  __rw__ uint32_t CLK_M4_GPIO_CFG;
  __ro__ uint32_t CLK_M4_GPIO_STAT;
  __rw__ uint32_t CLK_M4_LCD_CFG;
  __ro__ uint32_t CLK_M4_LCD_STAT;

  /* Offset 0x0420 */
  __rw__ uint32_t CLK_M4_ETHERNET_CFG;
  __ro__ uint32_t CLK_M4_ETHERNET_STAT;
  __rw__ uint32_t CLK_M4_USB0_CFG;
  __ro__ uint32_t CLK_M4_USB0_STAT;
  __rw__ uint32_t CLK_M4_EMC_CFG;
  __ro__ uint32_t CLK_M4_EMC_STAT;
  __rw__ uint32_t CLK_M4_SDIO_CFG;
  __ro__ uint32_t CLK_M4_SDIO_STAT;

  /* Offset 0x0440 */
  __rw__ uint32_t CLK_M4_DMA_CFG;
  __ro__ uint32_t CLK_M4_DMA_STAT;
  __rw__ uint32_t CLK_M4_M4CORE_CFG;
  __ro__ uint32_t CLK_M4_M4CORE_STAT;
  __ne__ uint32_t RESERVED4[6];
  __rw__ uint32_t CLK_M4_SCT_CFG;
  __ro__ uint32_t CLK_M4_SCT_STAT;
  __rw__ uint32_t CLK_M4_USB1_CFG;
  __ro__ uint32_t CLK_M4_USB1_STAT;
  __rw__ uint32_t CLK_M4_EMCDIV_CFG;
  __ro__ uint32_t CLK_M4_EMCDIV_STAT;

  /* Offset 0x0480 */
  __rw__ uint32_t CLK_M4_FLASHA_CFG;
  __ro__ uint32_t CLK_M4_FLASHA_STAT;
  __rw__ uint32_t CLK_M4_FLASHB_CFG;
  __ro__ uint32_t CLK_M4_FLASHB_STAT;
  __rw__ uint32_t CLK_M4_M0APP_CFG;
  __ro__ uint32_t CLK_M4_M0APP_STAT;
  __rw__ uint32_t CLK_M4_ADCHS_CFG;
  __ro__ uint32_t CLK_M4_ADCHS_STAT;

  /* Offset 0x04A0 */
  __rw__ uint32_t CLK_M4_EEPROM_CFG;
  __ro__ uint32_t CLK_M4_EEPROM_STAT;
  __ne__ uint32_t RESERVED5[22];

  /* Offset 0x0500 */
  __rw__ uint32_t CLK_M4_WWDT_CFG;
  __ro__ uint32_t CLK_M4_WWDT_STAT;
  __rw__ uint32_t CLK_M4_USART0_CFG;
  __ro__ uint32_t CLK_M4_USART0_STAT;
  __rw__ uint32_t CLK_M4_UART1_CFG;
  __ro__ uint32_t CLK_M4_UART1_STAT;
  __rw__ uint32_t CLK_M4_SSP0_CFG;
  __ro__ uint32_t CLK_M4_SSP0_STAT;

  /* Offset 0x0520 */
  __rw__ uint32_t CLK_M4_TIMER0_CFG;
  __ro__ uint32_t CLK_M4_TIMER0_STAT;
  __rw__ uint32_t CLK_M4_TIMER1_CFG;
  __ro__ uint32_t CLK_M4_TIMER1_STAT;
  __rw__ uint32_t CLK_M4_SCU_CFG;
  __ro__ uint32_t CLK_M4_SCU_STAT;
  __rw__ uint32_t CLK_M4_CREG_CFG;
  __ro__ uint32_t CLK_M4_CREG_STAT;
  __ne__ uint32_t RESERVED6[48];

  /* Offset 0x0600 */
  __rw__ uint32_t CLK_M4_RIT_CFG;
  __ro__ uint32_t CLK_M4_RIT_STAT;
  __rw__ uint32_t CLK_M4_USART2_CFG;
  __ro__ uint32_t CLK_M4_USART2_STAT;
  __rw__ uint32_t CLK_M4_USART3_CFG;
  __ro__ uint32_t CLK_M4_USART3_STAT;
  __rw__ uint32_t CLK_M4_TIMER2_CFG;
  __ro__ uint32_t CLK_M4_TIMER2_STAT;

  /* Offset 0x0620 */
  __rw__ uint32_t CLK_M4_TIMER3_CFG;
  __ro__ uint32_t CLK_M4_TIMER3_STAT;
  __rw__ uint32_t CLK_M4_SSP1_CFG;
  __ro__ uint32_t CLK_M4_SSP1_STAT;
  __rw__ uint32_t CLK_M4_QEI_CFG;
  __ro__ uint32_t CLK_M4_QEI_STAT;
  __ne__ uint32_t RESERVED7[50];

  /* Offset 0x0700 */
  __rw__ uint32_t CLK_PERIPH_BUS_CFG;
  __ro__ uint32_t CLK_PERIPH_BUS_STAT;
  __ne__ uint32_t RESERVED8[2];
  __rw__ uint32_t CLK_PERIPH_CORE_CFG;
  __ro__ uint32_t CLK_PERIPH_CORE_STAT;
  __rw__ uint32_t CLK_PERIPH_SGPIO_CFG;
  __ro__ uint32_t CLK_PERIPH_SGPIO_STAT;
  __ne__ uint32_t RESERVED9[56];

  /* Offset 0x0800 */
  __rw__ uint32_t CLK_USB0_CFG;
  __ro__ uint32_t CLK_USB0_STAT;
  __ne__ uint32_t RESERVED10[62];

  /* Offset 0x0900 */
  __rw__ uint32_t CLK_USB1_CFG;
  __ro__ uint32_t CLK_USB1_STAT;
  __ne__ uint32_t RESERVED11[62];

  /* Offset 0x0A00 */
  __rw__ uint32_t CLK_SPI_CFG;
  __ro__ uint32_t CLK_SPI_STAT;
  __ne__ uint32_t RESERVED12[62];

  /* Offset 0x0B00 */
  __rw__ uint32_t CLK_ADCHS_CFG;
  __ro__ uint32_t CLK_ADCHS_STAT;
} LPC_CCU1_Type;
/*------------------Clock Control Unit 2--------------------------------------*/
typedef struct
{
  __rw__ uint32_t PM;
  __ro__ uint32_t BASE_STAT;
  __ne__ uint32_t RESERVED0[62];

  /* Offset 0x0100 */
  __rw__ uint32_t CLK_AUDIO_CFG;
  __ro__ uint32_t CLK_AUDIO_STAT;
  __ne__ uint32_t RESERVED1[62];

  /* Offset 0x0200 */
  __rw__ uint32_t CLK_APB2_USART3_CFG;
  __ro__ uint32_t CLK_APB2_USART3_STAT;
  __ne__ uint32_t RESERVED2[62];

  /* Offset 0x0300 */
  __rw__ uint32_t CLK_APB2_USART2_CFG;
  __ro__ uint32_t CLK_APB2_USART2_STAT;
  __ne__ uint32_t RESERVED3[62];

  /* Offset 0x0400 */
  __rw__ uint32_t CLK_APB0_UART1_CFG;
  __ro__ uint32_t CLK_APB0_UART1_STAT;
  __ne__ uint32_t RESERVED4[62];

  /* Offset 0x0500 */
  __rw__ uint32_t CLK_APB0_USART0_CFG;
  __ro__ uint32_t CLK_APB0_USART0_STAT;
  __ne__ uint32_t RESERVED5[62];

  /* Offset 0x0600 */
  __rw__ uint32_t CLK_APB2_SSP1_CFG;
  __ro__ uint32_t CLK_APB2_SSP1_STAT;
  __ne__ uint32_t RESERVED6[62];

  /* Offset 0x0700 */
  __rw__ uint32_t CLK_APB0_SSP0_CFG;
  __ro__ uint32_t CLK_APB0_SSP0_STAT;
  __ne__ uint32_t RESERVED7[62];

  /* Offset 0x0800 */
  __rw__ uint32_t CLK_SDIO_CFG;
  __ro__ uint32_t CLK_SDIO_STAT;
} LPC_CCU2_Type;
/*------------------Configuration register------------------------------------*/
typedef struct
{
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t CREG0;
  __ne__ uint32_t RESERVED1[62];

  /* Offset 0x0100 */
  __rw__ uint32_t M4MEMMAP;
  __ne__ uint32_t RESERVED2[5];
  __rw__ uint32_t CREG5;
  __rw__ uint32_t DMAMUX;

  /* Offset 0x0120 */
  union
  {
    __rw__ uint32_t FLASHCFG[2];
    struct
    {
      __rw__ uint32_t FLASHCFGA;
      __rw__ uint32_t FLASHCFGB;
    };
  };
  __rw__ uint32_t ETBCFG;
  __rw__ uint32_t CREG6;
  __rw__ uint32_t M4TXEVENT;
  __ne__ uint32_t RESERVED3[51];

  /* Offset 0x0200 */
  __ro__ uint32_t CHIPID;
  __ne__ uint32_t RESERVED4[65];
  __rw__ uint32_t M0SUBMEMMAP;
  __ne__ uint32_t RESERVED5;
  __rw__ uint32_t M0SUBTXEVENT;
  __ne__ uint32_t RESERVED6[58];

  /* Offset 0x0400 */
  __rw__ uint32_t M0APPTXEVENT;
  __rw__ uint32_t M0APPMEMMAP;
  __ne__ uint32_t RESERVED7[62];

  /* Offset 0x0500 */
  __rw__ uint32_t USB0FLADJ;
  __ne__ uint32_t RESERVED8[63];

  /* Offset 0x0600 */
  __rw__ uint32_t USB1FLADJ;
} LPC_CREG_Type;
/*------------------Power Management Controller-------------------------------*/
typedef struct
{
  __rw__ uint32_t PD0_SLEEP0_HW_ENA;
  __ne__ uint32_t RESERVED0[6];
  __rw__ uint32_t PD0_SLEEP0_MODE;
} LPC_PMC_Type;
/*------------------System Configuration Unit---------------------------------*/
typedef struct
{
  union
  {
    __rw__ uint32_t SFSP0[2];
    struct
    {
      __rw__ uint32_t SFSP0_0;
      __rw__ uint32_t SFSP0_1;
    };
  };
  __ne__ uint32_t RESERVED0[30];

  /* Offset 0x0080 */
  union
  {
    __rw__ uint32_t SFSP1[21];
    struct
    {
      __rw__ uint32_t SFSP1_0;
      __rw__ uint32_t SFSP1_1;
      __rw__ uint32_t SFSP1_2;
      __rw__ uint32_t SFSP1_3;
      __rw__ uint32_t SFSP1_4;
      __rw__ uint32_t SFSP1_5;
      __rw__ uint32_t SFSP1_6;
      __rw__ uint32_t SFSP1_7;
      __rw__ uint32_t SFSP1_8;
      __rw__ uint32_t SFSP1_9;
      __rw__ uint32_t SFSP1_10;
      __rw__ uint32_t SFSP1_11;
      __rw__ uint32_t SFSP1_12;
      __rw__ uint32_t SFSP1_13;
      __rw__ uint32_t SFSP1_14;
      __rw__ uint32_t SFSP1_15;
      __rw__ uint32_t SFSP1_16;
      __rw__ uint32_t SFSP1_17;
      __rw__ uint32_t SFSP1_18;
      __rw__ uint32_t SFSP1_19;
      __rw__ uint32_t SFSP1_20;
    };
  };
  __ne__ uint32_t RESERVED1[11];

  /* Offset 0x0100 */
  union
  {
    __rw__ uint32_t SFSP2[14];
    struct
    {
      __rw__ uint32_t SFSP2_0;
      __rw__ uint32_t SFSP2_1;
      __rw__ uint32_t SFSP2_2;
      __rw__ uint32_t SFSP2_3;
      __rw__ uint32_t SFSP2_4;
      __rw__ uint32_t SFSP2_5;
      __rw__ uint32_t SFSP2_6;
      __rw__ uint32_t SFSP2_7;
      __rw__ uint32_t SFSP2_8;
      __rw__ uint32_t SFSP2_9;
      __rw__ uint32_t SFSP2_10;
      __rw__ uint32_t SFSP2_11;
      __rw__ uint32_t SFSP2_12;
      __rw__ uint32_t SFSP2_13;
    };
  };
  __ne__ uint32_t RESERVED2[18];

  /* Offset 0x0180 */
  union
  {
    __rw__ uint32_t SFSP3[9];
    struct
    {
      __rw__ uint32_t SFSP3_0;
      __rw__ uint32_t SFSP3_1;
      __rw__ uint32_t SFSP3_2;
      __rw__ uint32_t SFSP3_3;
      __rw__ uint32_t SFSP3_4;
      __rw__ uint32_t SFSP3_5;
      __rw__ uint32_t SFSP3_6;
      __rw__ uint32_t SFSP3_7;
      __rw__ uint32_t SFSP3_8;
    };
  };
  __ne__ uint32_t RESERVED3[23];

  /* Offset 0x0200 */
  union
  {
    __rw__ uint32_t SFSP4[11];
    struct
    {
    __rw__ uint32_t SFSP4_0;
    __rw__ uint32_t SFSP4_1;
    __rw__ uint32_t SFSP4_2;
    __rw__ uint32_t SFSP4_3;
    __rw__ uint32_t SFSP4_4;
    __rw__ uint32_t SFSP4_5;
    __rw__ uint32_t SFSP4_6;
    __rw__ uint32_t SFSP4_7;
    __rw__ uint32_t SFSP4_8;
    __rw__ uint32_t SFSP4_9;
    __rw__ uint32_t SFSP4_10;
    };
  };
  __ne__ uint32_t RESERVED4[21];

  /* Offset 0x0280 */
  union
  {
    __rw__ uint32_t SFSP5[8];
    struct
    {
      __rw__ uint32_t SFSP5_0;
      __rw__ uint32_t SFSP5_1;
      __rw__ uint32_t SFSP5_2;
      __rw__ uint32_t SFSP5_3;
      __rw__ uint32_t SFSP5_4;
      __rw__ uint32_t SFSP5_5;
      __rw__ uint32_t SFSP5_6;
      __rw__ uint32_t SFSP5_7;
    };
  };
  __ne__ uint32_t RESERVED5[24];

  /* Offset 0x0300 */
  union
  {
    __rw__ uint32_t SFSP6[13];
    struct
    {
      __rw__ uint32_t SFSP6_0;
      __rw__ uint32_t SFSP6_1;
      __rw__ uint32_t SFSP6_2;
      __rw__ uint32_t SFSP6_3;
      __rw__ uint32_t SFSP6_4;
      __rw__ uint32_t SFSP6_5;
      __rw__ uint32_t SFSP6_6;
      __rw__ uint32_t SFSP6_7;
      __rw__ uint32_t SFSP6_8;
      __rw__ uint32_t SFSP6_9;
      __rw__ uint32_t SFSP6_10;
      __rw__ uint32_t SFSP6_11;
      __rw__ uint32_t SFSP6_12;
    };
  };
  __ne__ uint32_t RESERVED6[19];

  /* Offset 0x0380 */
  union
  {
    __rw__ uint32_t SFSP7[8];
    struct
    {
      __rw__ uint32_t SFSP7_0;
      __rw__ uint32_t SFSP7_1;
      __rw__ uint32_t SFSP7_2;
      __rw__ uint32_t SFSP7_3;
      __rw__ uint32_t SFSP7_4;
      __rw__ uint32_t SFSP7_5;
      __rw__ uint32_t SFSP7_6;
      __rw__ uint32_t SFSP7_7;
    };
  };
  __ne__ uint32_t RESERVED7[24];

  /* Offset 0x0400 */
  union
  {
    __rw__ uint32_t SFSP8[9];
    struct
    {
      __rw__ uint32_t SFSP8_0;
      __rw__ uint32_t SFSP8_1;
      __rw__ uint32_t SFSP8_2;
      __rw__ uint32_t SFSP8_3;
      __rw__ uint32_t SFSP8_4;
      __rw__ uint32_t SFSP8_5;
      __rw__ uint32_t SFSP8_6;
      __rw__ uint32_t SFSP8_7;
      __rw__ uint32_t SFSP8_8;
    };
  };
  __ne__ uint32_t RESERVED8[23];

  /* Offset 0x0480 */
  union
  {
    __rw__ uint32_t SFSP9[15];
    struct
    {
      __rw__ uint32_t SFSP9_0;
      __rw__ uint32_t SFSP9_1;
      __rw__ uint32_t SFSP9_2;
      __rw__ uint32_t SFSP9_3;
      __rw__ uint32_t SFSP9_4;
      __rw__ uint32_t SFSP9_5;
      __rw__ uint32_t SFSP9_6;
      __rw__ uint32_t SFSP9_7;
      __rw__ uint32_t SFSP9_8;
      __rw__ uint32_t SFSP9_9;
      __rw__ uint32_t SFSP9_10;
      __rw__ uint32_t SFSP9_11;
      __rw__ uint32_t SFSP9_12;
      __rw__ uint32_t SFSP9_13;
      __rw__ uint32_t SFSP9_14;
    };
  };
  __ne__ uint32_t RESERVED9[17];

  /* Offset 0x0500 */
  union
  {
    __rw__ uint32_t SFSPA[5];
    struct
    {
      __rw__ uint32_t SFSPA_0;
      __rw__ uint32_t SFSPA_1;
      __rw__ uint32_t SFSPA_2;
      __rw__ uint32_t SFSPA_3;
      __rw__ uint32_t SFSPA_4;
    };
  };
  __ne__ uint32_t RESERVED11[27];

  /* Offset 0x0580 */
  union
  {
    __rw__ uint32_t SFSPB[7];
    struct
    {
      __rw__ uint32_t SFSPB_0;
      __rw__ uint32_t SFSPB_1;
      __rw__ uint32_t SFSPB_2;
      __rw__ uint32_t SFSPB_3;
      __rw__ uint32_t SFSPB_4;
      __rw__ uint32_t SFSPB_5;
      __rw__ uint32_t SFSPB_6;
    };
  };
  __ne__ uint32_t RESERVED12[25];

  /* Offset 0x0600 */
  union
  {
    __rw__ uint32_t SFSPC[15];
    struct
    {
      __rw__ uint32_t SFSPC_0;
      __rw__ uint32_t SFSPC_1;
      __rw__ uint32_t SFSPC_2;
      __rw__ uint32_t SFSPC_3;
      __rw__ uint32_t SFSPC_4;
      __rw__ uint32_t SFSPC_5;
      __rw__ uint32_t SFSPC_6;
      __rw__ uint32_t SFSPC_7;
      __rw__ uint32_t SFSPC_8;
      __rw__ uint32_t SFSPC_9;
      __rw__ uint32_t SFSPC_10;
      __rw__ uint32_t SFSPC_11;
      __rw__ uint32_t SFSPC_12;
      __rw__ uint32_t SFSPC_13;
      __rw__ uint32_t SFSPC_14;
    };
  };
  __ne__ uint32_t RESERVED13[17];

  /* Offset 0x0680 */
  union
  {
    __rw__ uint32_t SFSPD[17];
    struct
    {
      __rw__ uint32_t SFSPD_0;
      __rw__ uint32_t SFSPD_1;
      __rw__ uint32_t SFSPD_2;
      __rw__ uint32_t SFSPD_3;
      __rw__ uint32_t SFSPD_4;
      __rw__ uint32_t SFSPD_5;
      __rw__ uint32_t SFSPD_6;
      __rw__ uint32_t SFSPD_7;
      __rw__ uint32_t SFSPD_8;
      __rw__ uint32_t SFSPD_9;
      __rw__ uint32_t SFSPD_10;
      __rw__ uint32_t SFSPD_11;
      __rw__ uint32_t SFSPD_12;
      __rw__ uint32_t SFSPD_13;
      __rw__ uint32_t SFSPD_14;
      __rw__ uint32_t SFSPD_15;
      __rw__ uint32_t SFSPD_16;
    };
  };
  __ne__ uint32_t RESERVED14[15];

  /* Offset 0x0700 */
  union
  {
    __rw__ uint32_t SFSPE[16];
    struct
    {
      __rw__ uint32_t SFSPE_0;
      __rw__ uint32_t SFSPE_1;
      __rw__ uint32_t SFSPE_2;
      __rw__ uint32_t SFSPE_3;
      __rw__ uint32_t SFSPE_4;
      __rw__ uint32_t SFSPE_5;
      __rw__ uint32_t SFSPE_6;
      __rw__ uint32_t SFSPE_7;
      __rw__ uint32_t SFSPE_8;
      __rw__ uint32_t SFSPE_9;
      __rw__ uint32_t SFSPE_10;
      __rw__ uint32_t SFSPE_11;
      __rw__ uint32_t SFSPE_12;
      __rw__ uint32_t SFSPE_13;
      __rw__ uint32_t SFSPE_14;
      __rw__ uint32_t SFSPE_15;
    };
  };
  __ne__ uint32_t RESERVED15[16];

  /* Offset 0x0780 */
  union
  {
    __rw__ uint32_t SFSPF[12];
    struct
    {
      __rw__ uint32_t SFSPF_0;
      __rw__ uint32_t SFSPF_1;
      __rw__ uint32_t SFSPF_2;
      __rw__ uint32_t SFSPF_3;
      __rw__ uint32_t SFSPF_4;
      __rw__ uint32_t SFSPF_5;
      __rw__ uint32_t SFSPF_6;
      __rw__ uint32_t SFSPF_7;
      __rw__ uint32_t SFSPF_8;
      __rw__ uint32_t SFSPF_9;
      __rw__ uint32_t SFSPF_10;
      __rw__ uint32_t SFSPF_11;
    };
  };
  __ne__ uint32_t RESERVED16[276];

  /* Offset 0x0C00 */
  __rw__ uint32_t SFSPCLK0;
  __rw__ uint32_t SFSPCLK1;
  __rw__ uint32_t SFSPCLK2;
  __rw__ uint32_t SFSPCLK3;
  __ne__ uint32_t RESERVED17[28];

  /* Offset 0x0C80 */
  __rw__ uint32_t SFSUSB;
  __rw__ uint32_t SFSI2C0;

  union
  {
    __rw__ uint32_t ENAIO[3];
    struct
    {
      __rw__ uint32_t ENAIO0;
      __rw__ uint32_t ENAIO1;
      __rw__ uint32_t ENAIO2;
    };
  };

  __ne__ uint32_t RESERVED18[27];

  /* Offset 0x0D00 */
  __rw__ uint32_t EMCDELAYCLK;
  __ne__ uint32_t RESERVED19[31];

  /* Offset 0x0D80 */
  __rw__ uint32_t SDDELAY;
  __ne__ uint32_t RESERVED20[31];

  /* Offset 0x0E00 */
  union
  {
    __rw__ uint32_t PINTSEL[2];
    struct
    {
      __rw__ uint32_t PINTSEL0;
      __rw__ uint32_t PINTSEL1;
    };
  };
} LPC_SCU_Type;
/*------------------General Purpose Input/Output------------------------------*/
typedef struct
{
  __rw__ uint32_t ISEL;
  __rw__ uint32_t IENR;
  __wo__ uint32_t SIENR;
  __wo__ uint32_t CIENR;
  __rw__ uint32_t IENF;
  __wo__ uint32_t SIENF;
  __wo__ uint32_t CIENF;
  __rw__ uint32_t RISE;
  __rw__ uint32_t FALL;
  __rw__ uint32_t IST;
} LPC_GPIO_INT_Type;

typedef struct
{
  __rw__ uint32_t CTRL;
  __ne__ uint32_t RESERVED0[7];

  /* Offset 0x20 */
  union
  {
    __rw__ uint32_t PORT_POL[8];
    struct
    {
      __rw__ uint32_t PORT_POL0;
      __rw__ uint32_t PORT_POL1;
      __rw__ uint32_t PORT_POL2;
      __rw__ uint32_t PORT_POL3;
      __rw__ uint32_t PORT_POL4;
      __rw__ uint32_t PORT_POL5;
      __rw__ uint32_t PORT_POL6;
      __rw__ uint32_t PORT_POL7;
    };
  };

  /* Offset 0x40 */
  union
  {
    __rw__ uint32_t PORT_ENA[8];
    struct
    {
      __rw__ uint32_t PORT_ENA0;
      __rw__ uint32_t PORT_ENA1;
      __rw__ uint32_t PORT_ENA2;
      __rw__ uint32_t PORT_ENA3;
      __rw__ uint32_t PORT_ENA4;
      __rw__ uint32_t PORT_ENA5;
      __rw__ uint32_t PORT_ENA6;
      __rw__ uint32_t PORT_ENA7;
    };
  };
} LPC_GPIO_GROUP_INT_Type;

typedef struct
{
  union
  {
    __rw__ uint8_t B[256];
    struct
    {
      __rw__ uint8_t B0[32];
      __rw__ uint8_t B1[32];
      __rw__ uint8_t B2[32];
      __rw__ uint8_t B3[32];
      __rw__ uint8_t B4[32];
      __rw__ uint8_t B5[32];
      __rw__ uint8_t B6[32];
      __rw__ uint8_t B7[32];
    };
  };
  __ne__ uint32_t RESERVED0[960];

  /* Offset 0x1000 */
  union
  {
    __rw__ uint32_t W[64];
    struct
    {
      __rw__ uint32_t W0[32];
      __rw__ uint32_t W1[32];
      __rw__ uint32_t W2[32];
      __rw__ uint32_t W3[32];
      __rw__ uint32_t W4[32];
      __rw__ uint32_t W5[32];
      __rw__ uint32_t W6[32];
      __rw__ uint32_t W7[32];
    };
  };
  __ne__ uint32_t RESERVED1[768];

  /* Offset 0x2000 */
  union
  {
    __rw__ uint32_t DIR[8];
    struct
    {
      __rw__ uint32_t DIR0;
      __rw__ uint32_t DIR1;
      __rw__ uint32_t DIR2;
      __rw__ uint32_t DIR3;
      __rw__ uint32_t DIR4;
      __rw__ uint32_t DIR5;
      __rw__ uint32_t DIR6;
      __rw__ uint32_t DIR7;
    };
  };
  __ne__ uint32_t RESERVED2[24];

  /* Offset 0x2080 */
  union
  {
    __rw__ uint32_t MASK[8];
    struct
    {
      __rw__ uint32_t MASK0;
      __rw__ uint32_t MASK1;
      __rw__ uint32_t MASK2;
      __rw__ uint32_t MASK3;
      __rw__ uint32_t MASK4;
      __rw__ uint32_t MASK5;
      __rw__ uint32_t MASK6;
      __rw__ uint32_t MASK7;
    };
  };
  __ne__ uint32_t RESERVED3[24];

  /* Offset 0x2100 */
  union
  {
    __rw__ uint32_t PIN[8];
    struct
    {
      __rw__ uint32_t PIN0;
      __rw__ uint32_t PIN1;
      __rw__ uint32_t PIN2;
      __rw__ uint32_t PIN3;
      __rw__ uint32_t PIN4;
      __rw__ uint32_t PIN5;
      __rw__ uint32_t PIN6;
      __rw__ uint32_t PIN7;
    };
  };
  __ne__ uint32_t RESERVED4[24];

  /* Offset 0x2180 */
  union
  {
    __rw__ uint32_t MPIN[8];
    struct
    {
      __rw__ uint32_t MPIN0;
      __rw__ uint32_t MPIN1;
      __rw__ uint32_t MPIN2;
      __rw__ uint32_t MPIN3;
      __rw__ uint32_t MPIN4;
      __rw__ uint32_t MPIN5;
      __rw__ uint32_t MPIN6;
      __rw__ uint32_t MPIN7;
    };
  };
  __ne__ uint32_t RESERVED5[24];

  /* Offset 0x2200 */
  union
  {
    __rw__ uint32_t SET[8];
    struct
    {
      __rw__ uint32_t SET0;
      __rw__ uint32_t SET1;
      __rw__ uint32_t SET2;
      __rw__ uint32_t SET3;
      __rw__ uint32_t SET4;
      __rw__ uint32_t SET5;
      __rw__ uint32_t SET6;
      __rw__ uint32_t SET7;
    };
  };
  __ne__ uint32_t RESERVED6[24];

  /* Offset 0x2280 */
  union
  {
    __wo__ uint32_t CLR[8];
    struct
    {
      __wo__ uint32_t CLR0;
      __wo__ uint32_t CLR1;
      __wo__ uint32_t CLR2;
      __wo__ uint32_t CLR3;
      __wo__ uint32_t CLR4;
      __wo__ uint32_t CLR5;
      __wo__ uint32_t CLR6;
      __wo__ uint32_t CLR7;
    };
  };
  __ne__ uint32_t RESERVED7[24];

  /* Offset 0x2300 */
  union
  {
    __wo__ uint32_t NOT[8];
    struct
    {
      __wo__ uint32_t NOT0;
      __wo__ uint32_t NOT1;
      __wo__ uint32_t NOT2;
      __wo__ uint32_t NOT3;
      __wo__ uint32_t NOT4;
      __wo__ uint32_t NOT5;
      __wo__ uint32_t NOT6;
      __wo__ uint32_t NOT7;
    };
  };
} LPC_GPIO_Type;
/*------------------Serial GPIO-----------------------------------------------*/
typedef struct
{
  /* Offset 0x0000 */
  __rw__ uint32_t OUT_MUX_CFG[16];
  __rw__ uint32_t SGPIO_MUX_CFG[16];
  __rw__ uint32_t SLICE_MUX_CFG[16];
  __rw__ uint32_t REG[16];
  __rw__ uint32_t REG_SS[16];
  __rw__ uint32_t PRESET[16];
  __rw__ uint32_t COUNT[16];
  __rw__ uint32_t POS[16];

  /* Offset 0x0200 */
  __rw__ uint32_t MASK_A;
  __rw__ uint32_t MASK_H;
  __rw__ uint32_t MASK_I;
  __rw__ uint32_t MASK_P;
  __ro__ uint32_t GPIO_INREG;
  __rw__ uint32_t GPIO_OUTREG;
  __rw__ uint32_t GPIO_OENREG;
  __rw__ uint32_t CTRL_ENABLED;
  __rw__ uint32_t CTRL_DISABLED;
  __ne__ uint32_t RESERVED0[823];

  /* Offset 0x0F00: shift clock interrupt registers */
  __wo__ uint32_t CLR_EN_0;
  __wo__ uint32_t SET_EN_0;
  __ro__ uint32_t ENABLE_0;
  __ro__ uint32_t STATUS_0;
  __wo__ uint32_t CTR_STATUS_0;
  __wo__ uint32_t SET_STATUS_0;
  __ne__ uint32_t RESERVED1[2];

  /* Offset 0x0F20: exchange clock interrupt registers */
  __wo__ uint32_t CLR_EN_1;
  __wo__ uint32_t SET_EN_1;
  __ro__ uint32_t ENABLE_1;
  __ro__ uint32_t STATUS_1;
  __wo__ uint32_t CTR_STATUS_1;
  __wo__ uint32_t SET_STATUS_1;
  __ne__ uint32_t RESERVED2[2];

  /* Offset 0x0F40: pattern match interrupt registers */
  __wo__ uint32_t CLR_EN_2;
  __wo__ uint32_t SET_EN_2;
  __ro__ uint32_t ENABLE_2;
  __ro__ uint32_t STATUS_2;
  __wo__ uint32_t CTR_STATUS_2;
  __wo__ uint32_t SET_STATUS_2;
  __ne__ uint32_t RESERVED3[2];

  /* Offset 0x0F60: input bit match interrupt registers */
  __wo__ uint32_t CLR_EN_3;
  __wo__ uint32_t SET_EN_3;
  __ro__ uint32_t ENABLE_3;
  __ro__ uint32_t STATUS_3;
  __wo__ uint32_t CTR_STATUS_3;
  __wo__ uint32_t SET_STATUS_3;
} LPC_SGPIO_Type;
/*------------------Reset Generation Unit-------------------------------------*/
typedef struct
{
  __ne__ uint32_t RESERVED0[64];

  /* Offset 0x100 */
  union
  {
    __wo__ uint32_t RESET_CTRL[2];
    struct
    {
      __wo__ uint32_t RESET_CTRL0;
      __wo__ uint32_t RESET_CTRL1;
    };
  };
  __ne__ uint32_t RESERVED1[2];

  /* Offset 0x110 */
  union
  {
    __rw__ uint32_t RESET_STATUS[4];
    struct
    {
      __rw__ uint32_t RESET_STATUS0;
      __rw__ uint32_t RESET_STATUS1;
      __rw__ uint32_t RESET_STATUS2;
      __rw__ uint32_t RESET_STATUS3;
    };
  };
  __ne__ uint32_t RESERVED2[12];

  /* Offset 0x150 */
  union
  {
    __ro__ uint32_t RESET_ACTIVE_STATUS[2];
    struct
    {
      __ro__ uint32_t RESET_ACTIVE_STATUS0;
      __ro__ uint32_t RESET_ACTIVE_STATUS1;
    };
  };
  __ne__ uint32_t RESERVED3[170];

  /* Offset 0x400 */
  union
  {
    __rw__ uint32_t RESET_EXT_STAT[64];
    struct
    {
      __ne__ uint32_t RESERVED4;
      __rw__ uint32_t RESET_EXT_STAT1;
      __rw__ uint32_t RESET_EXT_STAT2;
      __ne__ uint32_t RESERVED5[2];
      __rw__ uint32_t RESET_EXT_STAT5;
      __ne__ uint32_t RESERVED6[2];
      __rw__ uint32_t RESET_EXT_STAT8;
      __rw__ uint32_t RESET_EXT_STAT9;
      __ne__ uint32_t RESERVED7[3];
      __rw__ uint32_t RESET_EXT_STAT13;
      __ne__ uint32_t RESERVED8[2];
      __rw__ uint32_t RESET_EXT_STAT16;
      __rw__ uint32_t RESET_EXT_STAT17;
      __rw__ uint32_t RESET_EXT_STAT18;
      __rw__ uint32_t RESET_EXT_STAT19;
      __rw__ uint32_t RESET_EXT_STAT20;
      __rw__ uint32_t RESET_EXT_STAT21;
      __rw__ uint32_t RESET_EXT_STAT22;
      __ne__ uint32_t RESERVED9[2];
      __rw__ uint32_t RESET_EXT_STAT25;
      __ne__ uint32_t RESERVED10;
      __rw__ uint32_t RESET_EXT_STAT27;
      __rw__ uint32_t RESET_EXT_STAT28;
      __rw__ uint32_t RESET_EXT_STAT29;
      __ne__ uint32_t RESERVED11[2];
      __rw__ uint32_t RESET_EXT_STAT32;
      __rw__ uint32_t RESET_EXT_STAT33;
      __rw__ uint32_t RESET_EXT_STAT34;
      __rw__ uint32_t RESET_EXT_STAT35;
      __rw__ uint32_t RESET_EXT_STAT36;
      __rw__ uint32_t RESET_EXT_STAT37;
      __rw__ uint32_t RESET_EXT_STAT38;
      __rw__ uint32_t RESET_EXT_STAT39;
      __rw__ uint32_t RESET_EXT_STAT40;
      __rw__ uint32_t RESET_EXT_STAT41;
      __rw__ uint32_t RESET_EXT_STAT42;
      __ne__ uint32_t RESERVED12;
      __rw__ uint32_t RESET_EXT_STAT44;
      __rw__ uint32_t RESET_EXT_STAT45;
      __rw__ uint32_t RESET_EXT_STAT46;
      __rw__ uint32_t RESET_EXT_STAT47;
      __rw__ uint32_t RESET_EXT_STAT48;
      __rw__ uint32_t RESET_EXT_STAT49;
      __rw__ uint32_t RESET_EXT_STAT50;
      __rw__ uint32_t RESET_EXT_STAT51;
      __rw__ uint32_t RESET_EXT_STAT52;
      __rw__ uint32_t RESET_EXT_STAT53;
      __rw__ uint32_t RESET_EXT_STAT54;
      __rw__ uint32_t RESET_EXT_STAT55;
      __rw__ uint32_t RESET_EXT_STAT56;
      __rw__ uint32_t RESET_EXT_STAT57;
      __rw__ uint32_t RESET_EXT_STAT58;
      __ne__ uint32_t RESERVED13;
      __rw__ uint32_t RESET_EXT_STAT60;
      __ne__ uint32_t RESERVED14[3];
    };
  };
} LPC_RGU_Type;
/*------------------Analog-to-Digital Converter-------------------------------*/
typedef struct
{
  __rw__ uint32_t CR;
  __rw__ uint32_t GDR;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t INTEN;
  union
  {
    __ro__ uint32_t DR[8];
    struct
    {
      __ro__ uint32_t DR0;
      __ro__ uint32_t DR1;
      __ro__ uint32_t DR2;
      __ro__ uint32_t DR3;
      __ro__ uint32_t DR4;
      __ro__ uint32_t DR5;
      __ro__ uint32_t DR6;
      __ro__ uint32_t DR7;
    };
  };
  __ro__ uint32_t STAT;
} LPC_ADC_Type;
/*------------------Digital-to-Analog Converter-------------------------------*/
typedef struct
{
  __rw__ uint32_t CR; /* Converter Register */
  __rw__ uint32_t CTRL; /* Control register */
  __rw__ uint16_t CNTVAL; /* Counter Value register */
} LPC_DAC_Type;
/*------------------Inter-Integrated Circuit----------------------------------*/
typedef struct
{
  __rw__ uint32_t CONSET;
  __ro__ uint32_t STAT;
  __rw__ uint32_t DAT;
  __rw__ uint32_t ADR0;
  __rw__ uint32_t SCLH;
  __rw__ uint32_t SCLL;
  __rw__ uint32_t CONCLR;
  __rw__ uint32_t MMCTRL;
  __rw__ uint32_t ADR1;
  __rw__ uint32_t ADR2;
  __rw__ uint32_t ADR3;
  __ro__ uint32_t DATA_BUFFER;
  __rw__ uint32_t MASK0;
  __rw__ uint32_t MASK1;
  __rw__ uint32_t MASK2;
  __rw__ uint32_t MASK3;
} LPC_I2C_Type;
/*------------------Synchronous Serial Port-----------------------------------*/
typedef struct
{
  __rw__ uint32_t CR0;
  __rw__ uint32_t CR1;
  __rw__ uint32_t DR;
  __ro__ uint32_t SR;
  __rw__ uint32_t CPSR;
  __rw__ uint32_t IMSC;
  __rw__ uint32_t RIS;
  __rw__ uint32_t MIS;
  __wo__ uint32_t ICR;
  __rw__ uint32_t DMACR;
} LPC_SSP_Type;
/*------------------Serial Peripheral Interface-------------------------------*/
typedef struct
{
  __rw__ uint32_t CR;
  __ro__ uint32_t SR;
  __rw__ uint32_t DR;
  __rw__ uint32_t CCR;
  __ne__ uint32_t RESERVED[3];
  __rw__ uint32_t INT;
} LPC_SPI_Type;
/*------------------Timer/Counter---------------------------------------------*/
typedef struct
{
  __rw__ uint32_t IR;
  __rw__ uint32_t TCR;
  __rw__ uint32_t TC;
  __rw__ uint32_t PR;
  __rw__ uint32_t PC;
  __rw__ uint32_t MCR;
  union
  {
    __rw__ uint32_t MR[4];
    struct
    {
      __rw__ uint32_t MR0;
      __rw__ uint32_t MR1;
      __rw__ uint32_t MR2;
      __rw__ uint32_t MR3;
    };
  };
  __rw__ uint32_t CCR;
  union
  {
    __ro__ uint32_t CR[4];
    struct
    {
      __ro__ uint32_t CR0;
      __ro__ uint32_t CR1;
      __ro__ uint32_t CR2;
      __ro__ uint32_t CR3;
    };
  };
  __rw__ uint32_t EMR;
  __ne__ uint32_t RESERVED1[12];
  __rw__ uint32_t CTCR;
} LPC_TIMER_Type;
/*------------------State Configurable Timer----------------------------------*/
typedef struct
{
  __rw__ uint32_t CONFIG;
  union
  {
    __rw__ uint32_t CTRL;
    __rw__ uint16_t CTRL_PART[2];
    struct
    {
      __rw__ uint16_t CTRL_L;
      __rw__ uint16_t CTRL_H;
    };
  };
  union
  {
    __rw__ uint32_t LIMIT;
    __rw__ uint16_t LIMIT_PART[2];
    struct
    {
      __rw__ uint16_t LIMIT_L;
      __rw__ uint16_t LIMIT_H;
    };
  };
  union
  {
    __rw__ uint32_t HALT;
    __rw__ uint16_t HALT_PART[2];
    struct
    {
      __rw__ uint16_t HALT_L;
      __rw__ uint16_t HALT_H;
    };
  };
  union
  {
    __rw__ uint32_t STOP;
    __rw__ uint16_t STOP_PART[2];
    struct
    {
      __rw__ uint16_t STOP_L;
      __rw__ uint16_t STOP_H;
    };
  };
  union
  {
    __rw__ uint32_t START;
    __rw__ uint16_t START_PART[2];
    struct
    {
      __rw__ uint16_t START_L;
      __rw__ uint16_t START_H;
    };
  };
  union
  {
    __rw__ uint32_t DITHER;
    __rw__ uint16_t DITHER_PART[2];
    struct
    {
      __rw__ uint16_t DITHER_L;
      __rw__ uint16_t DITHER_H;
    };
  };
  __ne__ uint32_t RESERVED0[9];

  /* Offset 0x0040 */
  union
  {
    __rw__ uint32_t COUNT;
    __rw__ uint16_t COUNT_PART[2];
    struct
    {
      __rw__ uint16_t COUNT_L;
      __rw__ uint16_t COUNT_H;
    };
  };
  union
  {
    __rw__ uint32_t STATE;
    __rw__ uint16_t STATE_PART[2];
    struct
    {
      __rw__ uint16_t STATE_L;
      __rw__ uint16_t STATE_H;
    };
  };
  __ro__ uint32_t INPUT;
  union
  {
    __rw__ uint32_t REGMODE;
    __rw__ uint16_t REGMODE_PART[2];
    struct
    {
      __rw__ uint16_t REGMODE_L;
      __rw__ uint16_t REGMODE_H;
    };
  };
  __rw__ uint32_t OUTPUT;
  __rw__ uint32_t OUTPUTDIRCTRL;
  __rw__ uint32_t RES;
  union
  {
    __rw__ uint32_t DMAREQ[2];
    struct
    {
      __rw__ uint32_t DMAREQ0;
      __rw__ uint32_t DMAREQ1;
    };
  };
  __ne__ uint32_t RESERVED1[35];
  __rw__ uint32_t EVEN;
  __rw__ uint32_t EVFLAG;
  __rw__ uint32_t CONEN;
  __rw__ uint32_t CONFLAG;

  /* Offset 0x0100 */
  union
  {
    union
    {
      __rw__ uint32_t MATCH[16];
      __rw__ uint16_t MATCH_PART[16][2];
    };
    union
    {
      __ro__ uint32_t CAP[16];
      __ro__ uint16_t CAP_PART[16][2];
    };
  };
  union
  {
    __rw__ uint32_t FRACMAT[6];
    __rw__ uint16_t FRACMAT_PART[6][2];
  };
  __ne__ uint32_t RESERVED2[42]; /* Alias registers unused */
  union
  {
    union
    {
      __rw__ uint32_t MATCHREL[16];
      __rw__ uint16_t MATCHREL_PART[16][2];
    };
    union
    {
      __rw__ uint32_t CAPCTRL[16];
      __rw__ uint16_t CAPCTRL_PART[16][2];
    };
  };
  union
  {
    __rw__ uint32_t FRACMATREL[6];
    __rw__ uint16_t FRACMATREL_PART[6][2];
  };
  __ne__ uint32_t RESERVED3[42]; /* Alias registers unused */
  struct
  {
    __rw__ uint32_t STATE;
    __rw__ uint32_t CTRL;
  } EV[16];
  __ne__ uint32_t RESERVED4[96];
  struct
  {
    __rw__ uint32_t SET;
    __rw__ uint32_t CLR;
  } OUT[16];
} LPC_SCT_Type;
/*------------------Universal Asynchronous Receiver Transmitter---------------*/
typedef struct
{
  union
  {
    __ro__ uint32_t RBR;
    __wo__ uint32_t THR;
    __rw__ uint32_t DLL;
  };
  union
  {
    __rw__ uint32_t DLM;
    __rw__ uint32_t IER;
  };
  union
  {
    __ro__ uint32_t IIR;
    __wo__ uint32_t FCR;
  };
  __rw__ uint32_t LCR;
  __rw__ uint32_t MCR;
  __ro__ uint32_t LSR;
  __ro__ uint32_t MSR;
  __rw__ uint32_t SCR;
  __rw__ uint32_t ACR;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t FDR;
  __ne__ uint32_t RESERVED1[8];
  __rw__ uint32_t RS485CTRL;
  __rw__ uint32_t RS485ADRMATCH;
  __rw__ uint32_t RS485DLY;
  __rw__ uint32_t SYNCCTRL;
  __rw__ uint32_t TER;
} LPC_USART_Type;
/*------------------Extended Universal Asynchronous Receiver Transmitter------*/
/* UART block with modem control, RS485 support and IrDA mode */
typedef struct
{
  union
  {
    __ro__ uint32_t RBR;
    __wo__ uint32_t THR;
    __rw__ uint32_t DLL;
  };
  union
  {
    __rw__ uint32_t DLM;
    __rw__ uint32_t IER;
  };
  union
  {
    __ro__ uint32_t IIR;
    __wo__ uint32_t FCR;
  };
  __rw__ uint32_t LCR;
  __ne__ uint32_t RESERVED0;
  __ro__ uint32_t LSR;
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t SCR;
  __rw__ uint32_t ACR;
  __rw__ uint32_t ICR;
  __rw__ uint32_t FDR;
  __rw__ uint32_t OSR;
  __ne__ uint32_t RESERVED2[4];
  __rw__ uint32_t HDEN;
  __ne__ uint32_t RESERVED3;
  __rw__ uint32_t SCICTRL;
  __rw__ uint32_t RS485CTRL;
  __rw__ uint32_t RS485ADRMATCH;
  __rw__ uint32_t RS485DLY;
  __rw__ uint32_t SYNCCTRL;
  __rw__ uint32_t TER;
} LPC_UART_Type;
/*------------------Inter IC Sound--------------------------------------------*/
typedef struct
{
  __rw__ uint32_t DAO;
  __rw__ uint32_t DAI;
  __wo__ uint32_t TXFIFO;
  __ro__ uint32_t RXFIFO;
  __ro__ uint32_t STATE;
  __rw__ uint32_t DMA1;
  __rw__ uint32_t DMA2;
  __rw__ uint32_t IRQ;
  __rw__ uint32_t TXRATE;
  __rw__ uint32_t RXRATE;
  __rw__ uint32_t TXBITRATE;
  __rw__ uint32_t RXBITRATE;
  __rw__ uint32_t TXMODE;
  __rw__ uint32_t RXMODE;
} LPC_I2S_Type;
/*------------------Repetitive Interrupt Timer--------------------------------*/
typedef struct
{
  __rw__ uint32_t COMPVAL;
  __rw__ uint32_t MASK;
  __rw__ uint32_t CTRL;
  __rw__ uint32_t COUNTER;
} LPC_RIT_Type;
/*------------------Alarm Timer-----------------------------------------------*/
typedef struct
{
  __rw__ uint32_t DOWNCOUNTER;
  __rw__ uint32_t PRESET;
  __ne__ uint32_t RESERVED[1012];

  /* Offset 0x0FD8 */
  __wo__ uint32_t CLR_EN;
  __wo__ uint32_t SET_EN;
  __ro__ uint32_t STATUS;
  __ro__ uint32_t ENABLE;
  __wo__ uint32_t CLR_STAT;
  __wo__ uint32_t SET_STAT;
} LPC_ATIMER_Type;
/*------------------Real-Time Clock-------------------------------------------*/
typedef struct
{
  /* Miscellaneous registers, offset 0x00 */
  __rw__ uint32_t ILR;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t CCR;
  __rw__ uint32_t CIIR;
  __rw__ uint32_t AMR;

  /* Consolidated time registers, offset 0x14 */
  __ro__ uint32_t CTIME0;
  __ro__ uint32_t CTIME1;
  __ro__ uint32_t CTIME2;

  /* Time counter registers, offset 0x20 */
  __rw__ uint32_t SEC;
  __rw__ uint32_t MIN;
  __rw__ uint32_t HOUR;
  __rw__ uint32_t DOM;
  __rw__ uint32_t DOW;
  __rw__ uint32_t DOY;
  __rw__ uint32_t MONTH;
  __rw__ uint32_t YEAR;
  __rw__ uint32_t CALIBRATION;
  __ne__ uint32_t RESERVED1[7];

  /* Alarm register group, offset 0x60 */
  __rw__ uint32_t ALSEC;
  __rw__ uint32_t ALMIN;
  __rw__ uint32_t ALHOUR;
  __rw__ uint32_t ALDOM;
  __rw__ uint32_t ALDOW;
  __rw__ uint32_t ALDOY;
  __rw__ uint32_t ALMON;
  __rw__ uint32_t ALYEAR;
} LPC_RTC_Type;
/*------------------Watchdog Timer--------------------------------------------*/
typedef struct
{
  __rw__ uint32_t MOD;
  __rw__ uint32_t TC;
  __wo__ uint32_t FEED;
  __ro__ uint32_t TV;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t WARNINT;
  __rw__ uint32_t WINDOW;
} LPC_WDT_Type;
/*------------------Motor Control Pulse-Width Modulation----------------------*/
typedef struct
{
  __ro__ uint32_t CON;
  __wo__ uint32_t CON_SET;
  __wo__ uint32_t CON_CLR;
  __ro__ uint32_t CAPCON;
  __wo__ uint32_t CAPCON_SET;
  __wo__ uint32_t CAPCON_CLR;
  __rw__ uint32_t TC0;
  __rw__ uint32_t TC1;
  __rw__ uint32_t TC2;
  __rw__ uint32_t LIM0;
  __rw__ uint32_t LIM1;
  __rw__ uint32_t LIM2;
  __rw__ uint32_t MAT0;
  __rw__ uint32_t MAT1;
  __rw__ uint32_t MAT2;
  __rw__ uint32_t DT;

  /* Offset 0x40 */
  __rw__ uint32_t MCCP;
  __ro__ uint32_t CAP0;
  __ro__ uint32_t CAP1;
  __ro__ uint32_t CAP2;
  __ro__ uint32_t INTEN;
  __wo__ uint32_t INTEN_SET;
  __wo__ uint32_t INTEN_CLR;
  __ro__ uint32_t CNTCON;
  __wo__ uint32_t CNTCON_SET;
  __wo__ uint32_t CNTCON_CLR;
  __ro__ uint32_t INTF;
  __wo__ uint32_t INTF_SET;
  __wo__ uint32_t INTF_CLR;
  __wo__ uint32_t CAP_CLR;
} LPC_MCPWM_Type;
/*------------------Quadrature Encoder Interface------------------------------*/
typedef struct
{
  __wo__ uint32_t CON;
  __ro__ uint32_t STAT;
  __rw__ uint32_t CONF;
  __ro__ uint32_t POS;
  __rw__ uint32_t MAXPOS;
  __rw__ uint32_t CMPOS0;
  __rw__ uint32_t CMPOS1;
  __rw__ uint32_t CMPOS2;
  __ro__ uint32_t INXCNT;
  __rw__ uint32_t INXCMP0;
  __rw__ uint32_t LOAD;
  __ro__ uint32_t TIME;
  __ro__ uint32_t VEL;
  __ro__ uint32_t CAP;
  __rw__ uint32_t VELCOMP;
  __rw__ uint32_t FILTERPHA;

  /* Offset 0x0040 */
  __rw__ uint32_t FILTERPHB;
  __rw__ uint32_t FILTERINX;
  __rw__ uint32_t WINDOW;
  __rw__ uint32_t INXCMP1;
  __rw__ uint32_t INXCMP2;
  __ne__ uint32_t RESERVED0[993];

  /* Offset 0x0FD8 */
  __wo__ uint32_t IEC;
  __wo__ uint32_t IES;
  __ro__ uint32_t INTSTAT;
  __ro__ uint32_t IE;
  __wo__ uint32_t CLR;
  __wo__ uint32_t SET;
} LPC_QEI_Type;
/*------------------C_CAN controller------------------------------------------*/
typedef struct
{
  __rw__ uint32_t CNTL;
  __rw__ uint32_t STAT;
  __ro__ uint32_t EC;
  __rw__ uint32_t BT;
  __ro__ uint32_t INT;
  __rw__ uint32_t TEST;
  __rw__ uint32_t BRPE;
  __ne__ uint32_t RESERVED0;

  /* Offset 0x0020 */
  struct
  {
    __rw__ uint32_t CMDREQ;
    __rw__ uint32_t CMDMSK;
    __rw__ uint32_t MSK1;
    __rw__ uint32_t MSK2;
    __rw__ uint32_t ARB1;
    __rw__ uint32_t ARB2;
    __rw__ uint32_t MCTRL;
    __rw__ uint32_t DA1;
    __rw__ uint32_t DA2;
    __rw__ uint32_t DB1;
    __rw__ uint32_t DB2;
    __ne__ uint32_t RESERVED[13];
  } IF[2];

  __ne__ uint32_t RESERVED1[8];

  /* Offset 0x0100 */
  __ro__ uint32_t TXREQ1;
  __ro__ uint32_t TXREQ2;
  __ne__ uint32_t RESERVED2[6];

  /* Offset 0x0120 */
  __ro__ uint32_t ND1;
  __ro__ uint32_t ND2;
  __ne__ uint32_t RESERVED3[6];

  /* Offset 0x0140 */
  __ro__ uint32_t IR1;
  __ro__ uint32_t IR2;
  __ne__ uint32_t RESERVED4[6];

  /* Offset 0x0160 */
  __ro__ uint32_t MSGV1;
  __ro__ uint32_t MSGV2;
  __ne__ uint32_t RESERVED5[6];

  /* Offset 0x0180 */
  __rw__ uint32_t CLKDIV;
} LPC_CAN_Type;
/*------------------SD/MMC card interface-------------------------------------*/
typedef struct
{
  __rw__ uint32_t CTRL;
  __rw__ uint32_t PWREN;
  __rw__ uint32_t CLKDIV;
  __rw__ uint32_t CLKSRC;
  __rw__ uint32_t CLKENA;
  __rw__ uint32_t TMOUT;
  __rw__ uint32_t CTYPE;
  __rw__ uint32_t BLKSIZ;

  /* Offset 0x0020 */
  __rw__ uint32_t BYTCNT;
  __rw__ uint32_t INTMASK;
  __rw__ uint32_t CMDARG;
  __rw__ uint32_t CMD;
  union
  {
    __ro__ uint32_t RESP[4];
    struct
    {
      __ro__ uint32_t RESP0;
      __ro__ uint32_t RESP1;
      __ro__ uint32_t RESP2;
      __ro__ uint32_t RESP3;
    };
  };

  /* Offset 0x0040 */
  __ro__ uint32_t MINTSTS;
  __rw__ uint32_t RINTSTS;
  __ro__ uint32_t STATUS;
  __rw__ uint32_t FIFOTH;
  __ro__ uint32_t CDETECT;
  __ro__ uint32_t WRTPRT;
  __ne__ uint32_t RESERVED0;
  __ro__ uint32_t TCBCNT;
  __ro__ uint32_t TBBCNT;
  __rw__ uint32_t DEBNCE;
  __ne__ uint32_t RESERVED1[4];
  __rw__ uint32_t RST_N;
  __ne__ uint32_t RESERVED2;

  /* Offset 0x0080 */
  __rw__ uint32_t BMOD;
  __wo__ uint32_t PLDMND;
  __rw__ uint32_t DBADDR;
  __rw__ uint32_t IDSTS;
  __rw__ uint32_t IDINTEN;
  __ro__ uint32_t DSCADDR;
  __ro__ uint32_t BUFADDR;
  __ne__ uint32_t RESERVED3;
  __rw__ uint32_t DATA[];
} LPC_SDMMC_Type;
/*------------------General Purpose Direct Memory Access controller-----------*/
/* Channel registers */
typedef struct
{
  __rw__ uint32_t SRCADDR;
  __rw__ uint32_t DESTADDR;
  __rw__ uint32_t LLI;
  __rw__ uint32_t CONTROL;
  __rw__ uint32_t CONFIG;
  __ne__ uint32_t RESERVED[3];
} LPC_GPDMA_CHANNEL_Type;

/* Common registers */
typedef struct
{
  __ro__ uint32_t INTSTAT;
  __ro__ uint32_t INTTCSTAT;
  __wo__ uint32_t INTTCCLEAR;
  __ro__ uint32_t INTERRSTAT;
  __wo__ uint32_t INTERRCLEAR;
  __ro__ uint32_t RAWINTTCSTAT;
  __ro__ uint32_t RAWINTERRSTAT;
  __ro__ uint32_t ENBLDCHNS;
  __rw__ uint32_t SOFTBREQ;
  __rw__ uint32_t SOFTSREQ;
  __rw__ uint32_t SOFTLBREQ;
  __rw__ uint32_t SOFTLSREQ;
  __rw__ uint32_t CONFIG;
  __rw__ uint32_t SYNC;
  __ne__ uint32_t RESERVED[50];

  /* Offset 0x100: channel registers */
  LPC_GPDMA_CHANNEL_Type CHANNELS[8];
} LPC_GPDMA_Type;
/*------------------Universal Serial Bus--------------------------------------*/
typedef struct
{
  __ne__ uint32_t RESERVED0[36];
  __rw__ uint32_t SBUSCFG; /* System bus interface configuration */
  __ne__ uint32_t RESERVED1[27];
  __ro__ uint32_t CAPLENGTH; /* Capability register length */
  __ro__ uint32_t HCSPARAMS; /* Host controller structural parameters */
  __ro__ uint32_t HCCPARAMS; /* Host controller capability parameters */
  __ne__ uint32_t RESERVED2[5];
  __ro__ uint32_t DCIVERSION;/* Device interface version number */
  __ne__ uint32_t RESERVED3[7];

  union
  {
    __rw__ uint32_t USBCMD_H; /* USB command (host mode) */
    __rw__ uint32_t USBCMD_D; /* USB command (device mode) */
  };

  union
  {
    __rw__ uint32_t USBSTS_H; /* USB status (host mode) */
    __rw__ uint32_t USBSTS_D; /* USB status (device mode) */
  };

  union
  {
    __rw__ uint32_t USBINTR_H; /* USB interrupt enable (host mode) */
    __rw__ uint32_t USBINTR_D; /* USB interrupt enable (device mode) */
  };

  union
  {
    __rw__ uint32_t FRINDEX_H; /* USB frame index (host mode) */
    __ro__ uint32_t FRINDEX_D; /* USB frame index (device mode) */
  };
  __ne__ uint32_t RESERVED4;

  union
  {
    __rw__ uint32_t PERIODICLISTBASE; /* Frame list base address */
    __rw__ uint32_t DEVICEADDR; /* USB device address */
  };

  union
  {
    /* Address of endpoint list in memory (host mode) */
    __rw__ uint32_t ASYNCLISTADDR;
    /* Address of endpoint list in memory (device mode) */
    __rw__ uint32_t ENDPOINTLISTADDR;
  };


  __rw__ uint32_t TTCTRL; /* Buffer status for embedded TT (host mode) */
  __rw__ uint32_t BURSTSIZE; /* Programmable burst size */
  __rw__ uint32_t TXFILLTUNING; /* Control performance tuning (host mode) */
  __ne__ uint32_t RESERVED5[2];
  __rw__ uint32_t ULPIVIEWPORT; /* ULPI viewport, only for USB1 */
  __rw__ uint32_t BINTERVAL; /* Length of virtual frame */
  __rw__ uint32_t ENDPTNAK; /* Endpoint NAK status (device mode) */
  __rw__ uint32_t ENDPTNAKEN; /* Endpoint NAK Enable (device mode) */
  __ne__ uint32_t RESERVED6;

  union
  {
    __rw__ uint32_t PORTSC1_H; /* Port 1 status/control (host mode) */
    __rw__ uint32_t PORTSC1_D; /* Port 1 status/control (device mode) */
  };

  __ne__ uint32_t RESERVED7[7];
  __rw__ uint32_t OTGSC; /* OTG Status and Control, only for USB0 */

  union
  {
    __rw__ uint32_t USBMODE_H; /* USB mode (host mode) */
    __rw__ uint32_t USBMODE_D; /* USB mode (device mode) */
  };

  __rw__ uint32_t ENDPTSETUPSTAT;
  __rw__ uint32_t ENDPTPRIME;
  __rw__ uint32_t ENDPTFLUSH;
  __ro__ uint32_t ENDPTSTAT;
  __rw__ uint32_t ENDPTCOMPLETE;

  /* Endpoint control registers */
  union
  {
    struct
    {
      __rw__ uint32_t ENDPTCTRL0;
      __rw__ uint32_t ENDPTCTRL1;
      __rw__ uint32_t ENDPTCTRL2;
      __rw__ uint32_t ENDPTCTRL3;
      __rw__ uint32_t ENDPTCTRL4;
      __rw__ uint32_t ENDPTCTRL5;
    };

    __rw__ uint32_t ENDPTCTRL[6];
  };
} LPC_USB_Type;
/*------------------Ethernet Media Access Controller--------------------------*/
typedef struct
{
  __rw__ uint32_t MAC_CONFIG;
  __rw__ uint32_t MAC_FRAME_FILTER;
  __rw__ uint32_t MAC_HASHTABLE_HIGH;
  __rw__ uint32_t MAC_HASHTABLE_LOW;
  __rw__ uint32_t MAC_MII_ADDR;
  __rw__ uint32_t MAC_MII_DATA;
  __rw__ uint32_t MAC_FLOW_CTRL;
  __rw__ uint32_t MAC_VLAN_TAG;
  __ne__ uint32_t RESERVED0;
  __ro__ uint32_t MAC_DEBUG;
  __rw__ uint32_t MAC_RWAKE_FRFLT;
  __rw__ uint32_t MAC_PMT_CTRL_STAT;
  __ne__ uint32_t RESERVED1[2];
  __rw__ uint32_t MAC_INTR_MASK;
  __rw__ uint32_t MAC_ADDR0_HIGH;
  __rw__ uint32_t MAC_ADDR0_LOW;
  __ne__ uint32_t RESERVED2[430];

  /* Offset 0x0700 */
  __rw__ uint32_t MAC_TIMESTP_CTRL;
  __rw__ uint32_t SUBSECOND_INCR;
  __ro__ uint32_t SECONDS;
  __ro__ uint32_t NANOSECONDS;
  __rw__ uint32_t SECONDSUPDATE;
  __rw__ uint32_t NANOSECONDSUPDATE;
  __rw__ uint32_t ADDEND;
  __rw__ uint32_t TARGETSECONDS;
  __rw__ uint32_t TARGETNANOSECONDS;
  __rw__ uint32_t HIGHWORD;
  __ro__ uint32_t TIMESTAMPSTAT;
  __ne__ uint32_t RESERVED3[565];

  /* Offset 0x1000 */
  __rw__ uint32_t DMA_BUS_MODE;
  __rw__ uint32_t DMA_TRANS_POLL_DEMAND;
  __rw__ uint32_t DMA_REC_POLL_DEMAND;
  __rw__ uint32_t DMA_REC_DES_ADDR;
  __rw__ uint32_t DMA_TRANS_DES_ADDR;
  __rw__ uint32_t DMA_STAT;
  __rw__ uint32_t DMA_OP_MODE;
  __rw__ uint32_t DMA_INT_EN;
  __ro__ uint32_t DMA_MFRM_BUFOF;
  __rw__ uint32_t DMA_REC_INT_WDT;
  __ne__ uint32_t RESERVED4[8];
  __ro__ uint32_t DMA_CURHOST_TRANS_DES;
  __ro__ uint32_t DMA_CURHOST_REC_DES;
  __ro__ uint32_t DMA_CURHOST_TRANS_BUF;
  __ro__ uint32_t DMA_CURHOST_REC_BUF;
} LPC_EMAC_Type;
/*------------------External Memory Controller--------------------------------*/
typedef struct
{
  __rw__ uint32_t CONTROL;
  __ro__ uint32_t STATUS;
  __rw__ uint32_t CONFIG;
  __ne__ uint32_t RESERVED0[5];

  /* Offset 0x0020 */
  __rw__ uint32_t DYNAMICCONTROL;
  __rw__ uint32_t DYNAMICREFRESH;
  __rw__ uint32_t DYNAMICREADCONFIG;
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t DYNAMICRP;
  __rw__ uint32_t DYNAMICRAS;
  __rw__ uint32_t DYNAMICSREX;
  __rw__ uint32_t DYNAMICAPR;
  __rw__ uint32_t DYNAMICDAL;
  __rw__ uint32_t DYNAMICWR;
  __rw__ uint32_t DYNAMICRC;
  __rw__ uint32_t DYNAMICRFC;
  __rw__ uint32_t DYNAMICXSR;
  __rw__ uint32_t DYNAMICRRD;
  __rw__ uint32_t DYNAMICMRD;
  __ne__ uint32_t RESERVED2[9];

  /* Offset 0x0080 */
  __rw__ uint32_t STATICEXTENDEDWAIT;
  __ne__ uint32_t RESERVED3[31];

  /* Offset 0x0100 */
  struct
  {
    __rw__ uint32_t CONFIG;
    __rw__ uint32_t RASCAS;
    __ne__ uint32_t RESERVED[6];
  } DYNAMIC[4];

  __ne__ uint32_t RESERVED4[32];

  /* Offset 0x0200 */
  struct
  {
    __rw__ uint32_t CONFIG;
    __rw__ uint32_t WAITWEN;
    __rw__ uint32_t WAITOEN;
    __rw__ uint32_t WAITRD;
    __rw__ uint32_t WAITPAG;
    __rw__ uint32_t WAITWR;
    __rw__ uint32_t WAITTURN;
    __ne__ uint32_t RESERVED;
  } STATIC[4];
} LPC_EMC_Type;
/*------------------Global Input Multiplexer Array----------------------------*/
typedef struct
{
  union
  {
    struct
    {
      __rw__ uint32_t CAP0_0_IN;
      __rw__ uint32_t CAP0_1_IN;
      __rw__ uint32_t CAP0_2_IN;
      __rw__ uint32_t CAP0_3_IN;
      __rw__ uint32_t CAP1_0_IN;
      __rw__ uint32_t CAP1_1_IN;
      __rw__ uint32_t CAP1_2_IN;
      __rw__ uint32_t CAP1_3_IN;
      __rw__ uint32_t CAP2_0_IN;
      __rw__ uint32_t CAP2_1_IN;
      __rw__ uint32_t CAP2_2_IN;
      __rw__ uint32_t CAP2_3_IN;
      __rw__ uint32_t CAP3_0_IN;
      __rw__ uint32_t CAP3_1_IN;
      __rw__ uint32_t CAP3_2_IN;
      __rw__ uint32_t CAP3_3_IN;
    };

    struct
    {
      __rw__ uint32_t IN[4];
    } CAP[4];
  };

  union
  {
    struct
    {
      __rw__ uint32_t CTIN_0_IN;
      __rw__ uint32_t CTIN_1_IN;
      __rw__ uint32_t CTIN_2_IN;
      __rw__ uint32_t CTIN_3_IN;
      __rw__ uint32_t CTIN_4_IN;
      __rw__ uint32_t CTIN_5_IN;
      __rw__ uint32_t CTIN_6_IN;
      __rw__ uint32_t CTIN_7_IN;
    };

    __rw__ uint32_t CTIN_IN[8];
  };

  __rw__ uint32_t VADC_TRIGGER_IN;
  __rw__ uint32_t EVENTROUTER_13_IN;
  __rw__ uint32_t EVENTROUTER_14_IN;
  __rw__ uint32_t EVENTROUTER_16_IN;
  __rw__ uint32_t ADCSTART0_IN;
  __rw__ uint32_t ADCSTART1_IN;
} LPC_GIMA_Type;
/*------------------EEPROM----------------------------------------------------*/
typedef struct
{
  __rw__ uint32_t CMD;
  __ro__ uint32_t RESERVED0;
  __rw__ uint32_t RWSTATE;
  __rw__ uint32_t AUTOPROG;
  __rw__ uint32_t WSTATE;
  __rw__ uint32_t CLKDIV;
  __rw__ uint32_t PWRDWN;
  __ro__ uint32_t RESERVED1[1007];
  __wo__ uint32_t INTENCLR;
  __wo__ uint32_t INTENSET;
  __ro__ uint32_t INTSTAT;
  __ro__ uint32_t INTEN;
  __wo__ uint32_t INTSTATCLR;
} LPC_EEPROM_Type;
/*------------------Event router----------------------------------------------*/
typedef struct
{
  __rw__ uint32_t HILO;
  __rw__ uint32_t EDGE;
  __ro__ uint32_t RESERVED0[1012];
  __wo__ uint32_t CLR_EN;
  __wo__ uint32_t SET_EN;
  __ro__ uint32_t STATUS;
  __ro__ uint32_t ENABLE;
  __wo__ uint32_t CLR_STAT;
  __wo__ uint32_t SET_STAT;
} LPC_EVENTROUTER_Type;
/*------------------LCD controller--------------------------------------------*/
typedef struct
{
  __rw__ uint32_t TIMH;
  __rw__ uint32_t TIMV;
  __rw__ uint32_t POL;
  __rw__ uint32_t LE;
  __rw__ uint32_t UPBASE;
  __rw__ uint32_t LPBASE;
  __rw__ uint32_t CTRL;
  __rw__ uint32_t INTMSK;
  __ro__ uint32_t INTRAW;
  __ro__ uint32_t INTSTAT;
  __wo__ uint32_t INTCLR;
  __ro__ uint32_t UPCURR;
  __ro__ uint32_t LPCURR;
  __ro__ uint32_t RESERVED0[115];
  __rw__ uint32_t PAL[256];
  __ro__ uint32_t RESERVED1[128];
  __rw__ uint32_t CRSR_IMG[256];
  __rw__ uint32_t CRSR_CTRL;
  __rw__ uint32_t CRSR_CFG;
  __rw__ uint32_t CRSR_PAL0;
  __rw__ uint32_t CRSR_PAL1;
  __rw__ uint32_t CRSR_XY;
  __rw__ uint32_t CRSR_CLIP;
  __ro__ uint32_t RESERVED2[2];
  __rw__ uint32_t CRSR_INTMSK;
  __wo__ uint32_t CRSR_INTCLR;
  __ro__ uint32_t CRSR_INTRAW;
  __ro__ uint32_t CRSR_INTSTAT;
} LPC_LCD_Type;
/*------------------SPI Flash Interface---------------------------------------*/
typedef struct
{
  __rw__ uint32_t CTRL;
  __rw__ uint32_t CMD;
  __rw__ uint32_t ADDR;
  __rw__ uint32_t IDATA;
  __rw__ uint32_t CLIMIT;
  __rw__ uint32_t DATA;
  __rw__ uint32_t MCMD;
  __rw__ uint32_t STAT;
} LPC_SPIFI_Type;
/*------------------Battery-powered Backup registers--------------------------*/
typedef struct
{
  __rw__ uint8_t DATA[256];
} LPC_BACKUP_Type;
/*------------------One-Time Programmable memory------------------------------*/
typedef struct
{
  /*
   * Contains unique part identifier for flashless parts.
   * Reserved for parts with on-chip flash.
   */
  __ro__ uint32_t BANK0[4];

  /* User-programmable memory */
  __ro__ uint32_t BANK1[4];
  __ro__ uint32_t BANK2[4];
  __ro__ uint32_t BANK3[4];
} LPC_OTP_Type;
/*----------------------------------------------------------------------------*/
/* Base addresses */
#define LPC_AHB_BASE              0x40000000UL
#define LPC_RTC_DOMAIN_BASE       0x40040000UL
#define LPC_CLK_DOMAIN_BASE       0x40050000UL
#define LPC_APB0_BASE             0x40080000UL
#define LPC_APB1_BASE             0x400A0000UL
#define LPC_APB2_BASE             0x400C0000UL
#define LPC_APB3_BASE             0x400E0000UL
#define LPC_GPIO_BASE             0x400F4000UL
#define LPC_SPI_BASE              0x40100000UL
#define LPC_SGPIO_BASE            0x40101000UL

/* External memory regions */
#define LPC_EMC_CS0_BASE          0x1C000000UL
#define LPC_EMC_CS1_BASE          0x1D000000UL
#define LPC_EMC_CS2_BASE          0x1E000000UL
#define LPC_EMC_CS3_BASE          0x1F000000UL
#define LPC_EMC_DYCS0_BASE        0x28000000UL
#define LPC_EMC_DYCS1_BASE        0x30000000UL
#define LPC_EMC_DYCS2_BASE        0x60000000UL
#define LPC_EMC_DYCS3_BASE        0x70000000UL

/* AHB peripherals */
#define LPC_SCT_BASE              (LPC_AHB_BASE + 0x00000)
#define LPC_GPDMA_BASE            (LPC_AHB_BASE + 0x02000)
#define LPC_SPIFI_BASE            (LPC_AHB_BASE + 0x03000)
#define LPC_SDMMC_BASE            (LPC_AHB_BASE + 0x04000)
#define LPC_EMC_BASE              (LPC_AHB_BASE + 0x05000)
#define LPC_USB0_BASE             (LPC_AHB_BASE + 0x06000)
#define LPC_USB1_BASE             (LPC_AHB_BASE + 0x07000)
#define LPC_LCD_BASE              (LPC_AHB_BASE + 0x08000)
#define LPC_EEPROM_BASE           (LPC_AHB_BASE + 0x0E000)
#define LPC_ETHERNET_BASE         (LPC_AHB_BASE + 0x10000)

/* APB0 peripherals */
#define LPC_WDT_BASE              (LPC_APB0_BASE + 0x0000)
#define LPC_USART0_BASE           (LPC_APB0_BASE + 0x1000)
#define LPC_UART1_BASE            (LPC_APB0_BASE + 0x2000)
#define LPC_SSP0_BASE             (LPC_APB0_BASE + 0x3000)
#define LPC_TIMER0_BASE           (LPC_APB0_BASE + 0x4000)
#define LPC_TIMER1_BASE           (LPC_APB0_BASE + 0x5000)
#define LPC_SCU_BASE              (LPC_APB0_BASE + 0x6000)
#define LPC_GPIO_INT_BASE         (LPC_APB0_BASE + 0x7000)
#define LPC_GPIO_GROUP_INT0_BASE  (LPC_APB0_BASE + 0x8000)
#define LPC_GPIO_GROUP_INT1_BASE  (LPC_APB0_BASE + 0x9000)

/* APB1 peripherals */
#define LPC_MCPWM_BASE            (LPC_APB1_BASE + 0x0000)
#define LPC_I2C0_BASE             (LPC_APB1_BASE + 0x1000)
#define LPC_I2S0_BASE             (LPC_APB1_BASE + 0x2000)
#define LPC_I2S1_BASE             (LPC_APB1_BASE + 0x3000)
#define LPC_CAN1_BASE             (LPC_APB1_BASE + 0x4000)

/* APB2 peripherals */
#define LPC_RIT_BASE              (LPC_APB2_BASE + 0x0000)
#define LPC_USART2_BASE           (LPC_APB2_BASE + 0x1000)
#define LPC_USART3_BASE           (LPC_APB2_BASE + 0x2000)
#define LPC_TIMER2_BASE           (LPC_APB2_BASE + 0x3000)
#define LPC_TIMER3_BASE           (LPC_APB2_BASE + 0x4000)
#define LPC_SSP1_BASE             (LPC_APB2_BASE + 0x5000)
#define LPC_QEI_BASE              (LPC_APB2_BASE + 0x6000)
#define LPC_GIMA_BASE             (LPC_APB2_BASE + 0x7000)

/* APB3 peripherals */
#define LPC_I2C1_BASE             (LPC_APB3_BASE + 0x0000)
#define LPC_DAC_BASE              (LPC_APB3_BASE + 0x1000)
#define LPC_CAN0_BASE             (LPC_APB3_BASE + 0x2000)
#define LPC_ADC0_BASE             (LPC_APB3_BASE + 0x3000)
#define LPC_ADC1_BASE             (LPC_APB3_BASE + 0x4000)

/* RTC domain peripherals */
#define LPC_ATIMER_BASE           (LPC_RTC_DOMAIN_BASE + 0x0000)
#define LPC_BACKUP_BASE           (LPC_RTC_DOMAIN_BASE + 0x1000)
#define LPC_PMC_BASE              (LPC_RTC_DOMAIN_BASE + 0x2000)
#define LPC_CREG_BASE             (LPC_RTC_DOMAIN_BASE + 0x3000)
#define LPC_EVENTROUTER_BASE      (LPC_RTC_DOMAIN_BASE + 0x4000)
#define LPC_OTP_BASE              (LPC_RTC_DOMAIN_BASE + 0x5000)
#define LPC_RTC_BASE              (LPC_RTC_DOMAIN_BASE + 0x6000)

/* Clocking and reset control peripherals */
#define LPC_CGU_BASE              (LPC_CLK_DOMAIN_BASE + 0x0000)
#define LPC_CCU1_BASE             (LPC_CLK_DOMAIN_BASE + 0x1000)
#define LPC_CCU2_BASE             (LPC_CLK_DOMAIN_BASE + 0x2000)
#define LPC_RGU_BASE              (LPC_CLK_DOMAIN_BASE + 0x3000)
/*----------------------------------------------------------------------------*/
/* Peripheral declaration */
#define LPC_GPIO          ((LPC_GPIO_Type *)LPC_GPIO_BASE)
#define LPC_SPI           ((LPC_SPI_Type *)LPC_SPI_BASE)
#define LPC_SGPIO         ((LPC_SGPIO_Type *)LPC_SGPIO_BASE)

#define LPC_SCT           ((LPC_SCT_Type *)LPC_SCT_BASE)
#define LPC_GPDMA         ((LPC_GPDMA_Type *)LPC_GPDMA_BASE)
#define LPC_SPIFI         ((LPC_SPIFI_Type *)LPC_SPIFI_BASE)
#define LPC_SDMMC         ((LPC_SDMMC_Type *)LPC_SDMMC_BASE)
#define LPC_EMC           ((LPC_EMC_Type *)LPC_EMC_BASE)
#define LPC_USB0          ((LPC_USB_Type *)LPC_USB0_BASE)
#define LPC_USB1          ((LPC_USB_Type *)LPC_USB1_BASE)
#define LPC_LCD           ((LPC_LCD_Type *)LPC_LCD_BASE)
#define LPC_EEPROM        ((LPC_EEPROM_Type *)LPC_EEPROM_BASE)
#define LPC_ETHERNET      ((LPC_ETHERNET_Type *)LPC_ETHERNET_BASE)

#define LPC_WDT           ((LPC_WDT_Type *)LPC_WDT_BASE)
#define LPC_USART0        ((LPC_USART_Type *)LPC_USART0_BASE)
#define LPC_UART1         ((LPC_UART_Type *)LPC_UART1_BASE)
#define LPC_SSP0          ((LPC_SSP_Type *)LPC_SSP0_BASE)
#define LPC_TIMER0        ((LPC_TIMER_Type *)LPC_TIMER0_BASE)
#define LPC_TIMER1        ((LPC_TIMER_Type *)LPC_TIMER1_BASE)
#define LPC_SCU           ((LPC_SCU_Type *)LPC_SCU_BASE)
#define LPC_GPIO_INT      ((LPC_GPIO_INT_Type *)LPC_GPIO_INT_BASE)
#define LPC_GPIO_GROUP_INT0 \
		((LPC_GPIO_GROUP_INT_Type *)LPC_GPIO_GROUP_INT0_BASE)
#define LPC_GPIO_GROUP_INT1 \
		((LPC_GPIO_GROUP_INT_Type *)LPC_GPIO_GROUP_INT1_BASE)

#define LPC_MCPWM         ((LPC_MCPWM_Type *)LPC_MCPWM_BASE)
#define LPC_I2C0          ((LPC_I2C_Type *)LPC_I2C0_BASE)
#define LPC_I2S0          ((LPC_I2S_Type *)LPC_I2S0_BASE)
#define LPC_I2S1          ((LPC_I2S_Type *)LPC_I2S1_BASE)
#define LPC_CAN1          ((LPC_CAN_Type *)LPC_CAN1_BASE)

#define LPC_RIT           ((LPC_RIT_Type *)LPC_RIT_BASE)
#define LPC_USART2        ((LPC_USART_Type *)LPC_USART2_BASE)
#define LPC_USART3        ((LPC_USART_Type *)LPC_USART3_BASE)
#define LPC_TIMER2        ((LPC_TIMER_Type *)LPC_TIMER2_BASE)
#define LPC_TIMER3        ((LPC_TIMER_Type *)LPC_TIMER3_BASE)
#define LPC_SSP1          ((LPC_SSP_Type *)LPC_SSP1_BASE)
#define LPC_QEI           ((LPC_QEI_Type *)LPC_QEI_BASE)
#define LPC_GIMA          ((LPC_GIMA_Type *)LPC_GIMA_BASE)

#define LPC_I2C1          ((LPC_I2C_Type *)LPC_I2C1_BASE)
#define LPC_DAC           ((LPC_DAC_Type *)LPC_DAC_BASE)
#define LPC_CAN0          ((LPC_CAN_Type *)LPC_CAN0_BASE)
#define LPC_ADC0          ((LPC_ADC_Type *)LPC_ADC0_BASE)
#define LPC_ADC1          ((LPC_ADC_Type *)LPC_ADC1_BASE)

#define LPC_ATIMER        ((LPC_ATIMER_Type *)LPC_ATIMER_BASE)
#define LPC_BACKUP        ((LPC_BACKUP_Type *)LPC_BACKUP_BASE)
#define LPC_PMC           ((LPC_PMC_Type *)LPC_PMC_BASE)
#define LPC_CREG          ((LPC_CREG_Type *)LPC_CREG_BASE)
#define LPC_EVENTROUTER   ((LPC_EVENTROUTER_Type *)LPC_EVENTROUTER_BASE)
#define LPC_OTP           ((LPC_OTP_Type *)LPC_OTP_BASE)
#define LPC_RTC           ((LPC_RTC_Type *)LPC_RTC_BASE)

#define LPC_CGU           ((LPC_CGU_Type *)LPC_CGU_BASE)
#define LPC_CCU1          ((LPC_CCU1_Type *)LPC_CCU1_BASE)
#define LPC_CCU2          ((LPC_CCU2_Type *)LPC_CCU2_BASE)
#define LPC_RGU           ((LPC_RGU_Type *)LPC_RGU_BASE)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC43XX_PLATFORM_DEFS_H_ */
