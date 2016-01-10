/*
 * hid.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <usb/hid.h>
#include <usb/hid_defs.h>
/*----------------------------------------------------------------------------*/
static enum result deviceInit(void *, const void *);
static void deviceDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct HidClass deviceTable = {
    .size = 0, /* Abstract class */
    .init = deviceInit,
    .deinit = deviceDeinit,

    .getReport = 0,
    .setReport = 0,
    .updateStatus = 0
};
/*----------------------------------------------------------------------------*/
const struct HidClass * const Hid = &deviceTable;
/*----------------------------------------------------------------------------*/
static enum result deviceInit(void *object, const void *configBase)
{
  const struct HidConfig * const config = configBase;
  struct Hid * const device = object;
  const struct HidBaseConfig driverConfig = {
      .owner = device,
      .device = config->device,
      .descriptor = config->descriptor,
      .descriptorSize = config->descriptorSize,
      .reportSize = config->reportSize,
      .endpoint.interrupt = config->endpoint.interrupt
  };

  device->driver = init(HidBase, &driverConfig);
  if (!device->driver)
    return E_MEMORY;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void deviceDeinit(void *object)
{
  const struct Hid * const device = object;

  deinit(device->driver);
}
