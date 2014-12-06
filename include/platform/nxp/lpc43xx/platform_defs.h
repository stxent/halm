/*
 * platform/nxp/lpc43xx/platform_defs.h
 * Based on original by NXP
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_LPC43XX_PLATFORM_DEFS_H_
#define PLATFORM_NXP_LPC43XX_PLATFORM_DEFS_H_
/*----------------------------------------------------------------------------*/
#define NVIC_PRIORITY_SIZE 3
/*------------------Clock Generation Unit-------------------------------------*/
typedef struct
{
  __ne__ uint32_t RESERVED0[5];
  __rw__ uint32_t FREQ_MON;
  __rw__ uint32_t XTAL_OSC_CTRL;
  __ro__ uint32_t PLL0USB_STAT;

  /* Offset 0x020 */
  __rw__ uint32_t PLL0USB_CTRL;
  __rw__ uint32_t PLL0USB_MDIV;
  __rw__ uint32_t PLL0USB_NP_DIV;
  __ro__ uint32_t PLL0AUDIO_STAT;
  __rw__ uint32_t PLL0AUDIO_CTRL;
  __rw__ uint32_t PLL0AUDIO_MDIV;
  __rw__ uint32_t PLL0AUDIO_NP_DIV;
  __rw__ uint32_t PLL0AUDIO_FRAC;

  /* Offset 0x040 */
  __ro__ uint32_t PLL1_STAT;
  __rw__ uint32_t PLL1_CTRL;
  __rw__ uint32_t IDIVA_CTRL;
  __rw__ uint32_t IDIVB_CTRL;
  __rw__ uint32_t IDIVC_CTRL;
  __rw__ uint32_t IDIVD_CTRL;
  __rw__ uint32_t IDIVE_CTRL;
  __ro__ uint32_t BASE_SAFE_CLK;

  /* Offset 0x060 */
  __rw__ uint32_t BASE_USB0_CLK;
  __rw__ uint32_t BASE_PERIPH_CLK;
  __rw__ uint32_t BASE_USB1_CLK;
  __rw__ uint32_t BASE_M4_CLK;
  __rw__ uint32_t BASE_SPIFI_CLK;
  __rw__ uint32_t BASE_SPI_CLK;
  __rw__ uint32_t BASE_PHY_RX_CLK;
  __rw__ uint32_t BASE_PHY_TX_CLK;

  /* Offset 0x080 */
  __rw__ uint32_t BASE_APB1_CLK;
  __rw__ uint32_t BASE_APB3_CLK;
  __rw__ uint32_t BASE_LCD_CLK;
  __rw__ uint32_t BASE_ADCHS_CLK;
  __rw__ uint32_t BASE_SDIO_CLK;
  __rw__ uint32_t BASE_SSP0_CLK;
  __rw__ uint32_t BASE_SSP1_CLK;
  __rw__ uint32_t BASE_UART0_CLK;

  /* Offset 0x0A0 */
  __rw__ uint32_t BASE_UART1_CLK;
  __rw__ uint32_t BASE_UART2_CLK;
  __rw__ uint32_t BASE_UART3_CLK;
  __rw__ uint32_t BASE_OUT_CLK;
  __ne__ uint32_t RESERVED1[4];

  /* Offset 0x0C0 */
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

  /* Offset 0x100 */
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

  /* Offset 0x200 */
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

  /* Offset 0x300 */
  __rw__ uint32_t CLK_SPIFI_CFG;
  __ro__ uint32_t CLK_SPIFI_STAT;
  __ne__ uint32_t RESERVED3[62];

  /* Offset 0x400 */
  __rw__ uint32_t CLK_M4_BUS_CFG;
  __ro__ uint32_t CLK_M4_BUS_STAT;
  __rw__ uint32_t CLK_M4_SPIFI_CFG;
  __ro__ uint32_t CLK_M4_SPIFI_STAT;
  __rw__ uint32_t CLK_M4_GPIO_CFG;
  __ro__ uint32_t CLK_M4_GPIO_STAT;
  __rw__ uint32_t CLK_M4_LCD_CFG;
  __ro__ uint32_t CLK_M4_LCD_STAT;

  /* Offset 0x420 */
  __rw__ uint32_t CLK_M4_ETHERNET_CFG;
  __ro__ uint32_t CLK_M4_ETHERNET_STAT;
  __rw__ uint32_t CLK_M4_USB0_CFG;
  __ro__ uint32_t CLK_M4_USB0_STAT;
  __rw__ uint32_t CLK_M4_EMC_CFG;
  __ro__ uint32_t CLK_M4_EMC_STAT;
  __rw__ uint32_t CLK_M4_SDIO_CFG;
  __ro__ uint32_t CLK_M4_SDIO_STAT;

  /* Offset 0x440 */
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

  /* Offset 0x480 */
  __rw__ uint32_t CLK_M4_FLASHA_CFG;
  __ro__ uint32_t CLK_M4_FLASHA_STAT;
  __rw__ uint32_t CLK_M4_FLASHB_CFG;
  __ro__ uint32_t CLK_M4_FLASHB_STAT;
  __rw__ uint32_t CLK_M4_M0APP_CFG;
  __ro__ uint32_t CLK_M4_M0APP_STAT;
  __rw__ uint32_t CLK_M4_ADCHS_CFG;
  __ro__ uint32_t CLK_M4_ADCHS_STAT;

  /* Offset 0x4A0 */
  __rw__ uint32_t CLK_M4_EEPROM_CFG;
  __ro__ uint32_t CLK_M4_EEPROM_STAT;
  __ne__ uint32_t RESERVED5[22];

  /* Offset 0x500 */
  __rw__ uint32_t CLK_M4_WWDT_CFG;
  __ro__ uint32_t CLK_M4_WWDT_STAT;
  __rw__ uint32_t CLK_M4_USART0_CFG;
  __ro__ uint32_t CLK_M4_USART0_STAT;
  __rw__ uint32_t CLK_M4_UART1_CFG;
  __ro__ uint32_t CLK_M4_UART1_STAT;
  __rw__ uint32_t CLK_M4_SSP0_CFG;
  __ro__ uint32_t CLK_M4_SSP0_STAT;

  /* Offset 0x520 */
  __rw__ uint32_t CLK_M4_TIMER0_CFG;
  __ro__ uint32_t CLK_M4_TIMER0_STAT;
  __rw__ uint32_t CLK_M4_TIMER1_CFG;
  __ro__ uint32_t CLK_M4_TIMER1_STAT;
  __rw__ uint32_t CLK_M4_SCU_CFG;
  __ro__ uint32_t CLK_M4_SCU_STAT;
  __rw__ uint32_t CLK_M4_CREG_CFG;
  __ro__ uint32_t CLK_M4_CREG_STAT;
  __ne__ uint32_t RESERVED6[48];

  /* Offset 0x600 */
  __rw__ uint32_t CLK_M4_RIT_CFG;
  __ro__ uint32_t CLK_M4_RIT_STAT;
  __rw__ uint32_t CLK_M4_USART2_CFG;
  __ro__ uint32_t CLK_M4_USART2_STAT;
  __rw__ uint32_t CLK_M4_USART3_CFG;
  __ro__ uint32_t CLK_M4_USART3_STAT;
  __rw__ uint32_t CLK_M4_TIMER2_CFG;
  __ro__ uint32_t CLK_M4_TIMER2_STAT;

  /* Offset 0x620 */
  __rw__ uint32_t CLK_M4_TIMER3_CFG;
  __ro__ uint32_t CLK_M4_TIMER3_STAT;
  __rw__ uint32_t CLK_M4_SSP1_CFG;
  __ro__ uint32_t CLK_M4_SSP1_STAT;
  __rw__ uint32_t CLK_M4_QEI_CFG;
  __ro__ uint32_t CLK_M4_QEI_STAT;
  __ne__ uint32_t RESERVED7[50];

  /* Offset 0x700 */
  __rw__ uint32_t CLK_PERIPH_BUS_CFG;
  __ro__ uint32_t CLK_PERIPH_BUS_STAT;
  __ne__ uint32_t RESERVED8[2];
  __rw__ uint32_t CLK_PERIPH_CORE_CFG;
  __ro__ uint32_t CLK_PERIPH_CORE_STAT;
  __rw__ uint32_t CLK_PERIPH_SGPIO_CFG;
  __ro__ uint32_t CLK_PERIPH_SGPIO_STAT;
  __ne__ uint32_t RESERVED9[56];

  /* Offset 0x800 */
  __rw__ uint32_t CLK_USB0_CFG;
  __ro__ uint32_t CLK_USB0_STAT;
  __ne__ uint32_t RESERVED10[62];

  /* Offset 0x900 */
  __rw__ uint32_t CLK_USB1_CFG;
  __ro__ uint32_t CLK_USB1_STAT;
  __ne__ uint32_t RESERVED11[62];

  /* Offset 0xA00 */
  __rw__ uint32_t CLK_SPI_CFG;
  __ro__ uint32_t CLK_SPI_STAT;
  __ne__ uint32_t RESERVED12[62];

  /* Offset 0xB00 */
  __rw__ uint32_t CLK_ADCHS_CFG;
  __ro__ uint32_t CLK_ADCHS_STAT;
} LPC_CCU1_Type;
/*------------------Clock Control Unit 2--------------------------------------*/
typedef struct
{
  __rw__ uint32_t PM;
  __ro__ uint32_t BASE_STAT;
  __ne__ uint32_t RESERVED0[62];

  /* Offset 0x100 */
  __rw__ uint32_t CLK_AUDIO_CFG;
  __ro__ uint32_t CLK_AUDIO_STAT;
  __ne__ uint32_t RESERVED1[62];

  /* Offset 0x200 */
  __rw__ uint32_t CLK_APB2_USART3_CFG;
  __ro__ uint32_t CLK_APB2_USART3_STAT;
  __ne__ uint32_t RESERVED2[62];

  /* Offset 0x300 */
  __rw__ uint32_t CLK_APB2_USART2_CFG;
  __ro__ uint32_t CLK_APB2_USART2_STAT;
  __ne__ uint32_t RESERVED3[62];

  /* Offset 0x400 */
  __rw__ uint32_t CLK_APB0_UART1_CFG;
  __ro__ uint32_t CLK_APB0_UART1_STAT;
  __ne__ uint32_t RESERVED4[62];

  /* Offset 0x500 */
  __rw__ uint32_t CLK_APB0_USART0_CFG;
  __ro__ uint32_t CLK_APB0_USART0_STAT;
  __ne__ uint32_t RESERVED5[62];

  /* Offset 0x600 */
  __rw__ uint32_t CLK_APB2_SSP1_CFG;
  __ro__ uint32_t CLK_APB2_SSP1_STAT;
  __ne__ uint32_t RESERVED6[62];

  /* Offset 0x700 */
  __rw__ uint32_t CLK_APB0_SSP0_CFG;
  __ro__ uint32_t CLK_APB0_SSP0_STAT;
  __ne__ uint32_t RESERVED7[62];

  /* Offset 0x800 */
  __rw__ uint32_t CLK_SDIO_CFG;
  __ro__ uint32_t CLK_SDIO_STAT;
} LPC_CCU2_Type;
/*------------------Configuration register------------------------------------*/
typedef struct
{
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t CREG0;
  __ne__ uint32_t RESERVED1[62];

  /* Offset 0x100 */
  __rw__ uint32_t M4MEMMAP;
  __ne__ uint32_t RESERVED2[5];
  __rw__ uint32_t CREG5;
  __rw__ uint32_t DMAMUX;

  /* Offset 0x120 */
  __rw__ uint32_t FLASHCFGA;
  __rw__ uint32_t FLASHCFGB;
  __rw__ uint32_t ETBCFG;
  __rw__ uint32_t CREG6;
  __rw__ uint32_t M4TXEVENT;
  __ne__ uint32_t RESERVED3[51];

  /* Offset 0x200 */
  __ro__ uint32_t CHIPID;
  __ne__ uint32_t RESERVED4[65];
  __rw__ uint32_t M0SUBMEMMAP;
  __ne__ uint32_t RESERVED5;
  __rw__ uint32_t M0SUBTXEVENT;
  __ne__ uint32_t RESERVED6[58];

  /* Offset 0x400 */
  __rw__ uint32_t M0APPTXEVENT;
  __rw__ uint32_t M0APPMEMMAP;
  __ne__ uint32_t RESERVED7[62];

  /* Offset 0x500 */
  __rw__ uint32_t USB0FLADJ;
  __ne__ uint32_t RESERVED8[63];

  /* Offset 0x600 */
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

  /* Offset 0x080 */
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

  /* Offset 0x100 */
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

  /* Offset 0x180 */
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

  /* Offset 0x200 */
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

  /* Offset 0x280 */
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

  /* Offset 0x300 */
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

  /* Offset 0x380 */
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

  /* Offset 0x400 */
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

  /* Offset 0x480 */
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

  /* Offset 0x500 */
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

  /* Offset 0x580 */
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

  /* Offset 0x600 */
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

  /* Offset 0x680 */
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

  /* Offset 0x700 */
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

  /* Offset 0x780 */
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

  /* Offset 0xC00 */
  __rw__ uint32_t SFSPCLK0;
  __rw__ uint32_t SFSPCLK1;
  __rw__ uint32_t SFSPCLK2;
  __rw__ uint32_t SFSPCLK3;
  __ne__ uint32_t RESERVED17[28];

  /* Offset 0xC80 */
  __rw__ uint32_t SFSUSB;
  __rw__ uint32_t SFSI2C0;
  __rw__ uint32_t ENAIO0;
  __rw__ uint32_t ENAIO1;
  __rw__ uint32_t ENAIO2;
  __ne__ uint32_t RESERVED18[27];

  /* Offset 0xD00 */
  __rw__ uint32_t EMCDELAYCLK;
  __ne__ uint32_t RESERVED19[31];

  /* Offset 0xD80 */
  __rw__ uint32_t SDDELAY;
  __ne__ uint32_t RESERVED20[31];

  /* Offset 0xE00 */
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
  /* Offset 0x040 */
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
  /* Offset 0x100 */
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
///*------------------Inter IC Sound--------------------------------------------*/
//typedef struct
//{
//  __rw__ uint32_t I2SDAO;
//  __rw__ uint32_t I2SDAI;
//  __wo__ uint32_t I2STXFIFO;
//  __ro__ uint32_t I2SRXFIFO;
//  __ro__ uint32_t I2SSTATE;
//  __rw__ uint32_t I2SDMA1;
//  __rw__ uint32_t I2SDMA2;
//  __rw__ uint32_t I2SIRQ;
//  __rw__ uint32_t I2STXRATE;
//  __rw__ uint32_t I2SRXRATE;
//  __rw__ uint32_t I2STXBITRATE;
//  __rw__ uint32_t I2SRXBITRATE;
//  __rw__ uint32_t I2STXMODE;
//  __rw__ uint32_t I2SRXMODE;
//} LPC_I2S_Type;
///*------------------Repetitive Interrupt Timer--------------------------------*/
//typedef struct
//{
//  __rw__ uint32_t RICOMPVAL;
//  __rw__ uint32_t RIMASK;
//  __rw__ uint32_t RICTRL;
//  __rw__ uint32_t RICOUNTER;
//} LPC_RIT_Type;
///*------------------Real-Time Clock-------------------------------------------*/
//typedef struct
//{
//  /* Miscellaneous registers, offset 0x00 */
//  __rw__ uint32_t ILR;
//  __ne__ uint32_t RESERVED0;
//  __rw__ uint32_t CCR;
//  __rw__ uint32_t CIIR;
//  __rw__ uint32_t AMR;
//
//  /* Consolidated time registers, offset 0x14 */
//  __ro__ uint32_t CTIME0;
//  __ro__ uint32_t CTIME1;
//  __ro__ uint32_t CTIME2;
//
//  /* Time counter registers, offset 0x20 */
//  __rw__ uint32_t SEC;
//  __rw__ uint32_t MIN;
//  __rw__ uint32_t HOUR;
//  __rw__ uint32_t DOM;
//  __rw__ uint32_t DOW;
//  __rw__ uint32_t DOY;
//  __rw__ uint32_t MONTH;
//  __rw__ uint32_t YEAR;
//  __rw__ uint32_t CALIBRATION;
//
//  /* General purpose registers, offset 0x44 */
//  __rw__ uint32_t GPREG0;
//  __rw__ uint32_t GPREG1;
//  __rw__ uint32_t GPREG2;
//  __rw__ uint32_t GPREG3;
//  __rw__ uint32_t GPREG4;
//
//  /* Miscellaneous registers, offset 0x5C */
//  __rw__ uint32_t RTC_AUXEN;
//  __rw__ uint32_t RTC_AUX;
//
//  /* Alarm register group, offset 0x60 */
//  __rw__ uint32_t ALSEC;
//  __rw__ uint32_t ALMIN;
//  __rw__ uint32_t ALHOUR;
//  __rw__ uint32_t ALDOM;
//  __rw__ uint32_t ALDOW;
//  __rw__ uint32_t ALDOY;
//  __rw__ uint32_t ALMON;
//  __rw__ uint32_t ALYEAR;
//} LPC_RTC_Type;
///*------------------Digital-to-Analog Converter-------------------------------*/
//typedef struct
//{
//  __rw__ uint32_t CR; /* Converter Register */
//  __rw__ uint32_t CTRL; /* Control register */
//  __rw__ uint16_t CNTVAL; /* Counter Value register */
//} LPC_DAC_Type;
///*------------------Watchdog Timer--------------------------------------------*/
//typedef struct
//{
//  __rw__ uint32_t MOD;
//  __rw__ uint32_t TC;
//  __wo__ uint32_t FEED;
//  __ro__ uint32_t TV;
//  __rw__ uint32_t WDCLKSEL;
//} LPC_WDT_Type;
///*------------------Motor Control Pulse-Width Modulation----------------------*/
//typedef struct
//{
//  __ro__ uint32_t MCCON;
//  __wo__ uint32_t MCCON_SET;
//  __wo__ uint32_t MCCON_CLR;
//  __ro__ uint32_t MCCAPCON;
//  __wo__ uint32_t MCCAPCON_SET;
//  __wo__ uint32_t MCCAPCON_CLR;
//  __rw__ uint32_t MCTIM0;
//  __rw__ uint32_t MCTIM1;
//  __rw__ uint32_t MCTIM2;
//  __rw__ uint32_t MCPER0;
//  __rw__ uint32_t MCPER1;
//  __rw__ uint32_t MCPER2;
//  __rw__ uint32_t MCPW0;
//  __rw__ uint32_t MCPW1;
//  __rw__ uint32_t MCPW2;
//  __rw__ uint32_t MCDEADTIME;
//  __rw__ uint32_t MCCCP;
//  __rw__ uint32_t MCCR0;
//  __rw__ uint32_t MCCR1;
//  __rw__ uint32_t MCCR2;
//  __ro__ uint32_t MCINTEN;
//  __wo__ uint32_t MCINTEN_SET;
//  __wo__ uint32_t MCINTEN_CLR;
//  __ro__ uint32_t MCCNTCON;
//  __wo__ uint32_t MCCNTCON_SET;
//  __wo__ uint32_t MCCNTCON_CLR;
//  __ro__ uint32_t MCINTFLAG;
//  __wo__ uint32_t MCINTFLAG_SET;
//  __wo__ uint32_t MCINTFLAG_CLR;
//  __wo__ uint32_t MCCAP_CLR;
//} LPC_MCPWM_Type;
///*------------------Quadrature Encoder Interface------------------------------*/
//typedef struct
//{
//  __wo__ uint32_t QEICON;
//  __ro__ uint32_t QEISTAT;
//  __rw__ uint32_t QEICONF;
//  __ro__ uint32_t QEIPOS;
//  __rw__ uint32_t QEIMAXPOS;
//  __rw__ uint32_t CMPOS0;
//  __rw__ uint32_t CMPOS1;
//  __rw__ uint32_t CMPOS2;
//  __ro__ uint32_t INXCNT;
//  __rw__ uint32_t INXCMP;
//  __rw__ uint32_t QEILOAD;
//  __ro__ uint32_t QEITIME;
//  __ro__ uint32_t QEIVEL;
//  __ro__ uint32_t QEICAP;
//  __rw__ uint32_t VELCOMP;
//  __rw__ uint32_t FILTER;
//  __ne__ uint32_t RESERVED0[998];
//  __wo__ uint32_t QEIIEC;
//  __wo__ uint32_t QEIIES;
//  __ro__ uint32_t QEIINTSTAT;
//  __ro__ uint32_t QEIIE;
//  __wo__ uint32_t QEICLR;
//  __wo__ uint32_t QEISET;
//} LPC_QEI_Type;
///*------------------Controller Area Network-----------------------------------*/
///* Identifier masks */
//typedef struct
//{
//  __rw__ uint32_t MASK[512];
//} LPC_CANAF_RAM_Type;
//
///* Acceptance Filter registers */
//typedef struct
//{
//  __rw__ uint32_t AFMR;
//  __rw__ uint32_t SFF_SA;
//  __rw__ uint32_t SFF_GRP_SA;
//  __rw__ uint32_t EFF_SA;
//  __rw__ uint32_t EFF_GRP_SA;
//  __rw__ uint32_t ENDOFTABLE;
//  __ro__ uint32_t LUTERRAD;
//  __ro__ uint32_t LUTERR;
//  __rw__ uint32_t FCANIE;
//  __rw__ uint32_t FCANIC0;
//  __rw__ uint32_t FCANIC1;
//} LPC_CANAF_Type;
//
///* Central registers */
//typedef struct
//{
//  __ro__ uint32_t TXSR;
//  __ro__ uint32_t RXSR;
//  __ro__ uint32_t MSR;
//} LPC_CANCR_Type;
//
///* Controller registers */
//typedef struct
//{
//  __rw__ uint32_t MOD;
//  __wo__ uint32_t CMR;
//  __rw__ uint32_t GSR;
//  __ro__ uint32_t ICR;
//  __rw__ uint32_t IER;
//  __rw__ uint32_t BTR;
//  __rw__ uint32_t EWL;
//  __ro__ uint32_t SR;
//  __rw__ uint32_t RFS;
//  __rw__ uint32_t RID;
//  __rw__ uint32_t RDA;
//  __rw__ uint32_t RDB;
//  __rw__ uint32_t TFI1;
//  __rw__ uint32_t TID1;
//  __rw__ uint32_t TDA1;
//  __rw__ uint32_t TDB1;
//  __rw__ uint32_t TFI2;
//  __rw__ uint32_t TID2;
//  __rw__ uint32_t TDA2;
//  __rw__ uint32_t TDB2;
//  __rw__ uint32_t TFI3;
//  __rw__ uint32_t TID3;
//  __rw__ uint32_t TDA3;
//  __rw__ uint32_t TDB3;
//} LPC_CAN_Type;
/*------------------SD/MMC card interface-------------------------------------*/
typedef struct
{
  /* Offset 0x000 */
  __rw__ uint32_t CTRL;
  __rw__ uint32_t PWREN;
  __rw__ uint32_t CLKDIV;
  __rw__ uint32_t CLKSRC;
  __rw__ uint32_t CLKENA;
  __rw__ uint32_t TMOUT;
  __rw__ uint32_t CTYPE;
  __rw__ uint32_t BLKSIZ;

  /* Offset 0x020 */
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

  /* Offset 0x040 */
  __ro__ uint32_t MINTSTS;
  __rw__ uint32_t RINTSTS;
  __ro__ uint32_t STATUS;
  __rw__ uint32_t FIFOTH;
  __ro__ uint32_t CDETECT;
  __ro__ uint32_t WRTPRT;
  __ne__ uint32_t RESERVED0;
  __ro__ uint32_t TCBCNT;

  /* Offset 0x060 */
  __ro__ uint32_t TBBCNT;
  __rw__ uint32_t DEBNCE;
  __ne__ uint32_t RESERVED1[4];
  __rw__ uint32_t RST_N;
  __ne__ uint32_t RESERVED2;

  /* Offset 0x080 */
  __rw__ uint32_t BMOD;
  __wo__ uint32_t PLDMND;
  __rw__ uint32_t DBADDR;
  __rw__ uint32_t IDSTS;
  __rw__ uint32_t IDINTEN;
  __ro__ uint32_t DSCADDR;
  __ro__ uint32_t BUFADDR;
  __ne__ uint32_t RESERVED3;

  /* Offset 0x100 */
  __rw__ uint32_t DATA[];
} LPC_SDMMC_Type;
/*------------------General Purpose Direct Memory Access----------------------*/
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
} LPC_GPDMA_Type;

