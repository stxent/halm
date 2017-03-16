#Copyright (C) 2015 xent
#Project is distributed under the terms of the GNU General Public License v3.0

ifeq ($(CONFIG_USB_TRACE),y)
  PROJECT_FLAGS += CONFIG_USB_TRACE

  CSOURCES += usb/usb_trace.c
endif

ifeq ($(CONFIG_USB_DEVICE),y)
  PROJECT_FLAGS += CONFIG_USB_DEVICE_CONTROL_REQUESTS

  CSOURCES += usb/usb_control.c
  CSOURCES += usb/usb_request.c
  CSOURCES += usb/usb_string.c
endif

ifeq ($(CONFIG_USB_DEVICE_COMPOSITE),y)
  PROJECT_FLAGS += CONFIG_USB_DEVICE_COMPOSITE

  CSOURCES += usb/composite_device.c
endif

ifeq ($(CONFIG_USB_DEVICE_HS),y)
  PROJECT_FLAGS += CONFIG_USB_DEVICE_HS
endif

ifeq ($(CONFIG_USB_DEVICE_CDC),y)
  CSOURCES += usb/cdc_acm.c
  CSOURCES += usb/cdc_acm_base.c
endif

ifeq ($(CONFIG_USB_DEVICE_DFU),y)
  CSOURCES += usb/dfu.c
endif

ifeq ($(CONFIG_USB_DEVICE_HID),y)
  CSOURCES += usb/hid.c
  CSOURCES += usb/hid_base.c
endif

ifeq ($(CONFIG_USB_DEVICE_MSC),y)
  CSOURCES += usb/msc.c
endif
