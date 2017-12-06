# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

if(${CONFIG_USB_TRACE})
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/usb_trace.c")
endif()

if(${CONFIG_USB_DEVICE})
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/usb_control.c")
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/usb_request.c")
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/usb_string.c")
endif()

if(${CONFIG_USB_DEVICE_COMPOSITE})
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/composite_device.c")
endif()

if(${CONFIG_USB_DEVICE_CDC})
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/cdc_acm.c")
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/cdc_acm_base.c")
endif()

if(${CONFIG_USB_DEVICE_DFU})
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dfu.c")
endif()

if(${CONFIG_USB_DEVICE_HID})
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/hid.c")
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/hid_base.c")
endif()

if(${CONFIG_USB_DEVICE_MSC})
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/msc.c")
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/msc_datapath.c")
endif()

if(SOURCE_FILES)
    add_library(usb OBJECT ${SOURCE_FILES})
endif()