/* Channel registers */
typedef struct
{
  __rw__ uint32_t SRCADDR;
  __rw__ uint32_t DESTADDR;
  __rw__ uint32_t LLI;
  __rw__ uint32_t CONTROL;
  __rw__ uint32_t CONFIG;
} LPC_GPDMACH_Type;
///*------------------Universal Serial Bus--------------------------------------*/
//typedef struct
//{
//  /* USB Host registers */
//  __ro__ uint32_t HcRevision;
//  __rw__ uint32_t HcControl;
//  __rw__ uint32_t HcCommandStatus;
//  __rw__ uint32_t HcInterruptStatus;
//  __rw__ uint32_t HcInterruptEnable;
//  __rw__ uint32_t HcInterruptDisable;
//  __rw__ uint32_t HcHCCA;
//  __ro__ uint32_t HcPeriodCurrentED;
//  __rw__ uint32_t HcControlHeadED;
//  __rw__ uint32_t HcControlCurrentED;
//  __rw__ uint32_t HcBulkHeadED;
//  __rw__ uint32_t HcBulkCurrentED;
//  __ro__ uint32_t HcDoneHead;
//  __rw__ uint32_t HcFmInterval;
//  __ro__ uint32_t HcFmRemaining;
//  __ro__ uint32_t HcFmNumber;
//  __rw__ uint32_t HcPeriodicStart;
//  __rw__ uint32_t HcLSTreshold;
//  __rw__ uint32_t HcRhDescriptorA;
//  __rw__ uint32_t HcRhDescriptorB;
//  __rw__ uint32_t HcRhStatus;
//  __rw__ uint32_t HcRhPortStatus1;
//  __rw__ uint32_t HcRhPortStatus2;
//  __ne__ uint32_t RESERVED0[40];
//  __ro__ uint32_t Module_ID;
//
//  /* USB On-The-Go registers */
//  __ro__ uint32_t OTGIntSt;
//  __rw__ uint32_t OTGIntEn;
//  __wo__ uint32_t OTGIntSet;
//  __wo__ uint32_t OTGIntClr;
//  __rw__ uint32_t OTGStCtrl;
//  __rw__ uint32_t OTGTmr;
//  __ne__ uint32_t RESERVED1[58];
//
//  /* USB Device Interrupt registers */
//  __ro__ uint32_t USBDevIntSt;
//  __rw__ uint32_t USBDevIntEn;
//  __wo__ uint32_t USBDevIntClr;
//  __wo__ uint32_t USBDevIntSet;
//
//  /* USB Device SIE Command registers */
//  __wo__ uint32_t USBCmdCode;
//  __ro__ uint32_t USBCmdData;
//
//  /* USB Device Transfer registers */
//  __ro__ uint32_t USBRxData;
//  __wo__ uint32_t USBTxData;
//  __ro__ uint32_t USBRxPLen;
//  __wo__ uint32_t USBTxPLen;
//  __rw__ uint32_t USBCtrl;
//  __wo__ uint32_t USBDevIntPri;
//
//  /* USB Device Endpoint Interrupt registers */
//  __ro__ uint32_t USBEpIntSt;
//  __rw__ uint32_t USBEpIntEn;
//  __wo__ uint32_t USBEpIntClr;
//  __wo__ uint32_t USBEpIntSet;
//  __wo__ uint32_t USBEpIntPri;
//
//  /* USB Device Endpoint Realization registers */
//  __rw__ uint32_t USBReEp;
//  __wo__ uint32_t USBEpInd;
//  __rw__ uint32_t USBMaxPSize;
//
//  /* USB Device DMA registers */
//  __ro__ uint32_t USBDMARSt;
//  __wo__ uint32_t USBDMARClr;
//  __wo__ uint32_t USBDMARSet;
//  __ne__ uint32_t RESERVED2[9];
//  __rw__ uint32_t USBUDCAH;
//  __ro__ uint32_t USBEpDMASt;
//  __wo__ uint32_t USBEpDMAEn;
//  __wo__ uint32_t USBEpDMADis;
//  __ro__ uint32_t USBDMAIntSt;
//  __rw__ uint32_t USBDMAIntEn;
//  __ne__ uint32_t RESERVED3[2];
//  __ro__ uint32_t USBEoTIntSt;
//  __wo__ uint32_t USBEoTIntClr;
//  __wo__ uint32_t USBEoTIntSet;
//  __ro__ uint32_t USBNDDRIntSt;
//  __wo__ uint32_t USBNDDRIntClr;
//  __wo__ uint32_t USBNDDRIntSet;
//  __ro__ uint32_t USBSysErrIntSt;
//  __wo__ uint32_t USBSysErrIntClr;
//  __wo__ uint32_t USBSysErrIntSet;
//  __ne__ uint32_t RESERVED4[15];
//
//  /* USB OTG I2C registers */
//  union
//  {
//    __ro__ uint32_t I2C_RX;
//    __wo__ uint32_t I2C_TX;
//  };
//  __ro__ uint32_t I2C_STS;
//  __rw__ uint32_t I2C_CTL;
//  __rw__ uint32_t I2C_CLKHI;
//  __wo__ uint32_t I2C_CLKLO;
//  __ne__ uint32_t RESERVED5[824];
//
//  /* USB Clock Control registers */
//  union
//  {
//    __rw__ uint32_t USBClkCtrl;
//    __rw__ uint32_t OTGClkCtrl;
//  };
//  union
//  {
//    __ro__ uint32_t USBClkSt;
//    __ro__ uint32_t OTGClkSt;
//  };
//} LPC_USB_Type;
///*------------------Ethernet Media Access Controller--------------------------*/
//typedef struct
//{
//  /* MAC registers */
//  __rw__ uint32_t MAC1;
//  __rw__ uint32_t MAC2;
//  __rw__ uint32_t IPGT;
//  __rw__ uint32_t IPGR;
//  __rw__ uint32_t CLRT;
//  __rw__ uint32_t MAXF;
//  __rw__ uint32_t SUPP;
//  __rw__ uint32_t TEST;
//  __rw__ uint32_t MCFG;
//  __rw__ uint32_t MCMD;
//  __rw__ uint32_t MADR;
//  __wo__ uint32_t MWTD;
//  __ro__ uint32_t MRDD;
//  __ro__ uint32_t MIND;
//  __ne__ uint32_t RESERVED0[2];
//  __rw__ uint32_t SA0;
//  __rw__ uint32_t SA1;
//  __rw__ uint32_t SA2;
//  __ne__ uint32_t RESERVED1[45];
//
//  /* Control registers */
//  __rw__ uint32_t Command;
//  __ro__ uint32_t Status;
//  __rw__ uint32_t RxDescriptor;
//  __rw__ uint32_t RxStatus;
//  __rw__ uint32_t RxDescriptorNumber;
//  __ro__ uint32_t RxProduceIndex;
//  __rw__ uint32_t RxConsumeIndex;
//  __rw__ uint32_t TxDescriptor;
//  __rw__ uint32_t TxStatus;
//  __rw__ uint32_t TxDescriptorNumber;
//  __rw__ uint32_t TxProduceIndex;
//  __ro__ uint32_t TxConsumeIndex;
//  __ne__ uint32_t RESERVED2[10];
//  __ro__ uint32_t TSV0;
//  __ro__ uint32_t TSV1;
//  __ro__ uint32_t RSV;
//  __ne__ uint32_t RESERVED3[3];
//  __rw__ uint32_t FlowControlCounter;
//  __ro__ uint32_t FlowControlStatus;
//  __ne__ uint32_t RESERVED4[34];
//
//  /* Rx Filter registers */
//  __rw__ uint32_t RxFilterCtrl;
//  __rw__ uint32_t RxFilterWoLStatus;
//  __rw__ uint32_t RxFilterWoLClear;
//  __ne__ uint32_t RESERVED5;
//  __rw__ uint32_t HashFilterL;
//  __rw__ uint32_t HashFilterH;
//  __ne__ uint32_t RESERVED6[882];
//
//  /* Module Control registers */
//  __ro__ uint32_t IntStatus;
//  __rw__ uint32_t IntEnable;
//  __wo__ uint32_t IntClear;
//  __wo__ uint32_t IntSet;
//  __ne__ uint32_t RESERVED7;
//  __rw__ uint32_t PowerDown;
//  __ne__ uint32_t RESERVED8;
//  __rw__ uint32_t Module_ID;
//} LPC_EMAC_Type;
/*----------------------------------------------------------------------------*/
/* Base addresses */
#define LPC_AHB_BASE              (0x40000000UL)
#define LPC_RTC_DOMAIN_BASE       (0x40040000UL)
#define LPC_CLK_DOMAIN_BASE       (0x40050000UL)
#define LPC_APB0_BASE             (0x40080000UL)
#define LPC_APB1_BASE             (0x400A0000UL)
#define LPC_APB2_BASE             (0x400C0000UL)
#define LPC_APB3_BASE             (0x400E0000UL)
#define LPC_GPIO_BASE             (0x400F4000UL)
#define LPC_SPI_BASE              (0x40100000UL)
#define LPC_SGPIO_BASE            (0x40101000UL)

