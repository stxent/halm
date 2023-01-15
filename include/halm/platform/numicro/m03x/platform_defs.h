/*
 * halm/platform/numicro/m03x/platform_defs.h
 * Based on original from Nuvoton
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_PLATFORM_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_M03X_PLATFORM_DEFS_H_
#define HALM_PLATFORM_NUMICRO_M03X_PLATFORM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#define NVIC_PRIORITY_SIZE 2
/*------------------Analog Comparator registers-------------------------------*/
typedef struct
{
  __rw__ uint32_t CTL[2]; /* Control registers */
  __rw__ uint32_t STATUS; /* Status Register */
  __rw__ uint32_t VREF;   /* Reference Voltage Control */
  __rw__ uint32_t CALCTL; /* Calibration Control */
  __ro__ uint32_t CALSR;  /* Calibration Status */
} NM_ACMP_Type;
/*------------------Analog-to-Digital Converter-------------------------------*/
typedef struct
{
  __ro__ uint32_t ADDR[30];
  __ne__ uint32_t RESERVED0[2];

  /* Offset 0x080 */
  __rw__ uint32_t ADCR; /* Control Register */
  __rw__ uint32_t ADCHER; /* Channel Enable Register */
  __rw__ uint32_t ADCMPR[2]; /* Compare Registers */
  __rw__ uint32_t ADSR0; /* Status Register 0 */
  __ro__ uint32_t ADSR1; /* Status Register 1 */
  __ro__ uint32_t ADSR2; /* Status Register 2 */
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t ESMPCTL; /* Extend Sample Time Control register */
  __rw__ uint32_t CFDCTL; /* Channel Floating Detect Control register */
  __ne__ uint32_t RESERVED2[22];

  /* Offset 0x100 */
  __ro__ uint32_t ADPDMA; /* PDMA Current Transfer Data register */
  __ne__ uint32_t RESERVED3[31];
  __rw__ uint32_t ADCALR; /* Calibration Mode Register */
  __rw__ uint32_t ADCALSTSR; /* Calibration Status Register */
  __rw__ uint32_t ADCALDBR; /* Calibration Debug Mode Register */
} NM_ADC_Type;
/*------------------Clock Controller------------------------------------------*/
typedef struct
{
  __rw__ uint32_t PWRCTL; /* System Power-down Control register */
  __rw__ uint32_t AHBCLK; /* AHB Devices Clock Enable Control register */
  __rw__ uint32_t APBCLK0; /* APB Devices Clock Enable Control register 0 */
  __rw__ uint32_t APBCLK1; /* APB Devices Clock Enable Control register 1 */

  union
  {
    struct
    {
      __rw__ uint32_t CLKSEL0; /* Clock Source Select Control register 0 */
      __rw__ uint32_t CLKSEL1; /* Clock Source Select Control register 1 */
      __rw__ uint32_t CLKSEL2; /* Clock Source Select Control register 2 */
      __rw__ uint32_t CLKSEL3; /* Clock Source Select Control register 3 */
    };

    __rw__ uint32_t CLKSEL[4];
  };

  union
  {
    struct
    {
      __rw__ uint32_t CLKDIV0; /* Clock Divider Number Register 0 */
      __ne__ uint32_t RESERVED0[3];
      __rw__ uint32_t CLKDIV4; /* Clock Divider Number register 4 */
      __rw__ uint32_t PCLKDIV; /* APB Clock Divider register */
    };

    __rw__ uint32_t CLKDIV[6];
  };

  __ne__ uint32_t RESERVED1[2];
  __rw__ uint32_t PLLCTL; /* PLL Control register */
  __ne__ uint32_t RESERVED2[3];
  __ro__ uint32_t STATUS; /* Clock Status Monitor register */
  __ne__ uint32_t RESERVED3[3];
  __rw__ uint32_t CLKOCTL; /* Clock Output Control register */
  __ne__ uint32_t RESERVED4[3];
  __rw__ uint32_t CLKDCTL; /* Clock Fail Detector Control register */
  __rw__ uint32_t CLKDSTS; /* Clock Fail Detector Status register */
  __rw__ uint32_t CDUPB; /* Clock Frequency Range Detector upper boundary register */ // TODO
  __rw__ uint32_t CDLOWB; /* Clock Frequency Range Detector lower boundary register */ // TODO
  __rw__ uint32_t LDOCTL; /* LDO Control register */
  __ne__ uint32_t RESERVED5[12];
  __rw__ uint32_t HXTFSEL; /* HXT Filter Select control register */
  __ne__ uint32_t RESERVED9[14];
  __rw__ uint32_t TESTCLK; /* Test Clock control register */
} NM_CLK_Type;
/*------------------CRC Controller--------------------------------------------*/
typedef struct
{
  __rw__ uint32_t CTL; /* Control register */
  __rw__ uint32_t DAT; /* Write Data register */
  __rw__ uint32_t SEED; /* Seed register */
  __ro__ uint32_t CHECKSUM; /* Checksum register */
} NM_CRC_Type;
/*------------------External Bus Interface------------------------------------*/
typedef struct
{
  __rw__ uint32_t CTL0; /* Bank 0 Control register */
  __rw__ uint32_t TCTL0; /* Bank 0 Timing Control register */
  __ne__ uint32_t RESERVED0[2];
  __rw__ uint32_t CTL1; /* Bank 1 Control register */
  __rw__ uint32_t TCTL1; /* Bank 1 Timing Control register */
} NM_EBI_Type;
/*------------------Flash Memory Controller-----------------------------------*/
typedef struct
{
  __rw__ uint32_t ISPCTL; /* ISP Control register */
  __rw__ uint32_t ISPADDR; /* ISP Address register */
  __rw__ uint32_t ISPDAT; /* ISP Data register */
  __rw__ uint32_t ISPCMD; /* ISP Command register */
  __rw__ uint32_t ISPTRG; /* ISP Trigger Control register */
  __ro__ uint32_t DFBA; /* Data Flash Base Address */
  __rw__ uint32_t FTCTL; /* Flash Access Time Control register */
  __rw__ uint32_t ICPCTL; /* Flash ICP Enable Control register */
  __ne__ uint32_t RESERVED0[8];
  __rw__ uint32_t ISPSTS; /* ISP Status register */
  __ne__ uint32_t RESERVED1[15];
  __rw__ uint32_t MPDAT0; /* ISP Data 0 register */
  __rw__ uint32_t MPDAT1; /* ISP Data 1 register */
  __rw__ uint32_t MPDAT2; /* ISP Data 2 register */
  __rw__ uint32_t MPDAT3; /* ISP Data 3 register */
  __ne__ uint32_t RESERVED2[12];
  __ro__ uint32_t MPSTS; /* ISP Multi-Program Status register */
  __ro__ uint32_t MPADDR; /* ISP Multi-Program Address register */
  __ne__ uint32_t RESERVED3[973];
  __ro__ uint32_t VERSION; /* FMC Version register */
} NM_FMC_Type;
/*------------------General Purpose I/O---------------------------------------*/
typedef struct
{
  __rw__ uint32_t DBCTL; /* Interrupt Debounce Control register */
} NM_GPIO_DBCTL_Type;

