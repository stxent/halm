/*
 * halm/generic/ram_proxy.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_RAM_PROXY_H_
#define HALM_GENERIC_RAM_PROXY_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const RamProxy;

struct RamProxyConfig
{
  /** Mandatory: memory arena. */
  void *arena;
  /** Mandatory: memory capacity in bytes. */
  size_t capacity;
  /**
   * Optional: size of the memory granule in bytes. If left uninitialized
   * 1 KiB granule size will be used.
   */
  size_t granule;
};

struct RamProxy
{
  struct Interface base;

  void (*callback)(void *);
  void *callbackArgument;

  uint8_t *arena;
  size_t capacity;
  size_t granule;
  size_t position;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_RAM_PROXY_H_ */