/* AHB peripherals */
#define LPC_SCT_BASE              (LPC_AHB_BASE + 0x00000)
#define LPC_GPDMA_BASE            (LPC_AHB_BASE + 0x02000)
#define LPC_SPIFI_BASE            (LPC_AHB_BASE + 0x03000)
#define LPC_SDMMC_BASE            (LPC_AHB_BASE + 0x04000)
#define LPC_EMC_BASE              (LPC_AHB_BASE + 0x05000)
#define LPC_USB0_BASE             (LPC_AHB_BASE + 0x06000)
#define LPC_USB1_BASE             (LPC_AHB_BASE + 0x07000)
#define LPC_LCD_BASE              (LPC_AHB_BASE + 0x08000)
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

/* GPDMA channels */
#define LPC_GPDMACH0_BASE         (LPC_GPDMA_BASE + 0x0100)
#define LPC_GPDMACH1_BASE         (LPC_GPDMA_BASE + 0x0120)
#define LPC_GPDMACH2_BASE         (LPC_GPDMA_BASE + 0x0140)
#define LPC_GPDMACH3_BASE         (LPC_GPDMA_BASE + 0x0160)
#define LPC_GPDMACH4_BASE         (LPC_GPDMA_BASE + 0x0180)
#define LPC_GPDMACH5_BASE         (LPC_GPDMA_BASE + 0x01A0)
#define LPC_GPDMACH6_BASE         (LPC_GPDMA_BASE + 0x01C0)
#define LPC_GPDMACH7_BASE         (LPC_GPDMA_BASE + 0x01E0)
/*----------------------------------------------------------------------------*/
/* Peripheral declaration */
#define LPC_GPIO          ((LPC_GPIO_Type *)LPC_GPIO_BASE)
//#define LPC_SPI           ((LPC_SPI_Type *)LPC_SPI_BASE)
//#define LPC_SGPIO         ((LPC_SGPIO_Type *)LPC_SGPIO_BASE)