typedef struct
{
  struct
  {
    __rw__ uint32_t PDIO[16]; /* Pin Data Input/Output register */
  } GPIO[8];
} NM_GPIO_PDIO_Type;

typedef struct
{
  __rw__ uint32_t MODE; /* I/O Mode control */
  __rw__ uint32_t DINOFF; /* Digital Input Path Disable control */
  __rw__ uint32_t DOUT; /* Data Output Value */
  __rw__ uint32_t DATMSK; /* Data Output Write Mask */
  __ro__ uint32_t PIN; /* Pin values */
  __rw__ uint32_t DBEN; /* Debounce Enable control */
  __rw__ uint32_t INTTYPE; /* Interrupt Trigger Type control */
  __rw__ uint32_t INTEN; /* Interrupt Enable control */
  __rw__ uint32_t INTSRC; /* Interrupt Source flags */
  __ne__ uint32_t RESERVED0[7];
} NM_GPIO_Type;
/*------------------Hardware Divider------------------------------------------*/
typedef struct
{
  __rw__ uint32_t DIVIDEND; /* Dividend source Register */
  __rw__ uint32_t DIVISOR; /* Divisor source resister */
  __rw__ uint32_t QUOTIENT; /* Quotient result resister */
  __rw__ uint32_t REM; /* Remainder result register */
  __ro__ uint32_t STATUS; /* Divider Status register */
} NM_HDIV_Type;
/*------------------Inter-Integrated Circuit----------------------------------*/
typedef struct
{
  __rw__ uint32_t CTL0; /* Control register 0 */
  __rw__ uint32_t ADDR0; /* Slave Address register 0 */
  __rw__ uint32_t DAT; /* Data register */
  __ro__ uint32_t STATUS0; /* Status register 0 */
  __rw__ uint32_t CLKDIV; /* Clock Divider register */
  __rw__ uint32_t TOCTL; /* Time-out Control register */
  __rw__ uint32_t ADDR1; /* Slave Address register 1 */
  __rw__ uint32_t ADDR2; /* Slave Address register 2 */
  __rw__ uint32_t ADDR3; /* Slave Address register 3 */
  __rw__ uint32_t ADDRMSK0; /* Slave Address Mask register 0 */
  __rw__ uint32_t ADDRMSK1; /* Slave Address Mask register 1 */
  __rw__ uint32_t ADDRMSK2; /* Slave Address Mask register 2 */
  __rw__ uint32_t ADDRMSK3; /* Slave Address Mask register 3 */
  __ne__ uint32_t RESERVED0[2];
  __rw__ uint32_t WKCTL; /* Wake-up Control register */
  __rw__ uint32_t WKSTS; /* Wake-up Status register */
  __rw__ uint32_t CTL1; /* Control register 1 */
  __rw__ uint32_t STATUS1; /* Status register 1 */
  __rw__ uint32_t TMCTL; /* Timing Configure Control register */
  __rw__ uint32_t BUSCTL; /* Bus Management Control register */
  __rw__ uint32_t BUSTCTL; /* Bus Management Timer Control register */
  __rw__ uint32_t BUSSTS; /* Bus Management Status register */
  __rw__ uint32_t PKTSIZE; /* Packet Error Checking Byte Number register */
  __ro__ uint32_t PKTCRC; /* Packet Error Checking Byte Value register */
  __rw__ uint32_t BUSTOUT; /* Bus Management Timer register */
  __rw__ uint32_t CLKTOUT; /* Bus Management Clock Low Timer register */
} NM_I2C_Type;
/*------------------Peripheral Direct Memory Access Controller----------------*/
typedef struct
{
  __rw__ uint32_t CTL; /* Descriptor Table Control register */
  __rw__ uint32_t SA; /* Source Address register */
  __rw__ uint32_t DA; /* Destination Address register */
  __rw__ uint32_t NEXT; /* Next descriptor address register */
} NM_PDMA_CHANNEL_Type;

