/*
 * dac_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <memory.h>
#include <platform/nxp/gen_1/dac_base.h>
#include <platform/nxp/gen_1/dac_defs.h>
#include <platform/nxp/lpc43xx/clocking.h>
#include <platform/nxp/lpc43xx/system.h>
/*----------------------------------------------------------------------------*/
static enum result configOutputPin(pin_t);
static void releaseOutputPin(pin_t);
static enum result setDescriptor(const struct DacBase *, struct DacBase *);
/*----------------------------------------------------------------------------*/
static enum result dacInit(void *, const void *);
static void dacDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass dacTable = {
    .size = sizeof(struct DacBase),
    .init = dacInit,
    .deinit = dacDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinEntry dacPins[] = {
    {
        .key = PIN(PORT_4, 4),
        .channel = 0,
        .value = 7
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const DacBase = &dacTable;
static struct DacBase *descriptor = 0;
/*----------------------------------------------------------------------------*/
static enum result configOutputPin(pin_t key)
{
  const struct PinEntry * const pinEntry = pinFind(dacPins, key, 0);

  if (!pinEntry)
    return E_VALUE;

  const struct Pin pin = pinInit(key);
  pinInput(pin);
  pinSetFunction(pin, pinEntry->value);

  /* Enable analog pin function */
  LPC_SCU->ENAIO2 |= 0x01;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void releaseOutputPin(pin_t key __attribute__((unused)))
{
  /* Disable analog pin function */
  LPC_SCU->ENAIO2 &= ~0x01;
}
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(const struct DacBase *state,
    struct DacBase *interface)
{
  return compareExchangePointer((void **)&descriptor, state,
      interface) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
uint32_t dacGetClock(const struct DacBase *interface __attribute__((unused)))
{
  return clockFrequency(Apb3Clock);
}
/*----------------------------------------------------------------------------*/
static enum result dacInit(void *object, const void *configBase)
{
  const struct DacBaseConfig * const config = configBase;
  struct DacBase * const interface = object;
  enum result res;

  /* Try to set peripheral descriptor */
  if ((res = setDescriptor(0, interface)) != E_OK)
    return res;

  if ((res = configOutputPin(config->pin)) != E_OK)
    return res;

  /* Enable clock to register interface and peripheral */
  sysClockEnable(CLK_APB3_DAC);
  /* Reset registers to default values */
  sysResetEnable(RST_DAC);

  interface->pin = config->pin;
  interface->reg = LPC_DAC;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void dacDeinit(void *object)
{
  const struct DacBase * const interface = object;

  sysClockDisable(CLK_APB3_DAC);
  releaseOutputPin(interface->pin);
  setDescriptor(interface, 0);
}