#define LPC_SCT           ((LPC_SCT_Type *)LPC_SCT_BASE)
#define LPC_GPDMA         ((LPC_GPDMA_Type *)LPC_GPDMA_BASE)
#define LPC_GPDMACH0      ((LPC_GPDMACH_Type *)LPC_GPDMACH0_BASE)
#define LPC_GPDMACH1      ((LPC_GPDMACH_Type *)LPC_GPDMACH1_BASE)
#define LPC_GPDMACH2      ((LPC_GPDMACH_Type *)LPC_GPDMACH2_BASE)
#define LPC_GPDMACH3      ((LPC_GPDMACH_Type *)LPC_GPDMACH3_BASE)
#define LPC_GPDMACH4      ((LPC_GPDMACH_Type *)LPC_GPDMACH4_BASE)
#define LPC_GPDMACH5      ((LPC_GPDMACH_Type *)LPC_GPDMACH5_BASE)
#define LPC_GPDMACH6      ((LPC_GPDMACH_Type *)LPC_GPDMACH6_BASE)
#define LPC_GPDMACH7      ((LPC_GPDMACH_Type *)LPC_GPDMACH7_BASE)
//#define LPC_SPIFI         ((LPC_SPIFI_Type *)LPC_SPIFI_BASE)
#define LPC_SDMMC         ((LPC_SDMMC_Type *)LPC_SDMMC_BASE)
//#define LPC_EMC           ((LPC_EMC_Type *)LPC_EMC_BASE)
//#define LPC_USB0          ((LPC_USB_Type *)LPC_USB0_BASE)
//#define LPC_USB1          ((LPC_USB_Type *)LPC_USB1_BASE)
//#define LPC_LCD           ((LPC_LCD_Type *)LPC_LCD_BASE)
//#define LPC_ETHERNET      ((LPC_ETHERNET_Type *)LPC_ETHERNET_BASE)

