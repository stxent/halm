/*
 * halm/usb/usb.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Abstract classes for USB Device, USB Endpoint and USB Driver.
 */

#ifndef HALM_USB_USB_H_
#define HALM_USB_USB_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <xcore/bits.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
#define USB_EP_DIRECTION_IN           BIT(7)
#define USB_EP_ADDRESS(value)         (value)
#define USB_EP_LOGICAL_ADDRESS(value) ((value) & 0x7F)
/*----------------------------------------------------------------------------*/
struct UsbDescriptor;
struct UsbSetupPacket;

typedef void (*usbDescriptorFunctor)(const void *, struct UsbDescriptor *,
    void *);
/*----------------------------------------------------------------------------*/
enum usbDeviceEvent
{
  USB_DEVICE_EVENT_RESET,
  USB_DEVICE_EVENT_SUSPEND,
  USB_DEVICE_EVENT_RESUME,
  USB_DEVICE_EVENT_FRAME,
  USB_DEVICE_EVENT_PORT_CHANGE
};
/*----------------------------------------------------------------------------*/
enum usbOption
{
  USB_SPEED,
  USB_COMPOSITE,
  USB_SELF_POWERED,
  USB_REMOTE_WAKEUP,

  USB_OPTION_END
};
/*----------------------------------------------------------------------------*/
enum usbRequestStatus
{
  /** Request completed successfully. */
  USB_REQUEST_COMPLETED,
  /** Request payload is a setup packet. */
  USB_REQUEST_SETUP,
  /** Endpoint is stalled. */
  USB_REQUEST_STALLED,
  /** Request is not completed. */
  USB_REQUEST_ERROR,
  /** Request is removed from the queue. */
  USB_REQUEST_CANCELLED
};
/*----------------------------------------------------------------------------*/
enum usbSpeed
{
  USB_LS,
  USB_FS,
  USB_HS,
  USB_SS
};
/*----------------------------------------------------------------------------*/
struct UsbRequest
{
  uint16_t capacity;
  uint16_t length;

  void (*callback)(void *, struct UsbRequest *, enum usbRequestStatus);
  void *callbackArgument;

  uint8_t *buffer;
};
/*----------------------------------------------------------------------------*/
void usbRequestInit(struct UsbRequest *, void *, uint16_t,
    void (*)(void *, struct UsbRequest *, enum usbRequestStatus), void *);
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct UsbDeviceClass
{
  CLASS_HEADER

  void *(*createEndpoint)(void *, uint8_t);
  uint8_t (*getInterface)(const void *);
  void (*setAddress)(void *, uint8_t);
  void (*setConnected)(void *, bool);

  enum result (*getOption)(const void *, enum usbOption, void *);
  enum result (*setOption)(void *, enum usbOption, const void *);

  enum result (*bind)(void *, void *);
  void (*unbind)(void *, const void *);
};
/*----------------------------------------------------------------------------*/
/**
 * Allocate an endpoint with specified logical address.
 * @param device Pointer to an UsbDevice object.
 * @param address Logical address of the endpoint.
 * @return Pointer to the UsbEndpoint object.
 */
static inline void *usbDevCreateEndpoint(void *device, uint8_t address)
{
  return ((const struct UsbDeviceClass *)CLASS(device))->createEndpoint(device,
      address);
}
/*----------------------------------------------------------------------------*/
/**
 * Get an index of a free interface.
 * @param device Pointer to an UsbDevice object.
 * @return @b Number of a free interface.
 */
static inline uint8_t usbDevGetInterface(const void *device)
{
  return ((const struct UsbDeviceClass *)CLASS(device))->getInterface(device);
}
/*----------------------------------------------------------------------------*/
/**
 * Set the address of the device.
 * @param device Pointer to an UsbDevice object.
 * @param address Device address. Must be in the range of 0 to 127.
 */
static inline void usbDevSetAddress(void *device, uint8_t address)
{
  ((const struct UsbDeviceClass *)CLASS(device))->setAddress(device, address);
}
/*----------------------------------------------------------------------------*/
/**
 * Allow or forbid the device to connect to host.
 * @param device Pointer to an UsbDevice object.
 * @param state Device state.
 */
static inline void usbDevSetConnected(void *device, bool state)
{
  ((const struct UsbDeviceClass *)CLASS(device))->setConnected(device, state);
}
/*----------------------------------------------------------------------------*/
enum result (*getOption)(void *, enum usbOption, void *);
enum result (*setOption)(void *, enum usbOption, const void *);
/*----------------------------------------------------------------------------*/
/**
 * Get the device option.
 * @param device Pointer to an UsbDevice object.
 * @param option Option to be read.
 * @param value Pointer to a buffer where a value of the option will be stored.
 * @return @b E_OK on success.
 */
static inline enum result usbDevGetOption(const void *device,
    enum usbOption option, void *value)
{
  return ((const struct UsbDeviceClass *)CLASS(device))->getOption(device,
      option, value);
}
/*----------------------------------------------------------------------------*/
/**
 * Set the device option.
 * @param device Pointer to an UsbDevice object.
 * @param option Option to be set.
 * @param value Pointer to a new value of the option.
 * @return @b E_OK on success.
 */
static inline enum result usbDevSetOption(void *device, enum usbOption option,
    const void *value)
{
  return ((const struct UsbDeviceClass *)CLASS(device))->setOption(device,
      option, value);
}
/*----------------------------------------------------------------------------*/
/**
 * Attach a device driver to the hardware device.
 * @param device Pointer to an UsbDevice object.
 * @param driver Device driver.
 * @return @b E_OK on success.
 */
