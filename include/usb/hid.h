/*
 * usb/hid.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef USB_HID_H_
#define USB_HID_H_
/*----------------------------------------------------------------------------*/
#include <usb/hid_base.h>
/*----------------------------------------------------------------------------*/
extern const struct HidClass * const Hid;
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct HidClass
{
  CLASS_HEADER

  enum result (*getReport)(void *, uint8_t, uint8_t, uint8_t *,
      uint16_t *, uint16_t);
  enum result (*setReport)(void *, uint8_t, uint8_t, const uint8_t *,
      uint16_t);
  void (*updateStatus)(void *, uint8_t);
};
/*----------------------------------------------------------------------------*/
static inline enum result hidGetReport(void *device, uint8_t reportType,
    uint8_t reportId, uint8_t *report, uint16_t *reportLength,
    uint16_t maxReportLength)
{
  return ((const struct HidClass *)CLASS(device))->getReport(device, reportType,
      reportId, report, reportLength, maxReportLength);
}
/*----------------------------------------------------------------------------*/
static inline enum result hidSetReport(void *device, uint8_t reportType,
    uint8_t reportId, const uint8_t *report, uint16_t reportLength)
{
  return ((const struct HidClass *)CLASS(device))->setReport(device, reportType,
      reportId, report, reportLength);
}
/*----------------------------------------------------------------------------*/
static inline void hidUpdateStatus(void *device, uint8_t status)
{
  ((const struct HidClass *)CLASS(device))->updateStatus(device, status);
}
/*----------------------------------------------------------------------------*/
struct HidConfig
{
  /** Mandatory: USB device. */
  void *device;
  /** Mandatory: report descriptor. */
  void *report;
  /** Mandatory: size of the report descriptor. */
  uint16_t reportSize;

  /** Optional: serial number string. */
  const char *serial;

  struct
  {
    /** Mandatory: identifier of the notification endpoint. */
    uint8_t interrupt;
  } endpoint;

  /** Optional: enable composite device mode. */
  bool composite;
};
/*----------------------------------------------------------------------------*/
struct Hid
{
  struct Entity parent;

  /* Lower half of the driver */
  struct HidBase *driver;
};
/*----------------------------------------------------------------------------*/
#endif /* USB_HID_H_ */
