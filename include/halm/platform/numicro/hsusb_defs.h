/*
 * halm/platform/numicro/hsusb_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_HSUSB_DEFS_H_
#define HALM_PLATFORM_NUMICRO_HSUSB_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Global Interrupt Status register--------------------------*/
#define GINTSTS_USBIF                   BIT(0)
#define GINTSTS_CEPIF                   BIT(1)
#define GINTSTS_EPIF(ep)                BIT((ep) + 2)
#define GINTSTS_EPIF_MASK               BIT_FIELD(MASK(12), 2)
#define GINTSTS_EPIF_VALUE(reg)         FIELD_VALUE((reg), GINTSTS_EPIF_MASK, 2)
/*------------------Global Interrupt Enable register--------------------------*/
#define GINTEN_USBIEN                   BIT(0)
#define GINTEN_CEPIEN                   BIT(1)
#define GINTEN_EPIEN(ep)                BIT((ep) + 2)
#define GINTEN_EPIEN_MASK               BIT_FIELD(MASK(12), 2)
#define GINTEN_EPIEN_VALUE(reg)         FIELD_VALUE((reg), GINTEN_EPIEN_MASK, 2)
/*------------------Bus Interrupt Status register-----------------------------*/
#define BUSINTSTS_SOFIF                 BIT(0)
#define BUSINTSTS_RSTIF                 BIT(1)
#define BUSINTSTS_RESUMEIF              BIT(2)
#define BUSINTSTS_SUSPENDIF             BIT(3)
#define BUSINTSTS_HISPDIF               BIT(4)
#define BUSINTSTS_DMADONEIF             BIT(5)
#define BUSINTSTS_PHYCLKVLDIF           BIT(6)
#define BUSINTSTS_VBUSDETIF             BIT(8)

#define BUSINTSTS_MASK \
    (BUSINTSTS_SOFIF | BUSINTSTS_RSTIF | BUSINTSTS_RESUMEIF \
        | BUSINTSTS_SUSPENDIF | BUSINTSTS_HISPDIF | BUSINTSTS_DMADONEIF \
        | BUSINTSTS_PHYCLKVLDIF | BUSINTSTS_VBUSDETIF)
/*------------------Bus Interrupt Enable register-----------------------------*/
#define BUSINTEN_SOFIEN                 BIT(0)
#define BUSINTEN_RSTIEN                 BIT(1)
#define BUSINTEN_RESUMEIEN              BIT(2)
#define BUSINTEN_SUSPENDIEN             BIT(3)
#define BUSINTEN_HISPDIEN               BIT(4)
#define BUSINTEN_DMADONEIEN             BIT(5)
#define BUSINTEN_PHYCLKVLDIEN           BIT(6)
#define BUSINTEN_VBUSDETIEN             BIT(8)
/*------------------Operational register--------------------------------------*/
#define OPER_RESUMEEN                   BIT(0)
#define OPER_HISPDEN                    BIT(1)
#define OPER_CURSPD                     BIT(2)
/*------------------Frame Count register--------------------------------------*/
#define FRAMECNT_MFRAMECNT_MASK         BIT_FIELD(MASK(3), 0)
#define FRAMECNT_MFRAMECNT(value)       BIT_FIELD((value), 0)
#define FRAMECNT_MFRAMECNT_VALUE(reg) \
    FIELD_VALUE((reg), FRAMECNT_MFRAMECNT_MASK, 0)

#define FRAMECNT_FRAMECNT_MASK         BIT_FIELD(MASK(11), 3)
#define FRAMECNT_FRAMECNT(value)       BIT_FIELD((value), 3)
#define FRAMECNT_FRAMECNT_VALUE(reg) \
    FIELD_VALUE((reg), FRAMECNT_FRAMECNT_MASK, 3)
/*------------------Test Mode register----------------------------------------*/
enum
{
  TESTMODE_NORMAL             = 0,
  TESTMODE_TEST_J             = 1,
  TESTMODE_TEST_K             = 2,
  TESTMODE_TEST_SE0           = 3,
  TESTMODE_TEST_PACKET        = 4,
  TESTMODE_TEST_FORCE_ENABLE  = 5
};

#define TEST_TESTMODE_MASK              BIT_FIELD(MASK(3), 0)
#define TEST_TESTMODE(value)            BIT_FIELD((value), 0)
#define TEST_TESTMODE_VALUE(reg) \
    FIELD_VALUE((reg), TEST_TESTMODE_MASK, 0)
