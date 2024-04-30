/*
 * usb_base.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/imxrt/imxrt106x/system_defs.h>
#include <halm/platform/imxrt/usb_base.h>
#include <halm/platform/imxrt/usb_defs.h>
#include <halm/platform/imxrt/system.h>
#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
/*----------------------------------------------------------------------------*/
#define ENDPOINT_REQUESTS   CONFIG_PLATFORM_USB_DEVICE_POOL_SIZE
#define USB_ENDPOINT_COUNT  16
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
    /* USB OTG1 */
    {
        .key = PIN(PORT_AD_B0, 2), /* USB_OTG1_PWR */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_AD_B1, 1), /* USB_OTG1_PWR */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_AD_B0, 3), /* USB_OTG1_OC */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_AD_B1, 3), /* USB_OTG1_OC */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_AD_B0, 1), /* USB_OTG1_ID */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_AD_B1, 2), /* USB_OTG1_ID */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_USB, PIN_USB_OTG1_DN), /* USB_OTG1_DN */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_USB, PIN_USB_OTG1_DP), /* USB_OTG1_DP */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_USB, PIN_USB_OTG1_ID), /* USB_OTG1_ID */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_USB, PIN_USB_OTG1_VBUS), /* USB_OTG1_VBUS */
        .channel = 0,
        .value = 0
    },

    /* USB OTG2 */
    {
        .key = PIN(PORT_EMC, 41), /* USB_OTG2_PWR */
        .channel = 1,
        .value = 3
    }, {
        .key = PIN(PORT_AD_B0, 15), /* USB_OTG2_PWR */
        .channel = 1,
        .value = 0
    }, {
        .key = PIN(PORT_EMC, 40), /* USB_OTG2_OC */
        .channel = 1,
        .value = 3
    }, {
        .key = PIN(PORT_AD_B0, 14), /* USB_OTG2_OC */
        .channel = 1,
        .value = 0
    }, {
        .key = PIN(PORT_AD_B0, 0), /* USB_OTG2_ID */
        .channel = 1,
        .value = 3
    }, {
        .key = PIN(PORT_AD_B1, 0), /* USB_OTG2_ID */
        .channel = 1,
        .value = 0
    }, {
        .key = PIN(PORT_USB, PIN_USB_OTG2_DN), /* USB_OTG2_DN */
        .channel = 1,
        .value = 0
    }, {
        .key = PIN(PORT_USB, PIN_USB_OTG2_DP), /* USB_OTG2_DP */
        .channel = 1,
        .value = 0
    }, {
        .key = PIN(PORT_USB, PIN_USB_OTG2_VBUS), /* USB_OTG2_VBUS */
        .channel = 1,
        .value = 0
    },

    /* End of pin function association list */
    {
        .key = 0
    }
};
/*----------------------------------------------------------------------------*/
static struct UsbBase *instances[2] = {NULL};
/*----------------------------------------------------------------------------*/
static void configPins(const struct UsbBaseConfig *config)
{
  const PinNumber pinArray[] = {
      config->dm, config->dp, config->vbus
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
void USB_OTG1_ISR(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void USB_OTG2_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
void USB_PHY1_ISR(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void USB_PHY2_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
static enum Result devInit(void *object, const void *configBase)
{
  const struct UsbBaseConfig * const config = configBase;
  struct UsbBase * const device = object;

  if (!setInstance(config->channel, device))
    return E_BUSY;

  configPins(config);

  if (!sysClockStatus(CLK_USBOH3))
    sysClockEnable(CLK_USBOH3);

  if (config->channel == 0)
  {
    device->irq.phy = USB_PHY1_IRQ;
    device->irq.usb = USB_OTG1_IRQ;
    device->phy = IMX_USBPHY1;
    device->reg = IMX_USB1;
    device->td.numberOfEndpoints = USB_ENDPOINT_COUNT;
  }
  else
  {
    device->irq.phy = USB_PHY2_IRQ;
    device->irq.usb = USB_OTG2_IRQ;
    device->phy = IMX_USBPHY2;
    device->reg = IMX_USB2;
    device->td.numberOfEndpoints = USB_ENDPOINT_COUNT;
  }

  device->channel = config->channel;
  device->handler = NULL;

  const size_t memoryPoolSize =
      sizeof(struct QueueHead) * device->td.numberOfEndpoints
      + sizeof(struct TransferDescriptor) * ENDPOINT_REQUESTS;
  uint8_t * const memoryPoolPointer = memalign(4096, memoryPoolSize);

  if (memoryPoolPointer == NULL)
    return E_MEMORY;

  device->td.heads = (struct QueueHead *)memoryPoolPointer;

  struct TransferDescriptor * const descriptors =
      (struct TransferDescriptor *)(memoryPoolPointer
          + sizeof(struct QueueHead) * device->td.numberOfEndpoints);

  if (!pointerArrayInit(&device->td.descriptors, ENDPOINT_REQUESTS))
    return E_MEMORY;
  for (size_t index = 0; index < ENDPOINT_REQUESTS; ++index)
    pointerArrayPushBack(&device->td.descriptors, descriptors + index);

#ifdef CONFIG_CORE_CORTEX_DCACHE
  device->region = mpuAddRegion((uintptr_t)memoryPoolPointer, memoryPoolSize,
      MPU_REGION_DEVICE, MPU_ACCESS_FULL, false, true);
  if (device->region < 0)
    return E_ACCESS;
#else
  device->region = -1;
#endif

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_USB_NO_DEINIT
static void devDeinit(void *object)
{
  struct UsbBase * const device = object;

#ifdef CONFIG_CORE_CORTEX_DCACHE
  mpuRemoveRegion(device->region);
#endif

  pointerArrayDeinit(&device->td.descriptors);
  free(device->td.heads);

  instances[device->channel] = NULL;

  /* Disable clock when the second module is not used */
  if (instances[device->channel ^ 1] == NULL)
    sysClockDisable(CLK_USBOH3);
}
#endif