//#define LPC_WDT           ((LPC_WDT_Type *)LPC_WDT_BASE)
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

//#define LPC_MCPWM         ((LPC_MCPWM_Type *)LPC_MCPWM_BASE)
#define LPC_I2C0          ((LPC_I2C_Type *)LPC_I2C0_BASE)
//#define LPC_I2S0          ((LPC_I2S_Type *)LPC_I2S0_BASE)
//#define LPC_I2S1          ((LPC_I2S_Type *)LPC_I2S1_BASE)
//#define LPC_CAN1          ((LPC_CAN_Type *)LPC_CAN1_BASE)

//#define LPC_RIT           ((LPC_RIT_Type *)LPC_RIT_BASE)
#define LPC_USART2        ((LPC_USART_Type *)LPC_USART2_BASE)
#define LPC_USART3        ((LPC_USART_Type *)LPC_USART3_BASE)
#define LPC_TIMER2        ((LPC_TIMER_Type *)LPC_TIMER2_BASE)
#define LPC_TIMER3        ((LPC_TIMER_Type *)LPC_TIMER3_BASE)
#define LPC_SSP1          ((LPC_SSP_Type *)LPC_SSP1_BASE)
//#define LPC_QEI           ((LPC_QEI_Type *)LPC_QEI_BASE)
//#define LPC_GIMA          ((LPC_GIMA_Type *)LPC_GIMA_BASE)

