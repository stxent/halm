# Copyright (C) 2017 xent
# Project is distributed under the terms of the MIT License

if(${CONFIG_USB_TRACE})
    list(APPEND SOURCE_FILES "usb_trace.c")
endif()

if(${CONFIG_USB_DEVICE})
    list(APPEND SOURCE_FILES "usb_control.c")
    list(APPEND SOURCE_FILES "usb_request.c")
    list(APPEND SOURCE_FILES "usb_string.c")
endif()

if(${CONFIG_USB_DEVICE_COMPOSITE})
    list(APPEND SOURCE_FILES "composite_device.c")
endif()

if(${CONFIG_USB_DEVICE_CDC})
    list(APPEND SOURCE_FILES "cdc_acm.c")
    list(APPEND SOURCE_FILES "cdc_acm_base.c")
endif()

if(${CONFIG_USB_DEVICE_DFU})
    list(APPEND SOURCE_FILES "dfu.c")
endif()

if(${CONFIG_USB_DEVICE_HID})
    list(APPEND SOURCE_FILES "hid.c")
    list(APPEND SOURCE_FILES "hid_base.c")
endif()

if(${CONFIG_USB_DEVICE_MSC})
    list(APPEND SOURCE_FILES "msc.c")
    list(APPEND SOURCE_FILES "msc_datapath.c")
endif()

if(SOURCE_FILES)
    add_library(halm_usb OBJECT ${SOURCE_FILES})
endif()
