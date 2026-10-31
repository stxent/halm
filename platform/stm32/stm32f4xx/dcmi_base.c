/*
 * dcmi_base.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/dcmi_base.h>
#include <halm/platform/stm32/dma_list.h>
#include <halm/platform/stm32/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void configPins(struct DcmiBase *, const struct DcmiBaseConfig *);
static void configSinglePin(PinNumber);
static bool setInstance(struct DcmiBase *);
/*----------------------------------------------------------------------------*/
static enum Result dcmiInit(void *, const void *);

#ifndef CONFIG_PLATFORM_STM32_DCMI_NO_DEINIT
static void dcmiDeinit(void *);
#else
#  define dcmiDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const DcmiBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = dcmiInit,
    .deinit = dcmiDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinGroupEntry dcmiPinGroups[] = {
    {
        /* DCMI_HSYNC */
        .begin = PIN(PORT_A, 4),
        .end = PIN(PORT_A, 4),
        .channel = 0,
        .value = 13
    }, {
        /* DCMI_PIXCLK */
        .begin = PIN(PORT_A, 6),
        .end = PIN(PORT_A, 6),
        .channel = 0,
        .value = 13
    }, {
        /* DCMI_D0 and DCMI_D1 */
        .begin = PIN(PORT_A, 9),
        .end = PIN(PORT_A, 10),
        .channel = 0,
        .value = 13
    }, {
        /* DCMI_D10, DCMI_D5, DCMI_VSYNC, DCMI_D6 and DCMI_D7 */
        .begin = PIN(PORT_B, 5),
        .end = PIN(PORT_B, 9),
        .channel = 0,
        .value = 13
    }, {
        /* DCMI_D0 to DCMI_D3, DCMI_D8, DCMI_D4 and DCMI_D9 */
        .begin = PIN(PORT_C, 6),
        .end = PIN(PORT_C, 12),
        .channel = 0,
        .value = 13
    }, {
        /* DCMI_D11 */
        .begin = PIN(PORT_D, 2),
        .end = PIN(PORT_D, 2),
        .channel = 0,
        .value = 13
    }, {
        /* DCMI_D2 and DCMI_D3 */
        .begin = PIN(PORT_E, 0),
        .end = PIN(PORT_E, 1),
        .channel = 0,
        .value = 13
    }, {
        /* DCMI_D4, DCMI_D6 and DCMI_D7 */
        .begin = PIN(PORT_E, 4),
        .end = PIN(PORT_E, 6),
        .channel = 0,
        .value = 13
    }, {
        /* DCMI_D12 */
        .begin = PIN(PORT_F, 11),
        .end = PIN(PORT_F, 11),
        .channel = 0,
        .value = 13
    }, {
        /* DCMI_D13 */
        .begin = PIN(PORT_G, 15),
        .end = PIN(PORT_G, 15),
        .channel = 0,
        .value = 13
    }, {
        /* DCMI_HSYNC and DCMI_D0 to DCMI_D3 */
        .begin = PIN(PORT_H, 8),
        .end = PIN(PORT_H, 12),
        .channel = 0,
        .value = 13
    }, {
        /* DCMI_D4 and DCMI_D11 */
        .begin = PIN(PORT_H, 14),
        .end = PIN(PORT_H, 15),
        .channel = 0,
        .value = 13
    }, {
        /*
         * DCMI_D13, DCMI_D8 to DCMI_D10, DCMI_D5, DCMI_VSYNC
         * and DCMI_D6 to DCMI_D7.
         */
        .begin = PIN(PORT_I, 0),
        .end = PIN(PORT_I, 7),
        .channel = 0,
        .value = 13
    }, {
        .begin = 0,
        .end = 0
    }
};
/*----------------------------------------------------------------------------*/
static struct DcmiBase *instance = nullptr;
/*----------------------------------------------------------------------------*/
static void configPins(struct DcmiBase *, const struct DcmiBaseConfig *config)
{
  const PinNumber pinArray[] = {
      config->pins.hsync,
      config->pins.vsync,
      config->pins.pixclk
  };

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
      configSinglePin(pinArray[index]);
  }

  for (const PinNumber *data = config->pins.data; *data; ++data)
    configSinglePin(*data);
}
/*----------------------------------------------------------------------------*/
static void configSinglePin(PinNumber number)
{
  const struct PinGroupEntry * const group = pinGroupFind(dcmiPinGroups,
      number, 0);
  assert(group != nullptr);

  const struct Pin pin = pinInit(number);

  pinInput(pin);
  pinSetFunction(pin, group->value);
}
/*----------------------------------------------------------------------------*/
static bool setInstance(struct DcmiBase *object)
{
  if (instance == nullptr)
  {
    instance = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void DCMI_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
void *dcmiMakeListDma(size_t number, uint8_t stream, enum DmaPriority priority)
{
  const struct DmaListConfig dmaConfig = {
      .number = number,
      .event = DMA_DCMI,
      .priority = priority,
      .type = DMA_TYPE_P2M,
      .stream = stream
  };

  return init(DmaList, &dmaConfig);
}
/*----------------------------------------------------------------------------*/
static enum Result dcmiInit(void *object, const void *configBase)
{
  const struct DcmiBaseConfig * const config = configBase;
  struct DcmiBase * const interface = object;

  if (!setInstance(interface))
    return E_BUSY;

  /* Configure camera interface pins */
  configPins(interface, config);

  /* Enable clock to register interface and peripheral */
  sysClockEnable(CLK_DCMI);
  /* Reset registers to initial state */
  sysResetPulse(RST_DCMI);

  interface->handler = nullptr;
  interface->irq = DCMI_IRQ;
  interface->reg = STM_DCMI;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_DCMI_NO_DEINIT
static void dcmiDeinit(void *)
{
  sysClockDisable(CLK_DCMI);
  instance = nullptr;
}
#endif
