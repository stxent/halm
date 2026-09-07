/*
 * hid.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
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

    .event = nullptr,
    .getReport = nullptr,
    .setReport = nullptr
};
/*----------------------------------------------------------------------------*/
static enum Result deviceInit(void *object, const void *configBase)
{
  const struct HidConfig * const config = configBase;
  assert(config != nullptr);
  assert(config->device != nullptr);
  assert(config->descriptor != nullptr);
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
  return device->driver != nullptr ? E_OK : E_ERROR;
}
/*----------------------------------------------------------------------------*/
static void deviceDeinit(void *object)
{
  struct Hid * const device = object;
  deinit(device->driver);
}
/*----------------------------------------------------------------------------*/
enum Result hidBind(struct Hid *device)
{
  return usbDevBind(device->driver->device, device->driver);
}
