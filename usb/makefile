#Copyright (C) 2015 xent
#Project is distributed under the terms of the GNU General Public License v3.0

ifeq ($(CONFIG_USB),y)
  FLAG_NAMES += CONFIG_USB_TRACE

  CSOURCES += usb/usb.c
  CSOURCES += usb/usb_device.c
  CSOURCES += usb/requests.c
endif

ifeq ($(CONFIG_USB_TRACE),y)
  CSOURCES += usb/usb_trace.c
endif
