/*
 * halm/platform/imxrt/imxrt106x/platform_defs.h
 * Based on original from NXP
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_IMXRT_PLATFORM_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_IMXRT_IMXRT106X_PLATFORM_DEFS_H_
#define HALM_PLATFORM_IMXRT_IMXRT106X_PLATFORM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*------------------Analog Comparator-----------------------------------------*/
typedef struct
{
  __rw__ uint8_t CR0; /* CMP Control Register 0 */
  __rw__ uint8_t CR1; /* CMP Control Register 1 */
  __rw__ uint8_t FPR; /* CMP Filter Period Register */
  __rw__ uint8_t SCR; /* CMP Status and Control Register */
  __rw__ uint8_t DACCR; /* DAC Control Register */
  __rw__ uint8_t MUXCR; /* MUX Control Register */
} IMX_ACMP_Type;
/*------------------Analog-to-Digital Converter-------------------------------*/
typedef struct
{
  __rw__ uint32_t HC[8]; /* Control register for HW triggers */
  __ro__ uint32_t HS; /* Status register for HW triggers */
  __ro__ uint32_t R[8]; /* Data result register for HW triggers */
  __rw__ uint32_t CFG; /* Configuration register */
  __rw__ uint32_t GC; /* General control register */
  __rw__ uint32_t GS; /* General status register */
  __rw__ uint32_t CV; /* Compare value register */
  __rw__ uint32_t OFS; /* Offset correction value register */
  __rw__ uint32_t CAL; /* Calibration value register */
} IMX_ADC_Type;
/*------------------ADC External Trigger Control------------------------------*/
typedef struct
{
  __rw__ uint32_t CTRL; /* Global Control register */
  __rw__ uint32_t DONE0_1_IRQ; /* DONE0 and DONE1 IRQ state register */
  __rw__ uint32_t DONE2_3_ERR_IRQ; /* DONE_2 and DONE_ERR IRQ state register */
  __rw__ uint32_t DMA_CTRL; /* DMA Control register */

  struct
  {
    __rw__ uint32_t CTRL; /* Control register */
    __rw__ uint32_t COUNTER; /* Counter register */
    __rw__ uint32_t CHAIN_1_0; /* Chain 0/1 register */
    __rw__ uint32_t CHAIN_3_2; /* Chain 2/3 register */
    __rw__ uint32_t CHAIN_5_4; /* Chain 4/5 register */
    __rw__ uint32_t CHAIN_7_6; /* Chain 6/7 register */
    __ro__ uint32_t RESULT_1_0; /* Result data 1/0 register */
    __ro__ uint32_t RESULT_3_2; /* Result data 3/2 register */
    __ro__ uint32_t RESULT_5_4; /* Result data 5/4 register */
    __ro__ uint32_t RESULT_7_6; /* Result data 7/6 register */
  } TRIG[8];
} IMX_ADC_ETC_Type;
/*------------------AHB to IP Bridge------------------------------------------*/
typedef struct
{
  __rw__ uint32_t MPR; /* Master Priviledge Register */
  __ne__ uint32_t RESERVED[15];
  __rw__ uint32_t OPACR;
  __rw__ uint32_t OPACR1;
  __rw__ uint32_t OPACR2;
  __rw__ uint32_t OPACR3;
  __rw__ uint32_t OPACR4;
} IMX_AIPSTZ_Type;
/*------------------And-Or Inverter-------------------------------------------*/
typedef struct
{
  struct
  {
    __rw__ uint16_t BFCRT01;
    __rw__ uint16_t BFCRT23;
  } BFCRT[4];
} IMX_AOI_Type;
/*------------------Bus Encryption Engine-------------------------------------*/
typedef struct
{
  __rw__ uint32_t CTRL; /* Control register */
  __rw__ uint32_t ADDR_OFFSET0; /* Offset region 0 register */
  __rw__ uint32_t ADDR_OFFSET1; /* Offset region 1 register */
  __wo__ uint32_t AES_KEY0_W0; /* AES Key 0 register */
  __wo__ uint32_t AES_KEY0_W1; /* AES Key 1 register */
  __wo__ uint32_t AES_KEY0_W2; /* AES Key 2 register */
  __wo__ uint32_t AES_KEY0_W3; /* AES Key 3 register */
  __rw__ uint32_t STATUS; /* Status register */
  __wo__ uint32_t CTR_NONCE0_W0; /* Nonce 00 register */
  __wo__ uint32_t CTR_NONCE0_W1; /* Nonce 01 register */
  __wo__ uint32_t CTR_NONCE0_W2; /* Nonce 02 register */
  __wo__ uint32_t CTR_NONCE0_W3; /* Nonce 03 register */
  __wo__ uint32_t CTR_NONCE1_W0; /* Nonce 10 register */
  __wo__ uint32_t CTR_NONCE1_W1; /* Nonce 11 register */
  __wo__ uint32_t CTR_NONCE1_W2; /* Nonce 12 register */
  __wo__ uint32_t CTR_NONCE1_W3; /* Nonce 13 register */
  __rw__ uint32_t REGION1_TOP; /* Region 1 Top Address register */
  __rw__ uint32_t REGION1_BOT; /* Region 1 Bottom Address register */
} IMX_BEE_Type;
/*------------------Clock Controller Module-----------------------------------*/
typedef struct
{
  __rw__ uint32_t CCR; /* Control Register */
  __ne__ uint32_t RESERVED0;
  __ro__ uint32_t CSR; /* Status Register */
  __rw__ uint32_t CCSR; /* Clock Switcher Register */
  __rw__ uint32_t CACRR; /* ARM Clock Root Register */
  __rw__ uint32_t CBCDR; /* Bus Clock Divider Register */
  __rw__ uint32_t CBCMR; /* Bus Clock Multiplexer Register */
  __rw__ uint32_t CSCMR1; /* Serial Clock Multiplexer Register 1 */
  __rw__ uint32_t CSCMR2; /* Serial Clock Multiplexer Register 2 */
  __rw__ uint32_t CSCDR1; /* Serial Clock Divider Register 1 */
  __rw__ uint32_t CS1CDR; /* Clock Divider Register 1 */
  __rw__ uint32_t CS2CDR; /* Clock Divider Register 2 */
  __rw__ uint32_t CDCDR; /* D1 Clock Divider Register */
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t CSCDR2; /* Serial Clock Divider Register 2 */
  __rw__ uint32_t CSCDR3; /* Serial Clock Divider Register 3 */
  __ne__ uint32_t RESERVED2[2];
  __ro__ uint32_t CDHIPR; /* Divider Handshake In-Process Register */
  __ne__ uint32_t RESERVED3[2];
  __rw__ uint32_t CLPCR; /* Low Power Control Register */
  __rw__ uint32_t CISR; /* Interrupt Status Register */
  __rw__ uint32_t CIMR; /* Interrupt Mask Register */
  __rw__ uint32_t CCOSR; /* Clock Output Source Register */
  __rw__ uint32_t CGPR; /* General Purpose Register */

  /* Offset 0x68 */
  union
  {
    struct
    {
      __rw__ uint32_t CCGR0;
      __rw__ uint32_t CCGR1;
      __rw__ uint32_t CCGR2;
      __rw__ uint32_t CCGR3;
      __rw__ uint32_t CCGR4;
      __rw__ uint32_t CCGR5;
      __rw__ uint32_t CCGR6;
    };
    __rw__ uint32_t CCGR[7]; /* Clock Gating Registers */
  };
  __ne__ uint32_t RESERVED4;

  /* Offset 0x88 */
  __rw__ uint32_t CMEOR; /* Module Enable Overide Register */
} IMX_CCM_Type;
/*------------------Clock Controller Modules - Analog PLLs--------------------*/
typedef struct
{
  /* Analog ARM PLL control register */
  __rw__ uint32_t PLL_ARM;
  __rw__ uint32_t PLL_ARM_SET;
  __rw__ uint32_t PLL_ARM_CLR;
  __rw__ uint32_t PLL_ARM_TOG;

  /* Analog USB1 480MHz PLL control register */
  __rw__ uint32_t PLL_USB1;
  __rw__ uint32_t PLL_USB1_SET;
  __rw__ uint32_t PLL_USB1_CLR;
  __rw__ uint32_t PLL_USB1_TOG;

  /* Analog USB2 480MHz PLL control register */
  __rw__ uint32_t PLL_USB2;
  __rw__ uint32_t PLL_USB2_SET;
  __rw__ uint32_t PLL_USB2_CLR;
  __rw__ uint32_t PLL_USB2_TOG;

  /* Offset 0x030 */
  /* Analog System PLL control register */
  __rw__ uint32_t PLL_SYS;
  __rw__ uint32_t PLL_SYS_SET;
  __rw__ uint32_t PLL_SYS_CLR;
  __rw__ uint32_t PLL_SYS_TOG;
  /* 528MHz System PLL Spread Spectrum register */
  __rw__ uint32_t PLL_SYS_SS;
  __ne__ uint32_t RESERVED2[3];
  /* Numerator of System PLL Fractional Loop Divider register */
  __rw__ uint32_t PLL_SYS_NUM;
  __ne__ uint32_t RESERVED3[3];
  /* Denominator of System PLL Fractional Loop Divider register */
  __rw__ uint32_t PLL_SYS_DENOM;
  __ne__ uint32_t RESERVED4[3];

  /* Offset 0x070 */
  /* Analog Audio PLL control register */
  __rw__ uint32_t PLL_AUDIO;
  __rw__ uint32_t PLL_AUDIO_SET;
  __rw__ uint32_t PLL_AUDIO_CLR;
  __rw__ uint32_t PLL_AUDIO_TOG;
  /* Numerator of Audio PLL Fractional Loop Divider register */
  __rw__ uint32_t PLL_AUDIO_NUM;
  __ne__ uint32_t RESERVED5[3];
  /* Denominator of Audio PLL Fractional Loop Divider register */
  __rw__ uint32_t PLL_AUDIO_DENOM;
  __ne__ uint32_t RESERVED6[3];

  /* Offset 0x0A0 */
  /* Analog Video PLL control register */
  __rw__ uint32_t PLL_VIDEO;
  __rw__ uint32_t PLL_VIDEO_SET;
  __rw__ uint32_t PLL_VIDEO_CLR;
  __rw__ uint32_t PLL_VIDEO_TOG;
  /* Numerator of Video PLL Fractional Loop Divider register */
  __rw__ uint32_t PLL_VIDEO_NUM;
  __ne__ uint32_t RESERVED7[3];
  /* Denominator of Video PLL Fractional Loop Divider register */
  __rw__ uint32_t PLL_VIDEO_DENOM;
  __ne__ uint32_t RESERVED8[7];

  /* Offset 0x0E0 */
  /* Analog ENET PLL control register */
  __rw__ uint32_t PLL_ENET;
  __rw__ uint32_t PLL_ENET_SET;
  __rw__ uint32_t PLL_ENET_CLR;
  __rw__ uint32_t PLL_ENET_TOG;

  /* Offset 0x0F0 */
  /* 480MHz Clock (PLL3) Phase Fractional Divider control register */
  __rw__ uint32_t PFD_480;
  __rw__ uint32_t PFD_480_SET;
  __rw__ uint32_t PFD_480_CLR;
  __rw__ uint32_t PFD_480_TOG;

  /* Offset 0x100 */
  /* 528MHz Clock (PLL2) Phase Fractional Divider control register */
  __rw__ uint32_t PFD_528;
  __rw__ uint32_t PFD_528_SET;
  __rw__ uint32_t PFD_528_CLR;
  __rw__ uint32_t PFD_528_TOG;
  __ne__ uint32_t RESERVED9[16];

  /* Offset 0x150 */
  __rw__ uint32_t MISC0; /* Miscellaneous register 0 */
  __rw__ uint32_t MISC0_SET;
  __rw__ uint32_t MISC0_CLR;
  __rw__ uint32_t MISC0_TOG;
  __rw__ uint32_t MISC1; /* Miscellaneous register 1 */
  __rw__ uint32_t MISC1_SET;
  __rw__ uint32_t MISC1_CLR;
  __rw__ uint32_t MISC1_TOG;
  __rw__ uint32_t MISC2; /* Miscellaneous register 2 */
  __rw__ uint32_t MISC2_SET;
  __rw__ uint32_t MISC2_CLR;
  __rw__ uint32_t MISC2_TOG;
} IMX_CCM_ANALOG_Type;
/*------------------CMOS Sensor Interface-------------------------------------*/
typedef struct {
  __rw__ uint32_t CR1; /* Control Register 1 */
  __rw__ uint32_t CR2; /* Control Register 2 */
  __rw__ uint32_t CR3; /* Control Register 3 */
  __ro__ uint32_t STATFIFO; /* Statistic FIFO register */
  __ro__ uint32_t RFIFO; /* RX FIFO register */
  __rw__ uint32_t RXCNT; /* RX Count register */
  __rw__ uint32_t SR; /* Status Register */
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t DMASA_STATFIFO; /* STATFIFO DMA Start Address register */
  __rw__ uint32_t DMATS_STATFIFO; /* STATFIFO DMA Transfer Size register */
  __rw__ uint32_t DMASA_FB1; /* Frame Buffer 1 DMA Start Address register */
  __rw__ uint32_t DMASA_FB2; /* Frame Buffer 2 DMA Transfer Size register */
  __rw__ uint32_t FBUF_PARA; /* Frame Buffer Parameter register */
  __rw__ uint32_t IMAG_PARA; /* Image Parameter register */
  __ne__ uint32_t RESERVED1[4];
  __rw__ uint32_t CR18; /* Control Register 18 */
  __rw__ uint32_t CR19; /* Control Register 19 */
} IMX_CSI_Type;
/*------------------Central Security Unit-------------------------------------*/
typedef struct
{
  __rw__ uint32_t CSL[32]; /* Config Security Level register */
  __ne__ uint8_t RESERVED0[384];

  /* Offset 0x200 */
  __rw__ uint32_t HP0; /* HP0 register */
  __ne__ uint8_t RESERVED1[20];

  /* Offset 0x218 */
  __rw__ uint32_t SA; /* Secure Access register */
  __ne__ uint8_t RESERVED2[316];

  /* Offset 0x358 */
  __rw__ uint32_t HPCONTROL0; /* HPCONTROL0 register */
} IMX_CSU_Type;
/*------------------DC-DC Converter-------------------------------------------*/
typedef struct
{
  union
  {
    struct
    {
      __rw__ uint32_t REG0;
      __rw__ uint32_t REG1;
      __rw__ uint32_t REG2;
      __rw__ uint32_t REG3;
    };
    __rw__ uint32_t REG[4];
  };
} IMX_DCDC_Type;
/*------------------Data Co-Processor-----------------------------------------*/
typedef struct
{
  __rw__ uint32_t CTRL; /* Control register 0 */
  __rw__ uint32_t CTRL_SET;
  __rw__ uint32_t CTRL_CLR;
  __rw__ uint32_t CTRL_TOG;
  __rw__ uint32_t STAT; /* Status register */
  __rw__ uint32_t STAT_SET;
  __rw__ uint32_t STAT_CLR;
  __rw__ uint32_t STAT_TOG;
  __rw__ uint32_t CHANNELCTRL; /* Channel Control register */
  __rw__ uint32_t CHANNELCTRL_SET;
  __rw__ uint32_t CHANNELCTRL_CLR;
  __rw__ uint32_t CHANNELCTRL_TOG;
  __rw__ uint32_t CAPABILITY0; /* Capability 0 register */
  __ne__ uint32_t RESERVED0[3];
  __ro__ uint32_t CAPABILITY1; /* Capability 1 register */
  __ne__ uint32_t RESERVED1[3];
  __rw__ uint32_t CONTEXT; /* Context buffer pointer */
  __ne__ uint32_t RESERVED2[3];
  __rw__ uint32_t KEY; /* Key index register */
  __ne__ uint32_t RESERVED3[3];
  __rw__ uint32_t KEYDATA; /* Key Data register */
  __ne__ uint32_t RESERVED4[3];

  /* Offset 0x80 */
  struct
  {
    __ro__ uint32_t PACKET; /* Work Packet N status register */
    __ne__ uint32_t RESERVED[3];
  } PACKET[7];
  __ne__ uint32_t RESERVED5[4];

  /* Offset 0x100 */
  struct
  {
    __rw__ uint32_t CMDPTR; /* Channel N Command Pointer Address register */
    __ne__ uint32_t RESERVED0[3];
    __rw__ uint32_t SEMA; /* Channel N Semaphore register */
    __ne__ uint32_t RESERVED1[3];
    __rw__ uint32_t STAT; /* Channel N Status register */
    __rw__ uint32_t STAT_SET;
    __rw__ uint32_t STAT_CLR;
    __rw__ uint32_t STAT_TOG;
    __rw__ uint32_t OPTS; /* Channel N Options register */
    __rw__ uint32_t OPTS_SET;
    __rw__ uint32_t OPTS_CLR;
    __rw__ uint32_t OPTS_TOG;
  } CH[4];
  __ne__ uint32_t RESERVED6[128];

  /* Offset 0x400 */
  __rw__ uint32_t DBGSELECT; /* Debug Select register */
  __ne__ uint32_t RESERVED7[3];
  __ro__ uint32_t DBGDATA; /* Debug Data register */
  __ne__ uint32_t RESERVED8[3];
  __rw__ uint32_t PAGETABLE; /* Page Table register */
  __ne__ uint32_t RESERVED9[3];
  __ro__ uint32_t VERSION; /* Version register */
} IMX_DCP_Type;
/*------------------Direct Memory Access Multiplexer--------------------------*/
typedef struct
{
  __rw__ uint32_t CHCFG[32];
} IMX_DMAMUX_Type;
/*------------------Enhanced Direct Memory Access-----------------------------*/
typedef struct
{
  /* Source Address */
  __rw__ uint32_t SADDR;
  /* Signed Source Address Offset */
  __rw__ int16_t SOFF;
  /* Transfer Attributes */
  __rw__ uint16_t ATTR;

  union
  {
    /* Minor Byte Count (Minor Loop Mapping off) */
    __rw__ uint32_t NBYTES_MLNO;
    /* Signed Minor Loop Offset (Minor Loop Mapping on and Offset off) */
    __rw__ uint32_t NBYTES_MLOFFNO;
    /* Signed Minor Loop Offset (Minor Loop Mapping and Offset on) */
    __rw__ uint32_t NBYTES_MLOFFYES;
  };

  /* Last Source Address Adjustment */
  __rw__ int32_t SLAST;
  /* Destination Address */
  __rw__ uint32_t DADDR;
  /* Signed Destination Address Offset */
  __rw__ int16_t DOFF;

  union
  {
    /* Current Minor Loop Link, Major Loop Count (Channel Linking off) */
    __rw__ uint16_t CITER_ELINKNO;
    /* Current Minor Loop Link, Major Loop Count (Channel Linking on) */
    __rw__ uint16_t CITER_ELINKYES;
  };

  /* Last Destination Address Adjustment/Scatter Gather Address */
  __rw__ int32_t DLAST_SGA;
  /* Control and Status register */
  __rw__ uint16_t CSR;

  union
  {
    /* Beginning Minor Loop Link, Major Loop Count (Channel Linking off) */
    __rw__ uint16_t BITER_ELINKNO;
    /* Beginning Minor Loop Link, Major Loop Count (Channel Linking on) */
    __rw__ uint16_t BITER_ELINKYES;
  };
} IMX_EDMA_TCD_Type;

