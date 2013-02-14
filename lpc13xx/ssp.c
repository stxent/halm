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
/* SSP settings */
/* In LPC13xx SSP clock divisor is number from 1 to 255, 0 to disable */
#define DEFAULT_DIV                     1
#define DEFAULT_DIV_VALUE               1
/*----------------------------------------------------------------------------*/
enum cleanup
{
  FREE_NONE = 0,
  FREE_DESCRIPTOR,
  FREE_SCK_PIN,
  FREE_MISO_PIN,
  FREE_MOSI_PIN,
  FREE_CS_PIN, /* TODO */
  FREE_PERIPHERAL,
  FREE_ALL
};
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
static void sspCleanup(struct Ssp *, enum cleanup);
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
static void * volatile descriptors[] = {0, 0};
static Mutex lock = MUTEX_UNLOCKED;
/*----------------------------------------------------------------------------*/
static void sspCleanup(struct Ssp *device, enum cleanup step)
{
  switch (step)
  {
    case FREE_ALL:
    case FREE_PERIPHERAL:
      /* Disable UART peripheral power */
      LPC_SYSCON->SSP0CLKDIV = 0;
      sysClockDisable(CLK_SSP); //FIXME
      break;
    case FREE_CS_PIN:
//      gpioDeinit(&device->csPin);
    case FREE_MOSI_PIN:
      gpioDeinit(&device->mosiPin);
    case FREE_MISO_PIN:
      gpioDeinit(&device->misoPin);
    case FREE_SCK_PIN:
      gpioDeinit(&device->sckPin);
    case FREE_DESCRIPTOR:
      /* Reset SSP descriptor */
      sspSetDescriptor(device->channel, 0);
    default:
      break;
  }
}
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
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result sspInit(void *object, const void *configPtr)
{
  /* Set pointer to device configuration data */
  const struct SspConfig *config = configPtr;
  struct Ssp *device = object;
  gpioFunc func;
  enum result res;

  /* TODO Add mater/slave select */
  /* Check device configuration data */
  if (!config)
    return E_ERROR;

  /* Try to set peripheral descriptor */
  device->channel = config->channel;
  if ((res = sspSetDescriptor(device->channel, device)) != E_OK)
    return res;

  /* Setup Serial Clock pin */
  func = gpioFindFunc(sspPins, config->sck);
  device->sckPin = gpioInit(config->sck, GPIO_OUTPUT);
  if (func == -1 || !gpioGetKey(&device->sckPin))
  {
    sspCleanup(device, FREE_DESCRIPTOR);
    return E_ERROR;
  }
  gpioSetFunc(&device->sckPin, func);

  /* Setup MISO pin */
  func = gpioFindFunc(sspPins, config->miso);
  device->misoPin = gpioInit(config->miso, GPIO_INPUT);
  if (func == -1 || !gpioGetKey(&device->misoPin))
  {
    sspCleanup(device, FREE_SCK_PIN);
    return E_ERROR;
  }
  gpioSetFunc(&device->misoPin, func);

  /* Setup MOSI pin */
  func = gpioFindFunc(sspPins, config->mosi);
  device->mosiPin = gpioInit(config->mosi, GPIO_OUTPUT);
  if (func == -1 || !gpioGetKey(&device->mosiPin))
  {
    sspCleanup(device, FREE_MISO_PIN);
    return E_ERROR;
  }
  gpioSetFunc(&device->mosiPin, func);

  /* Setup CS pin, only in slave mode */
//  func = gpioFindFunc(sspPins, config->cs);
//  device->csPin = gpioInit(config->cs, GPIO_INPUT);
//  if (func == -1 || !gpioGetKey(&device->csPin))
//  {
//    sspCleanup(device, FREE_MOSI_PIN);
//    return E_ERROR;
//  }
//  gpioSetFunc(&device->csPin, func);

  switch (config->channel)
  {
    case 0:
      sysClockEnable(CLK_SSP);
      LPC_SYSCON->SSP0CLKDIV = DEFAULT_DIV; /* Divide AHB clock */
      LPC_SYSCON->PRESETCTRL = 1; /* FIXME */
      device->reg = LPC_SSP0;
      device->irq = SSP0_IRQn;

      /* Set SCK0 pin location register */
      switch (config->sck)
      {
        case GPIO_TO_PIN(0, 10):
          LPC_IOCON->SCK_LOC = 0;
          break;
        case GPIO_TO_PIN(2, 11):
          LPC_IOCON->SCK_LOC = 1;
          break;
        case GPIO_TO_PIN(0, 6):
          LPC_IOCON->SCK_LOC = 2;
          break;
      }
      break;
    case 1:
      sysClockEnable(CLK_SSP); /* FIXME */
      LPC_SYSCON->SSP1CLKDIV = DEFAULT_DIV; /* Divide AHB clock */
      LPC_SYSCON->PRESETCTRL = 4; /* FIXME */
      device->reg = LPC_SSP1;
      device->irq = SSP1_IRQn;
      break;
  }

  device->reg->CR0 = 0;
  if (!config->frame)
  {
    device->reg->CR0 |= CR0_DSS(8);
  }
  else
  {
    if (config->frame < 4 || config->frame > 16)
    {
      sspCleanup(device, FREE_PERIPHERAL);
      return E_ERROR;
    }
    device->reg->CR0 |= CR0_DSS(config->frame);
  }

  if ((res = sspSetRate(device, config->rate)) != E_OK)
  {
    sspCleanup(device, FREE_PERIPHERAL);
    return res;
  }

  device->reg->CR1 = CR1_SSE; /* Enable peripheral */

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sspDeinit(void *object)
{
  sspCleanup(object, FREE_ALL);
}
