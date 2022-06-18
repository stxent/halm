/*
 * halm/generic/buffering_proxy.h
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_BUFFERING_PROXY_H_
#define HALM_GENERIC_BUFFERING_PROXY_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/pointer_array.h>
#include <halm/generic/pointer_queue.h>
#include <xcore/interface.h>
#include <xcore/stream.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const BufferingProxy;

struct BufferingProxyConfig
{
  /** Mandatory: underlying interface. */
  void *pipe;

  struct
  {
    /** Optional: input stream. */
    struct Stream *stream;
    /** Optional: buffer count. */
    size_t count;
    /** Optional: buffer size. */
    size_t size;
  } rx;

  struct
  {
    /** Optional: output stream. */
    struct Stream *stream;
    /** Optional: buffer count. */
    size_t count;
    /** Optional: buffer size. */
    size_t size;
  } tx;
};

struct BufferingProxy
{
  struct Interface base;

  void (*callback)(void *);
  void *callbackArgument;

  struct Interface *pipe;
  struct Stream *rx;
  struct Stream *tx;

  void *arena;
  size_t rxBufferCount;
  size_t rxBufferSize;
  size_t txBufferCount;
  size_t txBufferSize;
  PointerQueue rxQueue;
  PointerArray rxPool;
  PointerArray txPool;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_BUFFERING_PROXY_H_ */
