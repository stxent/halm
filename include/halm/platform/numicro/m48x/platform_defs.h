/*
 * halm/platform/numicro/m48x/platform_defs.h
 * Based on original from Nuvoton
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_PLATFORM_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_M48X_PLATFORM_DEFS_H_
#define HALM_PLATFORM_NUMICRO_M48X_PLATFORM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*------------------Analog Comparator registers-------------------------------*/
typedef struct
{
  __rw__ uint32_t CTL[2]; /* Control registers */
  __rw__ uint32_t STATUS; /* Status register */
  __rw__ uint32_t VREF; /* Reference Voltage Control */
} NM_ACMP_Type;
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
/*------------------Controller Area Network Controller------------------------*/
typedef struct
{
  __rw__ uint32_t CREQ; /* Command Request register */
  __rw__ uint32_t CMASK; /* Command Mask register */
  __rw__ uint32_t MASK1; /* Mask 1 register */
  __rw__ uint32_t MASK2; /* Mask 2 register */
  __rw__ uint32_t ARB1; /* Arbitration 1 register */
  __rw__ uint32_t ARB2; /* Arbitration 2 register */
  __rw__ uint32_t MCON; /* Message Control register */
  __rw__ uint32_t DAT_A1; /* Data A1 register */
  __rw__ uint32_t DAT_A2; /* Data A2 register */
  __rw__ uint32_t DAT_B1; /* Data B1 register */
  __rw__ uint32_t DAT_B2; /* Data B2 register */
  __ne__ uint32_t RESERVED0[13];
} NM_CAN_IF_Type;

