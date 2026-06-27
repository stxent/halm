/*
 * usb_base.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/lpc43xx/usb_base.h>
#include <halm/platform/lpc/lpc43xx/usb_defs.h>
#include <halm/platform/lpc/system.h>
#include <halm/platform/lpc/system_defs.h>
#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
/*----------------------------------------------------------------------------*/
#define ENDPOINT_REQUESTS     CONFIG_PLATFORM_USB_DEVICE_POOL_SIZE
#define USB0_ENDPOINT_COUNT   12
#define USB1_ENDPOINT_COUNT   8
/*----------------------------------------------------------------------------*/
static void configPins(const struct UsbBaseConfig *);
static bool setInstance(uint8_t, struct UsbBase *);
/*----------------------------------------------------------------------------*/
static enum Result devInit(void *, const void *);

#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *);
#else
#  define devDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const UsbBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = devInit,
    .deinit = devDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinEntry usbPins[] = {
    {
        .key = PIN(PORT_1, 5), /* USB0_PWR_FAULT */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_1, 7), /* USB0_PPWR */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_2, 0), /* USB0_PPWR */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_2, 1), /* USB0_PWR_FAULT */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_2, 3), /* USB0_PPWR */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_2, 4), /* USB0_PWR_FAULT */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_6, 3), /* USB0_PPWR */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(PORT_6, 6), /* USB0_PWR_FAULT */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_8, 0), /* USB0_PWR_FAULT */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(PORT_USB, PIN_USB0_DM), /* USB0_DM */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_USB, PIN_USB0_DP), /* USB0_DP */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_USB, PIN_USB0_ID), /* USB0_ID */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_USB, PIN_USB0_VBUS), /* USB0_VBUS */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_2, 5), /* USB1_VBUS1 */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(PORT_9, 5), /* USB1_PPWR */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(PORT_9, 6), /* USB1_PWR_FAULT */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(PORT_USB, PIN_USB1_DM), /* USB1_DM */
        .channel = 1,
        .value = 0
    }, {
        .key = PIN(PORT_USB, PIN_USB1_DP), /* USB1_DP */
        .channel = 1,
        .value = 0
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct UsbBase *instances[2] = {NULL};
/*----------------------------------------------------------------------------*/
static void configPins(const struct UsbBaseConfig *config)
{
  const PinNumber pinArray[] = {
      config->dm, config->dp, config->connect, config->vbus
  };

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      const struct PinEntry * const pinEntry = pinFind(usbPins, pinArray[index],
          config->channel);
      assert(pinEntry != NULL);

      const struct Pin pin = pinInit(pinArray[index]);

      pinInput(pin);
      pinSetFunction(pin, pinEntry->value);
    }
  }
}
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct UsbBase *object)
{
  assert(channel < ARRAY_SIZE(instances));

  if (instances[channel] == NULL)
  {
    instances[channel] = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void USB0_ISR(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void USB1_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
void usbBaseOtgTerminationControl(struct UsbBase *device, bool enable)
{
  if (device->channel == 0)
  {
    LPC_USB_Type * const reg = device->reg;

    if (enable)
      reg->OTGSC &= ~OTGSC_OT;
    else
      reg->OTGSC |= OTGSC_OT;
  }
}
/*----------------------------------------------------------------------------*/
void usbBaseVbusDischargeControl(struct UsbBase *device, bool enable)
{
  if (device->channel == 0)
  {
    LPC_USB_Type * const reg = device->reg;

    if (enable)
      reg->OTGSC |= OTGSC_VD;
    else
      reg->OTGSC &= ~OTGSC_VD;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result devInit(void *object, const void *configBase)
{
  const struct UsbBaseConfig * const config = configBase;
  struct UsbBase * const device = object;

  if (!setInstance(config->channel, device))
    return E_BUSY;

  configPins(config);

  if (config->channel == 0)
  {
    sysClockEnable(CLK_M4_USB0);
    sysClockEnable(CLK_USB0);
    sysResetEnable(RST_USB0);

    device->irq = USB0_IRQ;
    device->reg = LPC_USB0;
    device->td.numberOfEndpoints = USB0_ENDPOINT_COUNT;

    LPC_CREG->CREG0 &= ~CREG0_USB0PHY;
    LPC_USB0->OTGSC = 0;
  }
  else
  {
    sysClockEnable(CLK_M4_USB1);
    sysClockEnable(CLK_USB1);
    sysResetEnable(RST_USB1);

    device->irq = USB1_IRQ;
    device->reg = LPC_USB1;
    device->td.numberOfEndpoints = USB1_ENDPOINT_COUNT;

    LPC_SCU->SFSUSB = SFSUSB_ESEA | SFSUSB_EPWR | SFSUSB_VBUS;
  }

  device->channel = config->channel;
  device->handler = NULL;

  device->td.heads =
      memalign(2048, sizeof(struct QueueHead) * device->td.numberOfEndpoints);
  if (device->td.heads == NULL)
    return E_MEMORY;

  device->td.memory =
      memalign(32, sizeof(struct TransferDescriptor) * ENDPOINT_REQUESTS);
  if (device->td.memory == NULL)
    return E_MEMORY;

  if (!pointerArrayInit(&device->td.descriptors, ENDPOINT_REQUESTS))
    return E_MEMORY;
  for (size_t index = 0; index < ENDPOINT_REQUESTS; ++index)
    pointerArrayPushBack(&device->td.descriptors, device->td.memory + index);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *object)
{
  struct UsbBase * const device = object;

  pointerArrayDeinit(&device->td.descriptors);
  free(device->td.memory);
  free(device->td.heads);

  if (device->channel == 0)
  {
    LPC_CREG->CREG0 |= CREG0_USB0PHY;

    sysClockDisable(CLK_USB0);
    sysClockDisable(CLK_M4_USB0);
  }
  else
  {
    LPC_SCU->SFSUSB &= ~(SFSUSB_EPWR | SFSUSB_VBUS);

    sysClockDisable(CLK_USB1);
    sysClockDisable(CLK_M4_USB1);
  }

  instances[device->channel] = NULL;
}
#endif
