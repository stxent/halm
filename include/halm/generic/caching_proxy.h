/*
 * halm/generic/caching_proxy.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_CACHING_PROXY_H_
#define HALM_GENERIC_CACHING_PROXY_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/pointer_array.h>
#include <xcore/interface.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const CachingProxy;

/** Caching proxy parameters. */
enum CachingProxyParameter
{
  /**
   * Number of read operations served from the cache. Read-only parameter.
   * Parameter type is \p uint32_t.
   */
  IF_CACHE_READ_MATCH = IF_PARAMETER_END,
  /**
   * Number of read operations that required an underlying access.
   * Read-only parameter. Parameter type is \p uint32_t.
   */
  IF_CACHE_READ_MISS,
  /**
   * Number of write operations that updated an existing cached block.
   * Read-only parameter. Parameter type is \p uint32_t.
   */
  IF_CACHE_WRITE_MATCH,
  /**
   * Number of write operations that allocated a new cached block.
   * Read-only parameter. Parameter type is \p uint32_t.
   */
  IF_CACHE_WRITE_MISS,
  /** End of the caching proxy parameter list. */
  IF_CACHE_PARAMETER_END
};
/*----------------------------------------------------------------------------*/
struct CachingProxyConfig
{
  /**
   * Mandatory: total size of the cache in bytes. The cache is split into
   * blocks of \p size bytes each, any remaining bytes are left unused.
   */
  size_t cache;
  /** Mandatory: size of a single block (sector) in bytes. */
  size_t size;
  /**
   * Optional: memory buffer for the cached data. The buffer must be able
   * hold all cache blocks. When the pointer is set to @b nullptr the buffer
   * is allocated on the heap and released during deinitialization.
   */
  void *arena;
  /** Mandatory: wrapped interface. */
  void *pipe;
};
/*----------------------------------------------------------------------------*/
struct CachingProxy
{
  struct Interface base;

  /* Configured memory arena, nullptr when the buffer is heap allocated */
  void *arena;
  /* Cache buffer: block descriptors followed by the cached data */
  void *buffer;

  void (*callback)(void *);
  void *callbackArgument;

  /* Wrapped interface */
  struct Interface *pipe;

  /* Number of cache blocks */
  size_t blockCount;
  /* Size of a single cache block in bytes */
  size_t blockSize;

  /* Current block number */
  uint32_t address;
  /* Current iteration */
  uint32_t timestamp;

  /* Pool of pointers to block descriptors */
  PointerArray pool;

  /* Cache access counters */
  struct
  {
    uint32_t rMatch;
    uint32_t rMiss;
    uint32_t wMatch;
    uint32_t wMiss;
  } statistics;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_CACHING_PROXY_H_ */
