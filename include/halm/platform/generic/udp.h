/*
 * halm/platform/generic/udp.h
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_GENERIC_UDP_H_
#define HALM_PLATFORM_GENERIC_UDP_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Udp;

struct UdpConfig
{
  const char *clientAddress;
  uint16_t clientPort;
  uint16_t serverPort;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_GENERIC_UDP_H_ */