typedef struct
{
  __rw__ uint32_t CR; /* Control Register */
  __ro__ uint32_t ES; /* Error Status register */
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t ERQ; /* Enable Request register */
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t EEI; /* Enable Error Interrupt register */
  __wo__ uint8_t CEEI; /* Clear Enable Error Interrupt register */
  __wo__ uint8_t SEEI; /* Set Enable Error Interrupt register */
  __wo__ uint8_t CERQ; /* Clear Enable Request register */
  __wo__ uint8_t SERQ; /* Set Enable Request register */
  __wo__ uint8_t CDNE; /* Clear DONE Status Bit register */
  __wo__ uint8_t SSRT; /* Set START Bit register */
  __wo__ uint8_t CERR; /* Clear Error register */
  __wo__ uint8_t CINT; /* Clear Interrupt Request register */
  __ne__ uint32_t RESERVED2;
  __rw__ uint32_t INT; /* Interrupt Request register */
  __ne__ uint32_t RESERVED3;
  __rw__ uint32_t ERR; /* Error register */
  __ne__ uint32_t RESERVED4;
  __ro__ uint32_t HRS; /* Hardware Request Status register */
  __ne__ uint32_t RESERVED5[3];
  __rw__ uint32_t EARS; /* Enable Asynchronous Request in Stop register */
  __ne__ uint32_t RESERVED6[46];

  /* Offset 0x0100 */
  __rw__ uint8_t DCHPRI[32];
  __ne__ uint32_t RESERVED7[952];

  /* Offset 0x1000 */
  IMX_EDMA_TCD_Type TCD[32];
} IMX_EDMA_Type;
/*------------------Quadrature Encoder/Decoder--------------------------------*/
typedef struct
{
  __rw__ uint16_t CTRL; /* Control register */
  __rw__ uint16_t FILT; /* Input Filter register */
  __rw__ uint16_t WTR; /* Watchdog Timeout register */
  __rw__ uint16_t POSD; /* Position Difference Counter register */
  __ro__ uint16_t POSDH; /* Position Difference Hold register */
  __rw__ uint16_t REV; /* Revolution Counter register */
  __ro__ uint16_t REVH; /* Revolution Hold register */
  __rw__ uint16_t UPOS; /* Upper Position Counter register */
  __rw__ uint16_t LPOS; /* Lower Position Counter register */
  __ro__ uint16_t UPOSH; /* Upper Position Hold register */
  __ro__ uint16_t LPOSH; /* Lower Position Hold register */
  __rw__ uint16_t UINIT; /* Upper Initialization register */
  __rw__ uint16_t LINIT; /* Lower Initialization register */
  __ro__ uint16_t IMR; /* Input Monitor register */
  __rw__ uint16_t TST; /* Test register */
  __rw__ uint16_t CTRL2; /* Control 2 register */
  __rw__ uint16_t UMOD; /* Upper Modulus register */
  __rw__ uint16_t LMOD; /* Lower Modulus register */
  __rw__ uint16_t UCOMP; /* Upper Position Compare register */
  __rw__ uint16_t LCOMP; /* Lower Position Compare register */
} IMX_ENC_Type;
/*------------------Ethernet MAC----------------------------------------------*/
typedef struct
{
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t EIR; /* Interrupt Event Register */
  __rw__ uint32_t EIMR; /* Interrupt Mask Register */
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t RDAR; /* Receive Descriptor Active Register - Ring 0 */
  __rw__ uint32_t TDAR; /* Transmit Descriptor Active Register - Ring 0 */
  __ne__ uint32_t RESERVED2[3];
  __rw__ uint32_t ECR; /* Ethernet Control Register */
  __ne__ uint32_t RESERVED3[6];
  __rw__ uint32_t MMFR; /* MII Management Frame Register */
  __rw__ uint32_t MSCR; /* MII Speed Control Register */
  __ne__ uint32_t RESERVED4[7];
  __rw__ uint32_t MIBC; /* MIB Control register */
  __ne__ uint32_t RESERVED5[7];
  __rw__ uint32_t RCR; /* Receive Control Register */
  __ne__ uint32_t RESERVED6[15];
  __rw__ uint32_t TCR; /* Transmit Control Register */
  __ne__ uint32_t RESERVED7[7];
  __rw__ uint32_t PALR; /* Physical Address Lower Register */
  __rw__ uint32_t PAUR; /* Physical Address Upper Register */
  __rw__ uint32_t OPD; /* Opcode/Pause Duration register */
  __rw__ uint32_t TXIC; /* Transmit Interrupt Coalescing Register */
  __ne__ uint32_t RESERVED8[3];

  /* Offset 0x100 */
  __rw__ uint32_t RXIC; /* Receive Interrupt Coalescing register */
  __ne__ uint32_t RESERVED9[5];
  __rw__ uint32_t IAUR; /* Descriptor Individual Upper Address Register */
  __rw__ uint32_t IALR; /* Descriptor Individual Lower Address Register */
  __rw__ uint32_t GAUR; /* Descriptor Group Upper Address Register */
  __rw__ uint32_t GALR; /* Descriptor Group Lower Address Register */
  __ne__ uint32_t RESERVED10[7];
  __rw__ uint32_t TFWR; /* Transmit FIFO Watermark Register */
  __ne__ uint32_t RESERVED11[14];
  __rw__ uint32_t RDSR; /* Receive Descriptor Start Register - Ring 0 */
  __rw__ uint32_t TDSR; /* Transmit Buffer Descriptor Start Register - Ring 0 */
  __rw__ uint32_t MRBR; /* Maximum Receive Buffer Size Register - Ring 0 */
  __ne__ uint32_t RESERVED12;
  __rw__ uint32_t RSFL; /* Receive FIFO Section Full threshold */
  __rw__ uint32_t RSEM; /* Receive FIFO Section Empty threshold */
  __rw__ uint32_t RAEM; /* Receive FIFO Almost Empty threshold */
  __rw__ uint32_t RAFL; /* Receive FIFO Almost Full threshold */
  __rw__ uint32_t TSEM; /* Transmit FIFO Section Empty threshold */
  __rw__ uint32_t TAEM; /* Transmit FIFO Almost Empty threshold */
  __rw__ uint32_t TAFL; /* Transmit FIFO Almost Full threshold */
  __rw__ uint32_t TIPG; /* Transmit Inter-Packet Gap */
  __rw__ uint32_t FTRL; /* Frame Truncation Length */
  __ne__ uint32_t RESERVED13[3];
  __rw__ uint32_t TACC; /* Transmit Accelerator Function Configuration */
  __rw__ uint32_t RACC; /* Receive Accelerator Function Configuration */
  __ne__ uint32_t RESERVED14[15];

  /* Offset 0x204 */
  __ro__ uint32_t RMON_T_PACKETS; /* TX Packet count */
  __ro__ uint32_t RMON_T_BC_PKT; /* TX Broadcast Packets */
  __ro__ uint32_t RMON_T_MC_PKT; /* TX Multicast Packets */

  /* TX Packets with CRC/Align errors */
  __ro__ uint32_t RMON_T_CRC_ALIGN;
  /* TX Packets less than 64 bytes and good CRC */
  __ro__ uint32_t RMON_T_UNDERSIZE;
  /* TX Packets greater than MAX_FL bytes and good CRC */
  __ro__ uint32_t RMON_T_OVERSIZE;
  /* TX Packets less than 64 bytes and bad CRC */
  __ro__ uint32_t RMON_T_FRAG;
  /* TX Packets greater than MAX_FL bytes and bad CRC */
  __ro__ uint32_t RMON_T_JAB;

  __ro__ uint32_t RMON_T_COL; /* TX Collision count */
  __ro__ uint32_t RMON_T_P64; /* TX 64-Byte packets */
  __ro__ uint32_t RMON_T_P65TO127; /* TX 65- to 127-byte packets */
  __ro__ uint32_t RMON_T_P128TO255; /* TX 128- to 255-byte packets */
  __ro__ uint32_t RMON_T_P256TO511; /* TX 256- to 511-byte packets */
  __ro__ uint32_t RMON_T_P512TO1023; /* TX 512- to 1023-byte packets */
  __ro__ uint32_t RMON_T_P1024TO2047; /* TX 1024- to 2047-byte packets */
  __ro__ uint32_t RMON_T_P_GTE2048; /* TX packets greater than 2048 bytes */
  __ro__ uint32_t RMON_T_OCTETS; /* TX octets */
  __ne__ uint32_t RESERVED15;

  /* Offset 0x24C */
  __ro__ uint32_t IEEE_T_FRAME_OK; /* Frames sent correctly */
  __ro__ uint32_t IEEE_T_1COL; /* Frames sent with Single Collision */
  __ro__ uint32_t IEEE_T_MCOL; /* Frames sent with Multiple Collisions */
  __ro__ uint32_t IEEE_T_DEF; /* Frames sent after Deferral Delay */
  __ro__ uint32_t IEEE_T_LCOL; /* Frames sent with Late Collision */
  __ro__ uint32_t IEEE_T_EXCOL; /* Frames sent with Excessive Collisions */
  __ro__ uint32_t IEEE_T_MACERR; /* Frames sent with Tx FIFO Underrun */
  __ro__ uint32_t IEEE_T_CSERR; /* Frames sent with Carrier Sense Error */
  __ro__ uint32_t IEEE_T_SQE;
  __ro__ uint32_t IEEE_T_FDXFC; /* Flow Control Pause frames sent */
  __ro__ uint32_t IEEE_T_OCTETS_OK; /* Octet Count for frames without errors */
  __ne__ uint32_t RESERVED16[3];

  /* Offset 0x284 */
  __ro__ uint32_t RMON_R_PACKETS; /* RX Packet Count */
  __ro__ uint32_t RMON_R_BC_PKT; /* RX Broadcast Packets */
  __ro__ uint32_t RMON_R_MC_PKT; /* RX Multicast Packets */

  /* RX Packets with CRC/Align errors */
  __ro__ uint32_t RMON_R_CRC_ALIGN;
  /* RX Packets with less than 64 bytes and good CRC */
  __ro__ uint32_t RMON_R_UNDERSIZE;
  /* RX Packets greater than MAX_FL and good CRC */
  __ro__ uint32_t RMON_R_OVERSIZE;
  /* RX Packets less than 64 Bytes and bad CRC */
  __ro__ uint32_t RMON_R_FRAG;
  /* RX Packets greater than MAX_FL bytes and bad CRC */
  __ro__ uint32_t RMON_R_JAB;

  __ne__ uint32_t RESERVED17;
  __ro__ uint32_t RMON_R_P64; /* RX 64-Byte packets */
  __ro__ uint32_t RMON_R_P65TO127; /* RX 65- to 127-Byte packets */
  __ro__ uint32_t RMON_R_P128TO255; /* RX 128- to 255-Byte packets */
  __ro__ uint32_t RMON_R_P256TO511; /* RX 256- to 511-Byte packets */
  __ro__ uint32_t RMON_R_P512TO1023; /* RX 512- to 1023-Byte packets */
  __ro__ uint32_t RMON_R_P1024TO2047; /* RX 1024- to 2047-Byte packets */
  __ro__ uint32_t RMON_R_P_GTE2048; /* RX packets greater than 2048 bytes */
  __ro__ uint32_t RMON_R_OCTETS; /* RX octets */

  /* Offset 0x2C8 */
  __ro__ uint32_t IEEE_R_DROP; /* Frames not counted correctly */
  __ro__ uint32_t IEEE_R_FRAME_OK; /* Frames received correctly */
  __ro__ uint32_t IEEE_R_CRC; /* Frames received with CRC Error */
  __ro__ uint32_t IEEE_R_ALIGN; /* Frames received with Alignment Error */
  __ro__ uint32_t IEEE_R_MACERR; /* Receive FIFO overflow count */
  __ro__ uint32_t IEEE_R_FDXFC; /* Flow Control Pause frames received */
  __ro__ uint32_t IEEE_R_OCTETS_OK; /* Octet count for frames without errors */
  __ne__ uint32_t RESERVED18[71];

  /* Offset 0x400 */
  __rw__ uint32_t ATCR; /* Adjustable Timer Control Register */
  __rw__ uint32_t ATVR; /* Timer Value Register */
  __rw__ uint32_t ATOFF; /* Timer Offset register */
  __rw__ uint32_t ATPER; /* Timer Period register */
  __rw__ uint32_t ATCOR; /* Timer Correction register */
  __rw__ uint32_t ATINC; /* Time-Stamping Clock Period register */
  __ro__ uint32_t ATSTMP; /* Timestamp of Last Transmitted Frame */
  __ne__ uint32_t RESERVED19[122];

  /* Offset 0x604 */
  __rw__ uint32_t TGSR; /* Timer Global Status Register */
  struct
  {
    __rw__ uint32_t TCSR; /* Timer Control Status Register */
    __rw__ uint32_t TCCR; /* Timer Compare Capture Register */
  } CHANNEL[4];
} IMX_ENET_Type;
/*------------------External Watchdog Monitor---------------------------------*/
typedef struct
{
  __rw__ uint8_t CTRL; /* Control register */
  __wo__ uint8_t SERV; /* Service register */
  __rw__ uint8_t CMPL; /* Compare Low register */
  __rw__ uint8_t CMPH; /* Compare High register */
  __rw__ uint8_t CLKCTRL; /* Clock Control register */
  __rw__ uint8_t CLKPRESCALER; /* Clock Prescaler register */
} IMX_EWM_Type;
/*------------------Flexible Data-rate Controller Area Network----------------*/
typedef struct
{
  __rw__ uint32_t MCR; /* Module Configuration Register */
  __rw__ uint32_t CTRL1; /* Control 1 register */
  __rw__ uint32_t TIMER; /* Free Running Timer register */
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t RXMGMASK; /* RX Mailboxes Global Mask register */
  __rw__ uint32_t RX14MASK; /* RX Buffer 14 Mask register */
  __rw__ uint32_t RX15MASK; /* RX Buffer 15 Mask register */
  __rw__ uint32_t ECR; /* Error Counter Register */
  __rw__ uint32_t ESR1; /* Error and Status 1 Register */
  __rw__ uint32_t IMASK2; /* Interrupt Masks 2 register */
  __rw__ uint32_t IMASK1; /* Interrupt Masks 1 register */
  __rw__ uint32_t IFLAG2; /* Interrupt Flags 2 register */
  __rw__ uint32_t IFLAG1; /* Interrupt Flags 1 register */
  __rw__ uint32_t CTRL2; /* Control 2 register */
  __ro__ uint32_t ESR2; /* Error and Status 2 Register */
  __ne__ uint32_t RESERVED1[2];
  __ro__ uint32_t CRCR; /* CRC Register */
  __rw__ uint32_t RXFGMASK; /* RX FIFO Global Mask register */
  __ro__ uint32_t RXFIR; /* RX FIFO Information Register */

  /* Offset 0x0050 */
  __rw__ uint32_t CBT; /* CAN Bit Timing register */
  __ne__ uint32_t RESERVED2[523];

  /* Offset 0x0880 */
  __rw__ uint32_t RXIMR[64]; /* RX Individual Mask Registers */
  __ne__ uint32_t RESERVED3[156];

  /* Offset 0x0BF0 */
  __rw__ uint32_t EPRS; /* Enhanced CAN Bit Timing Prescalers */
  __rw__ uint32_t ENCBT; /* Enhanced Nominal CAN Bit Timing */
  __rw__ uint32_t EDCBT; /* Enhanced Data Phase CAN Bit Timing */
  __rw__ uint32_t ETDC; /* Enhanced Transceiver Delay Compensation */

  /* Offset 0x0C00 */
  __rw__ uint32_t FDCTRL; /* CAN FD Control register */
  __rw__ uint32_t FDCBT; /* CAN FD Bit Timing register */
  __ro__ uint32_t FDCRC; /* CAN FD CRC register */
  __rw__ uint32_t ERFCR; /* Enhanced RX FIFO Control Register */
  __rw__ uint32_t ERFIER; /* Enhanced RX FIFO Interrupt Enable Register */
  __rw__ uint32_t ERFSR; /* Enhanced RX FIFO Status Register */
  __ne__ uint32_t RESERVED4[6];

  /* Offset 0x0C30 */
  __ro__ uint32_t HR_TIME_STAMP[64]; /* High Resolution Time Stamp registers */
  __ne__ uint32_t RESERVED5[2228];

  /* Offset 0x3000 */
  __rw__ uint32_t ERFFEL[128]; /* Enhanced RX FIFO Filter Element registers */
} IMX_FDCAN_Type;
/*------------------Flexible Controller Area Network--------------------------*/
typedef struct
{
  __rw__ uint32_t MCR; /* Module Configuration Register */
  __rw__ uint32_t CTRL1; /* Control 1 register */
  __rw__ uint32_t TIMER; /* Free Running Timer register */
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t RXMGMASK; /* RX Mailboxes Global Mask register */
  __rw__ uint32_t RX14MASK; /* RX Buffer 14 Mask register */
  __rw__ uint32_t RX15MASK; /* RX Buffer 15 Mask register */
  __rw__ uint32_t ECR; /* Error Counter Register */
  __rw__ uint32_t ESR1; /* Error and Status 1 Register */
  __rw__ uint32_t IMASK2; /* Interrupt Masks 2 register */
  __rw__ uint32_t IMASK1; /* Interrupt Masks 1 register */
  __rw__ uint32_t IFLAG2; /* Interrupt Flags 2 register */
  __rw__ uint32_t IFLAG1; /* Interrupt Flags 1 register */
  __rw__ uint32_t CTRL2; /* Control 2 register */
  __ro__ uint32_t ESR2; /* Error and Status 2 Register */
  __ne__ uint32_t RESERVED1[2];
  __ro__ uint32_t CRCR; /* CRC Register */
  __rw__ uint32_t RXFGMASK; /* RX FIFO Global Mask register */
  __ro__ uint32_t RXFIR; /* RX FIFO Information Register */
  __ne__ uint32_t RESERVED2[2];
  __ro__ uint32_t DBG1; /* Debug 1 register */
  __ro__ uint32_t DBG2; /* Debug 2 register */
  __ne__ uint32_t RESERVED3[8];

  /* Offset 0x080 */
  struct
  {
    __rw__ uint32_t CS; /* Message Buffer N CS register */
    __rw__ uint32_t ID; /* Message Buffer N ID register */
    __rw__ uint32_t WORD0; /* Message Buffer N WORD0 register */
    __rw__ uint32_t WORD1; /* Message Buffer N WORD1 register */
  } MB[64];
  __ne__ uint32_t RESERVED4[256];

  /* Offset 0x880 */
  __rw__ uint32_t RXIMR[64]; /* RX Individual Mask Registers */
  __ne__ uint32_t RESERVED5[24];

  /* Offset 0x9E0 */
  __rw__ uint32_t GFWR; /* Glitch Filter Width Registers */
} IMX_FLEXCAN_Type;
/*------------------Flexible I/O----------------------------------------------*/
typedef struct
{
  __ro__ uint32_t VERID; /* Version ID register */
  __ro__ uint32_t PARAM; /* Parameter register */
  __rw__ uint32_t CTRL; /* FlexIO Control register */
  __ro__ uint32_t PIN; /* Pin State register */
  __rw__ uint32_t SHIFTSTAT; /* Shifter Status register */
  __rw__ uint32_t SHIFTERR; /* Shifter Error register */
  __rw__ uint32_t TIMSTAT; /* Timer Status register */
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t SHIFTSIEN; /* Shifter Status Interrupt Enable register */
  __rw__ uint32_t SHIFTEIEN; /* Shifter Error Interrupt Enable register */
  __rw__ uint32_t TIMIEN; /* Timer Interrupt Enable register */
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t SHIFTSDEN; /* Shifter Status DMA Enable register */
  __ne__ uint32_t RESERVED2[3];
  __rw__ uint32_t SHIFTSTATE; /* Shifter State register */
  __ne__ uint32_t RESERVED3[15];
  __rw__ uint32_t SHIFTCTL[8]; /* Shifter Control N register */
  __ne__ uint32_t RESERVED4[24];

  /* Offset 0x100 */
  __rw__ uint32_t SHIFTCFG[8]; /* Shifter Configuration N register */
  __ne__ uint32_t RESERVED5[56];

  /* Offset 0x200 */
  __rw__ uint32_t SHIFTBUF[8]; /* Shifter Buffer N register */
  __ne__ uint32_t RESERVED6[24];
  __rw__ uint32_t SHIFTBUFBIS[8]; /* Shifter N Bit Swapped register */
  __ne__ uint32_t RESERVED7[24];

  /* Offset 0x300 */
  __rw__ uint32_t SHIFTBUFBYS[8]; /* Shifter N Byte Swapped register */
  __ne__ uint32_t RESERVED8[24];
  __rw__ uint32_t SHIFTBUFBBS[8]; /* Shifter N Bit-Byte Swapped register */
  __ne__ uint32_t RESERVED9[24];

  /* Offset 0x400 */
  __rw__ uint32_t TIMCTL[8]; /* Timer Control N register */
  __ne__ uint32_t RESERVED10[24];
  __rw__ uint32_t TIMCFG[8]; /* Timer Configuration N register */
  __ne__ uint32_t RESERVED11[24];

  /* Offset 0x500 */
  __rw__ uint32_t TIMCMP[8]; /* Timer Compare N register */
  __ne__ uint32_t RESERVED12[88];
  __rw__ uint32_t SHIFTBUFNBS[8]; /* Shifter N Nibble-Byte Swapped register */
  __ne__ uint32_t RESERVED13[24];

  /* Offset 0x700 */
  __rw__ uint32_t SHIFTBUFHWS[8]; /* Shifter N Half-Word Swapped register */
  __ne__ uint32_t RESERVED14[24];
  __rw__ uint32_t SHIFTBUFNIS[8]; /* Shifter N Nibble Swapped register */
} IMX_FLEXIO_Type;
/*------------------Enhanced Flex Pulse Width Modulator-----------------------*/
typedef struct
{
  struct
  {
    __ro__ uint16_t CNT; /* Counter register */
    __rw__ uint16_t INIT; /* Initial count register */
    __rw__ uint16_t CTRL2; /* Control register 2 register */
    __rw__ uint16_t CTRL; /* Control register */
    __ne__ uint16_t RESERVED0;

    struct
    {
      __rw__ uint16_t VALUE; /* Value register N */
      __rw__ uint16_t FRACTION; /* Fractional value register N */
    } VAL[6];

    __rw__ uint16_t FRCTRL; /* Fractional Control register */
    __rw__ uint16_t OCTRL; /* Output Control register */
    __rw__ uint16_t STS; /* Status register */
    __rw__ uint16_t INTEN; /* Interrupt Enable register */
    __rw__ uint16_t DMAEN; /* DMA Enable register */
    __rw__ uint16_t TCTRL; /* Output Trigger Control register */
    __rw__ uint16_t DISMAP; /* Fault Disable Mapping register */
    __ne__ uint16_t RESERVED1;

    union
    {
      struct
      {
        __rw__ uint16_t DTCNT0;
        __rw__ uint16_t DTCNT1;
      };
      __rw__ uint16_t DTCNT[2]; /* Deadtime Count registers */
    };

    union
    {
      struct
      {
        __rw__ uint16_t CAPTCTRLA; /* Capture Control A register */
        __rw__ uint16_t CAPTCOMPA; /* Capture Compare A register */
        __rw__ uint16_t CAPTCTRLB; /* Capture Control B register */
        __rw__ uint16_t CAPTCOMPB; /* Capture Compare B register */
        __rw__ uint16_t CAPTCTRLX; /* Capture Control X register */
        __rw__ uint16_t CAPTCOMPX; /* Capture Compare X register */
      };

      struct
      {
        __rw__ uint16_t CTRL;
        __rw__ uint16_t COMP;
      } CAPT[3];
    };

    struct
    {
      __ro__ uint16_t VALUE; /* Capture Value register N */
      __ro__ uint16_t CYCLE; /* Capture Value Cycle register N */
    } CVAL[6];

    __ne__ uint16_t RESERVED2[4];
  } CHANNEL[4];

  __rw__ uint16_t OUTEN; /* Output Enable register */
  __rw__ uint16_t MASK; /* Mask register */
  __rw__ uint16_t SWCOUT; /* Software Controlled Output register */
  __rw__ uint16_t DTSRCSEL; /* PWM Source Select register */
  __rw__ uint16_t MCTRL; /* Master Control register */
  __rw__ uint16_t MCTRL2; /* Master Control 2 register */
  __rw__ uint16_t FCTRL; /* Fault Control register */
  __rw__ uint16_t FSTS; /* Fault Status register */
  __rw__ uint16_t FFILT; /* Fault Filter register */
  __rw__ uint16_t FTST; /* Fault Test register */
  __rw__ uint16_t FCTRL2; /* Fault Control 2 register */
} IMX_FLEXPWM_Type;
/*------------------FlexRAM Manager-------------------------------------------*/
typedef struct
{
  __rw__ uint32_t TCM_CTRL; /* TCM Control register */
  __ne__ uint32_t RESERVED[3];
  __rw__ uint32_t INT_STATUS; /* Interrupt Status register */
  __rw__ uint32_t INT_STAT_EN; /* Interrupt Status Enable register */
  __rw__ uint32_t INT_SIG_EN; /* Interrupt Enable register */
} IMX_FLEXRAM_Type;
/*------------------Flexible Serial Peripheral Interface----------------------*/
typedef struct
{
  __rw__ uint32_t MCR0; /* Module Control Register 0 */
  __rw__ uint32_t MCR1; /* Module Control Register 1 */
  __rw__ uint32_t MCR2; /* Module Control Register 2 */
  __rw__ uint32_t AHBCR; /* AHB Bus Control Register */
  __rw__ uint32_t INTEN; /* Interrupt Enable register */
  __rw__ uint32_t INTR; /* Interrupt Register */
  __rw__ uint32_t LUTKEY; /* LUT Key register */
  __rw__ uint32_t LUTCR; /* LUT Control Register */
  __rw__ uint32_t AHBRXBUFCR0[4]; /* AHB RX Buffer Control Registers */
  __ne__ uint32_t RESERVED0[12];
  __rw__ uint32_t FLSHCR0[4]; /* Flash Control Register 0 */
  __rw__ uint32_t FLSHCR1[4]; /* Flash Control Register 1 */
  __rw__ uint32_t FLSHCR2[4]; /* Flash Control Register 2 */
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t FLSHCR4; /* Flash Control Register 4 */
  __ne__ uint32_t RESERVED2[2];
  __rw__ uint32_t IPCR0; /* IP Control Register 0 */
  __rw__ uint32_t IPCR1; /* IP Control Register 1 */
  __ne__ uint32_t RESERVED3[2];
  __rw__ uint32_t IPCMD; /* IP Command register */
  __ne__ uint32_t RESERVED4;
  __rw__ uint32_t IPRXFCR; /* IP RX FIFO Control Register */
  __rw__ uint32_t IPTXFCR; /* IP TX FIFO Control Register */
  __rw__ uint32_t DLLCR[2]; /* Delay Line Control Registers */
  __ne__ uint32_t RESERVED5[6];
  __ro__ uint32_t STS0; /* Status register 0 */
  __ro__ uint32_t STS1; /* Status register 1 */
  __ro__ uint32_t STS2; /* Status register 2 */
  __ro__ uint32_t AHBSPNDSTS; /* AHB Suspend Status register */
  __ro__ uint32_t IPRXFSTS; /* IP RX FIFO Status register */
  __ro__ uint32_t IPTXFSTS; /* IP TX FIFO Status register */
  __ne__ uint32_t RESERVED6[2];

  /* Offset 0x100 */
  __ro__ uint32_t RFDR[32]; /* IP RX FIFO Data Registers */
  __wo__ uint32_t TFDR[32]; /* IP TX FIFO Data Registers */
  __rw__ uint32_t LUT[64];
} IMX_FLEXSPI_Type;
/*------------------General Power Controller----------------------------------*/
typedef struct
{
  __rw__ uint32_t CNTR; /* Interface Control register */
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t IMR[4]; /* IRQ Masking Registers 1..4 */
  __ro__ uint32_t ISR[4]; /* IRQ Status Registers 1..4 */
  __ne__ uint32_t RESERVED1[3];
  __rw__ uint32_t IMR5; /* IRQ Masking Register 5 */
  __ro__ uint32_t ISR5; /* IRQ Status Register 5 */
} IMX_GPC_Type;
/*------------------General Purpose Input/Output------------------------------*/
typedef struct
{
  __rw__ uint32_t DR; /* Data Register */
  __rw__ uint32_t GDIR; /* Direction register */
  __ro__ uint32_t PSR; /* Pad Status Register */

  union
  {
    struct
    {
      __rw__ uint32_t ICR1;
      __rw__ uint32_t ICR2;
    };
    __rw__ uint32_t ICR[2]; /* Interrupt Configuration Registers */
  };

  /* Offset 0x14 */
  __rw__ uint32_t IMR; /* Interrupt Mask Register */
  __rw__ uint32_t ISR; /* interrupt Status Register */
  __rw__ uint32_t EDGE_SEL; /* Edge Select register */
  __ne__ uint32_t RESERVED0[25];

  /* Offset 0x84 */
  __wo__ uint32_t DR_SET; /* Data Register SET */
  __wo__ uint32_t DR_CLEAR; /* Data Register CLEAR */
  __wo__ uint32_t DR_TOGGLE; /* Data Register TOGGLE */
} IMX_GPIO_Type;
/*------------------General Purpose Timer-------------------------------------*/
typedef struct
{
  __rw__ uint32_t CR; /* Control Register */
  __rw__ uint32_t PR; /* Prescaler Register */
  __rw__ uint32_t SR; /* Status Register */
  __rw__ uint32_t IR; /* Interrupt Register */
  __rw__ uint32_t OCR[3]; /* Output Compare Registers */
  __ro__ uint32_t ICR[2]; /* Input Capture Registers */
  __ro__ uint32_t CNT; /* Counter register */
} IMX_GPT_Type;
/*------------------IOMUX Controller------------------------------------------*/
typedef struct
{
  __ne__ uint32_t RESERVED[5];

  /* Offset 0x014 */
  union
  {
    struct
    {
      /* Offset 0x014 */
      __rw__ uint32_t SW_MUX_CTL_PAD_GPIO_EMC[42];
      /* Offset 0x0BC */
      __rw__ uint32_t SW_MUX_CTL_PAD_GPIO_AD_B0[16];
      /* Offset 0x0FC */
      __rw__ uint32_t SW_MUX_CTL_PAD_GPIO_AD_B1[16];
      /* Offset 0x13C */
      __rw__ uint32_t SW_MUX_CTL_PAD_GPIO_B0[16];
      /* Offset 0x17C */
      __rw__ uint32_t SW_MUX_CTL_PAD_GPIO_B1[16];
      /* Offset 0x1BC */
      __rw__ uint32_t SW_MUX_CTL_PAD_GPIO_SD_B0[6];
      /* Offset 0x1D4 */
      __rw__ uint32_t SW_MUX_CTL_PAD_GPIO_SD_B1[12];
    };

    __rw__ uint32_t SW_MUX_CTL_PAD_A[124];
  };

  /* Offset 0x204 */
  union
  {
    struct
    {
      /* Offset 0x204 */
      __rw__ uint32_t SW_PAD_CTL_PAD_GPIO_EMC[42];
      /* Offset 0x2AC */
      __rw__ uint32_t SW_PAD_CTL_PAD_GPIO_AD_B0[16];
      /* Offset 0x2EC */
      __rw__ uint32_t SW_PAD_CTL_PAD_GPIO_AD_B1[16];
      /* Offset 0x32C */
      __rw__ uint32_t SW_PAD_CTL_PAD_GPIO_B0[16];
      /* Offset 0x36C */
      __rw__ uint32_t SW_PAD_CTL_PAD_GPIO_B1[16];
      /* Offset 0x3AC */
      __rw__ uint32_t SW_PAD_CTL_PAD_GPIO_SD_B0[6];
      /* Offset 0x3C4 */
      __rw__ uint32_t SW_PAD_CTL_PAD_GPIO_SD_B1[12];
    };

    __rw__ uint32_t SW_PAD_CTL_PAD_A[124];
  };

  /* Offset 0x3F4 */
  __rw__ uint32_t SELECT_INPUT_A[154];

  /* Offset 0x65C */
  union
  {
    struct
    {
      /* Offset 0x65C */
      __rw__ uint32_t SW_MUX_CTL_PAD_GPIO_SPI_B0[14];
      /* Offset 0x694 */
      __rw__ uint32_t SW_MUX_CTL_PAD_GPIO_SPI_B1[8];
    };

    __rw__ uint32_t SW_MUX_CTL_PAD_B[22];
  };

  /* Offset 0x6B4 */
  union
  {
    struct
    {
      /* Offset 0x6B4 */
      __rw__ uint32_t SW_PAD_CTL_PAD_GPIO_SPI_B0[14];
      /* Offset 0x6EC */
      __rw__ uint32_t SW_PAD_CTL_PAD_GPIO_SPI_B1[8];
    };

    __rw__ uint32_t SW_PAD_CTL_PAD_B[22];
  };

  /* Offset 0x70C */
  __rw__ uint32_t SELECT_INPUT_B[33];
} IMX_IOMUXC_Type;
/*------------------IOMUXC General Purpose Registers--------------------------*/
typedef struct
{
  __rw__ uint32_t GPR[35];
} IMX_IOMUXC_GPR_Type;
/*------------------IOMUXC SNVS Registers-------------------------------------*/
typedef struct
{
  union
  {
    struct
    {
      __rw__ uint32_t SW_MUX_CTL_PAD_WAKEUP;
      __rw__ uint32_t SW_MUX_CTL_PAD_PMIC_ON_REQ;
      __rw__ uint32_t SW_MUX_CTL_PAD_PMIC_STBY_REQ;
    };
    __rw__ uint32_t SW_MUX_CTL_PAD[3];
  };

  __rw__ uint32_t SW_PAD_CTL_PAD_TEST_MODE;
  __rw__ uint32_t SW_PAD_CTL_PAD_POR_B;
  __rw__ uint32_t SW_PAD_CTL_PAD_ONOFF;

  union
  {
    struct
    {
      __rw__ uint32_t SW_PAD_CTL_PAD_WAKEUP;
      __rw__ uint32_t SW_PAD_CTL_PAD_PMIC_ON_REQ;
      __rw__ uint32_t SW_PAD_CTL_PAD_PMIC_STBY_REQ;
    };
    __rw__ uint32_t SW_PAD_CTL_PAD[3];
  };
} IMX_IOMUXC_SNVS_Type;
/*------------------IOMUXC SNVS General Purpose Registers---------------------*/
typedef struct
{
  __rw__ uint32_t GPR[4];
} IMX_IOMUXC_SNVS_GPR_Type;
/*------------------Keypad Port-----------------------------------------------*/
typedef struct
{
  __rw__ uint16_t KPCR; /* Keypad Control Register */
  __rw__ uint16_t KPSR; /* Keypad Status Register */
  __rw__ uint16_t KDDR; /* Keypad Data Direction Register */
  __rw__ uint16_t KPDR; /* Keypad Data Register */
} IMX_KPP_Type;
/*------------------Enhanced LCD Interface------------------------------------*/
typedef struct {
  __rw__ uint32_t CTRL; /* General Control register */
  __rw__ uint32_t CTRL_SET;
  __rw__ uint32_t CTRL_CLR;
  __rw__ uint32_t CTRL_TOG;
  __rw__ uint32_t CTRL1; /* General Control register 1 */
  __rw__ uint32_t CTRL1_SET;
  __rw__ uint32_t CTRL1_CLR;
  __rw__ uint32_t CTRL1_TOG;
  __rw__ uint32_t CTRL2; /* General Control register 2 */
  __rw__ uint32_t CTRL2_SET;
  __rw__ uint32_t CTRL2_CLR;
  __rw__ uint32_t CTRL2_TOG;

  /* Offset 0x030 */
  __rw__ uint32_t TRANSFER_COUNT; /* Horizontal and Vertical Valid Data Count */
  __ne__ uint32_t RESERVED0[3];
  __rw__ uint32_t CUR_BUF; /* Current Buffer Address register */
  __ne__ uint32_t RESERVED1[3];
  __rw__ uint32_t NEXT_BUF; /* Next Buffer Address register */
  __ne__ uint32_t RESERVED2[7];
  __rw__ uint32_t VDCTRL0; /* VSYNC Mode and Dotclk Mode Control register 0 */
  __rw__ uint32_t VDCTRL0_SET;
  __rw__ uint32_t VDCTRL0_CLR;
  __rw__ uint32_t VDCTRL0_TOG;
  __rw__ uint32_t VDCTRL1; /* VSYNC Mode and Dotclk Mode Control register 1 */
  __ne__ uint32_t RESERVED3[3];
  __rw__ uint32_t VDCTRL2; /* VSYNC Mode and Dotclk Mode Control register 2 */
  __ne__ uint32_t RESERVED4[3];
  __rw__ uint32_t VDCTRL3; /* VSYNC Mode and Dotclk Mode Control register 3 */
  __ne__ uint32_t RESERVED5[3];
  __rw__ uint32_t VDCTRL4; /* VSYNC Mode and Dotclk Mode Control register 4 */
  __ne__ uint32_t RESERVED6[55];

  /* Offset 0x190 */
  __rw__ uint32_t BM_ERROR_STAT; /* Bus Master Error Status register */
  __ne__ uint32_t RESERVED7[3];
  __rw__ uint32_t CRC_STAT; /* CRC Status register */
  __ne__ uint32_t RESERVED8[3];
  __ro__ uint32_t STAT; /* Interface Status register */
  __ne__ uint32_t RESERVED9[115];

  /* Offset 0x380 */
  __rw__ uint32_t PIGEONCTRL0; /* Pigeon Mode Control register 0 */
  __rw__ uint32_t PIGEONCTRL0_SET;
  __rw__ uint32_t PIGEONCTRL0_CLR;
  __rw__ uint32_t PIGEONCTRL0_TOG;
  __rw__ uint32_t PIGEONCTRL1; /* Pigeon Mode Control register 1 */
  __rw__ uint32_t PIGEONCTRL1_SET;
  __rw__ uint32_t PIGEONCTRL1_CLR;
  __rw__ uint32_t PIGEONCTRL1_TOG;
  __rw__ uint32_t PIGEONCTRL2; /* Pigeon Mode Control register 2 */
  __rw__ uint32_t PIGEONCTRL2_SET;
  __rw__ uint32_t PIGEONCTRL2_CLR;
  __rw__ uint32_t PIGEONCTRL2_TOG;
  __ne__ uint32_t RESERVED10[276];

  /* Offset 0x800 */
  struct {
    __rw__ uint32_t PIGEON_0; /* Panel Interface Signal Generator register 0 */
    __ne__ uint32_t RESERVED0[3];
    __rw__ uint32_t PIGEON_1; /* Panel Interface Signal Generator register 1 */
    __ne__ uint32_t RESERVED1[3];
    __rw__ uint32_t PIGEON_2; /* Panel Interface Signal Generator register 2 */
    __ne__ uint32_t RESERVED2[7];
  } PIGEON[12];

  /* Offset 0xB00 */
  __rw__ uint32_t LUT_CTRL; /* Look Up Table Control register */
  __ne__ uint32_t RESERVED11[3];
  __rw__ uint32_t LUT0_ADDR; /* Lookup Table 0 Index register */
  __ne__ uint32_t RESERVED12[3];
  __rw__ uint32_t LUT0_DATA; /* Lookup Table 0 Data register */
  __ne__ uint32_t RESERVED13[3];
  __rw__ uint32_t LUT1_ADDR; /* Lookup Table 1 Index register */
  __ne__ uint32_t RESERVED14[3];
  __rw__ uint32_t LUT1_DATA; /* Lookup Table 1 Data register */
} IMX_LCDIF_Type;
/*------------------Low Power Inter-Integrated Circuit------------------------*/
typedef struct
{
  __ro__ uint32_t VERID; /* Version ID register */
  __ro__ uint32_t PARAM; /* Parameter register */
  __ne__ uint32_t RESERVED0[2];

  /* Offset 0x010 */
  __rw__ uint32_t MCR; /* Master Control Register */
  __rw__ uint32_t MSR; /* Master Status Register */
  __rw__ uint32_t MIER; /* Master Interrupt Enable Register */
  __rw__ uint32_t MDER; /* Master DMA Enable Register */

  /* Offset 0x020 */
  __rw__ uint32_t MCFGR0; /* Master Configuration Register 0*/
  __rw__ uint32_t MCFGR1; /* Master Configuration Register 1*/
  __rw__ uint32_t MCFGR2; /* Master Configuration Register 2*/
  __rw__ uint32_t MCFGR3; /* Master Configuration Register 3*/
  __ne__ uint32_t RESERVED1[4];

  /* Offset 0x040 */
  __rw__ uint32_t MDMR; /* Master Data Match Register */
  __ne__ uint32_t RESERVED2;
  __rw__ uint32_t MCCR0; /* Master Clock Configuration Register 0 */
  __ne__ uint32_t RESERVED3;
  __rw__ uint32_t MCCR1; /* Master Clock Configuration Register 1 */
  __ne__ uint32_t RESERVED4;
  __rw__ uint32_t MFCR; /* Master FIFO Control Register */
  __ro__ uint32_t MFSR; /* Master FIFO Status Register */
  __wo__ uint32_t MTDR; /* Master Transmit Data Register */
  __ne__ uint32_t RESERVED5[3];

  /* Offset 0x070 */
  __ro__ uint32_t MRDR; /* Master Receive Data Register */
  __ne__ uint32_t RESERVED6[39];

  /* Offset 0x110 */
  __rw__ uint32_t SCR; /* Slave Control Register */
  __rw__ uint32_t SSR; /* Slave Status Register */
  __rw__ uint32_t SIER; /* Slave Interrupt Enable Register */
  __rw__ uint32_t SDER; /* Slave DMA Enable Register */
  __ne__ uint32_t RESERVED7;
  __rw__ uint32_t SCFGR1; /* Slave Configuration Register 1 */
  __rw__ uint32_t SCFGR2; /* Slave Configuration Register 2 */
  __ne__ uint32_t RESERVED8[5];

  /* Offset 0x140 */
  __rw__ uint32_t SAMR; /* Slave Address Match Register */
  __ne__ uint32_t RESERVED9[3];
  __ro__ uint32_t SASR; /* Slave Address Status Register */
  __rw__ uint32_t STAR; /* Slave Transmit ACK Register */
  __ne__ uint32_t RESERVED10[2];

  /* Offset 0x160 */
  __wo__ uint32_t STDR; /* Slave Transmit Data Register */
  __ne__ uint32_t RESERVED11[3];
  __ro__ uint32_t SRDR; /* Slave Receive Data Register */
} IMX_LPI2C_Type;
/*------------------Low Power Serial Peripheral Interface---------------------*/
typedef struct
{
  __ro__ uint32_t VERID; /* Version ID register */
  __ro__ uint32_t PARAM; /* Parameter register */
  __ne__ uint32_t RESERVED0[2];

  /* Offset 0x10 */
  __rw__ uint32_t CR; /* Control Register */
  __rw__ uint32_t SR; /* Status Register */
  __rw__ uint32_t IER; /* Interrupt Enable Register */
  __rw__ uint32_t DER; /* DMA Enable Register */

  /* Offset 0x20 */
  __rw__ uint32_t CFGR0; /* Configuration Register 0 */
  __rw__ uint32_t CFGR1; /* Configuration Register 1 */
  __ne__ uint32_t RESERVED1[2];

  /* Offset 0x30 */
  union
  {
    struct
    {
      __rw__ uint32_t DMR0;
      __rw__ uint32_t DMR1;
    };
    __rw__ uint32_t DMR[2]; /* Data Match Registers */
  };
  __ne__ uint32_t RESERVED2[2];

  /* Offset 0x40 */
  __rw__ uint32_t CCR; /* Clock Configuration Register */
  __ne__ uint32_t RESERVED3[5];
  __rw__ uint32_t FCR; /* FIFO Control Register */
  __ro__ uint32_t FSR; /* FIFO Status Register */

  /* Offset 0x60 */
  __rw__ uint32_t TCR; /* Transmit Command Register */
  __wo__ uint32_t TDR; /* Transmit Data Register */
  __ne__ uint32_t RESERVED4[2];

  /* Offset 0x70 */
  __ro__ uint32_t RSR; /* Receive Status Register */
  __ro__ uint32_t RDR; /* Receive Data Register */
} IMX_LPSPI_Type;
/*------------------Low Power Universal Asynchronous Receiver/Transmitter-----*/
typedef struct
{
  __ro__ uint32_t VERID; /* Version ID register */
  __ro__ uint32_t PARAM; /* Parameter register */
  __rw__ uint32_t GLOBAL; /* Global register */
  __rw__ uint32_t PINCFG; /* Pin Configuration register */
  __rw__ uint32_t BAUD; /* Baud Rate register */
  __rw__ uint32_t STAT; /* Status register */
  __rw__ uint32_t CTRL; /* Control register */
  __rw__ uint32_t DATA; /* Data register */
  __rw__ uint32_t MATCH; /* Match Address register */
  __rw__ uint32_t MODIR; /* Modem IrDA register */
  __rw__ uint32_t FIFO; /* FIFO register */
  __rw__ uint32_t WATER; /* Watermark register */
} IMX_LPUART_Type;
/*------------------On-Chip OTP Controller------------------------------------*/
typedef struct
{
  __rw__ uint32_t CTRL; /* Control and Status register */
  __rw__ uint32_t CTRL_SET;
  __rw__ uint32_t CTRL_CLR;
  __rw__ uint32_t CTRL_TOG;
  __rw__ uint32_t TIMING; /* Timing register */
  __ne__ uint32_t RESERVED0[3];
  __rw__ uint32_t DATA; /* Write Data register */
  __ne__ uint32_t RESERVED1[3];
  __rw__ uint32_t READ_CTRL; /* Write Data register */
  __ne__ uint32_t RESERVED2[3];
  __rw__ uint32_t READ_FUSE_DATA; /* Read Data register */
  __ne__ uint32_t RESERVED3[3];
  __rw__ uint32_t SW_STICKY; /* Sticky Bit register */
  __ne__ uint32_t RESERVED4[3];

  /* Offset 0x060 */
  __rw__ uint32_t SCS; /* Software Controllable Signals register */
  __rw__ uint32_t SCS_SET;
  __rw__ uint32_t SCS_CLR;
  __rw__ uint32_t SCS_TOG;
  __ne__ uint32_t RESERVED5[8];
  __ro__ uint32_t VERSION; /* Version register */
  __ne__ uint32_t RESERVED6[27];

  /* Offset 0x100 */
  __rw__ uint32_t TIMING2; /* Timing register 2 */
  __ne__ uint32_t RESERVED7[191];

  /* Offset 0x400 */
  __rw__ uint32_t LOCK; /* Value of OTP Bank 0 Word 0 (Lock Controls) */
  __ne__ uint32_t RESERVED8[3];

  /* Offset 0x410 */
  struct
  {
    __rw__ uint32_t WORD; /* Value of OTP Bank 0 Words 1..7 */
    __ne__ uint32_t RESERVED[3];
  } CFG[7];

  /* Offset 0x480 */
  struct
  {
    __rw__ uint32_t WORD; /* Value of OTP Bank 1 Words 0..4 */
    __ne__ uint32_t RESERVED[3];
  } MEM[5];

  /* Offset 0x4D0 */
  struct
  {
    __rw__ uint32_t WORD; /* Value of OTP Bank 1 Words 5..7 */
    __ne__ uint32_t RESERVED[3];
  } ANA[3];
  __ne__ uint32_t RESERVED9[32];

  /* Offset 0x580 */
  struct
  {
    __rw__ uint32_t WORD; /* Value of OTP Bank 3 Words 0..7 */
    __ne__ uint32_t RESERVED[3];
  } SRK[8];

  /* Offset 0x600 */
  struct
  {
    __rw__ uint32_t WORD; /* Value of OTP Bank 4 Words 0..1 */
    __ne__ uint32_t RESERVED[3];
  } SJC_RESP[2];

  /* Offset 0x620 */
  struct
  {
    __rw__ uint32_t WORD; /* Value of OTP Bank 4 Words 2..4 */
    __ne__ uint32_t RESERVED[3];
  } MAC[3];
  __ne__ uint32_t RESERVED10[3];

  /* Offset 0x660 */
  struct
  {
    __rw__ uint32_t WORD; /* Value of OTP Bank 4 Words 6..7 */
    __ne__ uint32_t RESERVED[3];
  } GP[2];

  /* Offset 0x680 */
  __rw__ uint32_t SW_GP1; /* Value of OTP Bank 5 Word 0 */
  __ne__ uint32_t RESERVED11[3];

  /* Offset 0x690 */
  struct
  {
    __rw__ uint32_t WORD; /* Value of OTP Bank 5 Words 1..4 */
    __ne__ uint32_t RESERVED[3];
  } SW_GP2[3];

  /* Offset 0x6D0 */
  struct
  {
    __rw__ uint32_t WORD; /* Value of OTP Bank 4 Words 6..7 */
    __ne__ uint32_t RESERVED[3];
  } MISC_CONF[2];

  /* Offset 0x6F0 */
  __rw__ uint32_t SRK_REVOKE; /* Value of OTP Bank 5 Word 7 */
  __ne__ uint32_t RESERVED12[99];

  /* Offset 0x880 */
  struct
  {
    __rw__ uint32_t WORD; /* Value of OTP Bank 7 Words 0..3 */
    __ne__ uint32_t RESERVED[3];
  } SW_GP3[4];

  /* Offset 0x8C0 */
  struct
  {
    __rw__ uint32_t WORD; /* Value of OTP Bank 7 Words 4..7 */
    __ne__ uint32_t RESERVED[3];
  } SW_GP4[4];
} IMX_OCOTP_Type;
/*------------------Power Gating Control--------------------------------------*/
typedef struct
{
  __ne__ uint32_t RESERVED0[136];

  /* Offset 0x220 */
  __rw__ uint32_t MEGA_CTRL; /* Mega Control register */
  __rw__ uint32_t MEGA_PUPSCR; /* Mega Power Up Sequence Control Register */
  __rw__ uint32_t MEGA_PDNSCR; /* Mega Pull Down Sequence Control Register */
  __rw__ uint32_t MEGA_SR; /* Mega Power Gating Controller Status Register */
  __ne__ uint32_t RESERVED1[28];

  /* Offset 0x2A0 */
  __rw__ uint32_t CPU_CTRL; /* CPU Control register */
  __rw__ uint32_t CPU_PUPSCR; /* CPU Power Up Sequence Control Register */
  __rw__ uint32_t CPU_PDNSCR; /* CPU Pull Down Sequence Control Register */
  __rw__ uint32_t CPU_SR; /* CPU Power Gating Controller Status Register */
} IMX_PGC_Type;
/*------------------Periodic Interrupt Timer----------------------------------*/
typedef struct
{
  __rw__ uint32_t MCR; /* Module Control Register */
  __ne__ uint32_t RESERVED0[55];

  /* Offset 0x0E0 */
  __ro__ uint32_t LTMR64H; /* Upper Lifetime Timer register */
  __ro__ uint32_t LTMR64L; /* Lower Lifetime Timer register */
  __ne__ uint32_t RESERVED1[6];

  /* Offset 0x100 */
  struct
  {
    __rw__ uint32_t LDVAL; /* Timer Load Value register */
    __ro__ uint32_t CVAL; /* Current Timer Value register */
    __rw__ uint32_t TCTRL; /* Timer Control register */
    __rw__ uint32_t TFLG; /* Timer Flag register */
  } CHANNEL[4];
} IMX_PIT_Type;
/*------------------Power Management Unit-------------------------------------*/
typedef struct
{
  __ne__ uint32_t RESERVED[68];

  /* Offset 0x110 */
  __rw__ uint32_t REG_1P1; /* Regulator 1P1 register */
  __rw__ uint32_t REG_1P1_SET;
  __rw__ uint32_t REG_1P1_CLR;
  __rw__ uint32_t REG_1P1_TOG;
  __rw__ uint32_t REG_3P0; /* Regulator 3P0 register */
  __rw__ uint32_t REG_3P0_SET;
  __rw__ uint32_t REG_3P0_CLR;
  __rw__ uint32_t REG_3P0_TOG;
  __rw__ uint32_t REG_2P5; /* Regulator 2P5 register */
  __rw__ uint32_t REG_2P5_SET;
  __rw__ uint32_t REG_2P5_CLR;
  __rw__ uint32_t REG_2P5_TOG;
  __rw__ uint32_t REG_CORE; /* Digital Regulator Core register */
  __rw__ uint32_t REG_CORE_SET;
  __rw__ uint32_t REG_CORE_CLR;
  __rw__ uint32_t REG_CORE_TOG;
  __rw__ uint32_t MISC0; /* Miscellaneous register 0 */
  __rw__ uint32_t MISC0_SET;
  __rw__ uint32_t MISC0_CLR;
  __rw__ uint32_t MISC0_TOG;
  __rw__ uint32_t MISC1; /* Miscellaneous register 1 */
  __rw__ uint32_t MISC1_SET;
  __rw__ uint32_t MISC1_CLR;
  __rw__ uint32_t MISC1_TOG;
  __rw__ uint32_t MISC2; /* Miscellaneous register 2 */
  __rw__ uint32_t MISC2_SET;
  __rw__ uint32_t MISC2_CLR;
  __rw__ uint32_t MISC2_TOG;
} IMX_PMU_Type;
/*------------------Pixel Pipeline--------------------------------------------*/
typedef struct {
  __rw__ uint32_t CTRL; /* Control register 0 */
  __rw__ uint32_t CTRL_SET;
  __rw__ uint32_t CTRL_CLR;
  __rw__ uint32_t CTRL_TOG;
  __rw__ uint32_t STAT; /* Status register */
  __rw__ uint32_t STAT_SET;
  __rw__ uint32_t STAT_CLR;
  __rw__ uint32_t STAT_TOG;

  /* Offset 0x020 */
  __rw__ uint32_t OUT_CTRL; /* Output Buffer Control register */
  __rw__ uint32_t OUT_CTRL_SET;
  __rw__ uint32_t OUT_CTRL_CLR;
  __rw__ uint32_t OUT_CTRL_TOG;
  __rw__ uint32_t OUT_BUF; /* Output Frame Buffer Pointer */
  __ne__ uint32_t RESERVED0[3];
  __rw__ uint32_t OUT_BUF2; /* Output Frame Buffer Pointer 2 */
  __ne__ uint32_t RESERVED1[3];
  __rw__ uint32_t OUT_PITCH; /* Output Buffer Pitch */
  __ne__ uint32_t RESERVED2[3];
  __rw__ uint32_t OUT_LRC; /* Output Surface Lower Right Coordinate */
  __ne__ uint32_t RESERVED3[3];
  __rw__ uint32_t OUT_PS_ULC; /* Processed Surface Upper Left Coordinate */
  __ne__ uint32_t RESERVED4[3];
  __rw__ uint32_t OUT_PS_LRC; /* Processed Surface Lower Right Coordinate */
  __ne__ uint32_t RESERVED5[3];
  __rw__ uint32_t OUT_AS_ULC; /* Alpha Surface Upper Left Coordinate */
  __ne__ uint32_t RESERVED6[3];
  __rw__ uint32_t OUT_AS_LRC; /* Alpha Surface Lower Right Coordinate */
  __ne__ uint32_t RESERVED7[3];

  /* Offset 0x0B0 */
  __rw__ uint32_t PS_CTRL; /* Processed Surface (PS) Control register */
  __rw__ uint32_t PS_CTRL_SET;
  __rw__ uint32_t PS_CTRL_CLR;
  __rw__ uint32_t PS_CTRL_TOG;
  __rw__ uint32_t PS_BUF; /* PS Input Buffer Address */
  __ne__ uint32_t RESERVED8[3];
  __rw__ uint32_t PS_UBUF; /* PS U/Cb or 2 Plane UV Input Buffer Address */
  __ne__ uint32_t RESERVED9[3];
  __rw__ uint32_t PS_VBUF; /* PS V/Cr Input Buffer Address */
  __ne__ uint32_t RESERVED10[3];
  __rw__ uint32_t PS_PITCH; /* Processed Surface Pitch */
  __ne__ uint32_t RESERVED11[3];
  __rw__ uint32_t PS_BACKGROUND; /* PS Background Color */
  __ne__ uint32_t RESERVED12[3];
  __rw__ uint32_t PS_SCALE; /* PS Scale Factor register */
  __ne__ uint32_t RESERVED13[3];
  __rw__ uint32_t PS_OFFSET; /* PS Scale Offset register */
  __ne__ uint32_t RESERVED14[3];
  __rw__ uint32_t PS_CLRKEYLOW; /* PS Color Key Low */
  __ne__ uint32_t RESERVED15[3];
  __rw__ uint32_t PS_CLRKEYHIGH; /* PS Color Key High */
  __ne__ uint32_t RESERVED16[3];

  /* Offset 0x150 */
  __rw__ uint32_t AS_CTRL; /* Alpha Surface Control */
  __ne__ uint32_t RESERVED17[3];
  __rw__ uint32_t AS_BUF; /* Alpha Surface Buffer Pointer */
  __ne__ uint32_t RESERVED18[3];
  __rw__ uint32_t AS_PITCH; /* Alpha Surface Pitch */
  __ne__ uint32_t RESERVED19[3];

  /* Offset 0x180 */
  __rw__ uint32_t AS_CLRKEYLOW; /* Overlay Color Key Low */
  __ne__ uint32_t RESERVED20[3];
  __rw__ uint32_t AS_CLRKEYHIGH; /* Overlay Color Key High */
  __ne__ uint32_t RESERVED21[3];

  /* Offset 0x1A0 */
  union
  {
    struct
    {
      __rw__ uint32_t CSC1_COEF0;
      __ne__ uint32_t RESERVED22[3];
      __rw__ uint32_t CSC1_COEF1;
      __ne__ uint32_t RESERVED23[3];
      __rw__ uint32_t CSC1_COEF2;
      __ne__ uint32_t RESERVED24[3];
    };

    struct
    {
      __rw__ uint32_t COEF; /* Color Space Conversion Coefficient registers */
      __ne__ uint32_t RESERVED[3];
    } CSC1[3];
  };
  __ne__ uint32_t RESERVED25[84];

  /* Offset 0x320 */
  __rw__ uint32_t POWER; /* Power control register, offset: 0x320 */
  __ne__ uint32_t RESERVED26[55];

  /* Offset 0x400 */
  __rw__ uint32_t NEXT; /* Next frame pointer */
  __ne__ uint32_t RESERVED27[15];

  /* Offset 0x440 */
  __rw__ uint32_t PORTER_DUFF_CTRL; /* Alpha Engine A control register */
} IMX_PXP_Type;
/*------------------High Reliability Watchdog Timer---------------------------*/
typedef struct
{
  __rw__ uint32_t CS; /* Control and Status register */
  __rw__ uint32_t CNT; /* Counter register */
  __rw__ uint32_t TOVAL; /* Timeout Value register */
  __rw__ uint32_t WIN; /* Window register */
} IMX_RTWDOG_Type;
/*------------------Synchronous Audio Interface-------------------------------*/
typedef struct
{
  __ro__ uint32_t VERID; /* Version ID register */
  __ro__ uint32_t PARAM; /* Parameter register */
  __rw__ uint32_t TCSR; /* Transmit Control Register */
  __rw__ uint32_t TCR1; /* Transmit Configuration Register 1 */
  __rw__ uint32_t TCR2; /* Transmit Configuration Register 2 */
  __rw__ uint32_t TCR3; /* Transmit Configuration Register 3 */
  __rw__ uint32_t TCR4; /* Transmit Configuration Register 4 */
  __rw__ uint32_t TCR5; /* Transmit Configuration Register 5 */

  /* Offset 0x20 */
  __wo__ uint32_t TDR[4]; /* Transmit Data Registers */
  __ne__ uint32_t RESERVED0[4];
  __ro__ uint32_t TFR[4]; /* Transmit FIFO Registers */
  __ne__ uint32_t RESERVED1[4];
  __rw__ uint32_t TMR; /* Transmit Mask Registers */
  __ne__ uint32_t RESERVED2[9];

  /* Offset 0x88 */
  __rw__ uint32_t RCSR; /* Receive Control Register */
  __rw__ uint32_t RCR1; /* Receive Configuration Register 1 */
  __rw__ uint32_t RCR2; /* Receive Configuration Register 2 */
  __rw__ uint32_t RCR3; /* Receive Configuration Register 3 */
  __rw__ uint32_t RCR4; /* Receive Configuration Register 4 */
  __rw__ uint32_t RCR5; /* Receive Configuration Register 5 */
  __ro__ uint32_t RDR[4]; /* Receive Data Registers */
  __ne__ uint32_t RESERVED3[4];
  __ro__ uint32_t RFR[4]; /* Receive FIFO Registers */
  __ne__ uint32_t RESERVED4[4];
  __rw__ uint32_t RMR; /* Receive Mask Registers */
} IMX_SAI_Type;
/*------------------Smart External Memory Controller--------------------------*/
typedef struct
{
  __rw__ uint32_t MCR; /* Module Control Register */
  __rw__ uint32_t IOCR; /* IO MUX Control Register */
  __rw__ uint32_t BMCR0; /* AXI Bus Master Control Register 0 */
  __rw__ uint32_t BMCR1; /* AXI Bus Master Control Register 1 */

  /* Offset 0x10 */
  __rw__ uint32_t BR[9]; /* Base Registers */
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t INTEN; /* Interrupt Enable register */
  __rw__ uint32_t INTR; /* Interrupt Register */

  /* Offset 0x40 */
  __rw__ uint32_t SDRAMCR0; /* SDRAM Control Register 0 */
  __rw__ uint32_t SDRAMCR1; /* SDRAM Control Register 1 */
  __rw__ uint32_t SDRAMCR2; /* SDRAM Control Register 2 */
  __rw__ uint32_t SDRAMCR3; /* SDRAM Control Register 3 */
  __rw__ uint32_t NANDCR0; /* NAND Control Register 0 */
  __rw__ uint32_t NANDCR1; /* NAND Control Register 1 */
  __rw__ uint32_t NANDCR2; /* NAND Control Register 2 */
  __rw__ uint32_t NANDCR3; /* NAND Control Register 3 */

  /* Offset 0x60 */
  __rw__ uint32_t NORCR0; /* NOR Control Register 0 */
  __rw__ uint32_t NORCR1; /* NOR Control Register 1 */
  __rw__ uint32_t NORCR2; /* NOR Control Register 2 */
  __rw__ uint32_t NORCR3; /* NOR Control Register 3 */
  __rw__ uint32_t SRAMCR0; /* SRAM Control Register 0 */
  __rw__ uint32_t SRAMCR1; /* SRAM Control Register 1 */
  __rw__ uint32_t SRAMCR2; /* SRAM Control Register 2 */
  __rw__ uint32_t SRAMCR3; /* SRAM Control Register 3 */

  /* Offset 0x80 */
  __rw__ uint32_t DBICR0; /* DBI-B Control Register 0 */
  __rw__ uint32_t DBICR1; /* DBI-B Control Register 1 */
  __ne__ uint32_t RESERVED1[2];
  __rw__ uint32_t IPCR0; /* IP Command Control Register 0 */
  __rw__ uint32_t IPCR1; /* IP Command Control Register 1 */
  __rw__ uint32_t IPCR2; /* IP Command Control Register 2 */
  __rw__ uint32_t IPCMD; /* IP Command register */

  /* Offset 0xA0 */
  __rw__ uint32_t IPTXDAT; /* TX Data register */
  __ne__ uint32_t RESERVED2[3];
  __ro__ uint32_t IPRXDAT; /* RX Data register */
  __ne__ uint32_t RESERVED3[3];

  /* Offset 0xC0 */
  union
  {
    struct
    {
      __ro__ uint32_t STS0;
      __ne__ uint32_t RESERVED4;
      __ro__ uint32_t STS2;
      __ne__ uint32_t RESERVED5[9];
      __ro__ uint32_t STS12;
      __ne__ uint32_t RESERVED6[3];
    };
    __ro__ uint32_t STS[16]; /* Status registers */
  };
} IMX_SEMC_Type;
/*------------------Secure Non-Volatile Storage-------------------------------*/
typedef struct
{
  __rw__ uint32_t HPLR; /* Lock Register */
  __rw__ uint32_t HPCOMR; /* Command Register */
  __rw__ uint32_t HPCR; /* Control Register */
  __rw__ uint32_t HPSICR; /* Security Interrupt Control Register */
  __rw__ uint32_t HPSVCR; /* Security Violation Control Register */
  __rw__ uint32_t HPSR; /* Status Register */
  __rw__ uint32_t HPSVSR; /* Security Violation Status Register */
  __rw__ uint32_t HPHACIVR; /* High Assurance Counter IV Register */
  __ro__ uint32_t HPHACR; /* High Assurance Counter Register */
  __rw__ uint32_t HPRTCMR; /* Real Time Counter MSB Register */
  __rw__ uint32_t HPRTCLR; /* Real Time Counter LSB Register */
  __rw__ uint32_t HPTAMR; /* Time Alarm MSB Register */
  __rw__ uint32_t HPTALR; /* Time Alarm LSB Register */
  __rw__ uint32_t LPLR; /* Lock Register */
  __rw__ uint32_t LPCR; /* Control Register */
  __rw__ uint32_t LPMKCR; /* Master Key Control Register */
  __rw__ uint32_t LPSVCR; /* Security Violation Control Register */
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t LPSECR; /* Security Events Configuration Register */
  __rw__ uint32_t LPSR; /* Status Register */
  __rw__ uint32_t LPSRTCMR; /* Secure Real Time Counter MSB Register */
  __rw__ uint32_t LPSRTCLR; /* Secure Real Time Counter LSB Register */
  __rw__ uint32_t LPTAR; /* Time Alarm Register */
  __rw__ uint32_t LPSMCMR; /* Secure Monotonic Counter MSB Register */
  __rw__ uint32_t LPSMCLR; /* Secure Monotonic Counter LSB Register */
  __rw__ uint32_t LPLVDR; /* Digital Low-Voltage Detector Register */
  __rw__ uint32_t LPGPR0_ALIAS; /* General Purpose Register 0 (alias) */
  __rw__ uint32_t LPZMKR[8]; /* Zeroizable Master Key Register */
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t LPGPR_ALIAS[4]; /* General Purpose Registers 0..3 (alias) */
  __ne__ uint32_t RESERVED2[24];

  /* Offset 0x100 */
  __rw__ uint32_t LPGPR[8]; /* General Purpose Registers */
  __ne__ uint32_t RESERVED3[694];

  /* Offset 0xBF8 */
  __ro__ uint32_t HPVIDR1; /* Version ID Register 1 */
  __ro__ uint32_t HPVIDR2; /* Version ID Register 2 */
} IMX_SNVS_Type;
/*------------------Sony/Philips Digital Interface----------------------------*/
typedef struct
{
  __rw__ uint32_t SCR; /* SPDIF Configuration register */
  __rw__ uint32_t SRCD; /* CDText Control register */
  __rw__ uint32_t SRPC; /* PhaseConfig register */
  __rw__ uint32_t SIE; /* InterruptEn register */

  /* Offset 0x10 */
  union
  {
    __wo__ uint32_t SIC; /* InterruptClear register */
    __ro__ uint32_t SIS; /* InterruptStat register */
  };

  /* Offset 0x14 */
  __ro__ uint32_t SRL; /* SPDIFRxLeft Register */
  __ro__ uint32_t SRR; /* SPDIFRxRight Register */
  __ro__ uint32_t SRCSH; /* SPDIFRxCChannel_h Register */

  /* Offset 0x20 */
  __ro__ uint32_t SRCSL; /* SPDIFRxCChannel_l register */
  __ro__ uint32_t SRU; /* UchannelRx register */
  __ro__ uint32_t SRQ; /* QchannelRx register */
  __wo__ uint32_t STL; /* SPDIFTxLeft register */
  __wo__ uint32_t STR; /* SPDIFTxRight register */
  __rw__ uint32_t STCSCH; /* SPDIFTxCChannelCons_h register */
  __rw__ uint32_t STCSCL; /* SPDIFTxCChannelCons_l register */
  __ne__ uint32_t RESERVED0[2];

  /* Offset 0x44 */
  __ro__ uint32_t SRFM; /* FreqMeas register */
  __ne__ uint32_t RESERVED1[2];
  __rw__ uint32_t STC; /* SPDIFTxClk register */
} IMX_SPDIF_Type;
/*------------------System Reset Controller-----------------------------------*/
typedef struct
{
  __rw__ uint32_t SCR; /* System Control Register */
  __ro__ uint32_t SBMR1; /* System Boot Mode Register 1 */
  __rw__ uint32_t SRSR; /* System Reset Status Register */
  __ne__ uint32_t RESERVED[4];
  __ro__ uint32_t SBMR2; /* System Boot Mode Register 2 */
  __rw__ uint32_t GPR[10]; /* General Purpose registers */
} IMX_SRC_Type;
/*------------------Temperature Monitor---------------------------------------*/
typedef struct
{
  __ne__ uint32_t RESERVED0[96];
  __rw__ uint32_t TEMPSENSE0; /* Temperature Sensor control register 0 */
  __rw__ uint32_t TEMPSENSE0_SET;
  __rw__ uint32_t TEMPSENSE0_CLR;
  __rw__ uint32_t TEMPSENSE0_TOG;
  __rw__ uint32_t TEMPSENSE1; /* Temperature Sensor control register 1 */
  __rw__ uint32_t TEMPSENSE1_SET;
  __rw__ uint32_t TEMPSENSE1_CLR;
  __rw__ uint32_t TEMPSENSE1_TOG;
  __ne__ uint32_t RESERVED1[60];
  __rw__ uint32_t TEMPSENSE2; /* Temperature Sensor control register 2 */
  __rw__ uint32_t TEMPSENSE2_SET;
  __rw__ uint32_t TEMPSENSE2_CLR;
  __rw__ uint32_t TEMPSENSE2_TOG;
} IMX_TEMPMON_Type;
/*------------------Quad Timer------------------------------------------------*/
typedef struct
{
  struct
  {
    union
    {
      struct
      {
        __rw__ uint16_t COMP1;
        __rw__ uint16_t COMP2;
      };
      __rw__ uint16_t COMP[2]; /* Timer Channel Compare registers */
    };

    __rw__ uint16_t CAPT; /* Timer Channel Capture register */
    __rw__ uint16_t LOAD; /* Timer Channel Load register */
    __rw__ uint16_t HOLD; /* Timer Channel Hold register */
    __rw__ uint16_t CNTR; /* Timer Channel Counter register */
    __rw__ uint16_t CTRL; /* Timer Channel Control register */
    __rw__ uint16_t SCTRL; /* Timer Channel Status and Control register */

    union
    {
      struct
      {
        __rw__ uint16_t CMPLD1;
        __rw__ uint16_t CMPLD2;
      };
      __rw__ uint16_t CMPLD[2]; /* Timer Channel Comparator Load registers */
    };

    __rw__ uint16_t CSCTRL; /* Timer Channel Comparator Status and Control */
    __rw__ uint16_t FILT; /* Timer Channel Input Filter register */
    __rw__ uint16_t DMA; /* Timer Channel DMA Enable register */
    __ne__ uint16_t RESERVED0[2];
    __rw__ uint16_t ENBL; /* Timer Channel Enable register */
  } CHANNEL[4];
} IMX_TMR_Type;
/*------------------Standalone True Random Number Generator-------------------*/
typedef struct
{
  __rw__ uint32_t MCTL; /* Miscellaneous Control register */
  __rw__ uint32_t SCMISC; /* Statistical Check Miscellaneous register */
  __rw__ uint32_t PKRRNG; /* Poker Range register */

  union
  {
    __rw__ uint32_t PKRMAX; /* Poker Maximum Limit register */
    __ro__ uint32_t PKRSQ; /* Poker Square Calculation Result register */
  };

  /* Offset 0x10 */
  __rw__ uint32_t SDCTL; /* Seed Control register */

  union
  {
    __rw__ uint32_t SBLIM; /* Sparse Bit Limit register */
    __ro__ uint32_t TOTSAM; /* Total Samples register */
  };

  /* Offset 0x19 */
  __rw__ uint32_t FRQMIN; /* Frequency Count Minimum Limit register */

  union
  {
    __ro__ uint32_t FRQCNT; /* Frequency Count register */
    __rw__ uint32_t FRQMAX; /* Frequency Count Maximum Limit register */
  };

  union
  {
    __ro__ uint32_t SCMC; /* Statistical Check Monobit Count register */
    __rw__ uint32_t SCML; /* Statistical Check Monobit Limit register */
  };

  /* Offset 0x24 */
  struct
  {
    union
    {
      __ro__ uint32_t COUNT; /* Statistical Check Run Length Count register */
      __rw__ uint32_t LIMIT; /* Statistical Check Run Length Limit register */
    };
  } SCR[6];

  /* Offset 0x3C */
  __ro__ uint32_t STATUS; /* Status register */

  /* Offset 0x40 */
  __ro__ uint32_t ENT[16]; /* Entropy Read registers */

  /* Offset 0x80 */
  __ro__ uint32_t PKRCNT10; /* Statistical Check Poker Count 1 and 0 register */
  __ro__ uint32_t PKRCNT32; /* Statistical Check Poker Count 3 and 2 register */
  __ro__ uint32_t PKRCNT54; /* Statistical Check Poker Count 5 and 4 register */
  __ro__ uint32_t PKRCNT76; /* Statistical Check Poker Count 7 and 6 register */
  __ro__ uint32_t PKRCNT98; /* Statistical Check Poker Count 9 and 8 register */
  __ro__ uint32_t PKRCNTBA; /* Statistical Check Poker Count B and A register */
  __ro__ uint32_t PKRCNTDC; /* Statistical Check Poker Count D and C register */
  __ro__ uint32_t PKRCNTFE; /* Statistical Check Poker Count F and E register */
  __rw__ uint32_t SEC_CFG; /* Security Configuration register */
  __rw__ uint32_t INT_CTRL; /* Interrupt Control register */
  __rw__ uint32_t INT_MASK; /* Mask register */
  __ro__ uint32_t INT_STATUS; /* Interrupt Status register */
  __ne__ uint8_t RESERVED0[64];

  /* Offset 0xF0 */
  __ro__ uint32_t VID1; /* Version ID register 1 */
  __ro__ uint32_t VID2; /* Version ID register 2 */
} IMX_TRNG_Type;
/*------------------Universal Serial Bus--------------------------------------*/
typedef struct
{
  __ro__ uint32_t ID; /* Identification register */
  __ro__ uint32_t HWGENERAL; /* Hardware General register */
  __ro__ uint32_t HWHOST; /* Host Hardware Parameters */
  __ro__ uint32_t HWDEVICE; /* Device Hardware Parameters */
  __ro__ uint32_t HWTXBUF; /* TX Buffer Hardware Parameters */
  __ro__ uint32_t HWRXBUF; /* RX Buffer Hardware Parameters */
  __ne__ uint32_t RESERVED0[26];

  /* Offset 0x080 */
  union
  {
    struct
    {
      __rw__ uint32_t GPTIMER0LD;
      __rw__ uint32_t GPTIMER0CTRL;
      __rw__ uint32_t GPTIMER1LD;
      __rw__ uint32_t GPTIMER1CTRL;
    };

    struct
    {
      __rw__ uint32_t LD; /* General Purpose Timer N Load register */
      __rw__ uint32_t CTRL; /* General Purpose Timer N Control register */
    } GPTIMER[2];
  };
  __rw__ uint32_t SBUSCFG; /* System Bus Config */
  __ne__ uint32_t RESERVED1[27];

  /* Offset 0x100 */
  __ro__ uint8_t CAPLENGTH; /* Capability Registers Length */
  __ne__ uint8_t RESERVED2[1];
  __ro__ uint16_t HCIVERSION; /* Host Controller Interface Version */
  __ro__ uint32_t HCSPARAMS; /* Host Controller Structural Parameters */
  __ro__ uint32_t HCCPARAMS; /* Host Controller Capability Parameters */
  __ne__ uint32_t RESERVED3[5];
  __ro__ uint16_t DCIVERSION; /* Device Controller Interface Version */
  __ne__ uint16_t RESERVED4;
  __ro__ uint32_t DCCPARAMS; /* Device Controller Capability Parameters */
  __ne__ uint32_t RESERVED5[6];

  /* Offset 0x140 */
  __rw__ uint32_t USBCMD; /* USB Command register */
  __rw__ uint32_t USBSTS; /* USB Status register */
  __rw__ uint32_t USBINTR; /* Interrupt Enable Register */
  __rw__ uint32_t FRINDEX; /* USB Frame Index register */
  __ne__ uint32_t RESERVED6;

  /* Offset 0x154 */
  union
  {
    __rw__ uint32_t DEVICEADDR; /* Device Address register */
    __rw__ uint32_t PERIODICLISTBASE; /* Frame List Base Address register */
  };
  union
  {
    __rw__ uint32_t ASYNCLISTADDR; /* Next Asynchronous Address register */
    __rw__ uint32_t ENDPTLISTADDR; /* Endpoint List Address register */
  };
  __ne__ uint32_t RESERVED7;

  /* Offset 0x160 */
  __rw__ uint32_t BURSTSIZE; /* Programmable Burst Size register */
  __rw__ uint32_t TXFILLTUNING; /* TX FIFO Fill Tuning register */
  __ne__ uint32_t RESERVED8[4];
  __rw__ uint32_t ENDPTNAK; /* Endpoint NAK register */
  __rw__ uint32_t ENDPTNAKEN; /* Endpoint NAK Enable register */
  __ro__ uint32_t CONFIGFLAG; /* Configure Flag register */
  __rw__ uint32_t PORTSC1; /* Port Status and Control register */
  __ne__ uint32_t RESERVED9[7];

  /* Offset 0x1A4 */
  __rw__ uint32_t OTGSC; /* On-The-Go Status and Control register */
  __rw__ uint32_t USBMODE; /* USB Device Mode register */

  /* Offset 0x1AC */
  __rw__ uint32_t ENDPTSETUPSTAT; /* Endpoint Setup Status register */
  __rw__ uint32_t ENDPTPRIME; /* Endpoint Prime register */
  __rw__ uint32_t ENDPTFLUSH; /* Endpoint Flush register */
  __ro__ uint32_t ENDPTSTAT; /* Endpoint Status register */
  __rw__ uint32_t ENDPTCOMPLETE; /* Endpoint Complete register */

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
      __rw__ uint32_t ENDPTCTRL6;
      __rw__ uint32_t ENDPTCTRL7;
    };

    __rw__ uint32_t ENDPTCTRL[8]; /* Endpoint 0..7 Control registers */
  };
} IMX_USB_Type;
/*------------------USB Non-Core Registers------------------------------------*/
typedef struct
{
  /* Offset 0x800 */
  __rw__ uint32_t USB_OTG_CTRL[2]; /* USB OTG Control register */
  __ne__ uint32_t RESERVED1[4];
  __rw__ uint32_t USB_OTG_PHY_CTRL_0[2]; /* OTG UTMI PHY Control 0 register */
} IMX_USBNC_Type;
/*------------------USB PHY---------------------------------------------------*/
typedef struct
{
  __rw__ uint32_t PWD; /* Power-Down register */
  __rw__ uint32_t PWD_SET;
  __rw__ uint32_t PWD_CLR;
  __rw__ uint32_t PWD_TOG;
  __rw__ uint32_t TX; /* Transmitter control register */
  __rw__ uint32_t TX_SET;
  __rw__ uint32_t TX_CLR;
  __rw__ uint32_t TX_TOG;

  /* Offset 0x20 */
  __rw__ uint32_t RX; /* Receiver control register */
  __rw__ uint32_t RX_SET;
  __rw__ uint32_t RX_CLR;
  __rw__ uint32_t RX_TOG;
  __rw__ uint32_t CTRL; /* General Control register */
  __rw__ uint32_t CTRL_SET;
  __rw__ uint32_t CTRL_CLR;
  __rw__ uint32_t CTRL_TOG;

  /* Offset 0x40 */
  __rw__ uint32_t STATUS; /* Status register */
  __ne__ uint32_t RESERVED0[3];
  __rw__ uint32_t DBG; /* Debug register */
  __rw__ uint32_t DBG_SET;
  __rw__ uint32_t DBG_CLR;
  __rw__ uint32_t DBG_TOG;

  /* Offset 0x60 */
  __ro__ uint32_t DBG0_STATUS; /* UTMI Debug Status register 0 */
  __ne__ uint32_t RESERVED1[3];
  __rw__ uint32_t DBG1; /* UTMI Debug Status register 1 */
  __rw__ uint32_t DBG1_SET;
  __rw__ uint32_t DBG1_CLR;
  __rw__ uint32_t DBG1_TOG;

  /* Offset 0x80 */
  __ro__ uint32_t VERSION; /* UTMI RTL Version register */
} IMX_USBPHY_Type;
/*------------------USB - Analog Part-----------------------------------------*/
typedef struct
{
  __ne__ uint32_t RESERVED[104];

  /* Offset 0x1A0 */
  struct
  {
    __rw__ uint32_t VBUS_DETECT; /* VBUS Detect register */
    __rw__ uint32_t VBUS_DETECT_SET;
    __rw__ uint32_t VBUS_DETECT_CLR;
    __rw__ uint32_t VBUS_DETECT_TOG;
    __rw__ uint32_t CHRG_DETECT; /* Charger Detect register */
    __rw__ uint32_t CHRG_DETECT_SET;
    __rw__ uint32_t CHRG_DETECT_CLR;
    __rw__ uint32_t CHRG_DETECT_TOG;
    __ro__ uint32_t VBUS_DETECT_STAT; /* VBUS Detect Status register */
    __ne__ uint32_t RESERVED0[3];
    __ro__ uint32_t CHRG_DETECT_STAT; /* Charger Detect Status register */
    __ne__ uint32_t RESERVED1[3];
    __rw__ uint32_t LOOPBACK; /* Loopback test register */
    __rw__ uint32_t LOOPBACK_SET;
    __rw__ uint32_t LOOPBACK_CLR;
    __rw__ uint32_t LOOPBACK_TOG;
    __rw__ uint32_t MISC; /* Miscellaneous register */
    __rw__ uint32_t MISC_SET;
    __rw__ uint32_t MISC_CLR;
    __rw__ uint32_t MISC_TOG;
  } CHANNEL[2];

  /* Offset 0x260 */
  __ro__ uint32_t DIGPROG; /* Chip Silicon version */
} IMX_USB_ANALOG_Type;
/*------------------Ultra Secured Digital Host Controller---------------------*/
typedef struct
{
  __rw__ uint32_t DS_ADDR; /* DMA System Address */
  __rw__ uint32_t BLK_ATT; /* Block Attributes */
  __rw__ uint32_t CMD_ARG; /* Command Argument */
  __rw__ uint32_t CMD_XFR_TYP; /* Command Transfer Type */

  /* Offset 0x10 */
  union
  {
    struct
    {
      __ro__ uint32_t CMD_RSP0;
      __ro__ uint32_t CMD_RSP1;
      __ro__ uint32_t CMD_RSP2;
      __ro__ uint32_t CMD_RSP3;
    };
    __ro__ uint32_t CMD_RSP[4]; /* Command Response registers */
  };

  /* Offset 0x20 */
  __rw__ uint32_t DATA_BUFF_ACC_PORT; /* Data Buffer Access Port register */
  __ro__ uint32_t PRES_STATE; /* Present State register */
  __rw__ uint32_t PROT_CTRL; /* Protocol Control register */
  __rw__ uint32_t SYS_CTRL; /* System Control register */

  /* Offset 0x30 */
  __rw__ uint32_t INT_STATUS; /* Interrupt Status register */
  __rw__ uint32_t INT_STATUS_EN; /* Interrupt Status Enable register */
  __rw__ uint32_t INT_SIGNAL_EN; /* Interrupt Signal Enable register */
  __rw__ uint32_t AUTOCMD12_ERR_STATUS; /* Auto CMD12 Error Status register */

  /* Offset 0x40 */
  __rw__ uint32_t HOST_CTRL_CAP; /* Host Controller Capabilities register */
  __rw__ uint32_t WTMK_LVL; /* Watermark Level register */
  __rw__ uint32_t MIX_CTRL; /* Mixer Control register */
  __ne__ uint32_t RESERVED0;

  /* Offset 0x50 */
  __wo__ uint32_t FORCE_EVENT; /* Force Event register */
  __ro__ uint32_t ADMA_ERR_STATUS; /* ADMA Error Status register */
  __rw__ uint32_t ADMA_SYS_ADDR; /* ADMA System Address register */
  __ne__ uint32_t RESERVED1;

  /* Offset 0x60 */
  __rw__ uint32_t DLL_CTRL; /* Delay Line Control register */
  __ro__ uint32_t DLL_STATUS; /* Delay Line Status register */
  __rw__ uint32_t CLK_TUNE_CTRL_STATUS; /* CLK Tuning Control and Status */
  __ne__ uint32_t RESERVED2[21];

  /* Offset 0xC0 */
  __rw__ uint32_t VEND_SPEC; /* Vendor Specific register */
  __rw__ uint32_t MMC_BOOT; /* MMC Boot register */
  __rw__ uint32_t VEND_SPEC2; /* Vendor Specific register 2 */
  __rw__ uint32_t TUNING_CTRL; /* Tuning Control register */
} IMX_USDHC_Type;
/*------------------Watchdog Timer--------------------------------------------*/
typedef struct
{
  __rw__ uint16_t WCR; /* Watchdog Control Register */
  __rw__ uint16_t WSR; /* Watchdog Service Register */
  __ro__ uint16_t WRSR; /* Watchdog Reset Status Register */
  __rw__ uint16_t WICR; /* Watchdog Interrupt Control Register */
  __rw__ uint16_t WMCR; /* Watchdog Miscellaneous Control Register */
} IMX_WDOG_Type;
/*------------------Inter-Peripheral Crossbar Switch A------------------------*/
typedef struct
{
  __rw__ uint16_t SEL[66];
  /* Offset 0x84 */
  __rw__ uint16_t CTRL[2];
} IMX_XBARA_Type;
/*------------------Inter-Peripheral Crossbar Switch B------------------------*/
typedef struct
{
  __rw__ uint16_t SEL[8];
} IMX_XBARB_Type;
/*------------------Inter-Peripheral Crossbar Switch C------------------------*/
typedef struct
{
  // TODO
  __rw__ uint16_t SEL[8];
} IMX_XBARC_Type;
/*------------------Crystal Oscillator 24 MHz---------------------------------*/
typedef struct
{
  __ne__ uint32_t RESERVED0[84];
  __rw__ uint32_t MISC0; /* Miscellaneous register 0 */
  __rw__ uint32_t MISC0_SET;
  __rw__ uint32_t MISC0_CLR;
  __rw__ uint32_t MISC0_TOG;
  __ne__ uint32_t RESERVED1[68];
  __rw__ uint32_t LOWPWR_CTRL; /* XTAL OSC (LP) Control register */
  __rw__ uint32_t LOWPWR_CTRL_SET;
  __rw__ uint32_t LOWPWR_CTRL_CLR;
  __rw__ uint32_t LOWPWR_CTRL_TOG;
  __ne__ uint32_t RESERVED2[8];
  __rw__ uint32_t OSC_CONFIG0; /* XTAL OSC Configuration 0 register */
  __rw__ uint32_t OSC_CONFIG0_SET;
  __rw__ uint32_t OSC_CONFIG0_CLR;
  __rw__ uint32_t OSC_CONFIG0_TOG;
  __rw__ uint32_t OSC_CONFIG1; /* XTAL OSC Configuration 1 register */
  __rw__ uint32_t OSC_CONFIG1_SET;
  __rw__ uint32_t OSC_CONFIG1_CLR;
  __rw__ uint32_t OSC_CONFIG1_TOG;
  __rw__ uint32_t OSC_CONFIG2; /* XTAL OSC Configuration 2 register */
  __rw__ uint32_t OSC_CONFIG2_SET;
  __rw__ uint32_t OSC_CONFIG2_CLR;
  __rw__ uint32_t OSC_CONFIG2_TOG;
} IMX_XTALOSC24M_Type;
/*----------------------------------------------------------------------------*/
typedef struct
{
  union
  {
    IMX_CCM_ANALOG_Type CCM_ANALOG;
    IMX_PMU_Type PMU;
    IMX_TEMPMON_Type TEMPMON;
    IMX_USB_ANALOG_Type USB_ANALOG;
    IMX_XTALOSC24M_Type XTALOSC24M;
  };
} IMX_ANATOP_Type;

