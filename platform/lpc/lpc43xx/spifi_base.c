/*
 * spifi_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/spifi_base.h>
#include <halm/platform/lpc/system.h>
#include <xcore/bits.h>
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
const struct PinEntry spifiPins[] = {
    {
        .key = PIN(PORT_3, 3), /* SPIFI_SCK */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_3, 4), /* SPIFI_SIO3 */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_3, 5), /* SPIFI_SIO2 */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_3, 6), /* SPIFI_MISO or SPIFI_SIO1 */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_3, 7), /* SPIFI_MOSI or SPIFI_SIO0 */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_3, 8), /* SPIFI_CS */
        .channel = 0,
        .value = 3
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct SpifiBase *instance = 0;
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

    const struct PinEntry * const pinEntry = pinFind(spifiPins,
        pinArray[index], 0);
    assert(pinEntry);

    const struct Pin pin = pinInit(pinArray[index]);

    pinInput(pin);
    pinSetFunction(pin, pinEntry->value);
  }

  interface->wide = wide;
}
/*----------------------------------------------------------------------------*/
static bool setInstance(struct SpifiBase *object)
{
  if (!instance)
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
uint32_t spifiGetClock(const struct SpifiBase *interface
    __attribute__((unused)))
{
  /* Clock frequency should not exceed 104 MHz */
  return clockFrequency(SpifiClock);
}
/*----------------------------------------------------------------------------*/
void *spifiGetMemoryAddress(const struct SpifiBase *interface)
{
  if (interface->debug)
    return (void *)LPC_SPIFI_DEBUG_BASE;
  else
    return (void *)LPC_SPIFI_BASE;
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

  interface->handler = 0;
  interface->irq = SPIFI_IRQ;
  interface->reg = LPC_SPIFI;
  interface->debug = config->debug;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void spifiDeinit(void *object __attribute__((unused)))
{
  sysClockDisable(CLK_SPIFI);
  sysClockDisable(CLK_M4_SPIFI);

  instance = 0;
}