/*------------------Control Endpoint Control register-------------------------*/
#define CEPCTL_NAKCLR                   BIT(0)
#define CEPCTL_STALLEN                  BIT(1)
#define CEPCTL_ZEROLEN                  BIT(2)
#define CEPCTL_FLUSH                    BIT(3)
/*------------------Control Endpoint Interrupt Enable register----------------*/
#define CEPINTEN_SETUPTKIEN             BIT(0)
#define CEPINTEN_SETUPPKIEN             BIT(1)
#define CEPINTEN_OUTTKIEN               BIT(2)
#define CEPINTEN_INTKIEN                BIT(3)
#define CEPINTEN_PINGIEN                BIT(4)
#define CEPINTEN_TXPKIEN                BIT(5)
#define CEPINTEN_RXPKIEN                BIT(6)
#define CEPINTEN_NAKIEN                 BIT(7)
#define CEPINTEN_STALLIEN               BIT(8)
#define CEPINTEN_ERRIEN                 BIT(9)
#define CEPINTEN_STSDONEIEN             BIT(10)
#define CEPINTEN_BUFFULLIEN             BIT(11)
#define CEPINTEN_BUFEMPTYIEN            BIT(12)
/*------------------Control Endpoint Interrupt Status register----------------*/
#define CEPINTSTS_SETUPTKIF             BIT(0)
#define CEPINTSTS_SETUPPKIF             BIT(1)
#define CEPINTSTS_OUTTKIF               BIT(2)
#define CEPINTSTS_INTKIF                BIT(3)
#define CEPINTSTS_PINGIF                BIT(4)
#define CEPINTSTS_TXPKIF                BIT(5)
#define CEPINTSTS_RXPKIF                BIT(6)
#define CEPINTSTS_NAKIF                 BIT(7)
#define CEPINTSTS_STALLIF               BIT(8)
#define CEPINTSTS_ERRIF                 BIT(9)
#define CEPINTSTS_STSDONEIF             BIT(10)
#define CEPINTSTS_BUFFULLIF             BIT(11)
#define CEPINTSTS_BUFEMPTYIF            BIT(12)

#define CEPINTSTS_MASK \
    (CEPINTSTS_SETUPTKIF | CEPINTSTS_SETUPPKIF | CEPINTSTS_OUTTKIF \
        | CEPINTSTS_INTKIF | CEPINTSTS_PINGIF | CEPINTSTS_TXPKIF \
        | CEPINTSTS_RXPKIF | CEPINTSTS_NAKIF | CEPINTSTS_STALLIF \
        | CEPINTSTS_ERRIF | CEPINTSTS_STSDONEIF | CEPINTSTS_BUFFULLIF \
        | CEPINTSTS_BUFEMPTYIF)
/*------------------DMA Control Status register-------------------------------*/
#define DMACTL_EPNUM_MASK               BIT_FIELD(MASK(4), 0)
#define DMACTL_EPNUM(value)             BIT_FIELD((value), 0)
#define DMACTL_EPNUM_VALUE(reg)         FIELD_VALUE((reg), DMACTL_EPNUM_MASK, 0)

#define DMACTL_DMARD                    BIT(4)
#define DMACTL_DMAEN                    BIT(5)
#define DMACTL_SGEN                     BIT(6)
#define DMACTL_DMARST                   BIT(7)
#define DMACTL_SVINEP                   BIT(8)
/*------------------Endpoint Interrupt Status register-------------------------*/
#define EPINTSTS_BUFFULLIF              BIT(0)
#define EPINTSTS_BUFEMPTYIF             BIT(1)
#define EPINTSTS_SHORTTXIF              BIT(2)
#define EPINTSTS_TXPKIF                 BIT(3)
#define EPINTSTS_RXPKIF                 BIT(4)
#define EPINTSTS_OUTTKIF                BIT(5)
#define EPINTSTS_INTKIF                 BIT(6)
#define EPINTSTS_PINGIF                 BIT(7)
#define EPINTSTS_NAKIF                  BIT(8)
#define EPINTSTS_STALLIF                BIT(9)
#define EPINTSTS_NYETIF                 BIT(10)
#define EPINTSTS_ERRIF                  BIT(11)
#define EPINTSTS_SHORTRXIF              BIT(12)

