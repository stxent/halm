#Copyright (C) 2015 xent
#Project is distributed under the terms of the GNU General Public License v3.0

ifeq ($(CONFIG_USB),y)
  PROJECT_FLAGS += CONFIG_USB_CONTROL_REQUESTS
  PROJECT_FLAGS += CONFIG_USB_TRACE
  PROJECT_FLAGS += CONFIG_USB_DEVICE_CURRENT
  PROJECT_FLAGS += CONFIG_USB_DEVICE_PRODUCT_ID CONFIG_USB_DEVICE_VENDOR_ID
  PROJECT_FLAGS += CONFIG_USB_DEVICE_PRODUCT_NAME CONFIG_USB_DEVICE_SERIAL CONFIG_USB_DEVICE_VENDOR_NAME

  CSOURCES += usb/usb.c
  CSOURCES += usb/usb_control.c
  CSOURCES += usb/usb_requests.c
endif

ifeq ($(CONFIG_USB_CDC),y)
  CSOURCES += usb/cdc_acm.c
  CSOURCES += usb/cdc_acm_base.c
endif

ifeq ($(CONFIG_USB_HID),y)
  CSOURCES += usb/hid.c
  CSOURCES += usb/hid_base.c
endif

ifeq ($(CONFIG_USB_TRACE),y)
  CSOURCES += usb/usb_trace.c
endif
