/*
 * halm/platform/numicro/m48x/usb_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_USB_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_M48X_USB_BASE_H_
#define HALM_PLATFORM_NUMICRO_M48X_USB_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/usb/usb.h>
#include <xcore/accel.h>
/*----------------------------------------------------------------------------*/
struct UsbEndpoint;
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void usbEpFlagsIterate(void (*callback)(struct UsbEndpoint *),
    struct UsbEndpoint **endpoints, uint32_t status)
{
  status = reverseBits32(status);

  do
  {
    const unsigned int index = countLeadingZeros32(status);

    callback(endpoints[index]);
    status -= (1UL << 31) >> index;
  }
  while (status);
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M48X_USB_BASE_H_ */