typedef struct
{
  __rw__ uint32_t CON; /* Control register */
  __rw__ uint32_t STATUS; /* Status register */
  __ro__ uint32_t ERR; /* Error Counter register */
  __rw__ uint32_t BTIME; /* Bit Timing register */
  __ro__ uint32_t IIDR; /* Interrupt Identifier register */
  __rw__ uint32_t TEST; /* Test register */
  __rw__ uint32_t BRPE; /* Baud Rate Prescaler Extension register */
  __ne__ uint32_t RESERVED0;

  NM_CAN_IF_Type IF[2];
  __ne__ uint32_t RESERVED1[8];

  /* Offset 0x0100 */
  union
  {
    __ro__ uint32_t TXREQ[2];
    struct
    {
      __ro__ uint32_t TXREQ1;
      __ro__ uint32_t TXREQ2;
    };
  };
  __ne__ uint32_t RESERVED2[6];

  /* Offset 0x0120 */
  union
  {
    __ro__ uint32_t NDAT[2];
    struct
    {
      __ro__ uint32_t NDAT1;
      __ro__ uint32_t NDAT2;
    };
  };
  __ne__ uint32_t RESERVED3[6];

  /* Offset 0x0140 */
  union
  {
    __ro__ uint32_t IPND[2];
    struct
    {
      __ro__ uint32_t IPND1;
      __ro__ uint32_t IPND2;
    };
  };
  __ne__ uint32_t RESERVED4[6];

  /* Offset 0x0160 */
  union
  {
    __ro__ uint32_t MVLD[2];
    struct
    {
      __ro__ uint32_t MVLD1;
      __ro__ uint32_t MVLD2;
    };
  };

  /* Offset 0x0168 */
  __rw__ uint32_t WU_EN; /* Wake-up Enable Control register */
  __rw__ uint32_t WU_STATUS; /* Wake-up Status register */
} NM_CAN_Type;
/*------------------Camera Capture Interface Controller-----------------------*/
typedef struct
{
  __rw__ uint32_t CTL; /* Control register */
  __rw__ uint32_t PAR; /* Parameter register */
  __rw__ uint32_t INT; /* Interrupt register */
  __rw__ uint32_t POSTERIZE; /* YUV Component Posterizing Factor register */
  __rw__ uint32_t MD; /* Motion Detection register */
  __rw__ uint32_t MDADDR; /* Motion Detection Output Address register */
  __rw__ uint32_t MDYADDR; /* Motion Detection Temp Y Output Address register */
  __rw__ uint32_t SEPIA; /* Sepia Effect Control register */
  __rw__ uint32_t CWSP; /* Cropping Window Starting Address register */
  __rw__ uint32_t CWS; /* Cropping Window Size register */
  __rw__ uint32_t PKTSL; /* Packet Scaling Factor register */
  __rw__ uint32_t PLNSL; /* Planar Scaling Factor register */
  __rw__ uint32_t FRCTL; /* Scaling Frame Rate Factor register */
  __rw__ uint32_t STRIDE; /* Frame Output Pixel Stride Width register */
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t FIFOTH; /* FIFO Threshold register */
  __rw__ uint32_t CMPADDR; /* Compare Memory Base Address register */
  __rw__ uint32_t LUMA_Y1_THD; /* Luminance Y8 to Y1 Threshold Value register */
  __rw__ uint32_t PKTSM; /* Packet Scaling Factor register */
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t PKTBA0; /* System Memory Packet Base Address 0 register */
} NM_CCAP_Type;
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
      __rw__ uint32_t CLKDIV0; /* Clock Divider Number register 0 */
      __rw__ uint32_t CLKDIV1; /* Clock Divider Number register 1 */
      __rw__ uint32_t CLKDIV2; /* Clock Divider Number register 2 */
      __rw__ uint32_t CLKDIV3; /* Clock Divider Number register 3 */
      __rw__ uint32_t CLKDIV4; /* Clock Divider Number register 4 */
      __rw__ uint32_t PCLKDIV; /* APB Clock Divider register */
    };

    __rw__ uint32_t CLKDIV[6];
  };

  __ne__ uint32_t RESERVED0[2];
  __rw__ uint32_t PLLCTL; /* PLL Control register */
  __ne__ uint32_t RESERVED1[3];
  __ro__ uint32_t STATUS; /* Clock Status Monitor register */
  __ne__ uint32_t RESERVED2[3];
  __rw__ uint32_t CLKOCTL; /* Clock Output Control register */
  __ne__ uint32_t RESERVED3[3];
  __rw__ uint32_t CLKDCTL; /* Clock Fail Detector Control register */
  __rw__ uint32_t CLKDSTS; /* Clock Fail Detector Status register */
  __rw__ uint32_t CDUPB; /* Clock Frequency Range Detector upper boundary */
  __rw__ uint32_t CDLOWB; /* Clock Frequency Range Detector lower boundary */
  __ne__ uint32_t RESERVED4[4];
  __rw__ uint32_t PMUCTL; /* Power Manager Control register */
  __rw__ uint32_t PMUSTS; /* Power Manager Status register */
  __rw__ uint32_t LDOCTL; /* LDO Control register */
  __rw__ uint32_t SWKDBCTL; /* Low-power de-bounce control register */
  __rw__ uint32_t PASWKCTL; /* GPA low-power control register */
  __rw__ uint32_t PBSWKCTL; /* GPB low-power control register */
  __rw__ uint32_t PCSWKCTL; /* GPC low-power control register */
  __rw__ uint32_t PDSWKCTL; /* GPD low-power control register */
  __rw__ uint32_t IOPDCTL; /* GPIO low-power control register */
} NM_CLK_Type;
/*------------------User Configuration Block----------------------------------*/
typedef struct
{
  __ro__ uint32_t CONFIG[4];
} NM_CONFIG_Type;
/*------------------Cyclic Redundancy Check Controller------------------------*/
typedef struct
{
  __rw__ uint32_t CTL; /* Control register */
  __rw__ uint32_t DAT; /* Write Data register */
  __rw__ uint32_t SEED; /* Seed register */
  __ro__ uint32_t CHECKSUM; /* Checksum register */
} NM_CRC_Type;
/*------------------Cryptographic Accelerator-----------------------------------*/
typedef struct
{
  // TODO Def length
  __rw__ uint32_t INTEN;          /* Crypto Interrupt Enable Control register */
  __rw__ uint32_t INTSTS;         /* Crypto Interrupt Flag */
  __rw__ uint32_t PRNG_CTL;       /* PRNG Control register */
  __wo__ uint32_t PRNG_SEED;      /* Seed for PRNG */
  __ro__ uint32_t PRNG_KEY[8];    /* PRNG Generated Key0 ~ Key7 */
  __ne__ uint32_t RESERVED0[8];
  __ro__ uint32_t AES_FDBCK[4];   /* AES Engine Output Feedback Data after Cryptographic Operation */
  __ro__ uint32_t TDES_FDBCKH;    /* TDES/DES Engine Output Feedback High Word Data after Cryptographic Operation */
  __ro__ uint32_t TDES_FDBCKL;    /* TDES/DES Engine Output Feedback Low Word Data after Cryptographic Operation */
  __ne__ uint32_t RESERVED1[38];
  __rw__ uint32_t AES_CTL;        /* AES Control register */
  __ro__ uint32_t AES_STS;        /* AES Engine Flag */
  __rw__ uint32_t AES_DATIN;      /* AES Engine Data Input Port register */
  __ro__ uint32_t AES_DATOUT;     /* AES Engine Data Output Port register */
  __rw__ uint32_t AES0_KEY[8];    /* AES Key Word 0~7 registers for Channel 0 */
  __rw__ uint32_t AES0_IV[4];     /* AES Initial Vector Word 0~3 registers for Channel 0 */
  __rw__ uint32_t AES0_SADDR;     /* AES DMA Source Address register for Channel 0 */
  __rw__ uint32_t AES0_DADDR;     /* AES DMA Destination Address register for Channel 0 */
  __rw__ uint32_t AES0_CNT;       /* AES Byte Count register for Channel 0 */
  __rw__ uint32_t AES1_KEY[8];    /* AES Key Word 0~7 registers for Channel 1 */
  __rw__ uint32_t AES1_IV[4];     /* AES Initial Vector Word 0~3 registers for Channel 1 */
  __rw__ uint32_t AES1_SADDR;     /* AES DMA Source Address register for Channel 1 */
  __rw__ uint32_t AES1_DADDR;     /* AES DMA Destination Address register for Channel 1 */
  __rw__ uint32_t AES1_CNT;       /* AES Byte Count register for Channel 1 */
  __rw__ uint32_t AES2_KEY[8];    /* AES Key Word 0~7 registers for Channel 2 */
  __rw__ uint32_t AES2_IV[4];     /* AES Initial Vector Word 0~3 registers for Channel 2 */
  __rw__ uint32_t AES2_SADDR;     /* AES DMA Source Address register for Channel 2 */
  __rw__ uint32_t AES2_DADDR;     /* AES DMA Destination Address register for Channel 2 */
  __rw__ uint32_t AES2_CNT;       /* AES Byte Count register for Channel 2 */
  __rw__ uint32_t AES3_KEY[8];    /* AES Key Word 0~7 registers for Channel 3 */
  __rw__ uint32_t AES3_IV[4];     /* AES Initial Vector Word 0~3 registers for Channel 3 */
  __rw__ uint32_t AES3_SADDR;     /* AES DMA Source Address register for Channel 3 */
  __rw__ uint32_t AES3_DADDR;     /* AES DMA Destination Address register for Channel 3 */
  __rw__ uint32_t AES3_CNT;       /* AES Byte Count register for Channel 3 */
  __rw__ uint32_t TDES_CTL;       /* TDES/DES Control register */
  __ro__ uint32_t TDES_STS;       /* TDES/DES Engine Flag */
  __rw__ uint32_t TDES0_KEY1H;    /* TDES/DES Key 1 High Word register for Channel 0 */
  __rw__ uint32_t TDES0_KEY1L;    /* TDES/DES Key 1 Low Word register for Channel 0 */
  __rw__ uint32_t TDES0_KEY2H;    /* TDES Key 2 High Word register for Channel 0 */
  __rw__ uint32_t TDES0_KEY2L;    /* TDES Key 2 Low Word register for Channel 0 */
  __rw__ uint32_t TDES0_KEY3H;    /* TDES Key 3 High Word register for Channel 0 */
  __rw__ uint32_t TDES0_KEY3L;    /* TDES Key 3 Low Word register for Channel 0 */
  __rw__ uint32_t TDES0_IVH;      /* TDES/DES Initial Vector High Word register for Channel 0 */
  __rw__ uint32_t TDES0_IVL;      /* TDES/DES Initial Vector Low Word register for Channel 0 */
  __rw__ uint32_t TDES0_SA;       /* TDES/DES DMA Source Address register for Channel 0 */
  __rw__ uint32_t TDES0_DA;       /* TDES/DES DMA Destination Address register for Channel 0 */
  __rw__ uint32_t TDES0_CNT;      /* TDES/DES Byte Count register for Channel 0 */
  __rw__ uint32_t TDES_DATIN;     /* TDES/DES Engine Input data Word register */
  __ro__ uint32_t TDES_DATOUT;    /* TDES/DES Engine Output data Word register */
  __ne__ uint32_t RESERVED2[3];
  __rw__ uint32_t TDES1_KEY1H;    /* TDES/DES Key 1 High Word register for Channel 1 */
  __rw__ uint32_t TDES1_KEY1L;    /* TDES/DES Key 1 Low Word register for Channel 1 */
  __rw__ uint32_t TDES1_KEY2H;    /* TDES Key 2 High Word register for Channel 1 */
  __rw__ uint32_t TDES1_KEY2L;    /* TDES Key 2 Low Word register for Channel 1 */
  __rw__ uint32_t TDES1_KEY3H;    /* TDES Key 3 High Word register for Channel 1 */
  __rw__ uint32_t TDES1_KEY3L;    /* TDES Key 3 Low Word register for Channel 1 */
  __rw__ uint32_t TDES1_IVH;      /* TDES/DES Initial Vector High Word register for Channel 1 */
  __rw__ uint32_t TDES1_IVL;      /* TDES/DES Initial Vector Low Word register for Channel 1 */
  __rw__ uint32_t TDES1_SA;       /* TDES/DES DMA Source Address register for Channel 1 */
  __rw__ uint32_t TDES1_DA;       /* TDES/DES DMA Destination Address register for Channel 1 */
  __rw__ uint32_t TDES1_CNT;      /* TDES/DES Byte Count register for Channel 1 */
  __ne__ uint32_t RESERVED3[5];
  __rw__ uint32_t TDES2_KEY1H;    /* TDES/DES Key 1 High Word register for Channel 2 */
  __rw__ uint32_t TDES2_KEY1L;    /* TDES/DES Key 1 Low Word register for Channel 2 */
  __rw__ uint32_t TDES2_KEY2H;    /* TDES Key 2 High Word register for Channel 2 */
  __rw__ uint32_t TDES2_KEY2L;    /* TDES Key 2 Low Word register for Channel 2 */
  __rw__ uint32_t TDES2_KEY3H;    /* TDES Key 3 High Word register for Channel 2 */
  __rw__ uint32_t TDES2_KEY3L;    /* TDES Key 3 Low Word register for Channel 2 */
  __rw__ uint32_t TDES2_IVH;      /* TDES/DES Initial Vector High Word register for Channel 2 */
  __rw__ uint32_t TDES2_IVL;      /* TDES/DES Initial Vector Low Word register for Channel 2 */
  __rw__ uint32_t TDES2_SA;       /* TDES/DES DMA Source Address register for Channel 2 */
  __rw__ uint32_t TDES2_DA;       /* TDES/DES DMA Destination Address register for Channel 2 */
  __rw__ uint32_t TDES2_CNT;      /* TDES/DES Byte Count register for Channel 2 */
  __ne__ uint32_t RESERVED4[5];
  __rw__ uint32_t TDES3_KEY1H;    /* TDES/DES Key 1 High Word register for Channel 3 */
  __rw__ uint32_t TDES3_KEY1L;    /* TDES/DES Key 1 Low Word register for Channel 3 */
  __rw__ uint32_t TDES3_KEY2H;    /* TDES Key 2 High Word register for Channel 3 */
  __rw__ uint32_t TDES3_KEY2L;    /* TDES Key 2 Low Word register for Channel 3 */
  __rw__ uint32_t TDES3_KEY3H;    /* TDES Key 3 High Word register for Channel 3 */
  __rw__ uint32_t TDES3_KEY3L;    /* TDES Key 3 Low Word register for Channel 3 */
  __rw__ uint32_t TDES3_IVH;      /* TDES/DES Initial Vector High Word register for Channel 3 */
  __rw__ uint32_t TDES3_IVL;      /* TDES/DES Initial Vector Low Word register for Channel 3 */
  __rw__ uint32_t TDES3_SA;       /* TDES/DES DMA Source Address register for Channel 3 */
  __rw__ uint32_t TDES3_DA;       /* TDES/DES DMA Destination Address register for Channel 3 */
  __rw__ uint32_t TDES3_CNT;      /* TDES/DES Byte Count register for Channel 3 */
  __ne__ uint32_t RESERVED5[3];
  __rw__ uint32_t HMAC_CTL;       /* SHA/HMAC Control register */
  __ro__ uint32_t HMAC_STS;       /* SHA/HMAC Status Flag */
  __ro__ uint32_t HMAC_DGST[16];  /* SHA/HMAC Digest Message 0~15 */
  __rw__ uint32_t HMAC_KEYCNT;    /* SHA/HMAC Key Byte Count register */
  __rw__ uint32_t HMAC_SADDR;     /* SHA/HMAC DMA Source Address register */
  __rw__ uint32_t HMAC_DMACNT;    /* SHA/HMAC Byte Count register */
  __rw__ uint32_t HMAC_DATIN;     /* SHA/HMAC Engine Non-DMA Mode Data Input Port register */
  __ne__ uint32_t RESERVED6[298];
  __rw__ uint32_t ECC_CTL;        /* ECC Control register */
  __ro__ uint32_t ECC_STS;        /* ECC Status register */
  __rw__ uint32_t ECC_X1[18];     /* ECC The X-coordinate word 0~17 of the first point */
  __rw__ uint32_t ECC_Y1[18];     /* ECC The Y-coordinate word 0~17 of the first point */
  __rw__ uint32_t ECC_X2[18];     /* ECC The X-coordinate word 0~17 of the second point */
  __rw__ uint32_t ECC_Y2[18];     /* ECC The Y-coordinate word 0~17 of the second point */
  __rw__ uint32_t ECC_A[18];      /* ECC The parameter CURVEA word 0~17 of elliptic curve */
  __rw__ uint32_t ECC_B[18];      /* ECC The parameter CURVEB word 0~17 of elliptic curve */
  __rw__ uint32_t ECC_N[18];      /* ECC The parameter CURVEN word 0~17 of elliptic curve */
  __wo__ uint32_t ECC_K[18];      /* ECC The scalar SCALARK word 0~17 of point multiplication */
  __rw__ uint32_t ECC_SADDR;      /* ECC DMA Source Address register */
  __rw__ uint32_t ECC_DADDR;      /* ECC DMA Destination Address register */
  __rw__ uint32_t ECC_STARTREG;   /* ECC Starting Address of Updated registers */
  __rw__ uint32_t ECC_WORDCNT;    /* ECC DMA Word Count */
} NM_CRYPTO_Type;
/*------------------Digital to Analog Converter-------------------------------*/
typedef struct
{
  __rw__ uint32_t CTL; /* Control register */
  __rw__ uint32_t SWTRG; /* Software Trigger Control register */
  __rw__ uint32_t DAT; /* Data Holding register */
  __ro__ uint32_t DATOUT; /* Data Output register */
  __rw__ uint32_t STATUS; /* Status register */
  __rw__ uint32_t TCTL; /* Timing Control register */
} NM_DAC_Type;
/*------------------Enhanced Analog to Digital Converter----------------------*/
typedef struct
{
  __ro__ uint32_t DAT[19]; /* Data registers 0~18 for Sample Modules 0~18 */
  __ro__ uint32_t CURDAT; /* PDMA Current Transfer Data register */
  __rw__ uint32_t CTL; /* Control register */
  __wo__ uint32_t SWTRG; /* Sample Module Software Start register */
  __rw__ uint32_t PENDSTS; /* Start of Conversion Pending Flag register */
  __rw__ uint32_t OVSTS; /* Sample Module Overrun Flag register */
  __ne__ uint32_t RESERVED0[8];
  __rw__ uint32_t SCTL[19]; /* Sample Module 0~18 Control registers */
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t INTSRC[4]; /* Interrupt 0~3 Source Enable Control registers */
  __rw__ uint32_t CMP[4]; /* Result Compare registers 0~3 */
  __ro__ uint32_t STATUS0; /* Status register 0 */
  __ro__ uint32_t STATUS1; /* Status register 1 */
  __rw__ uint32_t STATUS2; /* Status register 2 */
  __ro__ uint32_t STATUS3; /* Status register 3 */
  __ro__ uint32_t DDAT[4]; /* Double Data registers 0~3 */
  __rw__ uint32_t PWRM; /* Power Management register */
  __rw__ uint32_t CALCTL; /* Calibration Control register */
  __rw__ uint32_t CALDWRD; /* Calibration Load Word register */
  __ne__ uint32_t RESERVED2[5];
  __rw__ uint32_t PDMACTL; /* PDMA Control register */
} NM_EADC_Type;
/*------------------External Bus Interface------------------------------------*/
typedef struct
{
  __rw__ uint32_t CTL0; /* Bank 0 Control register */
  __rw__ uint32_t TCTL0; /* Bank 0 Timing Control register */
  __ne__ uint32_t RESERVED0[2];
  __rw__ uint32_t CTL1; /* Bank 1 Control register */
  __rw__ uint32_t TCTL1; /* Bank 1 Timing Control register */
  __ne__ uint32_t RESERVED1[2];
  __rw__ uint32_t CTL2; /* Bank 2 Control register */
  __rw__ uint32_t TCTL2; /* Bank 2 Timing Control register */
} NM_EBI_Type;
/*------------------Enhanced Input Capture Timer------------------------------*/
typedef struct
{
  __rw__ uint32_t CNT; /* Input Capture Counter */
  __rw__ uint32_t HLD0; /* Input Capture Hold register 0 */
  __rw__ uint32_t HLD1; /* Input Capture Hold register 1 */
  __rw__ uint32_t HLD2; /* Input Capture Hold register 2 */
  __rw__ uint32_t CNTCMP; /* Input Capture Compare register */
  __rw__ uint32_t CTL0; /* Input Capture Control register 0 */
  __rw__ uint32_t CTL1; /* Input Capture Control register 1 */
  __rw__ uint32_t STATUS; /* Input Capture Status register */
} NM_ECAP_Type;
/*------------------Ethernet MAC Controller-----------------------------------*/
typedef struct
{
  // TODO Def length
  __rw__ uint32_t CAMCTL;     /* CAM Comparison Control register */
  __rw__ uint32_t CAMEN;      /* CAM Enable register */
  __rw__ uint32_t CAM0M;      /* CAM0 Most Significant Word register */
  __rw__ uint32_t CAM0L;      /* CAM0 Least Significant Word register */
  __rw__ uint32_t CAM1M;      /* CAM1 Most Significant Word register */
  __rw__ uint32_t CAM1L;      /* CAM1 Least Significant Word register */
  __rw__ uint32_t CAM2M;      /* CAM2 Most Significant Word register */
  __rw__ uint32_t CAM2L;      /* CAM2 Least Significant Word register */
  __rw__ uint32_t CAM3M;      /* CAM3 Most Significant Word register */
  __rw__ uint32_t CAM3L;      /* CAM3 Least Significant Word register */
  __rw__ uint32_t CAM4M;      /* CAM4 Most Significant Word register */
  __rw__ uint32_t CAM4L;      /* CAM4 Least Significant Word register */
  __rw__ uint32_t CAM5M;      /* CAM5 Most Significant Word register */
  __rw__ uint32_t CAM5L;      /* CAM5 Least Significant Word register */
  __rw__ uint32_t CAM6M;      /* CAM6 Most Significant Word register */
  __rw__ uint32_t CAM6L;      /* CAM6 Least Significant Word register */
  __rw__ uint32_t CAM7M;      /* CAM7 Most Significant Word register */
  __rw__ uint32_t CAM7L;      /* CAM7 Least Significant Word register */
  __rw__ uint32_t CAM8M;      /* CAM8 Most Significant Word register */
  __rw__ uint32_t CAM8L;      /* CAM8 Least Significant Word register */
  __rw__ uint32_t CAM9M;      /* CAM9 Most Significant Word register */
  __rw__ uint32_t CAM9L;      /* CAM9 Least Significant Word register */
  __rw__ uint32_t CAM10M;     /* CAM10 Most Significant Word register */
  __rw__ uint32_t CAM10L;     /* CAM10 Least Significant Word register */
  __rw__ uint32_t CAM11M;     /* CAM11 Most Significant Word register */
  __rw__ uint32_t CAM11L;     /* CAM11 Least Significant Word register */
  __rw__ uint32_t CAM12M;     /* CAM12 Most Significant Word register */
  __rw__ uint32_t CAM12L;     /* CAM12 Least Significant Word register */
  __rw__ uint32_t CAM13M;     /* CAM13 Most Significant Word register */
  __rw__ uint32_t CAM13L;     /* CAM13 Least Significant Word register */
  __rw__ uint32_t CAM14M;     /* CAM14 Most Significant Word register */
  __rw__ uint32_t CAM14L;     /* CAM14 Least Significant Word register */
  __rw__ uint32_t CAM15MSB;   /* CAM15 Most Significant Word register */
  __rw__ uint32_t CAM15LSB;   /* CAM15 Least Significant Word register */
  __rw__ uint32_t TXDSA;      /* Transmit Descriptor Link List Start Address register */
  __rw__ uint32_t RXDSA;      /* Receive Descriptor Link List Start Address register */
  __rw__ uint32_t CTL;        /* MAC Control register */
  __rw__ uint32_t MIIMDAT;    /* MII Management Data register */
  __rw__ uint32_t MIIMCTL;    /* MII Management Control and Address register */
  __rw__ uint32_t FIFOCTL;    /* FIFO Threshold Control register */
  __wo__ uint32_t TXST;       /* Transmit Start Demand register */
  __wo__ uint32_t RXST;       /* Receive Start Demand register */
  __rw__ uint32_t MRFL;       /* Maximum Receive Frame Control register */
  __rw__ uint32_t INTEN;      /* MAC Interrupt Enable register */
  __rw__ uint32_t INTSTS;     /* MAC Interrupt Status register */
  __rw__ uint32_t GENSTS;     /* MAC General Status register */
  __rw__ uint32_t MPCNT;      /* Missed Packet Count register */
  __ro__ uint32_t RPCNT;      /* MAC Receive Pause Count register */
  __ne__ uint32_t RESERVED0[2];
  __rw__ uint32_t FRSTS;      /* DMA Receive Frame Status register */
  __ro__ uint32_t CTXDSA;     /* Current Transmit Descriptor Start Address register */
  __ro__ uint32_t CTXBSA;     /* Current Transmit Buffer Start Address register */
  __ro__ uint32_t CRXDSA;     /* Current Receive Descriptor Start Address register */
  __ro__ uint32_t CRXBSA;     /* Current Receive Buffer Start Address register */
  __ne__ uint32_t RESERVED1[9];
  __rw__ uint32_t TSCTL;      /* Time Stamp Control register */
  __ne__ uint32_t RESERVED2[3];
  __ro__ uint32_t TSSEC;      /* Time Stamp Counter Second register */
  __ro__ uint32_t TSSUBSEC;   /* Time Stamp Counter Sub Second register */
  __rw__ uint32_t TSINC;      /* Time Stamp Increment register */
  __rw__ uint32_t TSADDEND;   /* Time Stamp Addend register */
  __rw__ uint32_t UPDSEC;     /* Time Stamp Update Second register */
  __rw__ uint32_t UPDSUBSEC;  /* Time Stamp Update Sub Second register */
  __rw__ uint32_t ALMSEC;     /* Time Stamp Alarm Second register */
  __rw__ uint32_t ALMSUBSEC;  /* Time Stamp Alarm Sub Second register */
} NM_EMAC_Type;
/*------------------Pulse Width Modulation Controller-------------------------*/
typedef struct
{
  __rw__ uint32_t CTL0;         /* Control register 0 */
  __rw__ uint32_t CTL1;         /* Control register 1 */
  __rw__ uint32_t SYNC;         /* Synchronization register */
  __rw__ uint32_t SWSYNC;       /* Software Control Synchronization register */
  __rw__ uint32_t CLKSRC;       /* Clock Source register */
  __rw__ uint32_t CLKPSC[3];    /* Clock Prescale registers */
  __rw__ uint32_t CNTEN;        /* Counter Enable register */
  __rw__ uint32_t CNTCLR;       /* Clear Counter register */
  __rw__ uint32_t LOAD;         /* Load register */
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t PERIOD[6];    /* Period registers 0~5 */
  __ne__ uint32_t RESERVED1[2];
  __rw__ uint32_t CMPDAT[6];    /* Comparator registers 0~5 */
  __ne__ uint32_t RESERVED2[2];
  __rw__ uint32_t DTCTL[3];     /* Dead-Time Control registers */
  __ne__ uint32_t RESERVED3;
  __rw__ uint32_t PHS[3];       /* Counter Phase registers */
  __ne__ uint32_t RESERVED4;
  __ro__ uint32_t CNT[6];       /* Counter registers 0~5 */
  __ne__ uint32_t RESERVED5[2];
  __rw__ uint32_t WGCTL0;       /* Generation register 0 */
  __rw__ uint32_t WGCTL1;       /* Generation register 1 */
  __rw__ uint32_t MSKEN;        /* Mask Enable register */
  __rw__ uint32_t MSK;          /* Mask Data register */
  __rw__ uint32_t BNF;          /* Brake Noise Filter register */
  __rw__ uint32_t FAILBRK;      /* System Fail Brake Control register */
  __rw__ uint32_t BRKCTL[3];    /* Brake Edge Detect Control registers */
  __rw__ uint32_t POLCTL;       /* Pin Polar Inverse register */
  __rw__ uint32_t POEN;         /* Output Enable register */
  __wo__ uint32_t SWBRK;        /* Software Brake Control register */
  __rw__ uint32_t INTEN0;       /* Interrupt Enable register 0 */
  __rw__ uint32_t INTEN1;       /* Interrupt Enable register 1 */
  __rw__ uint32_t INTSTS0;      /* Interrupt Flag register 0 */
  __rw__ uint32_t INTSTS1;      /* Interrupt Flag register 1 */
  __ne__ uint32_t RESERVED6;
  __rw__ uint32_t DACTRGEN;     /* Trigger DAC Enable register */
  __rw__ uint32_t EADCTS0;      /* Trigger EADC Source Select register 0 */
  __rw__ uint32_t EADCTS1;      /* Trigger EADC Source Select register 1 */
  __rw__ uint32_t FTCMPDAT[3];  /* Free Trigger Compare registers */
  __ne__ uint32_t RESERVED7;
  __rw__ uint32_t SSCTL;        /* Synchronous Start Control register */
  __wo__ uint32_t SSTRG;        /* Synchronous Start Trigger register */
  __rw__ uint32_t LEBCTL;       /* Leading Edge Blanking Control register */
  __rw__ uint32_t LEBCNT;       /* Leading Edge Blanking Counter register */
  __rw__ uint32_t STATUS;       /* Status register */
  __ne__ uint32_t RESERVED8[3];
  __rw__ uint32_t IFA[6];       /* Interrupt Flag Accumulator registers 0~5 */
  __ne__ uint32_t RESERVED9[2];
  __rw__ uint32_t AINTSTS;      /* Accumulator Interrupt Flag register */
  __rw__ uint32_t AINTEN;       /* Accumulator Interrupt Enable register */
  __rw__ uint32_t APDMACTL;     /* Accumulator PDMA Control register */
  __ne__ uint32_t RESERVED10;
  __rw__ uint32_t FDEN;         /* Fault Detect Enable register */
  __rw__ uint32_t FDCTL[6];     /* Fault Detect Control registers 0~5 */
  __rw__ uint32_t FDIEN;        /* Fault Detect Interrupt Enable register */
  __rw__ uint32_t FDSTS;        /* Fault Detect Interrupt Flag register */
  __rw__ uint32_t EADCPSCCTL;   /* Trigger EADC Prescale Control register */
  __rw__ uint32_t EADCPSC0;     /* Trigger EADC Prescale register 0 */
  __rw__ uint32_t EADCPSC1;     /* Trigger EADC Prescale register 1 */
  __rw__ uint32_t EADCPSCNT0;   /* Trigger EADC Prescale Counter register 0 */
  __rw__ uint32_t EADCPSCNT1;   /* Trigger EADC Prescale Counter register 1 */
  __ne__ uint32_t RESERVED11[26];
  __rw__ uint32_t CAPINEN;      /* Capture Input Enable register */
  __rw__ uint32_t CAPCTL;       /* Capture Control register */
  __ro__ uint32_t CAPSTS;       /* Capture Status register */

  struct
  {
    __rw__ uint32_t RCAPDAT;    /* Rising Capture Data registers 0~5 */
    __rw__ uint32_t FCAPDAT;    /* Falling Capture Data registers 0~5 */
  } CAPDAT[6];

  __rw__ uint32_t PDMACTL;      /* PDMA Control register */
  __ro__ uint32_t PDMACAP[3];   /* Capture Channel PDMA registers */
  __ne__ uint32_t RESERVED12;
  __rw__ uint32_t CAPIEN;       /* Capture Interrupt Enable register */
  __rw__ uint32_t CAPIF;        /* Capture Interrupt Flag register */
  __ne__ uint32_t RESERVED13[43];
  __ro__ uint32_t PBUF[6];      /* PERIOD0~5 Buffer */
  __ro__ uint32_t CMPBUF[6];    /* CMPDAT0~5 Buffer */
  __ro__ uint32_t CPSCBUF[3];   /* CLKPSC0_1/2_3/4_5 Buffer */
  __ro__ uint32_t FTCBUF[3];    /* FTCMPDAT0_1/2_3/4_5 Buffer */
  __rw__ uint32_t FTCI;         /* FTCMPDAT Indicator register */
} NM_EPWM_Type;
/*------------------Flash Memory Controller-----------------------------------*/
typedef struct
{
  __rw__ uint32_t ISPCTL; /* ISP Control register */
  __rw__ uint32_t ISPADDR; /* ISP Address register */
  __rw__ uint32_t ISPDAT; /* ISP Data register */
  __rw__ uint32_t ISPCMD; /* ISP Command register */
  __rw__ uint32_t ISPTRG; /* ISP Trigger Control register */
  __ro__ uint32_t DFBA; /* Data Flash Base Address */
  __ne__ uint32_t RESERVED0[10];
  __rw__ uint32_t ISPSTS; /* ISP Status register */
  __ne__ uint32_t RESERVED1[2];
  __rw__ uint32_t CYCCTL; /* Flash Access Cycle Control register */
  __wo__ uint32_t KPKEY0; /* KPROM KEY0 Data register */
  __wo__ uint32_t KPKEY1; /* KPROM KEY1 Data register */
  __wo__ uint32_t KPKEY2; /* KPROM KEY2 Data register */
  __rw__ uint32_t KPKEYTRG; /* KPROM KEY Comparison Trigger Control register */
  __rw__ uint32_t KPKEYSTS; /* KPROM KEY Comparison Status register */
  __ro__ uint32_t KPKEYCNT; /* KPROM KEY-Unmatched Counting register */
  __ro__ uint32_t KPCNT; /* KPROM KEY-Unmatched Power-On Counting register */
  __ne__ uint32_t RESERVED2[5];
  __rw__ uint32_t MPDAT0; /* ISP Data 0 register */
  __rw__ uint32_t MPDAT1; /* ISP Data 1 register */
  __rw__ uint32_t MPDAT2; /* ISP Data 2 register */
  __rw__ uint32_t MPDAT3; /* ISP Data 3 register */
  __ne__ uint32_t RESERVED3[12];
  __ro__ uint32_t MPSTS; /* ISP Multi-Program Status register */
  __ro__ uint32_t MPADDR; /* ISP Multi-Program Address register */
  __ne__ uint32_t RESERVED4[2];
  __ro__ uint32_t XOMR0STS; /* XOM Region 0 Status register */
  __ro__ uint32_t XOMR1STS; /* XOM Region 1 Status register */
  __ro__ uint32_t XOMR2STS; /* XOM Region 2 Status register */
  __ro__ uint32_t XOMR3STS; /* XOM Region 3 Status register */
  __ro__ uint32_t XOMSTS; /* XOM Status register */
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
  __rw__ uint32_t MODE; /* I/O Mode Control */
  __rw__ uint32_t DINOFF; /* Digital Input Path Disable Control */
  __rw__ uint32_t DOUT; /* Data Output Value */
  __rw__ uint32_t DATMSK; /* Data Output Write Mask */
  __ro__ uint32_t PIN; /* Pin Value */
  __rw__ uint32_t DBEN; /* Debounce Enable Control register */
  __rw__ uint32_t INTTYPE; /* Interrupt Trigger Type Control */
  __rw__ uint32_t INTEN; /* Interrupt Enable Control register */
  __rw__ uint32_t INTSRC; /* Interrupt Source Flag */
  __rw__ uint32_t SMTEN; /* Input Schmitt Trigger Enable register */
  __rw__ uint32_t SLEWCTL; /* High Slew Rate Control register */
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t PUSEL; /* Pull-up and Pull-down Enable register */
  __ne__ uint32_t RESERVED1[3];
} NM_GPIO_Type;
/*------------------USB 2.0 High-Speed Device Controller----------------------*/
typedef struct
{
  __rw__ uint32_t CTL; /* Control register */
  __rw__ uint32_t PHYCTL; /* PHY Control register */
  __rw__ uint32_t INTEN; /* Interrupt Enable register */
  __rw__ uint32_t INTSTS; /* Interrupt Status register */
  __ro__ uint32_t STATUS; /* Status register */
} NM_HSOTG_Type;