typedef struct
{
  __ne__ uint8_t RESERVED0[0x7C000];
  IMX_AIPSTZ_Type AIPSTZ1;
  __ne__ uint8_t RESERVED1[0x4000 - sizeof(IMX_AIPSTZ_Type)];
  IMX_DCDC_Type DCDC;
  __ne__ uint8_t RESERVED2[0x4000 - sizeof(IMX_DCDC_Type)];
  IMX_PIT_Type PIT;
  __ne__ uint8_t RESERVED3[0x10000 - sizeof(IMX_PIT_Type)];
  IMX_ACMP_Type ACMP;
  __ne__ uint8_t RESERVED4[0x10000 - sizeof(IMX_ACMP_Type)];
  IMX_IOMUXC_SNVS_GPR_Type IOMUXC_SNVS_GPR;
  __ne__ uint8_t RESERVED5[0x4000 - sizeof(IMX_IOMUXC_SNVS_GPR_Type)];
  IMX_IOMUXC_SNVS_Type IOMUXC_SNVS;
  __ne__ uint8_t RESERVED6[0x4000 - sizeof(IMX_IOMUXC_SNVS_Type)];
  IMX_IOMUXC_GPR_Type IOMUXC_GPR;
  __ne__ uint8_t RESERVED7[0x4000 - sizeof(IMX_IOMUXC_GPR_Type)];
  IMX_FLEXRAM_Type FLEXRAM;
  __ne__ uint8_t RESERVED8[0x4000 - sizeof(IMX_FLEXRAM_Type)];
  IMX_EWM_Type EWM;
  __ne__ uint8_t RESERVED9[0x4000 - sizeof(IMX_EWM_Type)];
  IMX_WDOG_Type WDOG1;
  __ne__ uint8_t RESERVED10[0x4000 - sizeof(IMX_WDOG_Type)];
  IMX_RTWDOG_Type WDOG3;
  __ne__ uint8_t RESERVED11[0x4000 - sizeof(IMX_RTWDOG_Type)];
  IMX_GPIO_Type GPIO5;
  __ne__ uint8_t RESERVED12[0x4000 - sizeof(IMX_GPIO_Type)];
  IMX_ADC_Type ADC1;
  __ne__ uint8_t RESERVED13[0x4000 - sizeof(IMX_ADC_Type)];
  IMX_ADC_Type ADC2;
  __ne__ uint8_t RESERVED14[0x4000 - sizeof(IMX_ADC_Type)];
  IMX_TRNG_Type TRNG;
  __ne__ uint8_t RESERVED15[0x4000 - sizeof(IMX_TRNG_Type)];
  IMX_WDOG_Type WDOG2;
  __ne__ uint8_t RESERVED16[0x4000 - sizeof(IMX_WDOG_Type)];
  IMX_SNVS_Type SNVS;
  __ne__ uint8_t RESERVED17[0x4000 - sizeof(IMX_SNVS_Type)];
  IMX_ANATOP_Type ANATOP;
  __ne__ uint8_t RESERVED18[0x1000 - sizeof(IMX_ANATOP_Type)];
  IMX_USBPHY_Type USBPHY1;
  __ne__ uint8_t RESERVED19[0x1000 - sizeof(IMX_USBPHY_Type)];
  IMX_USBPHY_Type USBPHY2;
  __ne__ uint8_t RESERVED20[0x2000 - sizeof(IMX_USBPHY_Type)];
  IMX_CSU_Type CSU;
  __ne__ uint8_t RESERVED21[0xC000 - sizeof(IMX_CSU_Type)];
  IMX_EDMA_Type EDMA;
  __ne__ uint8_t RESERVED22[0x4000 - sizeof(IMX_EDMA_Type)];
  IMX_DMAMUX_Type DMAMUX;
  __ne__ uint8_t RESERVED23[0x8000 - sizeof(IMX_DMAMUX_Type)];
  IMX_GPC_Type GPC;
  __ne__ uint8_t RESERVED24[0x4000 - sizeof(IMX_GPC_Type)];
  IMX_SRC_Type SRC;
  __ne__ uint8_t RESERVED25[0x4000 - sizeof(IMX_SRC_Type)];
  IMX_CCM_Type CCM;
} AIPS1_DOMAIN_Type;

