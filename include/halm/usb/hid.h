/*
 * halm/usb/hid.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_USB_HID_H_
#define HALM_USB_HID_H_
/*----------------------------------------------------------------------------*/
#include <halm/usb/hid_base.h>
/*----------------------------------------------------------------------------*/
extern const struct HidClass * const Hid;
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct HidClass
{
  CLASS_HEADER

  void (*event)(void *, unsigned int);
  enum Result (*getReport)(void *, uint8_t, uint8_t, uint8_t *,
      uint16_t *, uint16_t);
  enum Result (*setReport)(void *, uint8_t, uint8_t, const uint8_t *,
      uint16_t);
};
/*----------------------------------------------------------------------------*/
static inline void hidEvent(void *device, unsigned int event)
{
  ((const struct HidClass *)CLASS(device))->event(device, event);
}
/*----------------------------------------------------------------------------*/
static inline enum Result hidGetReport(void *device, uint8_t reportType,
    uint8_t reportId, uint8_t *report, uint16_t *reportLength,
    uint16_t maxReportLength)
{
  return ((const struct HidClass *)CLASS(device))->getReport(device, reportType,
      reportId, report, reportLength, maxReportLength);
}
/*----------------------------------------------------------------------------*/
static inline enum Result hidSetReport(void *device, uint8_t reportType,
    uint8_t reportId, const uint8_t *report, uint16_t reportLength)
{
  return ((const struct HidClass *)CLASS(device))->setReport(device, reportType,
      reportId, report, reportLength);
}
/*----------------------------------------------------------------------------*/
struct HidConfig
{
  /** Mandatory: USB device. */
  void *device;

  /** Mandatory: report descriptor. */
  const void *descriptor;
  /** Mandatory: size of the report descriptor. */
  uint16_t descriptorSize;
  /** Mandatory: size of the report. */
  uint16_t reportSize;

  struct
  {
    /** Mandatory: identifier of the notification endpoint. */
    uint8_t interrupt;
  } endpoints;
};
/*----------------------------------------------------------------------------*/
struct Hid
{
  struct Entity base;

  /* Lower half of the driver */
  struct HidBase *driver;
};
/*----------------------------------------------------------------------------*/
enum Result hidBind(struct Hid *);
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_HID_H_ */