typedef struct
{
  union
  {
    __rw__ uint32_t EPDAT; /* Data register */

    struct
    {
      __rw__ uint8_t EPDAT_B;
      __ne__ uint8_t RESERVED0[3];
    };
  };

  __rw__ uint32_t EPINTSTS; /* Interrupt Status register */
  __rw__ uint32_t EPINTEN; /* Interrupt Enable register */
  __ro__ uint32_t EPDATCNT; /* Data Available Count register */
  __rw__ uint32_t EPRSPCTL; /* Response Control register */
  __rw__ uint32_t EPMPS; /* Maximum Packet Size register */
  __rw__ uint32_t EPTXCNT; /* Transfer Count register */
  __rw__ uint32_t EPCFG; /* Configuration register */
  __rw__ uint32_t EPBUFST; /* RAM Start Address register */
  __rw__ uint32_t EPBUFEND; /* RAM End Address register */
} NM_HSUSBD_EP_Type;

typedef struct
{
  __ro__ uint32_t GINTSTS; /* Global Interrupt Status register */
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t GINTEN; /* Global Interrupt Enable register */
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t BUSINTSTS; /* Bus Interrupt Status register */
  __rw__ uint32_t BUSINTEN; /* Bus Interrupt Enable register */
  __rw__ uint32_t OPER; /* Operational register */
  __ro__ uint32_t FRAMECNT; /* Frame Count register */
  __rw__ uint32_t FADDR; /* Function Address register */
  __rw__ uint32_t TEST; /* Test Mode register */

  /* Offset 0x028 */
  __rw__ uint32_t CEPDAT; /* Control-Endpoint Data Buffer */
  __rw__ uint32_t CEPCTL; /* Control-Endpoint Control register */
  __rw__ uint32_t CEPINTEN; /* Control-Endpoint Interrupt Enable */
  __rw__ uint32_t CEPINTSTS; /* Control-Endpoint Interrupt Status */
  __rw__ uint32_t CEPTXCNT; /* Control-Endpoint In-transfer Data Count */
  __ro__ uint32_t CEPRXCNT; /* Control-Endpoint Out-transfer Data Count */
  __ro__ uint32_t CEPDATCNT; /* Control-Endpoint data count */

  union
  {
    struct
    {
      __ro__ uint32_t SETUP1_0; /* Setup 1 & Setup 0 bytes */
      __ro__ uint32_t SETUP3_2; /* Setup 3 & Setup 2 bytes */
      __ro__ uint32_t SETUP5_4; /* Setup 5 & Setup 4 bytes */
      __ro__ uint32_t SETUP7_6; /* Setup 7 & Setup 6 bytes */
    };

    __ro__ uint32_t SETUP[4];
  };

  __rw__ uint32_t CEPBUFST; /* Control Endpoint RAM Start Address register */
  __rw__ uint32_t CEPBUFEND; /* Control Endpoint RAM End Address register */

  /* Offset 0x05C */
  __rw__ uint32_t DMACTL; /* DMA Control Status register */
  __rw__ uint32_t DMACNT; /* DMA Count register */

  /* Offset 0x064 */
  NM_HSUSBD_EP_Type EP[12];
  __ne__ uint32_t RESERVED2[303];

  /* Offset 0x700 */
  __rw__ uint32_t DMAADDR; /* AHB DMA Address register */
  __rw__ uint32_t PHYCTL; /* USB PHY Control register */
} NM_HSUSBD_Type;

