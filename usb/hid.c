/*
 * hid.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/usb/hid.h>
#include <halm/usb/hid_defs.h>
/*----------------------------------------------------------------------------*/
static enum result deviceInit(void *, const void *);
static void deviceDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct HidClass deviceTable = {
    .size = 0, /* Abstract class */
    .init = deviceInit,
    .deinit = deviceDeinit,

    .event = 0,
    .getReport = 0,
    .setReport = 0
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
/*----------------------------------------------------------------------------*/
enum result hidBind(struct Hid *device)
{
  enum result res;

  if ((res = usbDevBind(device->driver->device, device->driver)) != E_OK)
    return res;

#ifndef CONFIG_USB_DEVICE_COMPOSITE
  usbDevSetConnected(device->driver->device, true);
#endif

  return E_OK;
}
