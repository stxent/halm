/*
 * halm/platform/lpc/lpc43xx/ethernet.h
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC43XX_ETHERNET_H_
#define HALM_PLATFORM_LPC_LPC43XX_ETHERNET_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/lpc43xx/ethernet_base.h>
#include <xcore/interface.h>
#include <xcore/stream.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Ethernet;

struct EthernetStream;

struct EthernetConfig
{
  /** Mandatory: media access control address. */
  uint64_t address;
  /** Mandatory: bitrate. */
  uint32_t rate;
  /** Mandatory: number of receive descriptors. */
  size_t rxSize;
  /** Mandatory: number of transmit descriptors. */
  size_t txSize;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: enable half-duplex mode. */
  bool halfduplex;

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

struct Ethernet
{
  struct EthernetBase base;

  /* Input stream */
  struct EthernetStream *rxStream;
  /* Output stream */
  struct EthernetStream *txStream;

  /* RX descriptors */
  struct ReceiveDescriptor *rxList;
  /* RX descriptor list capacity */
  size_t rxSize;
  /* Number of pending TX descriptors */
  size_t rxCount;
  /* Index of the first RX descriptor */
  size_t rxIndex;

  /* TX descriptors */
  struct TransmitDescriptor *txList;
  /* TX descriptor list capacity */
  size_t txSize;
  /* Number of pending TX descriptors */
  size_t txCount;
  /* Index of the first TX descriptor */
  size_t txIndex;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

struct Stream *ethGetInput(struct Ethernet *);
struct Stream *ethGetOutput(struct Ethernet *);
struct Interface *ethMakeMDIO(struct Ethernet *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_ETHERNET_H_ */