typedef struct
{
  NM_PDMA_CHANNEL_Type CHANNELS[9]; /* Control registers of PDMA channels */

  /* Offset 0x090 */
  __ne__ uint32_t RESERVED0[28];

  /* Offset 0x100 */
  __ro__ uint32_t CURSCAT[9]; /* Current descriptor addresses of channels */
  __ne__ uint32_t RESERVED1[183];

  /* Offset 0x400 */
  __rw__ uint32_t CHCTL; /* Channel Control register */
  __wo__ uint32_t PAUSE; /* Transfer Pause control register */
  __wo__ uint32_t SWREQ; /* Software Request register */
  __ro__ uint32_t TRGSTS; /* Channel Request Status register */
  __rw__ uint32_t PRISET; /* Fixed Priority Setting register */
  __wo__ uint32_t PRICLR; /* Fixed Priority Clear register */
  __rw__ uint32_t INTEN; /* Interrupt Enable register */
  __rw__ uint32_t INTSTS; /* Interrupt Status register */
  __rw__ uint32_t ABTSTS; /* Channel Read/Write Target Abort Flag register */
  __rw__ uint32_t TDSTS; /* Channel Transfer Done Flag register */
  __rw__ uint32_t ALIGN; /* Transfer Alignment Status register */
  __ro__ uint32_t TACTSTS; /* Transfer Active Flag register */
  __rw__ uint32_t TOUTPSC; /* Time-out Prescaler register */
  __rw__ uint32_t TOUTEN; /* Time-out Enable register */
  __rw__ uint32_t TOUTIEN; /* Time-out Interrupt Enable register */
  __rw__ uint32_t SCATBA; /* Scatter-gather Descriptor Table Base Address */
  __rw__ uint32_t TOC0_1; /* Time-out period counter register 0 */
  __ne__ uint32_t RESERVED2[7];
  __rw__ uint32_t CHRST; /* Channel Reset register */
  __ne__ uint32_t RESERVED3[7];

  /* Offset 0x480 */
  union
  {
    struct
    {
      __rw__ uint32_t REQSEL0_3; /* Request Source Select register 0 */
      __rw__ uint32_t REQSEL4_7; /* Request Source Select register 1 */
      __rw__ uint32_t REQSEL8; /* Request Source Select register 2 */
    };

    __rw__ uint32_t REQSEL[3]; /* Request Source Select register */
  };
} NM_PDMA_Type;
/*------------------PWM Generator and Capture Timer---------------------------*/
typedef struct
{
  __rw__ uint32_t CTL0; /* Control register 0 */
  __rw__ uint32_t CTL1; /* Control register 1 */
  __ne__ uint32_t RESERVED0[2];
  __rw__ uint32_t CLKSRC; /* Clock Source register */
  __rw__ uint32_t CLKPSC[3]; /* Clock Prescale registers */
  __rw__ uint32_t CNTEN; /* Counter Enable register */
  __rw__ uint32_t CNTCLR; /* Clear Counter register */
  __ne__ uint32_t RESERVED1[2];
  __rw__ uint32_t PERIOD[6]; /* Period registers */
  __ne__ uint32_t RESERVED2[2];
  __rw__ uint32_t CMPDAT[6]; /* Comparator registers */
  __ne__ uint32_t RESERVED3[2];
  __rw__ uint32_t DTCTL[3]; /* Dead-Time Control registers */
  __ne__ uint32_t RESERVED4[5];
  __ro__ uint32_t CNT[6]; /* Counter registers */
  __ne__ uint32_t RESERVED5[2];

  /* Offset 0x0B0 */
  __rw__ uint32_t WGCTL0; /* Generation register 0 */
  __rw__ uint32_t WGCTL1; /* Generation register 1 */
  __rw__ uint32_t MSKEN; /* Mask Enable register */
  __rw__ uint32_t MSK; /* Mask Data register */
  __rw__ uint32_t BNF; /* Brake Noise Filter register */
  __rw__ uint32_t FAILBRK; /* System Fail Brake control register */
  __rw__ uint32_t BRKCTL[3]; /* Brake Edge Detect Control registers */
  __rw__ uint32_t POLCTL; /* Pin Polarity Inversion Control register */
  __rw__ uint32_t POEN; /* Output Enable register */
  __wo__ uint32_t SWBRK; /* Software Brake control register */
  __rw__ uint32_t INTEN0; /* Interrupt Enable register 0 */
  __rw__ uint32_t INTEN1; /* Interrupt Enable register 1 */
  __rw__ uint32_t INTSTS0; /* Interrupt Flag register 0 */
  __rw__ uint32_t INTSTS1; /* Interrupt Flag register 1 */
  __ne__ uint32_t RESERVED6[2];
  __rw__ uint32_t ADCTS0; /* Trigger ADC Source Select register 0 */
  __rw__ uint32_t ADCTS1; /* Trigger ADC Source Select register 1 */
  __ne__ uint32_t RESERVED7[4];

  /* Offset 0x110 */
  __rw__ uint32_t SSCTL; /* Synchronous Start Control register */
  __wo__ uint32_t SSTRG; /* Synchronous Start Trigger register */
  __ne__ uint32_t RESERVED8[2];
  __rw__ uint32_t STATUS; /* Status register */
  __ne__ uint32_t RESERVED9[55];

  /* Offset 0x200 */
  __rw__ uint32_t CAPINEN; /* Capture Input Enable register */
  __rw__ uint32_t CAPCTL; /* Capture Control register */
  __ro__ uint32_t CAPSTS; /* Capture Status register */

  /* Offset 0x20C */
  struct
  {
    __rw__ uint32_t RCAPDAT; /* Rising Capture Data register */
    __rw__ uint32_t FCAPDAT; /* Falling Capture Data register */
  } CAPDAT[6];

  __rw__ uint32_t PDMACTL; /* PDMA Control register */
  __ro__ uint32_t PDMACAP0_1; /* Capture Channel 01 PDMA register */
  __ro__ uint32_t PDMACAP2_3; /* Capture Channel 23 PDMA register */
  __ro__ uint32_t PDMACAP4_5; /* Capture Channel 45 PDMA register */
  __ne__ uint32_t RESERVED10;
  __rw__ uint32_t CAPIEN; /* Capture Interrupt Enable register */
  __rw__ uint32_t CAPIF; /* Capture Interrupt Flag register */
  __ne__ uint32_t RESERVED11[43];
  __ro__ uint32_t PBUF[6]; /* Period Buffer registers */
  __ro__ uint32_t CMPBUF[6]; /* Comparator Buffer registers */
} NM_PWM_Type;
/*------------------Basic PWM Generator and Capture Timer---------------------*/
typedef struct
{
  __rw__ uint32_t CTL0; /* Control register 0 */
  __rw__ uint32_t CTL1; /* Control register 1 */
  __ne__ uint32_t RESERVED0[2];
  __rw__ uint32_t CLKSRC; /* Clock Source register */
  __rw__ uint32_t CLKPSC; /* Clock Prescale register */
  __ne__ uint32_t RESERVED1[2];
  __rw__ uint32_t CNTEN; /* Counter Enable register */
  __rw__ uint32_t CNTCLR; /* Clear Counter register */
  __ne__ uint32_t RESERVED2[2];
  __rw__ uint32_t PERIOD; /* Period register */
  __ne__ uint32_t RESERVED3[7];
  __rw__ uint32_t CMPDAT[6]; /* Comparator registers */
  __ne__ uint32_t RESERVED4[10];
  __ro__ uint32_t CNT; /* Counter register */
  __ne__ uint32_t RESERVED5[7];
  __rw__ uint32_t WGCTL0; /* Generation register 0 */
  __rw__ uint32_t WGCTL1; /* Generation register 1 */
  __rw__ uint32_t MSKEN; /* Mask Enable register */
  __rw__ uint32_t MSK; /* Mask Data register */
  __ne__ uint32_t RESERVED6[5];
  __rw__ uint32_t POLCTL; /* Pin Polarity Inversion Control register */
  __rw__ uint32_t POEN; /* Output Enable register */
  __ne__ uint32_t RESERVED7;
  __rw__ uint32_t INTEN; /* Interrupt Enable register */
  __ne__ uint32_t RESERVED8;
  __rw__ uint32_t INTSTS; /* Interrupt Flag register */
  __ne__ uint32_t RESERVED9[3];
  __rw__ uint32_t EADCTS0; /* Trigger EADC Source Select register 0 */
  __rw__ uint32_t EADCTS1; /* Trigger EADC Source Select register 1 */
  __ne__ uint32_t RESERVED10[4];

  /* Offset 0x110 */
  __rw__ uint32_t SSCTL; /* Synchronous Start Control register */
  __wo__ uint32_t SSTRG; /* Synchronous Start Trigger register */
  __ne__ uint32_t RESERVED11[2];
  __rw__ uint32_t STATUS; /* Status register */
  __ne__ uint32_t RESERVED12[55];

  /* Offset 0x200 */
  __rw__ uint32_t CAPINEN; /* Capture Input Enable register */
  __rw__ uint32_t CAPCTL; /* Capture Control register */
  __ro__ uint32_t CAPSTS; /* Capture Status register */

  /* Offset 0x20C */
  struct
  {
    __rw__ uint32_t RCAPDAT; /* Rising Capture Data register */
    __rw__ uint32_t FCAPDAT; /* Falling Capture Data register */
  } CAPDAT[6];

  __ne__ uint32_t RESERVED13[5];
  __rw__ uint32_t CAPIEN; /* Capture Interrupt Enable register */
  __rw__ uint32_t CAPIF; /* Capture Interrupt Flag register */
  __ne__ uint32_t RESERVED14[43];
  __ro__ uint32_t PBUF; /* Period Buffer register */
  __ne__ uint32_t RESERVED15[5];
  __ro__ uint32_t CMPBUF[6]; /* Comparator Buffer registers */
} NM_BPWM_Type;
/*------------------Quad Serial Peripheral Interface--------------------------*/
typedef struct
{
  __rw__ uint32_t CTL; /* Control register */
  __rw__ uint32_t CLKDIV; /* Clock Divider register */
  __rw__ uint32_t SSCTL; /* Slave Select Control register */
  __rw__ uint32_t PDMACTL; /* PDMA Control register */
  __rw__ uint32_t FIFOCTL; /* FIFO Control register */
  __rw__ uint32_t STATUS; /* Status register */
  __ne__ uint32_t RESERVED0[2];
  __wo__ uint32_t TX; /* Data Transmit register */
  __ne__ uint32_t RESERVED1[3];
  __ro__ uint32_t RX; /* Data Receive register */
} NM_QSPI_Type;
/*------------------Serial Peripheral Interface-------------------------------*/
typedef struct
{
  __rw__ uint32_t CTL; /* Control register */
  __rw__ uint32_t CLKDIV; /* Clock Divider register */
  __rw__ uint32_t SSCTL; /* Slave Select Control register */
  __rw__ uint32_t PDMACTL; /* PDMA Control register */
  __rw__ uint32_t FIFOCTL; /* FIFO Control register */
  __rw__ uint32_t STATUS; /* Status register */
  __ne__ uint32_t RESERVED0[2];
  __wo__ uint32_t TX; /* Data Transmit register */
  __ne__ uint32_t RESERVED1[3];
  __ro__ uint32_t RX; /* Data Receive register */
  __ne__ uint32_t RESERVED2[11];
  __rw__ uint32_t I2SCTL; /* I2S Control register */
  __rw__ uint32_t I2SCLK; /* I2S Clock Divider Control register */
  __rw__ uint32_t I2SSTS; /* I2S Status register */
} NM_SPI_Type;
/*------------------Non-Maskable Interrupt------------------------------------*/
typedef struct
{
  __rw__ uint32_t NMIEN; /* Source Interrupt Enable register */
  __ro__ uint32_t NMISTS; /* Source Interrupt Status register */
} NM_NMI_Type;
/*------------------System Manager--------------------------------------------*/
typedef struct
{
  __ro__ uint32_t PDID; /* Part Device Identification Number register */
  __rw__ uint32_t RSTSTS; /* System Reset Status register */
  __rw__ uint32_t IPRST0; /* Peripheral Reset control register 0 */
  __rw__ uint32_t IPRST1; /* Peripheral Reset control register 1 */
  __rw__ uint32_t IPRST2; /* Peripheral Reset control register 2 */
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t BODCTL; /* Brown-out Detector Control register */
  __ne__ uint32_t RESERVED1[2];
  __rw__ uint32_t PORCTL; /* Power-On-reset Controller register */
  __ne__ uint32_t RESERVED2[2];

  /* Offset 0x030 */
  struct
  {
    __rw__ uint32_t MFPL; /* GPIO Low Byte Multiple Function control register */
    __rw__ uint32_t MFPH; /* GPIO High Byte Multiple Function control register */
  } GP[8];

  __ne__ uint32_t RESERVED3[2];
  __rw__ uint32_t LPLDOCTL; /* Low Power LDO Control register */
  __ne__ uint32_t RESERVED4[17];
  __rw__ uint32_t MODCTL; /* Modulation Control register */
  __ne__ uint32_t RESERVED5[3];
  __rw__ uint32_t SRAM_BISTCTL; /* System SRAM BIST Test Control register */
  __ro__ uint32_t SRAM_BISTSTS; /* System SRAM BIST Test Status register */
  __rw__ uint32_t SRAM_PARITY; /* System SRAM Parity Test Control register */
  __rw__ uint32_t SRAM_INTCTL; /* System SRAM Interrupt Enable Control register */ // TODO
  __rw__ uint32_t SRAM_STATUS; /* System SRAM Parity Error Status register */
  __ro__ uint32_t SRAM_ERRADDR; /* System SRAM Parity Check Error Address register */ // TODO
  __ne__ uint32_t RESERVED6[2];
  __rw__ uint32_t HIRCTRIMCTL; /* HIRC Trim Control register */
  __rw__ uint32_t HIRCTRIMIEN; /* HIRC Trim Interrupt Enable register */
  __rw__ uint32_t HIRCTRIMSTS; /* HIRC Trim Interrupt Status register */
  __ne__ uint32_t RESERVED7;

  /* Offset 0x100 */
  __wo__ uint32_t REGLCTL; /* Register Lock Control register */
  __ne__ uint32_t RESERVED8[58];
  __rw__ uint32_t PORDISAN; /* Analog POR Disable control register */

  /* Offset 0x100 */
// XXX Verify
//   __wo__ uint32_t REGLCTL; /* Register Lock Control register */
//   __ne__ uint32_t RESERVED8[5];
//   __rw__ uint32_t HIRCADJ; /* HIRC Trim Value register */
//   __ne__ uint32_t RESERVED9;
//   __ro__ uint32_t LDOTRIM; /* LDO Trim code register */
//   __ro__ uint32_t LVR16TRIM; /* LVR16 Trim code register */
//   __ne__ uint32_t RESERVED10[4];
//   __ro__ uint32_t LIRCT; /* Low Speed Internal Oscillator Trim code register */
//   __ne__ uint32_t RESERVED11[5];
//   __ro__ uint32_t LVR17TRIM; /* LVR17 Trim code register */
//   __ro__ uint32_t LVR20TRIM; /* LVR20 Trim code register */
//   __ro__ uint32_t LVR25TRIM; /* LVR25 Trim code register */
//   __ro__ uint32_t ULDOVITRIM; /* ULDO V Trim and I TRIM code register */
//   __rw__ uint32_t LVRITRIMSEL; /* LVR I Trim and Version Select register */
//   __ne__ uint32_t RESERVED12[9];
//   __rw__ uint32_t HIRCTCTL; /* HIRC Test Mode Control register */
//   __rw__ uint32_t ADCCHIP; /* ADC CHIP control register */
//   __rw__ uint32_t HXTTCTL; /* R/W HXT Test Mode Control register */
//   __ne__ uint32_t RESERVED13[22];
//   __rw__ uint32_t PORDISAN; /* Analog POR Disable control register
} NM_SYS_Type;
/*------------------Real Timer Clock------------------------------------------*/
typedef struct
{
  __rw__ uint32_t INIT; /* Initiation register */
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t FREQADJ; /* Frequency compensation register */
  __rw__ uint32_t TIME; /* Time loading register */
  __rw__ uint32_t CAL; /* Calendar loading register */
  __rw__ uint32_t CLKFMT; /* Time Scale Selection register */
  __rw__ uint32_t WEEKDAY; /* Day of the Week register */
  __rw__ uint32_t TALM; /* Time Alarm register */
  __rw__ uint32_t CALM; /* Calendar Alarm register */
  __ro__ uint32_t LEAPYEAR; /* Leap Year indication register */
  __rw__ uint32_t INTEN; /* Interrupt Enable register */
  __rw__ uint32_t INTSTS; /* Interrupt Status register */
  __rw__ uint32_t TICK; /* Time Tick register */
  __rw__ uint32_t TAMSK; /* Time Alarm Mask register */
  __rw__ uint32_t CAMSK; /* Calendar Alarm Mask register */
  __ne__ uint32_t RESERVED1[49];

  /* Offset 0x100 */
  __rw__ uint32_t LXTCTL; /* RTC 32.768 kHz oscillator control register */
} NM_RTC_Type;
/*------------------Timer Controller------------------------------------------*/
typedef struct
{
  __rw__ uint32_t CTL; /* Control register */
  __rw__ uint32_t CMP; /* Comparator register */
  __rw__ uint32_t INTSTS; /* Interrupt Status register */
  __ro__ uint32_t CNT; /* Data register */
  __ro__ uint32_t CAP; /* Capture Data register */
  __rw__ uint32_t EXTCTL; /* External Control register */
  __rw__ uint32_t EINTSTS; /* External Interrupt Status register */
} NM_TIMER_Type;
/*------------------Universal Asynchronous Receiver Transmitter---------------*/
typedef struct
{
  __rw__ uint32_t DAT; /* Receive/Transmit buffer register */
  __rw__ uint32_t INTEN; /* Interrupt Enable register */
  __rw__ uint32_t FIFO; /* FIFO control register */
  __rw__ uint32_t LINE; /* Line control register */
  __rw__ uint32_t MODEM; /* Modem control register */
  __rw__ uint32_t MODEMSTS; /* Modem Status register */
  __rw__ uint32_t FIFOSTS; /* FIFO Status register */
  __rw__ uint32_t INTSTS; /* Interrupt Status register */
  __rw__ uint32_t TOUT; /* Time-out register */
  __rw__ uint32_t BAUD; /* Baud rate divider register */
  __rw__ uint32_t IRDA; /* IrDA control register */
  __rw__ uint32_t ALTCTL; /* Alternate Control/Status register */
  __rw__ uint32_t FUNCSEL; /* Function Select register */
  __ne__ uint32_t RESERVED0[2];
  __rw__ uint32_t BRCOMP; /* Baud Rate Compensation register */
  __rw__ uint32_t WKCTL; /* Wake-up Control register */
  __rw__ uint32_t WKSTS; /* Wake-up Status register */
  __rw__ uint32_t DWKCOMP; /* Incoming Data Wake-up Compensation register */
} NM_UART_Type;
/*------------------USB 2.0 Full-Speed Device Controller----------------------*/
typedef struct
{
  __rw__ uint32_t BUFSEG; /* Buffer Segmentation register */
  __rw__ uint32_t MXPLD; /* Maximal Payload register */
  __rw__ uint32_t CFG; /* Configuration register */
  __rw__ uint32_t CFGP; /* Endpoint Stall and IO Ready control register */
} NM_USBD_EP_Type;