typedef struct
{
  __ne__ uint8_t RESERVED0[0x7C000];
  IMX_AIPSTZ_Type AIPSTZ2;
  __ne__ uint8_t RESERVED1[0x8000 - sizeof(IMX_AIPSTZ_Type)];
  IMX_LPUART_Type LPUART1;
  __ne__ uint8_t RESERVED2[0x4000 - sizeof(IMX_LPUART_Type)];
  IMX_LPUART_Type LPUART2;
  __ne__ uint8_t RESERVED3[0x4000 - sizeof(IMX_LPUART_Type)];
  IMX_LPUART_Type LPUART3;
  __ne__ uint8_t RESERVED4[0x4000 - sizeof(IMX_LPUART_Type)];
  IMX_LPUART_Type LPUART4;
  __ne__ uint8_t RESERVED5[0x4000 - sizeof(IMX_LPUART_Type)];
  IMX_LPUART_Type LPUART5;
  __ne__ uint8_t RESERVED6[0x4000 - sizeof(IMX_LPUART_Type)];
  IMX_LPUART_Type LPUART6;
  __ne__ uint8_t RESERVED7[0x4000 - sizeof(IMX_LPUART_Type)];
  IMX_LPUART_Type LPUART7;
  __ne__ uint8_t RESERVED8[0x4000 - sizeof(IMX_LPUART_Type)];
  IMX_LPUART_Type LPUART8;
  __ne__ uint8_t RESERVED9[0xC000 - sizeof(IMX_LPUART_Type)];
  IMX_FLEXIO_Type FLEXIO1;
  __ne__ uint8_t RESERVED10[0x4000 - sizeof(IMX_FLEXIO_Type)];
  IMX_FLEXIO_Type FLEXIO2;
  __ne__ uint8_t RESERVED11[0x8000 - sizeof(IMX_FLEXIO_Type)];
  IMX_GPIO_Type GPIO1;
  __ne__ uint8_t RESERVED12[0x4000 - sizeof(IMX_GPIO_Type)];
  IMX_GPIO_Type GPIO2;
  __ne__ uint8_t RESERVED13[0x4000 - sizeof(IMX_GPIO_Type)];
  IMX_GPIO_Type GPIO3;
  __ne__ uint8_t RESERVED14[0x4000 - sizeof(IMX_GPIO_Type)];
  IMX_GPIO_Type GPIO4;
  __ne__ uint8_t RESERVED15[0xC000 - sizeof(IMX_GPIO_Type)];
  IMX_FLEXCAN_Type FLEXCAN1;
  __ne__ uint8_t RESERVED16[0x4000 - sizeof(IMX_FLEXCAN_Type)];
  IMX_FLEXCAN_Type FLEXCAN2;
  __ne__ uint8_t RESERVED17[0x4000 - sizeof(IMX_FLEXCAN_Type)];
  IMX_FDCAN_Type FLEXCAN3;
  __ne__ uint8_t RESERVED18[0x4000 - sizeof(IMX_FDCAN_Type)];
  IMX_TMR_Type TMR1;
  __ne__ uint8_t RESERVED19[0x4000 - sizeof(IMX_TMR_Type)];
  IMX_TMR_Type TMR2;
  __ne__ uint8_t RESERVED20[0x4000 - sizeof(IMX_TMR_Type)];
  IMX_TMR_Type TMR3;
  __ne__ uint8_t RESERVED21[0x4000 - sizeof(IMX_TMR_Type)];
  IMX_TMR_Type TMR4;
  __ne__ uint8_t RESERVED22[0x4000 - sizeof(IMX_TMR_Type)];
  IMX_GPT_Type GPT1;
  __ne__ uint8_t RESERVED23[0x4000 - sizeof(IMX_GPT_Type)];
  IMX_GPT_Type GPT2;
  __ne__ uint8_t RESERVED24[0x4000 - sizeof(IMX_GPT_Type)];
  IMX_OCOTP_Type OCOTP;
  __ne__ uint8_t RESERVED25[0x4000 - sizeof(IMX_OCOTP_Type)];
  IMX_IOMUXC_Type IOMUXC;
  __ne__ uint8_t RESERVED26[0x4000 - sizeof(IMX_IOMUXC_Type)];
  IMX_KPP_Type KPP;
} AIPS2_DOMAIN_Type;