#define EPINTSTS_MASK \
    (EPINTSTS_SHORTTXIF | EPINTSTS_TXPKIF | EPINTSTS_RXPKIF \
        | EPINTSTS_OUTTKIF | EPINTSTS_INTKIF | EPINTSTS_PINGIF \
        | EPINTSTS_NAKIF | EPINTSTS_STALLIF | EPINTSTS_NYETIF \
        | EPINTSTS_ERRIF | EPINTSTS_SHORTRXIF)
/*------------------Endpoint Interrupt Enable register------------------------*/
#define EPINTEN_BUFFULLIEN              BIT(0)
#define EPINTEN_BUFEMPTYIEN             BIT(1)
#define EPINTEN_SHORTTXIEN              BIT(2)
#define EPINTEN_TXPKIEN                 BIT(3)
#define EPINTEN_RXPKIEN                 BIT(4)
#define EPINTEN_OUTTKIEN                BIT(5)
#define EPINTEN_INTKIEN                 BIT(6)
#define EPINTEN_PINGIEN                 BIT(7)
#define EPINTEN_NAKIEN                  BIT(8)
#define EPINTEN_STALLIEN                BIT(9)
#define EPINTEN_NYETIEN                 BIT(10)
#define EPINTEN_ERRIEN                  BIT(11)
#define EPINTEN_SHORTRXIEN              BIT(12)
/*------------------Endpoint Data Available Count register--------------------*/
#define EPDATCNT_DATCNT_MASK            BIT_FIELD(MASK(16), 0)
#define EPDATCNT_DATCNT(value)          BIT_FIELD((value), 0)
#define EPDATCNT_DATCNT_VALUE(reg) \
    FIELD_VALUE((reg), EPDATCNT_DATCNT_MASK, 0)

#define EPDATCNT_DMALOOP_MASK           BIT_FIELD(MASK(15), 16)
#define EPDATCNT_DMALOOP(value)         BIT_FIELD((value), 16)
#define EPDATCNT_DMALOOP_VALUE(reg) \
    FIELD_VALUE((reg), EPDATCNT_DMALOOP_MASK, 16)
/*------------------Endpoint Response Control register------------------------*/
enum
{
  MODE_AUTO_VALIDATE    = 0,
  MODE_MANUAL_VALIDATE  = 1,
  MODE_FLY_MODE         = 2
};

#define EPRSPCTL_FLUSH                  BIT(0)

#define EPRSPCTL_MODE_MASK              BIT_FIELD(MASK(2), 1)
#define EPRSPCTL_MODE(value)            BIT_FIELD((value), 1)
#define EPRSPCTL_MODE_VALUE(reg) \
    FIELD_VALUE((reg), EPRSPCTL_MODE_MASK, 1)

#define EPRSPCTL_TOGGLE                 BIT(3)
#define EPRSPCTL_HALT                   BIT(4)
#define EPRSPCTL_ZEROLEN                BIT(5)
#define EPRSPCTL_SHORTTXEN              BIT(6)
#define EPRSPCTL_DISBUF                 BIT(7)
/*------------------Endpoint Configuration register---------------------------*/
enum
{
  EPTYPE_BULK         = 1,
  EPTYPE_INTERRUPT    = 2,
  EPTYPE_ISOCHRONOUS  = 3
};

#define EPCFG_EPEN                      BIT(0)

#define EPCFG_EPTYPE_MASK               BIT_FIELD(MASK(2), 1)
#define EPCFG_EPTYPE(value)             BIT_FIELD((value), 1)
#define EPCFG_EPTYPE_VALUE(reg)         FIELD_VALUE((reg), EPCFG_EPTYPE_MASK, 1)

#define EPCFG_EPDIR                     BIT(3)

#define EPCFG_EPNUM_MASK                BIT_FIELD(MASK(4), 4)
#define EPCFG_EPNUM(value)              BIT_FIELD((value), 4)
#define EPCFG_EPNUM_VALUE(reg)          FIELD_VALUE((reg), EPCFG_EPNUM_MASK, 4)
/*------------------Endpoint RAM Start and End Adress registers---------------*/
#define EPBUF_ADDR_ALIGNMENT            4
/*------------------PHY Control register--------------------------------------*/
#define PHYCTL_DPPUEN                   BIT(8)
#define PHYCTL_PHYEN                    BIT(9)
#define PHYCTL_WKEN                     BIT(24)
#define PHYCTL_VBUSDET                  BIT(31)
/*----------------------------------------------------------------------------*/
#define EP_TO_NUMBER(ep)                ((ep) & 0x0F)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_HSUSB_DEFS_H_ */