static inline enum result usbDevBind(void *device, void *driver)
{
  return ((const struct UsbDeviceClass *)CLASS(device))->bind(device, driver);
}
/*----------------------------------------------------------------------------*/
/**
 * Detach the device driver from the hardware device.
 * @param device Pointer to an UsbDevice object.
 * @param driver Device driver.
 */
static inline void usbDevUnbind(void *device, const void *driver)
{
  ((const struct UsbDeviceClass *)CLASS(device))->unbind(device, driver);
}
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct UsbEndpointClass
{
  CLASS_HEADER

  void (*clear)(void *);
  void (*disable)(void *);
  void (*enable)(void *, uint8_t, uint16_t);
  enum result (*enqueue)(void *, struct UsbRequest *);
  bool (*isStalled)(void *);
  void (*setStalled)(void *, bool);
};
/*----------------------------------------------------------------------------*/
/**
 * Clear the queue of the endpoint.
 * Request status will be set to @b REQUEST_CANCELLED and endpoint callback
 * function will be called.
 * @param endpoint Pointer to an UsbEndpoint object.
 */
static inline void usbEpClear(void *endpoint)
{
  ((const struct UsbEndpointClass *)CLASS(endpoint))->clear(endpoint);
}
/*----------------------------------------------------------------------------*/
/**
 * Disable the endpoint.
 * @param endpoint Pointer to an UsbEndpoint object.
 */
static inline void usbEpDisable(void *endpoint)
{
  ((const struct UsbEndpointClass *)CLASS(endpoint))->disable(endpoint);
}
/*----------------------------------------------------------------------------*/
/**
 * Enable the endpoint.
 * @param endpoint Pointer to an UsbEndpoint object.
 * @param type Type of the endpoint.
 * @param size Size of the endpoint.
 */
static inline void usbEpEnable(void *endpoint, uint8_t type, uint16_t size)
{
  ((const struct UsbEndpointClass *)CLASS(endpoint))->enable(endpoint,
      type, size);
}
/*----------------------------------------------------------------------------*/
/**
 * Add the transport request to the queue.
 * @param endpoint Pointer to an UsbEndpoint object.
 * @param request Request to be added.
 * @return @b E_OK on success, @b E_FULL when the queue is full.
 */
static inline enum result usbEpEnqueue(void *endpoint,
    struct UsbRequest *request)
{
  return ((const struct UsbEndpointClass *)CLASS(endpoint))->enqueue(endpoint,
      request);
}
/*----------------------------------------------------------------------------*/
/**
 * Return current state of the endpoint.
 * @param endpoint Pointer to an UsbEndpoint object.
 * @return @b true when the endpoint is halted or @b false otherwise.
 */
static inline bool usbEpIsStalled(void *endpoint)
{
  return ((const struct UsbEndpointClass *)CLASS(endpoint))->
      isStalled(endpoint);
}
/*----------------------------------------------------------------------------*/
/**
 * Set or clear stall state of the endpoint.
 * @param endpoint Pointer to an UsbEndpoint object.
 * @param stalled Should be @b true to halt the endpoint or @b false
 * to start it.
 */
static inline void usbEpSetStalled(void *endpoint, bool stalled)
{
  ((const struct UsbEndpointClass *)CLASS(endpoint))->setStalled(endpoint,
      stalled);
}
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct UsbDriverClass
{
  CLASS_HEADER

  enum result (*configure)(void *, const struct UsbSetupPacket *,
      const uint8_t *, uint16_t, uint8_t *, uint16_t *, uint16_t);
  const usbDescriptorFunctor *(*describe)(const void *);
  void (*event)(void *, unsigned int);
};
/*----------------------------------------------------------------------------*/
struct UsbDriver
{
  struct Entity base;
};
/*----------------------------------------------------------------------------*/
/**
 * Process an USB Setup Packet with driver-specific handlers.
 * @param driver Pointer to an UsbDriver object.
 * @param packet USB Setup Packet.
 * @param payload Data stage of the setup packet.
 * @param payloadLength Number of bytes in the payload buffer.
 * @param response Pointer to a response buffer with a size of at least
 * @b maxLength bytes.
 * @param responseLength Number of bytes used in the response buffer.
 * Zero-length responses are allowed.
 * @param maxResponseLength Maximum length of the response buffer in bytes.
 * @return @b E_OK on success, @b E_VALUE when the buffer is too small.
 */
static inline enum result usbDriverConfigure(void *driver,
    const struct UsbSetupPacket *packet, const uint8_t *payload,
    uint16_t payloadLength, uint8_t *response, uint16_t *responseLength,
    uint16_t maxResponseLength)
{
  return ((const struct UsbDriverClass *)CLASS(driver))->configure(driver,
      packet, payload, payloadLength, response, responseLength,
      maxResponseLength);
}
/*----------------------------------------------------------------------------*/
/**
 * Get a list of descriptor functors.
 * @param driver Pointer to an UsbDriver object.
 * @return List of descriptor functors.
 */
static inline const usbDescriptorFunctor *usbDriverDescribe(const void *driver)
{
  return ((const struct UsbDriverClass *)CLASS(driver))->describe(driver);
}
/*----------------------------------------------------------------------------*/
/**
 * Handle USB event.
 * @param driver Pointer to an UsbDriver object.
 * @param event Event identifier.
 */
static inline void usbDriverEvent(void *driver, unsigned int event)
{
  ((const struct UsbDriverClass *)CLASS(driver))->event(driver, event);
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_USB_H_ */
