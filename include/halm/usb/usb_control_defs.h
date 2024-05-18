/*
 * halm/usb/usb_control_defs.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_USB_USB_CONTROL_DEFS_H_
#define HALM_USB_USB_CONTROL_DEFS_H_
/*----------------------------------------------------------------------------*/
#define EP0_BUFFER_SIZE   64
#define DATA_BUFFER_SIZE  (CONFIG_USB_DEVICE_CONTROL_REQUESTS * EP0_BUFFER_SIZE)
#define REQUEST_POOL_SIZE (CONFIG_USB_DEVICE_CONTROL_REQUESTS)

#define STRING_DESCRIPTOR_TEXT_LIMIT 126
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_USB_CONTROL_DEFS_H_ */
