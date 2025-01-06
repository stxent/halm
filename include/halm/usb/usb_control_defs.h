/*
 * halm/usb/usb_control_defs.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_USB_USB_CONTROL_DEFS_H_
#define HALM_USB_USB_CONTROL_DEFS_H_
/*----------------------------------------------------------------------------*/
#define EP0_PACKET_SIZE   64
#define EP0_REQUEST_COUNT (CONFIG_USB_DEVICE_CONTROL_REQUESTS)

#ifdef CONFIG_PLATFORM_USB_DEVICE_BUFFER_ALIGNMENT
#  define EP0_BUFFER_SIZE \
    ((EP0_PACKET_SIZE + CONFIG_PLATFORM_USB_DEVICE_BUFFER_ALIGNMENT - 1) \
        / CONFIG_PLATFORM_USB_DEVICE_BUFFER_ALIGNMENT \
        * CONFIG_PLATFORM_USB_DEVICE_BUFFER_ALIGNMENT)
#else
#  define EP0_BUFFER_SIZE EP0_PACKET_SIZE
#endif

#define REQUEST_POOL_SIZE             (EP0_REQUEST_COUNT + 1)

#define STRING_BUFFER_SIZE            (EP0_REQUEST_COUNT * EP0_BUFFER_SIZE)
#define STRING_DESCRIPTOR_TEXT_LIMIT  126
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_USB_CONTROL_DEFS_H_ */