#define LPC_I2C1          ((LPC_I2C_Type *)LPC_I2C1_BASE)
#define LPC_DAC           ((LPC_DAC_Type *)LPC_DAC_BASE)
//#define LPC_CAN0          ((LPC_CAN_Type *)LPC_CAN0_BASE)
#define LPC_ADC0          ((LPC_ADC_Type *)LPC_ADC0_BASE)
#define LPC_ADC1          ((LPC_ADC_Type *)LPC_ADC1_BASE)

//#define LPC_ATIMER        ((LPC_ATIMER_Type *)LPC_ATIMER_BASE)
//#define LPC_BACKUP        ((LPC_BACKUP_Type *)LPC_BACKUP_BASE)
#define LPC_PMC           ((LPC_PMC_Type *)LPC_PMC_BASE)
#define LPC_CREG          ((LPC_CREG_Type *)LPC_CREG_BASE)
//#define LPC_EVENTROUTER   ((LPC_EVENTROUTER_Type *)LPC_EVENTROUTER_BASE)
//#define LPC_OTP           ((LPC_OTP_Type *)LPC_OTP_BASE)
//#define LPC_RTC           ((LPC_RTC_Type *)LPC_RTC_BASE)

#define LPC_CGU           ((LPC_CGU_Type *)LPC_CGU_BASE)
#define LPC_CCU1          ((LPC_CCU1_Type *)LPC_CCU1_BASE)
#define LPC_CCU2          ((LPC_CCU2_Type *)LPC_CCU2_BASE)
#define LPC_RGU           ((LPC_RGU_Type *)LPC_RGU_BASE)
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC43XX_PLATFORM_DEFS_H_ */