typedef struct
{
  __rw__ uint32_t INTEN; /* Interrupt Enable register */
  __rw__ uint32_t INTSTS; /* Interrupt Event Status register */
  __rw__ uint32_t FADDR; /* Function Address register */
  __ro__ uint32_t EPSTS; /* Endpoint Status register */
  __rw__ uint32_t ATTR; /* Bus Status and Attribution register */
  __ro__ uint32_t VBUSDET; /* VBUS Detection register */
  __rw__ uint32_t STBUFSEG; /* SETUP Token Buffer Segmentation register */
  __ne__ uint32_t RESERVED0;
  __ro__ uint32_t EPSTS0; /* Endpoint Status register 0 */
  __ne__ uint32_t RESERVED1[25];
  __ro__ uint32_t LPMATTR; /* LPM Attribution register */
  __ro__ uint32_t FN; /* Frame number register */
  __rw__ uint32_t SE0; /* Device Drive SE0 Control register */
  __ne__ uint32_t RESERVED2[27];

  /* Offset 0x100 */
  __rw__ uint8_t SRAM[512];
  __ne__ uint32_t RESERVED3[128];

  /* Offset 0x500 */
  NM_USBD_EP_Type EP[8]; /* USB Device Endpoints */
} NM_USBD_Type;
/*------------------Watchdog Timer--------------------------------------------*/
typedef struct
{
  __rw__ uint32_t CTL; /* Control register */
  __rw__ uint32_t ALTCTL; /* Alternative Control register */
  __wo__ uint32_t RSTCNT; /* Reset Counter register */
} NM_WDT_Type;
/*------------------Window Watchdog Timer-------------------------------------*/
typedef struct
{
  __wo__ uint32_t RLDCNT; /* Reload Counter register */
  __rw__ uint32_t CTL; /* Control register */
  __rw__ uint32_t STATUS; /* Status register */
  __ro__ uint32_t CNT; /* Counter value register */
} NM_WWDT_Type;
/*------------------Universal Serial Control Interface Controller-------------*/
typedef struct
{
  __wo__ uint32_t CTL; /* Control register */
  __rw__ uint32_t INTEN; /* Interrupt Enable register */
  __rw__ uint32_t BRGEN; /* Baud Rate Generator register */
  __ro__ uint32_t DATIN0; /* Input Data signal configuration */
  __ro__ uint32_t CTLIN0; /* Input Control signal configuration */
  __ro__ uint32_t CLKIN; /* Input Clock signal configuration */
  __ro__ uint32_t LINECTL; /* Line Control register */
  __ro__ uint32_t TXDAT; /* Transmit Data register */
  __ro__ uint32_t RXDAT; /* Receive Data register */
  __ro__ uint32_t BUFCTL; /* Transmit/Receive Buffer Control register */
  __ro__ uint32_t BUFSTS; /* Transmit/Receive Buffer Status register */
  __ro__ uint32_t PDMACTL; /* PDMA Control register */
  __ro__ uint32_t WKCTL; /* Wake-up Control register */
  __ro__ uint32_t WKSTS; /* Wake-up Status register */
  __ro__ uint32_t PROTCTL; /* Protocol Control register */
  __ro__ uint32_t PROTIEN; /* Protocol Interrupt Enable register */
  __ro__ uint32_t PROTSTS; /* Protocol Status register */
} NM_USCI_Type;
/*----------------------------------------------------------------------------*/
typedef struct
{
  NM_SYS_Type SYS;
  __ne__ uint8_t RESERVED0[0x200 - sizeof(NM_SYS_Type)];
  NM_CLK_Type CLK;
  __ne__ uint8_t RESERVED1[0x200 - sizeof(NM_CLK_Type)];
  NM_NMI_Type NMI;
  __ne__ uint8_t RESERVED2[0x3C00 - sizeof(NM_NMI_Type)];
  NM_GPIO_Type GPIO[8];
  __ne__ uint8_t RESERVED3[0x440 - sizeof(NM_GPIO_Type) * 8];
  NM_GPIO_DBCTL_Type GPIO_DBCTL;
  __ne__ uint8_t RESERVED4[0x3C0 - sizeof(NM_GPIO_DBCTL_Type)];
  NM_GPIO_PDIO_Type GPIO_PDIO;
  __ne__ uint8_t RESERVED5[0x3800 - sizeof(NM_GPIO_PDIO_Type)];
  NM_PDMA_Type PDMA;
  __ne__ uint8_t RESERVED6[0x4000 - sizeof(NM_PDMA_Type)];
  NM_FMC_Type FMC;
  __ne__ uint8_t RESERVED7[0x4000 - sizeof(NM_FMC_Type)];
  NM_EBI_Type EBI;
  __ne__ uint8_t RESERVED8[0x4000 - sizeof(NM_EBI_Type)];
  NM_HDIV_Type HDIV;
  __ne__ uint8_t RESERVED9[0x1D000 - sizeof(NM_HDIV_Type)];
  NM_CRC_Type CRC;
  __ne__ uint8_t RESERVED10[0x8F000 - sizeof(NM_CRC_Type)];
  NM_USBD_Type USBD;
} AHB_DOMAIN_Type;

