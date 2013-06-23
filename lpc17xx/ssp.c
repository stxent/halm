/*
 * ssp.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include "system.h"
#include "mutex.h"
#include "ssp.h"
#include "ssp_defs.h"
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV         CLK_DIV1
#define DEFAULT_DIV_VALUE   1
/*----------------------------------------------------------------------------*/
static const struct GpioDescriptor sspPins[] = {
    {
        .key = GPIO_TO_PIN(0, 6), /* SSP1_SSEL */
        .channel = 1,
        .value = 2
    }, {
        .key = GPIO_TO_PIN(0, 7), /* SSP1_SCK */
        .channel = 1,
        .value = 2
    }, {
        .key = GPIO_TO_PIN(0, 8), /* SSP1_MISO */
        .channel = 1,
        .value = 2
    }, {
        .key = GPIO_TO_PIN(0, 9), /* SSP1_MOSI */
        .channel = 1,
        .value = 2
    }, {
        .key = GPIO_TO_PIN(0, 15), /* SSP0_SCK */
        .channel = 0,
        .value = 2
    }, {
        .key = GPIO_TO_PIN(0, 16), /* SSP0_SSEL */
        .channel = 0,
        .value = 2
    }, {
        .key = GPIO_TO_PIN(0, 17), /* SSP0_MISO */
        .channel = 0,
        .value = 2
    }, {
        .key = GPIO_TO_PIN(0, 18), /* SSP0_MOSI */
        .channel = 0,
        .value = 2
    }, {
        .key = GPIO_TO_PIN(1, 20), /* SSP0_SCK */
        .channel = 0,
        .value = 3
    }, {
        .key = GPIO_TO_PIN(1, 21), /* SSP0_SSEL */
        .channel = 0,
        .value = 3
    }, {
        .key = GPIO_TO_PIN(1, 23), /* SSP0_MISO */
        .channel = 0,
        .value = 3
    }, {
        .key = GPIO_TO_PIN(1, 24), /* SSP0_MOSI */
        .channel = 0,
        .value = 3
    }, {
        /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static enum result sspInit(void *, const void *);
static void sspDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass sspTable = {
    .size = 0, /* Abstract class */
    .init = sspInit,
    .deinit = sspDeinit,

    .read = 0,
    .write = 0,
    .get = 0,
    .set = 0
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass *Ssp = &sspTable;
/*----------------------------------------------------------------------------*/
static struct Ssp *descriptors[] = {0, 0};
static Mutex lock = MUTEX_UNLOCKED;
/*----------------------------------------------------------------------------*/
enum result sspSetDescriptor(uint8_t channel, void *descriptor)
{
  enum result res = E_ERROR;

  if (channel < sizeof(descriptors))
  {
    mutexLock(&lock);
    if (!descriptors[channel])
    {
      descriptors[channel] = descriptor;
      res = E_OK;
    }
    mutexUnlock(&lock);
  }
  return res;
}
/*----------------------------------------------------------------------------*/
void SSP0_IRQHandler(void)
{
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void SSP1_IRQHandler(void)
{
  if (descriptors[1])
    descriptors[1]->handler(descriptors[1]);
}
/*----------------------------------------------------------------------------*/
void sspSetRate(struct Ssp *device, uint32_t rate)
{
  uint16_t divider;

  divider = ((SystemCoreClock / DEFAULT_DIV_VALUE) >> 1) / rate - 1;
  /* FIXME Rewrite */
  device->reg->CPSR = 2;
  device->reg->CR0 &= ~CR0_SCR_MASK;
  device->reg->CR0 |= CR0_SCR(divider);
}
/*----------------------------------------------------------------------------*/
uint32_t sspGetRate(struct Ssp *device)
{
  uint32_t rate;
  uint16_t divider;

  divider = CR0_SCR_VALUE(device->reg->CR0);
  rate = ((SystemCoreClock / DEFAULT_DIV_VALUE) >> 1) / (divider + 1);
  return rate;
}
/*----------------------------------------------------------------------------*/
static enum result sspInit(void *object, const void *configPtr)
{
  const struct GpioDescriptor *pin;
  const struct SspConfig * const config = configPtr;
  struct Ssp *device = object;
  enum result res;

  /* TODO Add mater/slave select */
  /* Check device configuration data */
  assert(config);

  /* Try to set peripheral descriptor */
  device->channel = config->channel;
  if ((res = sspSetDescriptor(device->channel, device)) != E_OK)
    return res;

  /* Reset pointer to interrupt handler */
  device->handler = 0;

  /* Setup Serial Clock pin */
  pin = gpioFind(sspPins, config->sck, device->channel);
  assert(pin);
  device->sckPin = gpioInit(config->sck, GPIO_OUTPUT);
  gpioSetFunction(&device->sckPin, pin->value);

  /* Setup MISO pin */
  pin = gpioFind(sspPins, config->miso, device->channel);
  assert(pin);
  device->misoPin = gpioInit(config->miso, GPIO_INPUT);
  gpioSetFunction(&device->misoPin, pin->value);

  /* Setup MOSI pin */
  pin = gpioFind(sspPins, config->mosi, device->channel);
  assert(pin);
  device->mosiPin = gpioInit(config->mosi, GPIO_OUTPUT);
  gpioSetFunction(&device->mosiPin, pin->value);

  /* Setup CS pin, only in slave mode */
//  pin = gpioFindFunc(sspPins, config->cs);
//  assert(pin);
//  device->csPin = gpioInit(config->cs, GPIO_INPUT);
//  gpioSetFunction(&device->csPin, pin->value);

  switch (device->channel)
  {
    case 0:
      sysPowerEnable(PWR_SSP0);
      sysClockControl(CLK_SSP0, DEFAULT_DIV);
      device->reg = LPC_SSP0;
      device->irq = SSP0_IRQn;
      break;
    case 1:
      sysPowerEnable(PWR_SSP1);
      sysClockControl(CLK_SSP1, DEFAULT_DIV);
      device->reg = LPC_SSP1;
      device->irq = SSP1_IRQn;
      break;
  }

  device->reg->CR0 = 0;
  if (!config->frame)
    device->reg->CR0 |= CR0_DSS(8);
  else
  {
    assert(config->frame >= 4 && config->frame <= 16);
    device->reg->CR0 |= CR0_DSS(config->frame);
  }

  sspSetRate(device, config->rate); /* TODO Remove assert and add return val */
  device->reg->CR1 = CR1_SSE; /* Enable peripheral */

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sspDeinit(void *object)
{
  struct Ssp *device = object;

  /* Disable UART peripheral power */
  switch (device->channel)
  {
    case 0:
      sysPowerDisable(PWR_SSP0);
      break;
    case 1:
      sysPowerDisable(PWR_SSP1);
      break;
  }
//  gpioDeinit(&device->csPin);
  gpioDeinit(&device->mosiPin);
  gpioDeinit(&device->misoPin);
  gpioDeinit(&device->sckPin);
  /* Reset SSP descriptor */
  sspSetDescriptor(device->channel, 0);
}