typedef struct
{
  __ne__ uint8_t RESERVED0[0x7C000];
  IMX_AIPSTZ_Type AIPSTZ3;
  __ne__ uint8_t RESERVED1[0x28000 - sizeof(IMX_AIPSTZ_Type)];
  IMX_FLEXSPI_Type FLEXSPI1;
  __ne__ uint8_t RESERVED2[0x4000 - sizeof(IMX_FLEXSPI_Type)];
  IMX_FLEXSPI_Type FLEXSPI2;
  __ne__ uint8_t RESERVED3[0xC000 - sizeof(IMX_FLEXSPI_Type)];
  IMX_PXP_Type PXP;
  __ne__ uint8_t RESERVED4[0x4000 - sizeof(IMX_PXP_Type)];
  IMX_LCDIF_Type LCDIF;
  __ne__ uint8_t RESERVED5[0x4000 - sizeof(IMX_LCDIF_Type)];
  IMX_CSI_Type CSI;
  __ne__ uint8_t RESERVED6[0x4000 - sizeof(IMX_CSI_Type)];
  IMX_USDHC_Type USDHC1;
  __ne__ uint8_t RESERVED7[0x4000 - sizeof(IMX_USDHC_Type)];
  IMX_USDHC_Type USDHC2;
  __ne__ uint8_t RESERVED8[0x10000 - sizeof(IMX_USDHC_Type)];
  IMX_ENET_Type ENET2;
  __ne__ uint8_t RESERVED9[0x4000 - sizeof(IMX_ENET_Type)];
  IMX_ENET_Type ENET1;
  __ne__ uint8_t RESERVED10[0x8000 - sizeof(IMX_ENET_Type)];
  IMX_USB_Type USB1;
  __ne__ uint8_t RESERVED11[0x200 - sizeof(IMX_USB_Type)];
  IMX_USB_Type USB2;
  __ne__ uint8_t RESERVED12[0x600 - sizeof(IMX_USB_Type)];
  IMX_USBNC_Type USBNC;
  __ne__ uint8_t RESERVED13[0xF800 - sizeof(IMX_USBNC_Type)];
  IMX_SEMC_Type SEMC;
  __ne__ uint8_t RESERVED14[0xC000 - sizeof(IMX_SEMC_Type)];
  IMX_DCP_Type DCP;
} AIPS3_DOMAIN_Type;