typedef struct
{
  __ne__ uint8_t RESERVED0[0x40000];
  NM_WDT_Type WDT;
  __ne__ uint8_t RESERVED1[0x100 - sizeof(NM_WDT_Type)];
  NM_WWDT_Type WWDT;
  __ne__ uint8_t RESERVED2[0xF00 - sizeof(NM_WWDT_Type)];
  NM_RTC_Type RTC;
  __ne__ uint8_t RESERVED3[0x2000 - sizeof(NM_RTC_Type)];
  NM_ADC_Type ADC;
  __ne__ uint8_t RESERVED4[0x2000 - sizeof(NM_ADC_Type)];
  NM_ACMP_Type ACMP01;
  __ne__ uint8_t RESERVED5[0xB000 - sizeof(NM_ACMP_Type)];
  NM_TIMER_Type TIMER0;
  __ne__ uint8_t RESERVED6[0x20 - sizeof(NM_TIMER_Type)];
  NM_TIMER_Type TIMER1;
  __ne__ uint8_t RESERVED7[0x7FE0 - sizeof(NM_TIMER_Type)];
  NM_PWM_Type PWM0;
  __ne__ uint8_t RESERVED8[0x2000 - sizeof(NM_PWM_Type)];
  NM_BPWM_Type BPWM0;
  __ne__ uint8_t RESERVED9[0x6000 - sizeof(NM_BPWM_Type)];
  NM_QSPI_Type QSPI0;
  __ne__ uint8_t RESERVED10[0x1000 - sizeof(NM_QSPI_Type)];
  NM_SPI_Type SPI0;
  __ne__ uint8_t RESERVED11[0xF000 - sizeof(NM_SPI_Type)];
  NM_UART_Type UART0;
  __ne__ uint8_t RESERVED12[0x10000 - sizeof(NM_UART_Type)];
  NM_I2C_Type I2C0;
  __ne__ uint8_t RESERVED13[0x50000 - sizeof(NM_I2C_Type)];
  NM_USCI_Type USCI0;
} APB1_DOMAIN_Type;

