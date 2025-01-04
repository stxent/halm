/*
 * halm/usb/usb_control_defs.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_USB_USB_CONTROL_DEFS_H_
#define HALM_USB_USB_CONTROL_DEFS_H_
/*----------------------------------------------------------------------------*/
#define EP0_REQUEST_COUNT (CONFIG_USB_DEVICE_CONTROL_REQUESTS)
#define EP0_BUFFER_SIZE   64

#define REQUEST_POOL_SIZE             (EP0_REQUEST_COUNT + 1)
#define REQUEST_POOL_ARENA            (REQUEST_POOL_SIZE * EP0_BUFFER_SIZE)

#define STRING_BUFFER_SIZE            (EP0_REQUEST_COUNT * EP0_BUFFER_SIZE)
#define STRING_DESCRIPTOR_TEXT_LIMIT  126
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_USB_CONTROL_DEFS_H_ */