typedef struct
{
  __ro__ uint32_t EHCVNR; /* EHCI Version Number register */
  __ro__ uint32_t EHCSPR; /* EHCI Structural Parameters register */
  __ro__ uint32_t EHCCPR; /* EHCI Capability Parameters register */
  __ne__ uint32_t RESERVED0[5];
  __rw__ uint32_t UCMDR; /* Command register */
  __rw__ uint32_t USTSR; /* Status register */
  __rw__ uint32_t UIENR; /* Interrupt Enable register */
  __rw__ uint32_t UFINDR; /* Frame Index register */
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t UPFLBAR; /* Periodic Frame List Base Address register */
  __rw__ uint32_t UCALAR; /* Current Asynchronous List Address register */
  __rw__ uint32_t UASSTR; /* Asynchronous Schedule Sleep Timer register */
  __ne__ uint32_t RESERVED2[8];
  __rw__ uint32_t UCFGR; /* Configure Flag register */
  __rw__ uint32_t UPSCR[2]; /* Port 0 & 1 Status and Control register */
  __ne__ uint32_t RESERVED3[22];
  __rw__ uint32_t USBPCR0; /* PHY 0 Control register */
  __rw__ uint32_t USBPCR1; /* PHY 1 Control register */
} NM_HSUSBH_Type;
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
/*------------------I2S Interface Controller----------------------------------*/
typedef struct
{
  __rw__ uint32_t CTL0; /* Control register 0 */
  __rw__ uint32_t CLKDIV; /* Clock Divider register */
  __rw__ uint32_t IEN; /* Interrupt Enable register */
  __rw__ uint32_t STATUS0; /* Status register 0 */
  __wo__ uint32_t TXFIFO; /* Transmit FIFO register */
  __ro__ uint32_t RXFIFO; /* Receive FIFO register */
  __ne__ uint32_t RESERVED0[2];
  __rw__ uint32_t CTL1; /* Control register 1 */
  __rw__ uint32_t STATUS1; /* Status register 1 */
} NM_I2S_Type;
/*------------------Non-Maskable Interrupt------------------------------------*/
typedef struct
{
  __rw__ uint32_t NMIEN; /* Source Interrupt Enable register */
  __ro__ uint32_t NMISTS; /* Source Interrupt Status register */
  __ne__ uint32_t RESERVED0[62];

  /* Offset 0x100 */
  __rw__ uint32_t AHBMCTL; /* AHB Bus Matrix Priority Control register */
} NM_NMI_Type;
/*------------------Operational Amplifier-------------------------------------*/
typedef struct
{
  __rw__ uint32_t CTL; /* Control register */
  __rw__ uint32_t STATUS; /* Status register */
  __rw__ uint32_t CALCTL; /* Calibration Control register */
  __ro__ uint32_t CALST; /* Calibration Status register */
} NM_OPA_Type;
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
  NM_PDMA_CHANNEL_Type CHANNELS[16]; /* Control registers of PDMA channels */

  /* Offset 0x100 */
  __ro__ uint32_t CURSCAT[16]; /* Current descriptor addresses of channels */
  __ne__ uint32_t RESERVED0[176];

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
      __rw__ uint32_t REQSEL8_11; /* Request Source Select register 2 */
      __rw__ uint32_t REQSEL12_15; /* Request Source Select register 3 */
    };

    __rw__ uint32_t REQSEL[4]; /* Request Source Select register */
  };
  __ne__ uint32_t RESERVED4[28];

  /* Offset 0x500 */
  struct
  {
    __rw__ uint32_t STCR; /* Stride Transfer Count register */
    __rw__ uint32_t ASOCR; /* Address Stride Offset register */
  } STRIDE[6];
  __ne__ uint32_t RESERVED5[52];

  /* Offset 0x600 */
  struct
  {
    __rw__ uint32_t AICTL; /* Address Interval Control register */
    __rw__ uint32_t RCNT; /* Repeat Count register */
  } REPEAT[2];
} NM_PDMA_Type;
/*------------------Quadrature Encoder Interface------------------------------*/
typedef struct
{
  __rw__ uint32_t CNT; /* Counter register */
  __rw__ uint32_t CNTHOLD; /* Counter Hold register */
  __rw__ uint32_t CNTLATCH; /* Counter Index Latch register */
  __rw__ uint32_t CNTCMP; /* Counter Compare register */
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t CNTMAX; /* Preset Maximum Count register */
  __rw__ uint32_t CTL; /* Controller Control register */
  __ne__ uint32_t RESERVED1[4];
  __rw__ uint32_t STATUS; /* Controller Status register */
} NM_QEI_Type;
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
/*------------------Real Timer Clock------------------------------------------*/
typedef struct
{
  __rw__ uint32_t INIT; /* Initiation register */
  __rw__ uint32_t RWEN; /* Access Enable register */
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
  __rw__ uint32_t SPRCTL; /* Spare Functional Control register */
  __rw__ uint32_t SPR[20]; /* Spare registers 0~19 */
  __ne__ uint32_t RESERVED0[28];
  __rw__ uint32_t LXTCTL; /* 32.768 kHz Oscillator Control register */
  __rw__ uint32_t GPIOCTL0; /* GPIO Control 0 register */
  __rw__ uint32_t GPIOCTL1; /* GPIO Control 1 register */
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t DSTCTL; /* Daylight Saving Time Control register */
  __ne__ uint32_t RESERVED2[3];
  __rw__ uint32_t TAMPCTL; /* Tamper Pin Control register */
  __ne__ uint32_t RESERVED3;
  __rw__ uint32_t TAMPSEED; /* Tamper Dynamic Seed register */
  __ne__ uint32_t RESERVED4;
  __ro__ uint32_t TAMPTIME; /* Tamper Time register */
  __ro__ uint32_t TAMPCAL; /* Tamper Calendar register */
} NM_RTC_Type;
/*------------------Smart Card Host Interface Controller----------------------*/
typedef struct
{
  __rw__ uint32_t DAT; /* Receive/Transmit Holding Buffer register */
  __rw__ uint32_t CTL; /* Control register */
  __rw__ uint32_t ALTCTL; /* Alternate Control register */
  __rw__ uint32_t EGT; /* Extra Guard Time register */
  __rw__ uint32_t RXTOUT; /* Receive Buffer Time-out Counter register */
  __rw__ uint32_t ETUCTL; /* Element Time Unit Control register */
  __rw__ uint32_t INTEN; /* Interrupt Enable Control register */
  __rw__ uint32_t INTSTS; /* Interrupt Status register */
  __rw__ uint32_t STATUS; /* Transfer Status register */
  __rw__ uint32_t PINCTL; /* Pin Control State register */
  __rw__ uint32_t TMRCTL0; /* Internal Timer0 Control register */
  __rw__ uint32_t TMRCTL1; /* Internal Timer1 Control register */
  __rw__ uint32_t TMRCTL2; /* Internal Timer2 Control register */
  __rw__ uint32_t UARTCTL; /* UART Mode Control register */
  __ne__ uint32_t RESERVED0[5];
  __rw__ uint32_t ACTCTL; /* Activation Control register */
} NM_SC_Type;
/*------------------SD Card Host Interface------------------------------------*/
typedef struct
{
  __rw__ uint32_t FB[32]; /* Shared Buffer (FIFO) */
  __ne__ uint32_t RESERVED0[224];
  __rw__ uint32_t DMACTL; /* DMA Control and Status register */
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t DMASA; /* DMA Transfer Starting Address register */
  __ro__ uint32_t DMABCNT; /* DMA Transfer Byte Count register */
  __rw__ uint32_t DMAINTEN; /* DMA Interrupt Enable Control register */
  __rw__ uint32_t DMAINTSTS; /* DMA Interrupt Status register */
  __ne__ uint32_t RESERVED2[250];
  __rw__ uint32_t GCTL; /* Global Control and Status register */
  __rw__ uint32_t GINTEN; /* Global Interrupt Control register */
  __rw__ uint32_t GINTSTS; /* Global Interrupt Status register */
  __ne__ uint32_t RESERVED3[5];
  __rw__ uint32_t CTL; /* SD Control and Status register */
  __rw__ uint32_t CMDARG; /* SD Command Argument register */
  __rw__ uint32_t INTEN; /* SD Interrupt Control register */
  __rw__ uint32_t INTSTS; /* SD Interrupt Status register */
  __ro__ uint32_t RESP0; /* SD Receiving Response Token register 0 */
  __ro__ uint32_t RESP1; /* SD Receiving Response Token register 1 */
  __rw__ uint32_t BLEN; /* SD Block Length register */
  __rw__ uint32_t TOUT; /* SD Response/Data-in Time-out register */
} NM_SDH_Type;
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
/*------------------Serial Peripheral Interface Controller Master Mode--------*/
typedef struct
{
  __rw__ uint32_t CTL0; /* Control and Status register 0 */
  __rw__ uint32_t CTL1; /* Control register 1 */
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t RXCLKDLY; /* RX Clock Delay Control register */
  __ro__ uint32_t RX[4]; /* Data Receive registers 0~3 */
  __rw__ uint32_t TX[4]; /* Data Transmit registers 0~3 */
  __rw__ uint32_t SRAMADDR; /* SRAM Memory Address register */
  __rw__ uint32_t DMACNT; /* DMA Transfer Byte Count register */
  __rw__ uint32_t FADDR; /* SPI Flash Address register */
  __wo__ uint32_t KEY1; /* Cipher Key1 register */
  __wo__ uint32_t KEY2; /* Cipher Key2 register */
  __rw__ uint32_t DMMCTL; /* Direct Memory Mapping Mode Control register */
  __rw__ uint32_t CTL2; /* Control register 2 */
} NM_SPIM_Type;
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
  __rw__ uint32_t IVSCTL; /* Internal Voltage Source Control register */
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t PORCTL; /* Power-On-Reset Controller register */
  __rw__ uint32_t VREFCTL; /* VREF Control register */
  __rw__ uint32_t USBPHY; /* USB PHY Control register */

  /* Offset 0x030 */
  struct
  {
    __rw__ uint32_t MFPL; /* GPIO Low Byte Function control register */
    __rw__ uint32_t MFPH; /* GPIO High Byte Function control register */
  } GP[8];
  __ne__ uint32_t RESERVED2[4];

  /* Offset 0x080 */
  __rw__ uint32_t GP_MFOS[8]; /* Function Output Select register */
  __ne__ uint32_t RESERVED3[8];

  /* Offset 0x0C0 */
  __rw__ uint32_t SRAM_INTCTL; /* System SRAM Interrupt Enable Control register */
  __rw__ uint32_t SRAM_STATUS; /* System SRAM Parity Error Status register */
  __ro__ uint32_t SRAM_ERRADDR; /* System SRAM Parity Check Error Address register */
  __ne__ uint32_t RESERVED4[1];
  __rw__ uint32_t SRAM_BISTCTL; /* System SRAM BIST Test Control register */
  __ro__ uint32_t SRAM_BISTSTS; /* System SRAM BIST Test Status register */
  __ne__ uint32_t RESERVED5[3];
  __rw__ uint32_t HIRCTCTL; /* HIRC48M Trim Control register */
  __rw__ uint32_t HIRCTIEN; /* HIRC48M Trim Interrupt Enable register */
  __rw__ uint32_t HIRCTISTS; /* HIRC48M Trim Interrupt Status register */
  __rw__ uint32_t IRCTCTL; /* HIRC Trim Control register */
  __rw__ uint32_t IRCTIEN; /* HIRC Trim Interrupt Enable register */
  __rw__ uint32_t IRCTISTS; /* HIRC Trim Interrupt Status register */
  __ne__ uint32_t RESERVED6;

  /* Offset 0x100 */
  __rw__ uint32_t REGLCTL; /* Register Lock Control register */
  __ne__ uint32_t RESERVED7[58];
  __rw__ uint32_t PORDISAN; /* Analog POR Disable Control register */
  __ne__ uint32_t RESERVED8;
  __ro__ uint32_t CSERVER; /* Chip Series Version register */
  __rw__ uint32_t PLCTL; /* Power Level Control register */
  __ro__ uint32_t PLSTS; /* Power Level Status register */
} NM_SYS_Type;
/*------------------Timer Controller------------------------------------------*/
typedef struct
{
  __rw__ uint32_t CTL;        /* Timer Control register */
  __rw__ uint32_t CMP;        /* Timer Comparator register */
  __rw__ uint32_t INTSTS;     /* Timer Interrupt Status register */
  __rw__ uint32_t CNT;        /* Timer Data register */
  __ro__ uint32_t CAP;        /* Timer Capture Data register */
  __rw__ uint32_t EXTCTL;     /* Timer External Control register */
  __rw__ uint32_t EINTSTS;    /* Timer External Interrupt Status register */
  __rw__ uint32_t TRGCTL;     /* Timer Trigger Control register */
  __rw__ uint32_t ALTCTL;     /* Timer Alternative Control register */
  __ne__ uint32_t RESERVED0[7];
  __rw__ uint32_t PWMCTL;     /* PWM Control register */
  __rw__ uint32_t PWMCLKSRC;  /* PWM Counter Clock Source register */
  __rw__ uint32_t PWMCLKPSC;  /* PWM Counter Clock Pre-scale register */
  __rw__ uint32_t PWMCNTCLR;  /* PWM Clear Counter register */
  __rw__ uint32_t PWMPERIOD;  /* PWM Period register */
  __rw__ uint32_t PWMCMPDAT;  /* PWM Comparator register */
  __rw__ uint32_t PWMDTCTL;   /* PWM Dead-Time Control register */
  __ro__ uint32_t PWMCNT;     /* PWM Counter register */
  __rw__ uint32_t PWMMSKEN;   /* PWM Output Mask Enable register */
  __rw__ uint32_t PWMMSK;     /* PWM Output Mask Data Control register */
  __rw__ uint32_t PWMBNF;     /* PWM Brake Pin Noise Filter register */
  __rw__ uint32_t PWMFAILBRK; /* PWM System Fail Brake Control register */
  __rw__ uint32_t PWMBRKCTL;  /* PWM Brake Control register */
  __rw__ uint32_t PWMPOLCTL;  /* PWM Pin Output Polar Control register */
  __rw__ uint32_t PWMPOEN;    /* PWM Pin Output Enable register */
  __wo__ uint32_t PWMSWBRK;   /* PWM Software Trigger Brake Control register */
  __rw__ uint32_t PWMINTEN0;  /* PWM Interrupt Enable Register 0 */
  __rw__ uint32_t PWMINTEN1;  /* PWM Interrupt Enable Register 1 */
  __rw__ uint32_t PWMINTSTS0; /* PWM Interrupt Status Register 0 */
  __rw__ uint32_t PWMINTSTS1; /* PWM Interrupt Status Register 1 */
  __rw__ uint32_t PWMEADCTS;  /* PWM EADC Trigger Source Select register */
  __rw__ uint32_t PWMSCTL;    /* PWM Synchronous Control register */
  __wo__ uint32_t PWMSTRG;    /* PWM Synchronous Trigger register */
  __rw__ uint32_t PWMSTATUS;  /* PWM Status register */
  __ro__ uint32_t PWMPBUF;    /* PWM Period Buffer register */
  __ro__ uint32_t PWMCMPBUF;  /* PWM Comparator Buffer register */
} NM_TIMER_Type;
/*------------------True Random Number Generator------------------------------*/
typedef struct
{
  __rw__ uint32_t CTL; /* Control and Status register */
  __ro__ uint32_t DATA; /* Data register */
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t ACT; /* Activation register */
} NM_TRNG_Type;
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
  __rw__ uint32_t LINCTL; /* LIN Control register */
  __rw__ uint32_t LINSTS; /* LIN Status register */
  __rw__ uint32_t BRCOMP; /* Baud Rate Compensation register */
  __rw__ uint32_t WKCTL; /* Wake-up Control register */
  __rw__ uint32_t WKSTS; /* Wake-up Status register */
  __rw__ uint32_t DWKCOMP; /* Incoming Data Wake-up Compensation register */
} NM_UART_Type;
/*------------------USB 2.0 Full-Speed Device Controller----------------------*/
typedef struct
{
  __rw__ uint32_t CTL; /* Control register */
  __rw__ uint32_t PHYCTL; /* PHY Control register */
  __rw__ uint32_t INTEN; /* Interrupt Enable register */
  __rw__ uint32_t INTSTS; /* Interrupt Status register */
  __ro__ uint32_t STATUS; /* Status register */
} NM_OTG_Type;

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
  __ro__ uint32_t EPSTS1; /* Endpoint Status register 1 */
  __ne__ uint32_t RESERVED1[24];
  __ro__ uint32_t LPMATTR; /* LPM Attribution register */
  __ro__ uint32_t FN; /* Frame number register */
  __rw__ uint32_t SE0; /* Device Drive SE0 Control register */
  __ne__ uint32_t RESERVED2[27];

  /* Offset 0x100 */
  __rw__ uint8_t SRAM[1024];

  /* Offset 0x500 */
  NM_USBD_EP_Type EP[12]; /* USB Device Endpoints */
} NM_USBD_Type;