typedef struct
{
  __ne__ uint8_t RESERVED0[0x51000];
  NM_TIMER_Type TIMER2;
  __ne__ uint8_t RESERVED1[0x20 - sizeof(NM_TIMER_Type)];
  NM_TIMER_Type TIMER3;
  __ne__ uint8_t RESERVED2[0x7FE0 - sizeof(NM_TIMER_Type)];
  NM_PWM_Type PWM1;
  __ne__ uint8_t RESERVED3[0x2000 - sizeof(NM_PWM_Type)];
  NM_BPWM_Type BPWM1;
  __ne__ uint8_t RESERVED4[0x16000 - sizeof(NM_BPWM_Type)];
  NM_UART_Type UART1;
  __ne__ uint8_t RESERVED5[0x1000 - sizeof(NM_UART_Type)];
  NM_UART_Type UART2;
  __ne__ uint8_t RESERVED6[0x1000 - sizeof(NM_UART_Type)];
  NM_UART_Type UART3;
  __ne__ uint8_t RESERVED7[0x1000 - sizeof(NM_UART_Type)];
  NM_UART_Type UART4;
  __ne__ uint8_t RESERVED8[0x1000 - sizeof(NM_UART_Type)];
  NM_UART_Type UART5;
  __ne__ uint8_t RESERVED9[0x1000 - sizeof(NM_UART_Type)];
  NM_UART_Type UART6;
  __ne__ uint8_t RESERVED10[0x1000 - sizeof(NM_UART_Type)];
  NM_UART_Type UART7;
  __ne__ uint8_t RESERVED11[0xA000 - sizeof(NM_UART_Type)];
  NM_I2C_Type I2C1;
  __ne__ uint8_t RESERVED12[0x50000 - sizeof(NM_I2C_Type)];
  NM_USCI_Type USCI1;
} APB2_DOMAIN_Type;

