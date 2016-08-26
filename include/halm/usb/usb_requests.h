/*
 * halm/usb/usb_requests.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_USB_USB_REQUESTS_H_
#define HALM_USB_USB_REQUESTS_H_
/*----------------------------------------------------------------------------*/
#include <halm/usb/usb.h>
/*----------------------------------------------------------------------------*/
enum result usbExtractDescriptorData(const void *, uint16_t, uint16_t, void *,
    uint16_t *, uint16_t);
void usbFillConfigurationDescriptor(const void *, void *);
void usbFillDeviceDescriptor(const void *, void *);
enum result usbHandleDeviceRequest(void *, void *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *);
enum result usbHandleEndpointRequest(void *, const struct UsbSetupPacket *,
    uint8_t *, uint16_t *);
enum result usbHandleInterfaceRequest(const struct UsbSetupPacket *,
    uint8_t *, uint16_t *);
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_USB_REQUESTS_H_ */