typedef struct
{
  __ro__ uint32_t HcRevision; /* Revision register */
  __rw__ uint32_t HcControl; /* Control register */
  __rw__ uint32_t HcCommandStatus; /* Command Status register */
  __rw__ uint32_t HcInterruptStatus; /* Interrupt Status register */
  __rw__ uint32_t HcInterruptEnable; /* Interrupt Enable register */
  __rw__ uint32_t HcInterruptDisable; /* Interrupt Disable register */
  __rw__ uint32_t HcHCCA; /* Communication Area register */
  __rw__ uint32_t HcPeriodCurrentED; /* Period Current ED register */
  __rw__ uint32_t HcControlHeadED; /* Control Head ED register */
  __rw__ uint32_t HcControlCurrentED; /* Control Current ED register */
  __rw__ uint32_t HcBulkHeadED; /* Bulk Head ED register */
  __rw__ uint32_t HcBulkCurrentED; /* Bulk Current ED register */
  __rw__ uint32_t HcDoneHead; /* Done Head register */
  __rw__ uint32_t HcFmInterval; /* Frame Interval register */
  __ro__ uint32_t HcFmRemaining; /* Frame Remaining register */
  __ro__ uint32_t HcFmNumber; /* Frame Number register */
  __rw__ uint32_t HcPeriodicStart; /* Periodic Start register */
  __rw__ uint32_t HcLSThreshold; /* Low-speed Threshold register */
  __rw__ uint32_t HcRhDescriptorA; /* Root Hub Descriptor A register */
  __rw__ uint32_t HcRhDescriptorB; /* Root Hub Descriptor B register */
  __rw__ uint32_t HcRhStatus; /* Root Hub Status register */
  __rw__ uint32_t HcRhPortStatus[2]; /* Root Hub Port Status registers */
  __ne__ uint32_t RESERVED0[105];
  __rw__ uint32_t HcPhyControl; /* PHY Control register */
  __rw__ uint32_t HcMiscControl; /* Miscellaneous Control register */
} NM_USBH_Type;
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
  // TODO Verify
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
  NM_CLK_Type CLK;
  __ne__ uint8_t RESERVED1[0x100 - sizeof(NM_CLK_Type)];
  NM_NMI_Type NMI;
  __ne__ uint8_t RESERVED2[0x3D00 - sizeof(NM_NMI_Type)];
  NM_GPIO_Type GPIO[8];
  __ne__ uint8_t RESERVED3[0x440 - sizeof(NM_GPIO_Type) * 8];
  NM_GPIO_DBCTL_Type GPIO_DBCTL;
  __ne__ uint8_t RESERVED4[0x3C0 - sizeof(NM_GPIO_DBCTL_Type)];
  NM_GPIO_PDIO_Type GPIO_PDIO;
  __ne__ uint8_t RESERVED5[0x2800 - sizeof(NM_GPIO_PDIO_Type)];
  NM_SPIM_Type SPIM;
  __ne__ uint8_t RESERVED6[0x1000 - sizeof(NM_SPIM_Type)];
  NM_PDMA_Type PDMA;
  __ne__ uint8_t RESERVED7[0x1000 - sizeof(NM_PDMA_Type)];
  NM_USBH_Type USBH;
  __ne__ uint8_t RESERVED8[0x2000 - sizeof(NM_USBH_Type)];
  NM_EMAC_Type EMAC;
  __ne__ uint8_t RESERVED9[0x1000 - sizeof(NM_EMAC_Type)];
  NM_FMC_Type FMC;
  __ne__ uint8_t RESERVED10[0x1000 - sizeof(NM_FMC_Type)];
  NM_SDH_Type SDH0;
  __ne__ uint8_t RESERVED11[0x1000 - sizeof(NM_SDH_Type)];
  NM_SDH_Type SDH1;
  __ne__ uint8_t RESERVED12[0x2000 - sizeof(NM_SDH_Type)];
  NM_EBI_Type EBI;
  __ne__ uint8_t RESERVED13[0x9000 - sizeof(NM_EBI_Type)];
  NM_HSUSBD_Type HSUSBD;
  __ne__ uint8_t RESERVED14[0x1000 - sizeof(NM_HSUSBD_Type)];
  NM_HSUSBH_Type HSUSBH;
  __ne__ uint8_t RESERVED15[0x16000 - sizeof(NM_HSUSBH_Type)];
  NM_CCAP_Type CCAP;
  __ne__ uint8_t RESERVED16[0x1000 - sizeof(NM_CCAP_Type)];
  NM_CRC_Type CRC;
} AHB_DOMAIN_Type;

