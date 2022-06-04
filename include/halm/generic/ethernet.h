/*
 * halm/generic/ethernet.h
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_ETHERNET_H_
#define HALM_GENERIC_ETHERNET_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
enum EthParameter
{
  /** Enable full-duplex mode. Data pointer should be set to zero. */
  IF_ETH_FULL_DUPLEX = IF_PARAMETER_END,
  /** Enable half-duplex mode. Data pointer should be set to zero. */
  IF_ETH_HALF_DUPLEX,
  /** Enable 10 Mbit/s mode. Data pointer should be set to zero. */
  IF_ETH_10M,
  /** Enable 100 Mbit/s mode. Data pointer should be set to zero. */
  IF_ETH_100M,
  /** Enable 1000 Mbit/s mode. Data pointer should be set to zero. */
  IF_ETH_1000M
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_ETHERNET_H_ */
