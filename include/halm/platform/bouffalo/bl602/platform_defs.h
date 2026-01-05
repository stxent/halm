/*
 * halm/platform/bouffalo/bl602/platform_defs.h
 * Based on original from Bouffalo Lab
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_PLATFORM_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_BOUFFALO_BL602_PLATFORM_DEFS_H_
#define HALM_PLATFORM_BOUFFALO_BL602_PLATFORM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*------------------Always-On peripheral--------------------------------------*/
typedef struct
{
  /* Offset 0x000 */
  __rw__ uint32_t AON;
  __rw__ uint32_t AON_COMMON;
  __rw__ uint32_t AON_MISC;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t BG_SYS_TOP;
  __rw__ uint32_t DCDC18_TOP_0;
  __rw__ uint32_t DCDC18_TOP_1;
  __rw__ uint32_t LDO11SOC_DCTEST;

  /* Offset 0x020 */
  __rw__ uint32_t PSW_IRRCV;
  __ne__ uint32_t RESERVED1[23];

  /* Offset 0x080 */
  __rw__ uint32_t RF_TOP_AON;
  __rw__ uint32_t XTAL_CFG;
  __rw__ uint32_t TSEN;
  __ne__ uint32_t RESERVED2[29];

  /* Offset 0x100 */
  __rw__ uint32_t ACOMP0_CTRL;
  __rw__ uint32_t ACOMP1_CTRL;
  __rw__ uint32_t ACOMP_CTRL;
  __rw__ uint32_t GPADC_REG_CMD;
  __rw__ uint32_t GPADC_REG_CONFIG1;
  __rw__ uint32_t GPADC_REG_CONFIG2;
  __rw__ uint32_t GPADC_REG_SCN_POS1;
  __rw__ uint32_t GPADC_REG_SCN_POS2;

  /* Offset 0x120 */
  __rw__ uint32_t GPADC_REG_SCN_NEG1;
  __rw__ uint32_t GPADC_REG_SCN_NEG2;
  __rw__ uint32_t GPADC_REG_STATUS;
  __rw__ uint32_t GPADC_REG_ISR;
  __rw__ uint32_t GPADC_REG_RESULT;
  __rw__ uint32_t GPADC_REG_RAW_RESULT;
  __rw__ uint32_t GPADC_REG_DEFINE;
  __rw__ uint32_t HBNCORE_RESV0;

  /* Offset 0x140 */
  __rw__ uint32_t HBNCORE_RESV1;
} BL_AON_Type;
/*------------------Core Local Interrupt Controller---------------------------*/
typedef struct
{
  /* Offset 0x000000 */
  __rw__ uint32_t MSIP;
  __ne__ uint32_t RESERVED0[4095];

  /* Offset 0x004000 */
  __rw__ uint64_t MTIMECMP;
  __ne__ uint32_t RESERVED1[8188];

  /* Offset 0x00BFF8 */
  __rw__ uint64_t MTIME;
  __ne__ uint32_t RESERVED2[2083840];

  /* Offset 0x7FF000 */
  __rw__ uint8_t DCCGF; /* Disable CLIC Clock Gate Feature register */
  __ne__ uint8_t RESERVED3[4095];

  /* Offset 0x800000 */
  __rw__ uint8_t INTIP[143];
  __ne__ uint8_t RESERVED4[881];

  /* Offset 0x800400 */
  __rw__ uint8_t INTIE[143];
  __ne__ uint8_t RESERVED5[881];

  /* Offset 0x800800 */
  __rw__ uint8_t INTCFG[143];
  __ne__ uint8_t RESERVED6[881];

  /* Offset 0x800C00 */
  __rw__ uint8_t CLICCFG;
} BL_CLIC_Type;
/*------------------Global Register-------------------------------------------*/
typedef struct
{
  /* Offset 0x000 */
  __rw__ uint32_t CLK_CFG0;
  __rw__ uint32_t CLK_CFG1;
  __rw__ uint32_t CLK_CFG2;
  __rw__ uint32_t CLK_CFG3;

  /* Offset 0x010 */
  __rw__ uint32_t SWRST_CFG0;
  __rw__ uint32_t SWRST_CFG1;
  __rw__ uint32_t SWRST_CFG2;
  __rw__ uint32_t SWRST_CFG3;

  /* Offset 0x020 */
  __rw__ uint32_t CGEN_CFG0;
  __rw__ uint32_t CGEN_CFG1;
  __rw__ uint32_t CGEN_CFG2;
  __rw__ uint32_t CGEN_CFG3;

  /* Offset 0x030 */
  __rw__ uint32_t MBIST_CTL;
  __rw__ uint32_t MBIST_STAT;
  __ne__ uint32_t RESERVED0[6];

  /* Offset 0x50 */
  __rw__ uint32_t BMX_CFG1;
  __rw__ uint32_t BMX_CFG2;
  __rw__ uint32_t BMX_ERR_ADDR;
  __rw__ uint32_t BMX_DBG_OUT;

  /* Offset 0x60 */
  __rw__ uint32_t RSV0;
  __rw__ uint32_t RSV1;
  __rw__ uint32_t RSV2;
  __rw__ uint32_t RSV3;

  /* Offset 0x070 */
  __rw__ uint32_t SRAM_RET;
  __rw__ uint32_t SRAM_SLP;
  __rw__ uint32_t SRAM_PARM;
  __rw__ uint32_t SRAM_MISC;

  /* Offset 0x080 */
  __rw__ uint32_t GLB_PARM;
  __ne__ uint32_t RESERVED1[3];

  /* Offset 0x090 */
  __rw__ uint32_t CPU_CLK_CFG;
  __ne__ uint32_t RESERVED2[4];
  __rw__ uint32_t GPADC_32M_SRC_CTRL;
  __rw__ uint32_t DIG32K_WAKEUP_CTRL;
  __rw__ uint32_t WIFI_BT_COEX_CTRL;
  __ne__ uint32_t RESERVED3[4];

  /* Offset 0x0C0 */
  __rw__ uint32_t UART_SIG_SEL_0;
  __ne__ uint32_t RESERVED4[3];

  /* Offset 0x0D0 */
  __rw__ uint32_t DBG_SEL_LL;
  __rw__ uint32_t DBG_SEL_LH;
  __rw__ uint32_t DBG_SEL_HL;
  __rw__ uint32_t DBG_SEL_HH;
  __rw__ uint32_t DEBUG;
  __ne__ uint32_t RESERVED5[7];

  /* Offset 0x100 */
  __rw__ uint32_t GPIO_CFGCTL[15];
  __ne__ uint32_t RESERVED6[17];
  __rw__ uint32_t GPIO_CFGCTL30; /* GPIO_I */
  __rw__ uint32_t GPIO_CFGCTL31;
  __rw__ uint32_t GPIO_CFGCTL32; /* GPIO_O */
  __rw__ uint32_t GPIO_CFGCTL33;
  __rw__ uint32_t GPIO_CFGCTL34; /* GPIO_OE */
  __rw__ uint32_t GPIO_CFGCTL35;
  __ne__ uint32_t RESERVED7[2];

  /* Offset 0x1A0 */
  __rw__ uint32_t GPIO_INT_MASK1;
  __ne__ uint32_t RESERVED8;
  __rw__ uint32_t GPIO_INT_STAT1;
  __ne__ uint32_t RESERVED9;
  __rw__ uint32_t GPIO_INT_CLR1;
  __ne__ uint32_t RESERVED10[3];

  /* Offset 0x1C0 */
  __rw__ uint32_t GPIO_INT_MODE_SET1;
  __rw__ uint32_t GPIO_INT_MODE_SET2;
  __rw__ uint32_t GPIO_INT_MODE_SET3;
  __ne__ uint32_t RESERVED11[22];

  /* Offset 0x224 */
  __rw__ uint32_t LED_DRIVER;
  __ne__ uint32_t RESERVED12[56];

  /* Offset 0x308 */
  __rw__ uint32_t GPDAC_CTRL;
  __rw__ uint32_t GPDAC_ACTRL;
  __rw__ uint32_t GPDAC_BCTRL;
  __rw__ uint32_t GPDAC_DATA;
  __ne__ uint32_t RESERVED13[762];

  /* Offset 0xF00 */
  __rw__ uint32_t TZC_GLB_CTRL0;
  __rw__ uint32_t TZC_GLB_CTRL1;
  __rw__ uint32_t TZC_GLB_CTRL2;
  __rw__ uint32_t TZC_GLB_CTRL3;
} BL_GLB_Type;
/*------------------Hibernation controller------------------------------------*/
typedef struct
{
  /* Offset 0x000 */
  __rw__ uint32_t HBN_CTL;
  __rw__ uint32_t HBN_TIME_L;
  __rw__ uint32_t HBN_TIME_H;
  __rw__ uint32_t RTC_TIME_L;

  /* Offset 0x010 */
  __rw__ uint32_t RTC_TIME_H;
  __rw__ uint32_t HBN_IRQ_MODE;
  __rw__ uint32_t HBN_IRQ_STAT;
  __rw__ uint32_t HBN_IRQ_CLR;

  /* Offset 0x020 */
  __rw__ uint32_t HBN_PIR_CFG;
  __rw__ uint32_t HBN_PIR_VTH;
  __rw__ uint32_t HBN_PIR_INTERVAL;
  __rw__ uint32_t HBN_BOR_CFG;

  /* Offset 0x030 */
  __rw__ uint32_t HBN_GLB;
  __rw__ uint32_t HBN_SRAM;
  __ne__ uint32_t RESERVED0[50];

  /* Offset 0x100 */
  __rw__ uint32_t HBN_RSV0;
  __rw__ uint32_t HBN_RSV1;
  __rw__ uint32_t HBN_RSV2;
  __rw__ uint32_t HBN_RSV3;
  __ne__ uint32_t RESERVED1[60];

  /* Offset 0x200 */
  __rw__ uint32_t RC32K_CTRL0;
  __rw__ uint32_t XTAL32K;
} BL_HBN_Type;
/*------------------L1 Cache controller---------------------------------------*/
typedef struct
{
  /* Offset 0x000 */
  __rw__ uint32_t L1C_CONFIG;
  __ro__ uint32_t HIT_CNT_LSB;
  __ro__ uint32_t HIT_CNT_MSB;
  __ro__ uint32_t MISS_CNT;
} BL_L1C_Type;
/*------------------Power-Down Sleep peripheral-------------------------------*/
typedef struct
{
  /* Offset 0x000 */
  __rw__ uint32_t PDS_CTL;
  __rw__ uint32_t PDS_TIME1;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t PDS_INT;

  /* Offset 0x010 */
  __rw__ uint32_t PDS_CTL2;
  __rw__ uint32_t PDS_CTL3;
  __rw__ uint32_t PDS_CTL4;
  __rw__ uint32_t PDS_STAT;

  /* Offset 0x020 */
  __rw__ uint32_t PDS_RAM1;
  __ne__ uint32_t RESERVED1[183];

  /* Offset 0x300 */
  __rw__ uint32_t RC32M_CTRL0;
  __rw__ uint32_t RC32M_CTRL1;
  __ne__ uint32_t RESERVED2[62];

  /* Offset 0x400 */
  __rw__ uint32_t PU_RST_CLKPLL;
  __rw__ uint32_t CLKPLL_TOP_CTRL;
  __rw__ uint32_t CLKPLL_CP;
  __rw__ uint32_t CLKPLL_RZ;

  /* Offset 0x410 */
  __rw__ uint32_t CLKPLL_FBDV;
  __rw__ uint32_t CLKPLL_VCO;
  __rw__ uint32_t CLKPLL_SDM;
  __rw__ uint32_t CLKPLL_OUTPUT_EN;
} BL_PDS_Type;
/*------------------Timer and Watchdog peripherals----------------------------*/
typedef struct
{
  /* Offset 0x00 */
  __rw__ uint32_t TCCR;
  __ne__ uint32_t RESERVED0[3];

  /* Offset 0x10 */
  union
  {
    struct
    {
      __rw__ uint32_t TMR2_0;
      __rw__ uint32_t TMR2_1;
      __rw__ uint32_t TMR2_2;
      __rw__ uint32_t TMR3_0;
      __rw__ uint32_t TMR3_1;
      __rw__ uint32_t TMR3_2;
    };
    struct
    {
      __rw__ uint32_t MAT[3];
    } TMR[2];
  };
  __ne__ uint32_t RESERVED1;

  /* Offset 0x2C */
  union
  {
    struct
    {
      __rw__ uint32_t TCR2;
      __rw__ uint32_t TCR3;
    };
    __rw__ uint32_t TCR[2];
  };
  __ne__ uint32_t RESERVED2;

  /* Offset 0x38 */
  union
  {
    struct
    {
      __rw__ uint32_t TMSR2;
      __rw__ uint32_t TMSR3;
    };
    __rw__ uint32_t TMSR[2];
  };
  __ne__ uint32_t RESERVED3;

  /* Offset 0x44 */
  union
  {
    struct
    {
      __rw__ uint32_t TIER2;
      __rw__ uint32_t TIER3;
    };
    __rw__ uint32_t TIER[2];
  };
  __ne__ uint32_t RESERVED4;

  /* Offset 0x50 */
  union
  {
    struct
    {
      __rw__ uint32_t TPLVR2;
      __rw__ uint32_t TPLVR3;
    };
    __rw__ uint32_t TPLVR[2];
  };
  __ne__ uint32_t RESERVED5;

  /* Offset 0x5C */
  union
  {
    struct
    {
      __rw__ uint32_t TPLCR2;
      __rw__ uint32_t TPLCR3;
    };
    __rw__ uint32_t TPLCR[2];
  };

  /* Offset 0x64 */
  __rw__ uint32_t WMER;
  __rw__ uint32_t WMR;
  __rw__ uint32_t WVR;
  __rw__ uint32_t WSR;
  __ne__ uint32_t RESERVED6;

  /* Offset 0x78 */
  union
  {
    struct
    {
      __rw__ uint32_t TICR2;
      __rw__ uint32_t TICR3;
    };
    __rw__ uint32_t TICR[2];
  };

  /* Offset 0x80 */
  __rw__ uint32_t WICR;
  __rw__ uint32_t TCER;
  __rw__ uint32_t TCMR;
  __ne__ uint32_t RESERVED7;

  /* Offset 0x90 */
  union
  {
    struct
    {
      __rw__ uint32_t TILR2;
      __rw__ uint32_t TILR3;
    };
    __rw__ uint32_t TILR[2];
  };

  /* Offset 0x98 */
  __rw__ uint32_t WCR;
  __rw__ uint32_t WFAR;

  /* Offset 0xA0 */
  __rw__ uint32_t WSAR;
  __ne__ uint32_t RESERVED8;

  /* Offset 0xA8 */
  union
  {
    struct
    {
      __rw__ uint32_t TCVWR2;
      __rw__ uint32_t TCVWR3;
    };
    __rw__ uint32_t TCVWR[2];
  };
  __ne__ uint32_t RESERVED9;

  /* Offset 0xB4 */
  union
  {
    struct
    {
      __rw__ uint32_t TCVSYN2;
      __rw__ uint32_t TCVSYN3;
    };
    __rw__ uint32_t TCVSYN[2];
  };

  /* Offset 0xBC */
  __rw__ uint32_t TCDR;
} BL_TIMER_Type;
/*------------------UART peripherals------------------------------------------*/
typedef struct {
  /* Offset 0x00 */
  __rw__ uint32_t UTX_CONFIG;
  __rw__ uint32_t URX_CONFIG;
  __rw__ uint32_t BIT_PRD;
  __rw__ uint32_t DATA_CONFIG;

  /* Offset 0x10 */
  __rw__ uint32_t UTX_IR_POSITION;
  __rw__ uint32_t URX_IR_POSITION;
  __rw__ uint32_t URX_RTO_TIMER;
  __ne__ uint32_t RESERVED0;

  /* Offset 0x20 */
  __rw__ uint32_t INT_STS;
  __rw__ uint32_t INT_MASK;
  __rw__ uint32_t INT_CLEAR;
  __rw__ uint32_t INT_EN;

  /* Offset 0x30 */
  __rw__ uint32_t STATUS;
  __rw__ uint32_t STS_URX_ABR_PRD;
  __ne__ uint32_t RESERVED1[18];

  /* Offset 0x80 */
  __rw__ uint32_t FIFO_CONFIG0;
  __rw__ uint32_t FIFO_CONFIG1;
  __wo__ uint32_t FIFO_WDATA;
  __ro__ uint32_t FIFO_RDATA;
} BL_UART_Type;
/*----------------------------------------------------------------------------*/
// TODO
struct BL_TODO_Type {
  __ne__ uint32_t RESERVED0;
};