typedef union
{
  AHB_DOMAIN_Type AHB_DOMAIN;
  APB1_DOMAIN_Type APB1_DOMAIN;
  APB2_DOMAIN_Type APB2_DOMAIN;
} AHB_APB_DOMAIN_Type;
/*----------------------------------------------------------------------------*/
extern AHB_APB_DOMAIN_Type AHB_APB_DOMAIN;
/*----------------------------------------------------------------------------*/
#define NM_SYS        (&AHB_APB_DOMAIN.AHB_DOMAIN.SYS)
#define NM_CLK        (&AHB_APB_DOMAIN.AHB_DOMAIN.CLK)
#define NM_NMI        (&AHB_APB_DOMAIN.AHB_DOMAIN.NMI)
#define NM_GPIOA      (&AHB_APB_DOMAIN.AHB_DOMAIN.GPIO[0])
#define NM_GPIOB      (&AHB_APB_DOMAIN.AHB_DOMAIN.GPIO[1])
#define NM_GPIOC      (&AHB_APB_DOMAIN.AHB_DOMAIN.GPIO[2])
#define NM_GPIOD      (&AHB_APB_DOMAIN.AHB_DOMAIN.GPIO[3])
#define NM_GPIOE      (&AHB_APB_DOMAIN.AHB_DOMAIN.GPIO[4])
#define NM_GPIOF      (&AHB_APB_DOMAIN.AHB_DOMAIN.GPIO[5])
#define NM_GPIOG      (&AHB_APB_DOMAIN.AHB_DOMAIN.GPIO[6])
#define NM_GPIOH      (&AHB_APB_DOMAIN.AHB_DOMAIN.GPIO[7])
#define NM_GPIO_DBCTL (&AHB_APB_DOMAIN.AHB_DOMAIN.GPIO_DBCTL)
#define NM_GPIO_PDIO  (&AHB_APB_DOMAIN.AHB_DOMAIN.GPIO_PDIO)
#define NM_PDMA       (&AHB_APB_DOMAIN.AHB_DOMAIN.PDMA)
#define NM_FMC        (&AHB_APB_DOMAIN.AHB_DOMAIN.FMC)
#define NM_EBI        (&AHB_APB_DOMAIN.AHB_DOMAIN.EBI)
#define NM_HDIV       (&AHB_APB_DOMAIN.AHB_DOMAIN.HDIV)
#define NM_CRC        (&AHB_APB_DOMAIN.AHB_DOMAIN.CRC)
#define NM_USBD       (&AHB_APB_DOMAIN.AHB_DOMAIN.USBD)