typedef struct
{
  __ne__ uint8_t RESERVED0[0x41000];
  NM_RTC_Type RTC;
  __ne__ uint8_t RESERVED1[0x2000 - sizeof(NM_RTC_Type)];
  NM_EADC_Type EADC0;
  __ne__ uint8_t RESERVED2[0x2000 - sizeof(NM_EADC_Type)];
  NM_ACMP_Type ACMP01;
  __ne__ uint8_t RESERVED3[0x8000 - sizeof(NM_ACMP_Type)];
  NM_OTG_Type OTG;
  __ne__ uint8_t RESERVED4[0x2000 - sizeof(NM_OTG_Type)];
  NM_HSOTG_Type HSOTG;
  __ne__ uint8_t RESERVED5[0x2000 - sizeof(NM_HSOTG_Type)];
  NM_TIMER_Type TIMER2;
  __ne__ uint8_t RESERVED6[0x100 - sizeof(NM_TIMER_Type)];
  NM_TIMER_Type TIMER3;
  __ne__ uint8_t RESERVED7[0x7F00 - sizeof(NM_TIMER_Type)];
  NM_EPWM_Type EPWM1;
  __ne__ uint8_t RESERVED8[0x2000 - sizeof(NM_EPWM_Type)];
  NM_BPWM_Type BPWM1;
  __ne__ uint8_t RESERVED9[0x6000 - sizeof(NM_BPWM_Type)];
  NM_SPI_Type SPI0;
  __ne__ uint8_t RESERVED10[0x2000 - sizeof(NM_SPI_Type)];
  NM_SPI_Type SPI2;
  __ne__ uint8_t RESERVED11[0x6000 - sizeof(NM_SPI_Type)];
  NM_QSPI_Type QSPI1;
  __ne__ uint8_t RESERVED12[0x8000 - sizeof(NM_QSPI_Type)];
  NM_UART_Type UART1;
  __ne__ uint8_t RESERVED13[0x2000 - sizeof(NM_UART_Type)];
  NM_UART_Type UART3;
  __ne__ uint8_t RESERVED14[0x2000 - sizeof(NM_UART_Type)];
  NM_UART_Type UART5;
  __ne__ uint8_t RESERVED15[0x2000 - sizeof(NM_UART_Type)];
  NM_UART_Type UART7;
  __ne__ uint8_t RESERVED16[0xA000 - sizeof(NM_UART_Type)];
  NM_I2C_Type I2C1;
  __ne__ uint8_t RESERVED17[0xF000 - sizeof(NM_I2C_Type)];
  NM_SC_Type SC0;
  __ne__ uint8_t RESERVED18[0x1000 - sizeof(NM_SC_Type)];
  NM_SC_Type SC1;
  __ne__ uint8_t RESERVED19[0x1000 - sizeof(NM_SC_Type)];
  NM_SC_Type SC2;
  __ne__ uint8_t RESERVED20[0xF000 - sizeof(NM_SC_Type)];
  NM_CAN_Type CAN1;
  __ne__ uint8_t RESERVED21[0x10000 - sizeof(NM_CAN_Type)];
  NM_QEI_Type QEI1;
  __ne__ uint8_t RESERVED22[0x4000 - sizeof(NM_QEI_Type)];
  NM_ECAP_Type ECAP1;
  __ne__ uint8_t RESERVED23[0x4000 - sizeof(NM_ECAP_Type)];
  NM_TRNG_Type TRNG;
  __ne__ uint8_t RESERVED24[0x7000 - sizeof(NM_TRNG_Type)];
  NM_USBD_Type USBD;
  __ne__ uint8_t RESERVED25[0x11000 - sizeof(NM_USBD_Type)];
  NM_USCI_Type USCI1;
} APB1_DOMAIN_Type;

