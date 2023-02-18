/*
 * halm/platform/numicro/m03x/usb_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_USB_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_M03X_USB_BASE_H_
#define HALM_PLATFORM_NUMICRO_M03X_USB_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/usb/usb.h>
/*----------------------------------------------------------------------------*/
struct UsbEndpoint;
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline unsigned int usbEpCountTrailingZeros8(uint32_t value)
{
  /* It is assumed that the input value is not zero */
  unsigned int result = 1;

  if ((value & 0x0F) == 0)
  {
    result += 4;
    value >>= 4;
  }

  if ((value & 0x03) == 0)
  {
    result += 2;
    value >>= 2;
  }

  result -= value & 0x01;
  return result;
}

static inline void usbEpFlagsIterate(void (*callback)(struct UsbEndpoint *),
    struct UsbEndpoint **endpoints, uint32_t status)
{
  while (status)
  {
    const unsigned int index = usbEpCountTrailingZeros8(status);

    callback(endpoints[index]);
    status -= 1 << index;
  }
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M03X_USB_BASE_H_ */