#define NM_WDT        (&AHB_APB_DOMAIN.APB1_DOMAIN.WDT)
#define NM_WWDT       (&AHB_APB_DOMAIN.APB1_DOMAIN.WWDT)
#define NM_RTC        (&AHB_APB_DOMAIN.APB1_DOMAIN.RTC)
#define NM_ADC        (&AHB_APB_DOMAIN.APB1_DOMAIN.ADC)
#define NM_ACMP01     (&AHB_APB_DOMAIN.APB1_DOMAIN.ACMP01)
#define NM_TIMER0     (&AHB_APB_DOMAIN.APB1_DOMAIN.TIMER0)
#define NM_TIMER1     (&AHB_APB_DOMAIN.APB1_DOMAIN.TIMER1)
#define NM_PWM0       (&AHB_APB_DOMAIN.APB1_DOMAIN.PWM0)
#define NM_BPWM0      (&AHB_APB_DOMAIN.APB1_DOMAIN.BPWM0)
#define NM_QSPI0      (&AHB_APB_DOMAIN.APB1_DOMAIN.QSPI0)
#define NM_SPI0       (&AHB_APB_DOMAIN.APB1_DOMAIN.SPI0)
#define NM_UART0      (&AHB_APB_DOMAIN.APB1_DOMAIN.UART0)
#define NM_I2C0       (&AHB_APB_DOMAIN.APB1_DOMAIN.I2C0)
#define NM_USCI0      (&AHB_APB_DOMAIN.APB1_DOMAIN.USCI0)

#define NM_TIMER2     (&AHB_APB_DOMAIN.APB2_DOMAIN.TIMER2)
#define NM_TIMER3     (&AHB_APB_DOMAIN.APB2_DOMAIN.TIMER3)
#define NM_PWM1       (&AHB_APB_DOMAIN.APB2_DOMAIN.PWM1)
#define NM_BPWM1      (&AHB_APB_DOMAIN.APB2_DOMAIN.BPWM1)
#define NM_UART1      (&AHB_APB_DOMAIN.APB2_DOMAIN.UART1)
#define NM_UART2      (&AHB_APB_DOMAIN.APB2_DOMAIN.UART2)
#define NM_UART3      (&AHB_APB_DOMAIN.APB2_DOMAIN.UART3)
#define NM_UART4      (&AHB_APB_DOMAIN.APB2_DOMAIN.UART4)
#define NM_UART5      (&AHB_APB_DOMAIN.APB2_DOMAIN.UART5)
#define NM_UART6      (&AHB_APB_DOMAIN.APB2_DOMAIN.UART6)
#define NM_UART7      (&AHB_APB_DOMAIN.APB2_DOMAIN.UART7)
#define NM_I2C1       (&AHB_APB_DOMAIN.APB2_DOMAIN.I2C1)
#define NM_USCI1      (&AHB_APB_DOMAIN.APB2_DOMAIN.USCI1)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M03X_PLATFORM_DEFS_H_ */