typedef struct
{
  __ne__ uint8_t RESERVED0[0x40000];
  NM_WDT_Type WDT;
  __ne__ uint8_t RESERVED1[0x100 - sizeof(NM_WDT_Type)];
  NM_WWDT_Type WWDT;
  __ne__ uint8_t RESERVED2[0x5F00 - sizeof(NM_WWDT_Type)];
  NM_OPA_Type OPA0;
  __ne__ uint8_t RESERVED3[0x2000 - sizeof(NM_OPA_Type)];
  NM_I2S_Type I2S0;
  __ne__ uint8_t RESERVED4[0x3000 - sizeof(NM_I2S_Type)];
  NM_EADC_Type EADC1;
  __ne__ uint8_t RESERVED5[0x5000 - sizeof(NM_EADC_Type)];
  NM_TIMER_Type TIMER0;
  __ne__ uint8_t RESERVED6[0x100 - sizeof(NM_TIMER_Type)];
  NM_TIMER_Type TIMER1;
  __ne__ uint8_t RESERVED7[0x7F00 - sizeof(NM_TIMER_Type)];
  NM_EPWM_Type EPWM0;
  __ne__ uint8_t RESERVED8[0x2000 - sizeof(NM_EPWM_Type)];
  NM_BPWM_Type BPWM0;
  __ne__ uint8_t RESERVED9[0x6000 - sizeof(NM_BPWM_Type)];
  NM_QSPI_Type QSPI0;
  __ne__ uint8_t RESERVED10[0x2000 - sizeof(NM_QSPI_Type)];
  NM_SPI_Type SPI1;
  __ne__ uint8_t RESERVED11[0x2000 - sizeof(NM_SPI_Type)];
  NM_SPI_Type SPI3;
  __ne__ uint8_t RESERVED12[0xC000 - sizeof(NM_SPI_Type)];
  NM_UART_Type UART0;
  __ne__ uint8_t RESERVED13[0x2000 - sizeof(NM_UART_Type)];
  NM_UART_Type UART2;
  __ne__ uint8_t RESERVED14[0x2000 - sizeof(NM_UART_Type)];
  NM_UART_Type UART4;
  __ne__ uint8_t RESERVED15[0x2000 - sizeof(NM_UART_Type)];
  NM_UART_Type UART6;
  __ne__ uint8_t RESERVED16[0xA000 - sizeof(NM_UART_Type)];
  NM_I2C_Type I2C0;
  __ne__ uint8_t RESERVED17[0x2000 - sizeof(NM_I2C_Type)];
  NM_I2C_Type I2C2;
  __ne__ uint8_t RESERVED18[0x1E000 - sizeof(NM_I2C_Type)];
  NM_CAN_Type CAN0;
  __ne__ uint8_t RESERVED19[0x2000 - sizeof(NM_CAN_Type)];
  NM_CAN_Type CAN2;
  __ne__ uint8_t RESERVED20[0xE000 - sizeof(NM_CAN_Type)];
  NM_QEI_Type QEI0;
  __ne__ uint8_t RESERVED21[0x4000 - sizeof(NM_QEI_Type)];
  NM_ECAP_Type ECAP0;
  __ne__ uint8_t RESERVED22[0x1C000 - sizeof(NM_ECAP_Type)];
  NM_USCI_Type USCI0;
} APB2_DOMAIN_Type;

