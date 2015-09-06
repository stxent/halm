#Copyright (C) 2015 xent
#Project is distributed under the terms of the GNU General Public License v3.0

ifeq ($(CONFIG_USB),y)
  FLAG_NAMES += CONFIG_USB_TRACE
  FLAG_NAMES += CONFIG_USB_DEVICE_PRODUCT_NAME CONFIG_USB_DEVICE_SERIAL CONFIG_USB_DEVICE_VENDOR_NAME
  FLAG_NAMES += CONFIG_USB_DEVICE_CURRENT CONFIG_USB_DEVICE_PRODUCT_ID CONFIG_USB_DEVICE_VENDOR_ID

  CSOURCES += usb/requests.c
  CSOURCES += usb/usb.c
  CSOURCES += usb/usb_device.c
endif

ifeq ($(CONFIG_USB_CDC),y)
  CSOURCES += usb/cdc_acm.c
  CSOURCES += usb/cdc_acm_base.c
endif

ifeq ($(CONFIG_USB_TRACE),y)
  CSOURCES += usb/usb_trace.c
endif