typedef struct BL_TODO_Type BL_MIX_Type;
typedef struct BL_TODO_Type BL_GPIP_Type;
typedef struct BL_TODO_Type BL_SEC_Type;
typedef struct BL_TODO_Type BL_TZ_Type;
typedef struct BL_TODO_Type BL_TZ_Type;
typedef struct BL_TODO_Type BL_EFUSE_Type;
typedef struct BL_TODO_Type BL_SPI_Type;
typedef struct BL_TODO_Type BL_I2C_Type;
typedef struct BL_TODO_Type BL_PWM_Type;
typedef struct BL_TODO_Type BL_IRR_Type;
typedef struct BL_TODO_Type BL_QSPI_Type;
typedef struct BL_TODO_Type BL_DMA_Type;
typedef struct BL_TODO_Type BL_SDU_Type;
/*----------------------------------------------------------------------------*/
typedef struct
{
  BL_CLIC_Type CLIC;
} CLIC_DOMAIN_Type;

typedef struct
{
  BL_GLB_Type GLB;
  __ne__ uint8_t RESERVED0[0x1000 - sizeof(BL_GLB_Type)];
  BL_MIX_Type MIX;
  __ne__ uint8_t RESERVED1[0x1000 - sizeof(BL_MIX_Type)];
  BL_GPIP_Type GPIP;
  __ne__ uint8_t RESERVED2[0x2000 - sizeof(BL_GPIP_Type)];
  BL_SEC_Type SEC;
  __ne__ uint8_t RESERVED3[0x1000 - sizeof(BL_SEC_Type)];
  BL_TZ_Type TZ1;
  __ne__ uint8_t RESERVED4[0x1000 - sizeof(BL_TZ_Type)];
  BL_TZ_Type TZ2;
  __ne__ uint8_t RESERVED5[0x1000 - sizeof(BL_TZ_Type)];
  BL_EFUSE_Type EFUSE;
  __ne__ uint8_t RESERVED6[0x2000 - sizeof(BL_EFUSE_Type)];
  BL_L1C_Type L1C;
  __ne__ uint8_t RESERVED7[0x1000 - sizeof(BL_L1C_Type)];
  BL_UART_Type UART0;
  __ne__ uint8_t RESERVED8[0x100 - sizeof(BL_UART_Type)];
  BL_UART_Type UART1;
  __ne__ uint8_t RESERVED9[0x100 - sizeof(BL_UART_Type)];
  BL_SPI_Type SPI;
  __ne__ uint8_t RESERVED10[0x100 - sizeof(BL_SPI_Type)];
  BL_I2C_Type I2C;
  __ne__ uint8_t RESERVED11[0x100 - sizeof(BL_I2C_Type)];
  BL_PWM_Type PWM;
  __ne__ uint8_t RESERVED12[0x100 - sizeof(BL_PWM_Type)];
  BL_TIMER_Type TIMER;
  __ne__ uint8_t RESERVED13[0x100 - sizeof(BL_TIMER_Type)];
  BL_IRR_Type IRR;
  __ne__ uint8_t RESERVED14[0xA00 - sizeof(BL_IRR_Type)];
  BL_QSPI_Type QSPI;
  __ne__ uint8_t RESERVED15[0x1000 - sizeof(BL_QSPI_Type)];
  BL_DMA_Type DMA;
  __ne__ uint8_t RESERVED16[0x1000 - sizeof(BL_DMA_Type)];
  BL_SDU_Type SDU;
  __ne__ uint8_t RESERVED17[0x1000 - sizeof(BL_SDU_Type)];
  BL_PDS_Type PDS;
  __ne__ uint8_t RESERVED18[0x1000 - sizeof(BL_PDS_Type)];
  BL_HBN_Type HBN;
  __ne__ uint8_t RESERVED19[0x800 - sizeof(BL_HBN_Type)];
  BL_AON_Type AON;
} PB_DOMAIN_Type;
/*----------------------------------------------------------------------------*/
extern CLIC_DOMAIN_Type CLIC_DOMAIN;
extern PB_DOMAIN_Type PB_DOMAIN;
/*----------------------------------------------------------------------------*/
#define BL_CLIC         (&CLIC_DOMAIN.CLIC)

#define BL_GLB          (&PB_DOMAIN.GLB)
#define BL_L1C          (&PB_DOMAIN.L1C)
#define BL_UART0        (&PB_DOMAIN.UART0)
#define BL_UART1        (&PB_DOMAIN.UART1)
#define BL_TIMER        (&PB_DOMAIN.TIMER)
#define BL_PDS          (&PB_DOMAIN.PDS)
#define BL_HBN          (&PB_DOMAIN.HBN)
#define BL_AON          (&PB_DOMAIN.AON)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_BL602_PLATFORM_DEFS_H_ */