typedef struct
{
  __ne__ uint8_t RESERVED0[0x7C000];
  IMX_AIPSTZ_Type AIPSTZ4;
  __ne__ uint8_t RESERVED1[0x4000 - sizeof(IMX_AIPSTZ_Type)];
  IMX_SPDIF_Type SPDIF;
  __ne__ uint8_t RESERVED2[0x4000 - sizeof(IMX_SPDIF_Type)];
  IMX_SAI_Type SAI1;
  __ne__ uint8_t RESERVED3[0x4000 - sizeof(IMX_SAI_Type)];
  IMX_SAI_Type SAI2;
  __ne__ uint8_t RESERVED4[0x4000 - sizeof(IMX_SAI_Type)];
  IMX_SAI_Type SAI3;
  __ne__ uint8_t RESERVED5[0x8000 - sizeof(IMX_SAI_Type)];
  IMX_LPSPI_Type LPSPI1;
  __ne__ uint8_t RESERVED6[0x4000 - sizeof(IMX_LPSPI_Type)];
  IMX_LPSPI_Type LPSPI2;
  __ne__ uint8_t RESERVED7[0x4000 - sizeof(IMX_LPSPI_Type)];
  IMX_LPSPI_Type LPSPI3;
  __ne__ uint8_t RESERVED8[0x4000 - sizeof(IMX_LPSPI_Type)];
  IMX_LPSPI_Type LPSPI4;
  __ne__ uint8_t RESERVED9[0x10000 - sizeof(IMX_LPSPI_Type)];
  IMX_ADC_ETC_Type ADC_ETC;
  __ne__ uint8_t RESERVED10[0x4000 - sizeof(IMX_ADC_ETC_Type)];
  IMX_AOI_Type AOI1;
  __ne__ uint8_t RESERVED11[0x4000 - sizeof(IMX_AOI_Type)];
  IMX_AOI_Type AOI2;
  __ne__ uint8_t RESERVED12[0x4000 - sizeof(IMX_AOI_Type)];
  IMX_XBARA_Type XBAR1;
  __ne__ uint8_t RESERVED13[0x4000 - sizeof(IMX_XBARA_Type)];
  IMX_XBARB_Type XBAR2;
  __ne__ uint8_t RESERVED14[0x4000 - sizeof(IMX_XBARB_Type)];
  IMX_XBARC_Type XBAR3;
  __ne__ uint8_t RESERVED15[0x4000 - sizeof(IMX_XBARC_Type)];
  IMX_ENC_Type ENC1;
  __ne__ uint8_t RESERVED16[0x4000 - sizeof(IMX_ENC_Type)];
  IMX_ENC_Type ENC2;
  __ne__ uint8_t RESERVED17[0x4000 - sizeof(IMX_ENC_Type)];
  IMX_ENC_Type ENC3;
  __ne__ uint8_t RESERVED18[0x4000 - sizeof(IMX_ENC_Type)];
  IMX_ENC_Type ENC4;
  __ne__ uint8_t RESERVED19[0x8000 - sizeof(IMX_ENC_Type)];
  IMX_FLEXPWM_Type FLEXPWM1;
  __ne__ uint8_t RESERVED20[0x4000 - sizeof(IMX_FLEXPWM_Type)];
  IMX_FLEXPWM_Type FLEXPWM2;
  __ne__ uint8_t RESERVED21[0x4000 - sizeof(IMX_FLEXPWM_Type)];
  IMX_FLEXPWM_Type FLEXPWM3;
  __ne__ uint8_t RESERVED22[0x4000 - sizeof(IMX_FLEXPWM_Type)];
  IMX_FLEXPWM_Type FLEXPWM4;
  __ne__ uint8_t RESERVED23[0x4000 - sizeof(IMX_FLEXPWM_Type)];
  IMX_BEE_Type BEE;
  __ne__ uint8_t RESERVED24[0x4000 - sizeof(IMX_BEE_Type)];
  IMX_LPI2C_Type LPI2C1;
  __ne__ uint8_t RESERVED25[0x4000 - sizeof(IMX_LPI2C_Type)];
  IMX_LPI2C_Type LPI2C2;
  __ne__ uint8_t RESERVED26[0x4000 - sizeof(IMX_LPI2C_Type)];
  IMX_LPI2C_Type LPI2C3;
  __ne__ uint8_t RESERVED27[0x4000 - sizeof(IMX_LPI2C_Type)];
  IMX_LPI2C_Type LPI2C4;
} AIPS4_DOMAIN_Type;

