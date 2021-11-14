/*
 * halm/platform/lpc/lpc43xx/ethernet_mdio.h
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC43XX_ETHERNET_MDIO_H_
#define HALM_PLATFORM_LPC_LPC43XX_ETHERNET_MDIO_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const MDIO;

struct Ethernet;

struct MDIOConfig
{
  /** Mandatory: pointer to a parent object. */
  struct Ethernet *parent;
};

struct MDIO
{
  struct Interface base;

  /* Parent interface */
  struct Ethernet *parent;

  /* Phy address */
  uint8_t address;
  /* Phy register address */
  uint8_t position;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_ETHERNET_MDIO_H_ */
