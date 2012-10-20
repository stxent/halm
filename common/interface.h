/*
 * interface.h
 *
 *  Created on: Oct 20, 2012
 *      Author: xen
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_
/*------------------------------------------------------------------------------*/
#include <stdint.h>
#include "mutex.h"
/*------------------------------------------------------------------------------*/
enum ifResult {
  IF_OK = 0,
  IF_ERROR
} __attribute__((packed));
/*------------------------------------------------------------------------------*/
enum ifOption {
  IF_SPEED = 0,
  IF_BUFFER_SIZE,
  IF_SYNC,
  IF_QUEUE_RX,
  IF_QUEUE_TX
} __attribute__((packed));
/*------------------------------------------------------------------------------*/
struct Interface
{
  struct Mutex lock;
  void *dev;
  /* Start transmission, arguments: device address */
  enum ifResult (*start)(struct Interface *, uint8_t *);
  /* Stop transmission */
  void (*stop)(struct Interface *);
  /* Receive data, arguments: data buffer, message size */
  unsigned int (*read)(struct Interface *, uint8_t *, unsigned int);
  /* Send data, arguments: data buffer, message size */
  unsigned int (*write)(struct Interface *, const uint8_t *, unsigned int);
  /* Get interface option, arguments: option id, pointer to save value */
  enum ifResult (*getopt)(struct Interface *, enum ifOption, void *);
  /* Set interface option, arguments: option id, pointer to new value */
  enum ifResult (*setopt)(struct Interface *, enum ifOption, const void *);
//   /* Data transmitted handler */
//   void (*txhandler)(void *);
//   /* Data received handler */
//   void (*rxhandler)(void *);
};
/*------------------------------------------------------------------------------*/
enum ifResult ifStart(struct Interface *, uint8_t *);
void ifStop(struct Interface *);
unsigned int ifRead(struct Interface *, uint8_t *, unsigned int);
unsigned int ifWrite(struct Interface *, const uint8_t *, unsigned int);
enum ifResult ifGetOpt(struct Interface *, enum ifOption, void *);
enum ifResult ifSetOpt(struct Interface *, enum ifOption, const void *);
/*------------------------------------------------------------------------------*/
#endif /* INTERFACE_H_ */