typedef struct
{
  IMX_GPIO_Type GPIO6;
  __ne__ uint8_t RESERVED0[0x4000 - sizeof(IMX_GPIO_Type)];
  IMX_GPIO_Type GPIO7;
  __ne__ uint8_t RESERVED1[0x4000 - sizeof(IMX_GPIO_Type)];
  IMX_GPIO_Type GPIO8;
  __ne__ uint8_t RESERVED2[0x4000 - sizeof(IMX_GPIO_Type)];
  IMX_GPIO_Type GPIO9;
  __ne__ uint8_t RESERVED3[0x14000 - sizeof(IMX_GPIO_Type)];
  IMX_FLEXIO_Type FLEXIO3;
} AIPS5_DOMAIN_Type;
/*----------------------------------------------------------------------------*/
extern AIPS1_DOMAIN_Type AIPS1_DOMAIN;
extern AIPS2_DOMAIN_Type AIPS2_DOMAIN;
extern AIPS3_DOMAIN_Type AIPS3_DOMAIN;
extern AIPS4_DOMAIN_Type AIPS4_DOMAIN;
extern AIPS5_DOMAIN_Type AIPS5_DOMAIN;
/*----------------------------------------------------------------------------*/
#define IMX_AIPSTZ1         (&AIPS1_DOMAIN.AIPSTZ1)
#define IMX_DCDC            (&AIPS1_DOMAIN.DCDC)
#define IMX_PIT             (&AIPS1_DOMAIN.PIT)
#define IMX_ACMP            (&AIPS1_DOMAIN.ACMP)
#define IMX_IOMUXC_SNVS_GPR (&AIPS1_DOMAIN.IOMUXC_SNVS_GPR)
#define IMX_IOMUXC_SNVS     (&AIPS1_DOMAIN.IOMUXC_SNVS)
#define IMX_IOMUXC_GPR      (&AIPS1_DOMAIN.IOMUXC_GPR)
#define IMX_FLEXRAM         (&AIPS1_DOMAIN.FLEXRAM)
#define IMX_EWM             (&AIPS1_DOMAIN.EWM)
#define IMX_WDOG1           (&AIPS1_DOMAIN.WDOG1)
#define IMX_WDOG3           (&AIPS1_DOMAIN.WDOG3)
#define IMX_GPIO5           (&AIPS1_DOMAIN.GPIO5)
#define IMX_ADC1            (&AIPS1_DOMAIN.ADC1)
#define IMX_ADC2            (&AIPS1_DOMAIN.ADC2)
#define IMX_TRNG            (&AIPS1_DOMAIN.TRNG)
#define IMX_WDOG2           (&AIPS1_DOMAIN.WDOG2)
#define IMX_SNVS            (&AIPS1_DOMAIN.SNVS)
#define IMX_ANATOP          (&AIPS1_DOMAIN.ANATOP)
#define IMX_CCM_ANALOG      (&AIPS1_DOMAIN.ANATOP.CCM_ANALOG)
#define IMX_PMU             (&AIPS1_DOMAIN.ANATOP.PMU)
#define IMX_TEMPMON         (&AIPS1_DOMAIN.ANATOP.TEMPMON)
#define IMX_USB_ANALOG      (&AIPS1_DOMAIN.ANATOP.USB_ANALOG)
#define IMX_XTALOSC24M      (&AIPS1_DOMAIN.ANATOP.XTALOSC24M)
#define IMX_USBPHY1         (&AIPS1_DOMAIN.USBPHY1)
#define IMX_USBPHY2         (&AIPS1_DOMAIN.USBPHY2)
#define IMX_CSU             (&AIPS1_DOMAIN.CSU)
#define IMX_EDMA            (&AIPS1_DOMAIN.EDMA)
#define IMX_DMAMUX          (&AIPS1_DOMAIN.DMAMUX)
#define IMX_GPC             (&AIPS1_DOMAIN.GPC)
#define IMX_SRC             (&AIPS1_DOMAIN.SRC)
#define IMX_CCM             (&AIPS1_DOMAIN.CCM)

