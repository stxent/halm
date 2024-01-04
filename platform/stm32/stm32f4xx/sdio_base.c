/*
 * sdio_base.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/clocking.h>
#include <halm/platform/stm32/dma_oneshot.h>
#include <halm/platform/stm32/sdio_base.h>
#include <halm/platform/stm32/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(struct SdioBase *);
/*----------------------------------------------------------------------------*/
static enum Result sdioInit(void *, const void *);
static void sdioDeinit(void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const SdioBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = sdioInit,
    .deinit = sdioDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinGroupEntry sdioPinGroups[] = {
    {
        /* SDIO_D4 .. SDIO_D5 */
        .begin = PIN(PORT_B, 8),
        .end = PIN(PORT_B, 9),
        .channel = 0,
        .value = 12
    }, {
        /* SDIO_D6 .. SDIO_D7, SDIO_D0 .. SDIO_D3 and SDIO_CK */
        .begin = PIN(PORT_C, 6),
        .end = PIN(PORT_C, 12),
        .channel = 0,
        .value = 12
    }, {
        /* SDIO_CMD */
        .begin = PIN(PORT_D, 2),
        .end = PIN(PORT_D, 2),
        .channel = 0,
        .value = 12
    }, {
        /* End of pin function association list */
        .begin = 0,
        .end = 0
    }
};
/*----------------------------------------------------------------------------*/
static struct SdioBase *instance = NULL;
/*----------------------------------------------------------------------------*/
static bool setInstance(struct SdioBase *object)
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
void SDIO_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
uint32_t sdioGetClock(const struct SdioBase *interface __attribute__((unused)))
{
  return clockFrequency(UsbClock);
}
/*----------------------------------------------------------------------------*/
void *sdioMakeOneShotDma(uint8_t stream, enum DmaPriority priority,
    enum DmaType type)
{
  const struct DmaOneShotConfig config = {
      .event = dmaGetEventSdio(),
      .priority = priority,
      .type = type,
      .stream = stream
  };

  return init(DmaOneShot, &config);
}
/*----------------------------------------------------------------------------*/
static enum Result sdioInit(void *object, const void *configBase)
{
  const struct SdioBaseConfig * const config = configBase;
  struct SdioBase * const interface = object;

  if (!setInstance(interface))
    return E_BUSY;

  /* Configure input and output pins */
  sdioConfigPins(interface, config);

  /* Enable clocks to register interface and peripheral */
  sysClockEnable(CLK_SDIO);
  /* Reset registers to default values */
  sysResetEnable(RST_SDIO);
  sysResetDisable(RST_SDIO);

  interface->handler = NULL;
  interface->irq = SDIO_IRQ;
  interface->reg = STM_SDIO;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sdioDeinit(void *object __attribute__((unused)))
{
  sysClockDisable(CLK_SDIO);
  instance = NULL;
}
