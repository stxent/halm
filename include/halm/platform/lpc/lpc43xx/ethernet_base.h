/*
 * halm/platform/lpc/lpc43xx/ethernet_base.h
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC43XX_ETHERNET_BASE_H_
#define HALM_PLATFORM_LPC_LPC43XX_ETHERNET_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const EthernetBase;

struct EthernetBaseConfig
{
  /** Mandatory: RMII Reference Clock and MII Transmit clock. */
  PinNumber txclk;
  /** Mandatory: RMII Receive Data Valid. */
  PinNumber rxdv;
  /** Mandatory: RMII/MII Transmit Data Enable. */
  PinNumber txen;

  /**
   * RMII/MII Receive Data 0..3.
   * RXD0 and RXD1 are mandatory.
   * RXD2 and RXD3 are optional.
   */
  PinNumber rxd[4];
  /**
   * RMII/MII Transmit Data 0..3.
   * TXD0 and TXD1 are mandatory.
   * TXD2 and TXD3 are optional.
   */
  PinNumber txd[4];

  /** Optional: MII Collision detect. */
  PinNumber col;
  /** Optional: MII Carrier Sense. */
  PinNumber crs;
  /** Optional: MII Receive Clock. */
  PinNumber rxclk;
  /** Optional: MII Receive Error. */
  PinNumber rxer;
  /** Optional: MII Transmit Error. */
  PinNumber txer;

  /** Optional: MIIM Clock. */
  PinNumber mdc;
  /** Optional: MIIM Data Input and Output. */
  PinNumber mdio;
};

struct EthernetBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *);
  IrqNumber irq;

  bool miim;
  bool rmii;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_ETHERNET_BASE_H_ */
