/*
 * ssp.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "lpc13xx_sys.h"
#include "mutex.h"
#include "ssp.h"
#include "ssp_defs.h"
/*----------------------------------------------------------------------------*/
/* UART settings */
/* In LPC13xx SSP clock divisor is number from 1 to 255, 0 to disable */
#define DEFAULT_DIV                     1
#define DEFAULT_DIV_VALUE               1
/*----------------------------------------------------------------------------*/
/* UART pin function values */
static const struct GpioPinFunc sspPins[] = {
    {
        .key = GPIO_TO_PIN(0, 6), /* SSP0 SCK */
        .func = 2
    },
    {
        .key = GPIO_TO_PIN(0, 8), /* SSP0 MISO */
        .func = 1
    },
    {
        .key = GPIO_TO_PIN(0, 9), /* SSP0 MOSI */
        .func = 1
    },
    {
        .key = GPIO_TO_PIN(0, 10), /* SSP0 SCK */
        .func = 2
    },
    {
        .key = GPIO_TO_PIN(2, 1), /* SSP1 SCK */
        .func = 2
    },
    {
        .key = GPIO_TO_PIN(2, 2), /* SSP1 MISO */
        .func = 2
    },
    {
        .key = GPIO_TO_PIN(2, 3), /* SSP1 MOSI */
        .func = 2
    },
    {
        .key = GPIO_TO_PIN(2, 11), /* SSP0 SCK */
        .func = 1
    },
    {} /* End of pin function association list */
};
/*----------------------------------------------------------------------------*/
static enum result sspInit(void *, const void *);
static void sspDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct SspClass sspTable = {
    .parent = {
        .size = 0, /* Abstract class */
        .init = sspInit,
        .deinit = sspDeinit,

        .read = 0,
        .write = 0,
        .getopt = 0,
        .setopt = 0
    },
    .handler = 0
};
/*----------------------------------------------------------------------------*/
const struct SspClass *Ssp = &sspTable;
/*----------------------------------------------------------------------------*/
static void * volatile descriptors[] = {0};
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
void SSP_IRQHandler(void) /* FIXME SSP0? */
{
  if (descriptors[0])
    ((struct SspClass *)CLASS(descriptors[0]))->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
enum result sspSetRate(struct Ssp *device, uint32_t rate)
{
  uint16_t divider;

  divider = ((SystemCoreClock / DEFAULT_DIV_VALUE) >> 1) / rate - 1;
  /* FIXME Rewrite */
  device->reg->CPSR = 2;
  device->reg->CR0 &= ~CR0_SCR_MASK;
  device->reg->CR0 |= CR0_SCR(divider);
}
/*----------------------------------------------------------------------------*/
static void sspDeinit(void *object)
{
  struct Ssp *device = object;

  /* Disable SSP peripheral power */
  LPC_SYSCON->SSP0CLKDIV = 0;
  sysClockDisable(CLK_SSP);
  /* Release pins */
  gpioDeinit(&device->mosiPin);
  gpioDeinit(&device->misoPin);
  gpioDeinit(&device->sckPin);
  /* Reset SSP descriptor */
  sspSetDescriptor(device->channel, 0);
}
/*----------------------------------------------------------------------------*/
static enum result sspInit(void *object, const void *configPtr)
{
  /* Set pointer to device configuration data */
  const struct SspConfig *config = configPtr;
  struct Ssp *device = object;
  gpioFunc sckFunc, /*csFunc, */misoFunc, mosiFunc;

  /* TODO Check rate and frame width */
  /* Check device configuration data and availability */
  if (!config || sspSetDescriptor(config->channel, device) != E_OK)
    return E_ERROR;

  /* TODO Allow other pins as CS */
  /* Check pin mapping */
  sckFunc = gpioFindFunc(sspPins, config->sck);
//  csFunc = gpioFindFunc(sspPins, config->cs);
  misoFunc = gpioFindFunc(sspPins, config->miso);
  mosiFunc = gpioFindFunc(sspPins, config->mosi);
  if (sckFunc == -1 || /*csFunc == -1 || */misoFunc == -1 || mosiFunc == -1)
    return E_ERROR;

  /* Setup SSP pins */
  device->sckPin = gpioInit(config->sck, GPIO_OUTPUT);
  if (!gpioGetKey(&device->sckPin))
    return E_ERROR;

  /* TODO Different modes for slave and master*/
  device->misoPin = gpioInit(config->miso, GPIO_INPUT);
  if (!gpioGetKey(&device->misoPin))
  {
    gpioDeinit(&device->sckPin);
    return E_ERROR;
  }

  device->mosiPin = gpioInit(config->mosi, GPIO_OUTPUT);
  if (!gpioGetKey(&device->mosiPin))
  {
    gpioDeinit(&device->misoPin);
    gpioDeinit(&device->sckPin);
    return E_ERROR;
  }

  device->channel = config->channel;

  switch (config->channel)
  {
    case 0:
      sysClockEnable(CLK_SSP);
      LPC_SYSCON->SSP0CLKDIV = DEFAULT_DIV; /* Divide AHB clock */
      LPC_SYSCON->PRESETCTRL = 1; /* FIXME */
      device->reg = LPC_SSP0;
      device->irq = SSP0_IRQn;
      break;
  }

  /* TODO Set SCK0 pin location register */
  LPC_IOCON->SCK_LOC = 1; //FIXME 2.11
  /* Select the SSP function of pins */
  gpioSetFunc(&device->sckPin, sckFunc);
  gpioSetFunc(&device->misoPin, misoFunc);
  gpioSetFunc(&device->mosiPin, mosiFunc);

  device->reg->CR0 = 0;
  if (!config->size)
    device->reg->CR0 |= CR0_DSS(8);
  else
    device->reg->CR0 |= CR0_DSS(config->size); //FIXME Check

  sspSetRate(device, config->rate);

  device->reg->CR1 = CR1_SSE; /* Enable peripheral */

  return E_OK;
}
