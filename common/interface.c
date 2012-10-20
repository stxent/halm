/*
 * interface.c
 *
 *  Created on: Oct 20, 2012
 *      Author: xen
 */

#include "interface.h"
/*------------------------------------------------------------------------------*/
enum ifResult ifStart(struct Interface *iface, uint8_t *address)
{
  if (iface->start)
    return iface->start(iface, address);
  else
    return IF_OK;
}
/*------------------------------------------------------------------------------*/
void ifStop(struct Interface *iface)
{
  if (iface->stop)
    iface->stop(iface);
}
/*------------------------------------------------------------------------------*/
unsigned int ifRead(struct Interface *iface, uint8_t *buffer,
    unsigned int length)
{
  if (iface->read)
    return iface->read(iface, buffer, length);
  else
    return 0;
}
/*------------------------------------------------------------------------------*/
unsigned int ifWrite(struct Interface *iface, const uint8_t *buffer,
    unsigned int length)
{
  if (iface->write)
    return iface->write(iface, buffer, length);
  else
    return 0;
}
/*------------------------------------------------------------------------------*/
enum ifResult ifGetOpt(struct Interface *iface, enum ifOption option, void *data)
{
  if (iface->getopt)
    return iface->getopt(iface, option, data);
  else
    return IF_OK;
}
/*------------------------------------------------------------------------------*/
enum ifResult ifSetOpt(struct Interface *iface, enum ifOption option,
    const void *data)
{
  if (iface->setopt)
    return iface->setopt(iface, option, data);
  else
    return IF_OK;
}