#define IMX_AIPSTZ2         (&AIPS2_DOMAIN.AIPSTZ2)
#define IMX_LPUART1         (&AIPS2_DOMAIN.LPUART1)
#define IMX_LPUART2         (&AIPS2_DOMAIN.LPUART2)
#define IMX_LPUART3         (&AIPS2_DOMAIN.LPUART3)
#define IMX_LPUART4         (&AIPS2_DOMAIN.LPUART4)
#define IMX_LPUART5         (&AIPS2_DOMAIN.LPUART5)
#define IMX_LPUART6         (&AIPS2_DOMAIN.LPUART6)
#define IMX_LPUART7         (&AIPS2_DOMAIN.LPUART7)
#define IMX_LPUART8         (&AIPS2_DOMAIN.LPUART8)
#define IMX_FLEXIO1         (&AIPS2_DOMAIN.FLEXIO1)
#define IMX_FLEXIO2         (&AIPS2_DOMAIN.FLEXIO2)
#define IMX_GPIO1           (&AIPS2_DOMAIN.GPIO1)
#define IMX_GPIO2           (&AIPS2_DOMAIN.GPIO2)
#define IMX_GPIO3           (&AIPS2_DOMAIN.GPIO3)
#define IMX_GPIO4           (&AIPS2_DOMAIN.GPIO4)
#define IMX_FLEXCAN1        (&AIPS2_DOMAIN.FLEXCAN1)
#define IMX_FLEXCAN2        (&AIPS2_DOMAIN.FLEXCAN2)
#define IMX_FLEXCAN3        (&AIPS2_DOMAIN.FLEXCAN3)
#define IMX_TMR1            (&AIPS2_DOMAIN.TMR1)
#define IMX_TMR2            (&AIPS2_DOMAIN.TMR2)
#define IMX_TMR3            (&AIPS2_DOMAIN.TMR3)
#define IMX_TMR4            (&AIPS2_DOMAIN.TMR4)
#define IMX_GPT1            (&AIPS2_DOMAIN.GPT1)
#define IMX_GPT2            (&AIPS2_DOMAIN.GPT2)
#define IMX_OCOTP           (&AIPS2_DOMAIN.OCOTP)
#define IMX_IOMUXC          (&AIPS2_DOMAIN.IOMUXC)
#define IMX_KPP             (&AIPS2_DOMAIN.KPP)

#define IMX_AIPSTZ3         (&AIPS3_DOMAIN.AIPSTZ3)
#define IMX_FLEXSPI1        (&AIPS3_DOMAIN.FLEXSPI1)
#define IMX_FLEXSPI2        (&AIPS3_DOMAIN.FLEXSPI2)
#define IMX_PXP             (&AIPS3_DOMAIN.PXP)
#define IMX_LCDIF           (&AIPS3_DOMAIN.LCDIF)
#define IMX_CSI             (&AIPS3_DOMAIN.CSI)
#define IMX_USDHC1          (&AIPS3_DOMAIN.USDHC1)
#define IMX_USDHC2          (&AIPS3_DOMAIN.USDHC2)
#define IMX_ENET2           (&AIPS3_DOMAIN.ENET2)
#define IMX_ENET1           (&AIPS3_DOMAIN.ENET1)
#define IMX_USB1            (&AIPS3_DOMAIN.USB1)
#define IMX_USB2            (&AIPS3_DOMAIN.USB2)
#define IMX_USBNC           (&AIPS3_DOMAIN.USBNC)
#define IMX_SEMC            (&AIPS3_DOMAIN.SEMC)
#define IMX_DCP             (&AIPS3_DOMAIN.DCP)

#define IMX_AIPSTZ4         (&AIPS4_DOMAIN.AIPSTZ4)
#define IMX_SPDIF           (&AIPS4_DOMAIN.SPDIF)
#define IMX_SAI1            (&AIPS4_DOMAIN.SAI1)
#define IMX_SAI2            (&AIPS4_DOMAIN.SAI2)
#define IMX_SAI3            (&AIPS4_DOMAIN.SAI3)
#define IMX_LPSPI1          (&AIPS4_DOMAIN.LPSPI1)
#define IMX_LPSPI2          (&AIPS4_DOMAIN.LPSPI2)
#define IMX_LPSPI3          (&AIPS4_DOMAIN.LPSPI3)
#define IMX_LPSPI4          (&AIPS4_DOMAIN.LPSPI4)
#define IMX_ADC_ETC         (&AIPS4_DOMAIN.ADC_ETC)
#define IMX_AOI1            (&AIPS4_DOMAIN.AOI1)
#define IMX_AOI2            (&AIPS4_DOMAIN.AOI2)
#define IMX_XBAR1           (&AIPS4_DOMAIN.XBAR1)
#define IMX_XBAR2           (&AIPS4_DOMAIN.XBAR2)
#define IMX_XBAR3           (&AIPS4_DOMAIN.XBAR3)
#define IMX_ENC1            (&AIPS4_DOMAIN.ENC1)
#define IMX_ENC2            (&AIPS4_DOMAIN.ENC2)
#define IMX_ENC3            (&AIPS4_DOMAIN.ENC3)
#define IMX_FLEXPWM1        (&AIPS4_DOMAIN.FLEXPWM1)
#define IMX_FLEXPWM2        (&AIPS4_DOMAIN.FLEXPWM2)
#define IMX_FLEXPWM3        (&AIPS4_DOMAIN.FLEXPWM3)
#define IMX_FLEXPWM4        (&AIPS4_DOMAIN.FLEXPWM4)
#define IMX_BEE             (&AIPS4_DOMAIN.BEE)
#define IMX_LPI2C1          (&AIPS4_DOMAIN.LPI2C1)
#define IMX_LPI2C2          (&AIPS4_DOMAIN.LPI2C2)
#define IMX_LPI2C3          (&AIPS4_DOMAIN.LPI2C3)
#define IMX_LPI2C4          (&AIPS4_DOMAIN.LPI2C4)

#define IMX_GPIO6           (&AIPS5_DOMAIN.GPIO6)
#define IMX_GPIO7           (&AIPS5_DOMAIN.GPIO7)
#define IMX_GPIO8           (&AIPS5_DOMAIN.GPIO8)
#define IMX_GPIO9           (&AIPS5_DOMAIN.GPIO9)
#define IMX_FLEXIO3         (&AIPS5_DOMAIN.FLEXIO3)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_IMXRT_IMXRT106X_PLATFORM_DEFS_H_ */