typedef struct
{
  NM_CONFIG_Type CONFIG;
} CONFIG_DOMAIN_Type;

typedef struct
{
  __ne__ uint8_t RESERVED0[0x80000];
  NM_CRYPTO_Type CRYPTO;
} CRYPTO_DOMAIN_Type;

typedef union
{
  AHB_DOMAIN_Type AHB_DOMAIN;
  APB1_DOMAIN_Type APB1_DOMAIN;
  APB2_DOMAIN_Type APB2_DOMAIN;
} AHB_APB_DOMAIN_Type;
/*----------------------------------------------------------------------------*/
extern AHB_APB_DOMAIN_Type AHB_APB_DOMAIN;
extern CONFIG_DOMAIN_Type CONFIG_DOMAIN;
extern CRYPTO_DOMAIN_Type CRYPTO_DOMAIN;
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
#define NM_SPIM       (&AHB_APB_DOMAIN.AHB_DOMAIN.SPIM)
#define NM_PDMA       (&AHB_APB_DOMAIN.AHB_DOMAIN.PDMA)
#define NM_USBH       (&AHB_APB_DOMAIN.AHB_DOMAIN.USBH)
#define NM_EMAC       (&AHB_APB_DOMAIN.AHB_DOMAIN.EMAC)
#define NM_FMC        (&AHB_APB_DOMAIN.AHB_DOMAIN.FMC)
#define NM_SDH0       (&AHB_APB_DOMAIN.AHB_DOMAIN.SDH0)
#define NM_SDH1       (&AHB_APB_DOMAIN.AHB_DOMAIN.SDH1)
#define NM_EBI        (&AHB_APB_DOMAIN.AHB_DOMAIN.EBI)
#define NM_HSUSBD     (&AHB_APB_DOMAIN.AHB_DOMAIN.HSUSBD)
#define NM_HSUSBH     (&AHB_APB_DOMAIN.AHB_DOMAIN.HSUSBH)
#define NM_CCAP       (&AHB_APB_DOMAIN.AHB_DOMAIN.CCAP)
#define NM_CRC        (&AHB_APB_DOMAIN.AHB_DOMAIN.CRC)

#define NM_RTC        (&AHB_APB_DOMAIN.APB1_DOMAIN.RTC)
#define NM_EADC0      (&AHB_APB_DOMAIN.APB1_DOMAIN.EADC0)
#define NM_ACMP01     (&AHB_APB_DOMAIN.APB1_DOMAIN.ACMP01)
#define NM_OTG        (&AHB_APB_DOMAIN.APB1_DOMAIN.OTG)
#define NM_HSOTG      (&AHB_APB_DOMAIN.APB1_DOMAIN.HSOTG)
#define NM_TIMER2     (&AHB_APB_DOMAIN.APB1_DOMAIN.TIMER2)
#define NM_TIMER3     (&AHB_APB_DOMAIN.APB1_DOMAIN.TIMER3)
#define NM_EPWM1      (&AHB_APB_DOMAIN.APB1_DOMAIN.EPWM1)
#define NM_BPWM1      (&AHB_APB_DOMAIN.APB1_DOMAIN.BPWM1)
#define NM_SPI0       (&AHB_APB_DOMAIN.APB1_DOMAIN.SPI0)
#define NM_SPI2       (&AHB_APB_DOMAIN.APB1_DOMAIN.SPI2)
#define NM_QSPI1      (&AHB_APB_DOMAIN.APB1_DOMAIN.QSPI1)
#define NM_UART1      (&AHB_APB_DOMAIN.APB1_DOMAIN.UART1)
#define NM_UART3      (&AHB_APB_DOMAIN.APB1_DOMAIN.UART3)
#define NM_UART5      (&AHB_APB_DOMAIN.APB1_DOMAIN.UART5)
#define NM_UART7      (&AHB_APB_DOMAIN.APB1_DOMAIN.UART7)
#define NM_I2C1       (&AHB_APB_DOMAIN.APB1_DOMAIN.I2C1)
#define NM_SC0        (&AHB_APB_DOMAIN.APB1_DOMAIN.SC0)
#define NM_SC1        (&AHB_APB_DOMAIN.APB1_DOMAIN.SC1)
#define NM_SC2        (&AHB_APB_DOMAIN.APB1_DOMAIN.SC2)
#define NM_CAN1       (&AHB_APB_DOMAIN.APB1_DOMAIN.CAN1)
#define NM_QEI1       (&AHB_APB_DOMAIN.APB1_DOMAIN.QEI1)
#define NM_ECAP1      (&AHB_APB_DOMAIN.APB1_DOMAIN.ECAP1
#define NM_TRNG       (&AHB_APB_DOMAIN.APB1_DOMAIN.TRNG)
#define NM_USBD       (&AHB_APB_DOMAIN.APB1_DOMAIN.USBD)
#define NM_USCI1      (&AHB_APB_DOMAIN.APB1_DOMAIN.USCI1)

#define NM_WDT        (&AHB_APB_DOMAIN.APB2_DOMAIN.WDT)
#define NM_WWDT       (&AHB_APB_DOMAIN.APB2_DOMAIN.WWDT)
#define NM_OPA0       (&AHB_APB_DOMAIN.APB2_DOMAIN.OPA0)
#define NM_I2S0       (&AHB_APB_DOMAIN.APB2_DOMAIN.I2S0)
#define NM_EADC1      (&AHB_APB_DOMAIN.APB2_DOMAIN.EADC1)
#define NM_TIMER0     (&AHB_APB_DOMAIN.APB2_DOMAIN.TIMER0)
#define NM_TIMER1     (&AHB_APB_DOMAIN.APB2_DOMAIN.TIMER1)
#define NM_EPWM0      (&AHB_APB_DOMAIN.APB2_DOMAIN.EPWM0)
#define NM_BPWM0      (&AHB_APB_DOMAIN.APB2_DOMAIN.BPWM0)
#define NM_QSPI0      (&AHB_APB_DOMAIN.APB2_DOMAIN.QSPI0)
#define NM_SPI1       (&AHB_APB_DOMAIN.APB2_DOMAIN.SPI1)
#define NM_SPI3       (&AHB_APB_DOMAIN.APB2_DOMAIN.SPI3)
#define NM_UART0      (&AHB_APB_DOMAIN.APB2_DOMAIN.UART0)
#define NM_UART2      (&AHB_APB_DOMAIN.APB2_DOMAIN.UART2)
#define NM_UART4      (&AHB_APB_DOMAIN.APB2_DOMAIN.UART4)
#define NM_UART6      (&AHB_APB_DOMAIN.APB2_DOMAIN.UART6)
#define NM_I2C0       (&AHB_APB_DOMAIN.APB2_DOMAIN.I2C0)
#define NM_I2C2       (&AHB_APB_DOMAIN.APB2_DOMAIN.I2C2)
#define NM_CAN0       (&AHB_APB_DOMAIN.APB2_DOMAIN.CAN0)
#define NM_CAN2       (&AHB_APB_DOMAIN.APB2_DOMAIN.CAN2)
#define NM_QEI0       (&AHB_APB_DOMAIN.APB2_DOMAIN.QEI0)
#define NM_ECAP0      (&AHB_APB_DOMAIN.APB2_DOMAIN.ECAP0)
#define NM_USCI0      (&AHB_APB_DOMAIN.APB2_DOMAIN.USCI0)

#define NM_CONFIG     (&CONFIG_DOMAIN.CONFIG)
#define NM_CRYPTO     (&CRYPTO_DOMAIN.CRYPTO)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M48X_PLATFORM_DEFS_H_ */
