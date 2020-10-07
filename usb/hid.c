/*
 * hid.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/usb/hid.h>
#include <halm/usb/hid_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static enum Result deviceInit(void *, const void *);
static void deviceDeinit(void *);
/*----------------------------------------------------------------------------*/
const struct HidClass * const Hid = &(const struct HidClass){
    .size = 0, /* Abstract class */
    .init = deviceInit,
    .deinit = deviceDeinit,

    .event = 0,
    .getReport = 0,
    .setReport = 0
};
/*----------------------------------------------------------------------------*/
static enum Result deviceInit(void *object, const void *configBase)
{
  const struct HidConfig * const config = configBase;
  assert(config);
  assert(config->device);
  assert(config->descriptor);
  assert(config->descriptorSize);
  assert(config->reportSize);

  struct Hid * const device = object;
  const struct HidBaseConfig driverConfig = {
      .owner = device,
      .device = config->device,
      .descriptor = config->descriptor,
      .descriptorSize = config->descriptorSize,
      .reportSize = config->reportSize,
      .endpoints.interrupt = config->endpoints.interrupt
  };

  device->driver = init(HidBase, &driverConfig);
  return device->driver ? E_OK : E_ERROR;
}
/*----------------------------------------------------------------------------*/
static void deviceDeinit(void *object)
{
  deinit(object);
}
/*----------------------------------------------------------------------------*/
enum Result hidBind(struct Hid *device)
{
  return usbDevBind(device->driver->device, device->driver);
}
