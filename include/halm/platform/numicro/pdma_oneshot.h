/*
 * halm/platform/numicro/pdma_oneshot.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_PDMA_ONESHOT_H_
#define HALM_PLATFORM_NUMICRO_PDMA_ONESHOT_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/numicro/pdma_base.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const PdmaOneShot;

struct PdmaOneShotConfig
{
  /** Mandatory: request connection. */
  enum PdmaEvent event;
  /** Mandatory: channel number. */
  uint8_t channel;
};

struct PdmaOneShot
{
  struct PdmaBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* The destination address of the data to be transferred */
  uintptr_t destination;
  /* The source address of the data */
  uintptr_t source;
  /* Number of transfers */
  uint16_t transfers;

  /* State of the transfer */
  uint8_t state;
  /* Enable fixed priority mode */
  bool fixed;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_PDMA_ONESHOT_H_ */
