/*
 * spifi_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/spifi_base.h>
#include <halm/platform/lpc/system.h>
#include <xcore/bits.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void configPins(struct SpifiBase *, const struct SpifiBaseConfig *);
static bool setInstance(struct SpifiBase *);
/*----------------------------------------------------------------------------*/
static enum Result spifiInit(void *, const void *);
static void spifiDeinit(void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const SpifiBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = spifiInit,
    .deinit = spifiDeinit
};
/*----------------------------------------------------------------------------*/
static const struct PinGroupEntry spifiPinGroups[] = {
    {
        /*
         * P3_3 - SPIFI_SCK
         * P3_4 - SPIFI_SIO3
         * P3_5 - SPIFI_SIO2
         * P3_6 - SPIFI_MISO or SPIFI_SIO1
         * P3_7 - SPIFI_MOSI or SPIFI_SIO0
         * P3_8 - SPIFI_CS
         */
        .begin = PIN(PORT_3, 3),
        .end = PIN(PORT_3, 8),
        .channel = 0,
        .value = 3
    }, {
        /* End of pin function association list */
        .begin = 0,
        .end = 0
    }
};
/*----------------------------------------------------------------------------*/
static struct SpifiBase *instance = NULL;
/*----------------------------------------------------------------------------*/
static void configPins(struct SpifiBase *interface,
    const struct SpifiBaseConfig *config)
{
  const PinNumber pinArray[] = {
      config->cs,
      config->io0,
      config->io1,
      config->io2,
      config->io3,
      config->sck
  };
  bool wide = true;

  assert(config->cs && config->sck && config->io0 && config->io1);
  assert((!config->io2 && !config->io3) || (config->io2 && config->io3));

  if (config->io2 && config->io3)
    wide = true;

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (!pinArray[index])
      continue;

    const struct PinGroupEntry * const group = pinGroupFind(spifiPinGroups,
        pinArray[index], 0);
    assert(group != NULL);

    const struct Pin pin = pinInit(pinArray[index]);

    pinInput(pin);
    pinSetFunction(pin, group->value);
  }

  interface->wide = wide;
}
/*----------------------------------------------------------------------------*/
static bool setInstance(struct SpifiBase *object)
{
  if (instance == NULL)
  {
    instance = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void SPIFI_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
uint32_t spifiGetClock([[maybe_unused]] const struct SpifiBase *interface)
{
  /* Clock frequency should not exceed 104 MHz */
  return clockFrequency(SpifiClock);
}
/*----------------------------------------------------------------------------*/
void *spifiGetMemoryAddress([[maybe_unused]] const struct SpifiBase *interface,
    bool large)
{
  if (large)
    return (void *)LPC_SPIFI_BASE;
  else
    return (void *)LPC_SPIFI_DEBUG_BASE;
}
/*----------------------------------------------------------------------------*/
static enum Result spifiInit(void *object, const void *configBase)
{
  const struct SpifiBaseConfig * const config = configBase;
  struct SpifiBase * const interface = object;

  if (!setInstance(interface))
    return E_BUSY;

  configPins(interface, config);

  sysClockEnable(CLK_M4_SPIFI);
  sysClockEnable(CLK_SPIFI);
  sysResetEnable(RST_SPIFI);

  interface->handler = NULL;
  interface->irq = SPIFI_IRQ;
  interface->reg = LPC_SPIFI;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void spifiDeinit([[maybe_unused]] void *object)
{
  sysClockDisable(CLK_SPIFI);
  sysClockDisable(CLK_M4_SPIFI);

  instance = NULL;
}
