/*
 * ssp.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include "platform/nxp/ssp.h"
#include "platform/nxp/ssp_defs.h"
#include "platform/nxp/system.h"
#include "platform/nxp/lpc17xx/interrupts.h"
#include "platform/nxp/lpc17xx/power.h"
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV       CLK_DIV1
#define DEFAULT_DIV_VALUE 1
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
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t, struct Ssp *);
static inline enum result setupPins(struct Ssp *, const struct SspConfig *);
/*----------------------------------------------------------------------------*/
static enum result sspInit(void *, const void *);
static void sspDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass sspTable = {
    .size = 0, /* Abstract class */
    .init = sspInit,
    .deinit = sspDeinit,

    .callback = 0,
    .get = 0,
    .set = 0,
    .read = 0,
    .write = 0
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass *Ssp = &sspTable;
/*----------------------------------------------------------------------------*/
static struct Ssp *descriptors[] = {0, 0};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel, struct Ssp *interface)
{
  assert(channel < sizeof(descriptors));

  if (descriptors[channel])
    return E_BUSY;

  descriptors[channel] = interface;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static inline enum result setupPins(struct Ssp *interface,
    const struct SspConfig *config)
{
  const struct GpioDescriptor *pin;

  /* Setup MISO pin */
  if (!(pin = gpioFind(sspPins, config->miso, interface->channel)))
    return E_VALUE;
  interface->misoPin = gpioInit(config->miso, GPIO_INPUT);
  gpioSetFunction(&interface->misoPin, pin->value);

  /* Setup MOSI pin */
  if (!(pin = gpioFind(sspPins, config->mosi, interface->channel)))
    return E_VALUE;
  interface->mosiPin = gpioInit(config->mosi, GPIO_OUTPUT);
  gpioSetFunction(&interface->mosiPin, pin->value);

  /* Setup Serial Clock pin */
  if (!(pin = gpioFind(sspPins, config->sck, interface->channel)))
    return E_VALUE;
  interface->sckPin = gpioInit(config->sck, GPIO_OUTPUT);
  gpioSetFunction(&interface->sckPin, pin->value);

  /* Setup CS pin, only in slave mode */
//  if (!(pin = gpioFind(sspPins, config->cs, interface->channel)))
//    return E_VALUE;
//  interface->csPin = gpioInit(config->cs, GPIO_INPUT);
//  gpioSetFunction(&interface->csPin, pin->value);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
void SSP0_ISR(void)
{
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void SSP1_ISR(void)
{
  if (descriptors[1])
    descriptors[1]->handler(descriptors[1]);
}
/*----------------------------------------------------------------------------*/
void sspSetRate(struct Ssp *interface, uint32_t rate)
{
  LPC_SSP_TypeDef *reg = interface->reg;
  uint16_t divider;

  divider = ((sysCoreClock / DEFAULT_DIV_VALUE) >> 1) / rate - 1;
  /* FIXME Add more precise calculation of SSP dividers */
  reg->CPSR = 2;
  reg->CR0 &= ~CR0_SCR_MASK;
  reg->CR0 |= CR0_SCR(divider);
}
/*----------------------------------------------------------------------------*/
uint32_t sspGetRate(const struct Ssp *interface)
{
  uint32_t rate;
  uint16_t divider;

  /* FIXME Add more precise calculation of SSP rate */
  divider = CR0_SCR_VALUE(((LPC_SSP_TypeDef *)interface->reg)->CR0);
  rate = ((sysCoreClock / DEFAULT_DIV_VALUE) >> 1) / (divider + 1);
  return rate;
}
/*----------------------------------------------------------------------------*/
static enum result sspInit(void *object, const void *configPtr)
{
  const struct SspConfig * const config = configPtr;
  struct Ssp *interface = object;
  enum result res;

  /* When frame is set its value should be from 4 to 16 */
  if (config->frame && config->frame - 4 > 12)
    return E_VALUE;

  /* Try to set peripheral descriptor */
  interface->channel = config->channel;
  if ((res = setDescriptor(interface->channel, interface)) != E_OK)
    return res;

  if ((res = setupPins(interface, config)) != E_OK)
    return res;

  interface->handler = 0;

  switch (interface->channel)
  {
    case 0:
      sysPowerEnable(PWR_SSP0);
      sysClockControl(CLK_SSP0, DEFAULT_DIV);
      interface->reg = LPC_SSP0;
      interface->irq = SSP0_IRQ;
      break;
    case 1:
      sysPowerEnable(PWR_SSP1);
      sysClockControl(CLK_SSP1, DEFAULT_DIV);
      interface->reg = LPC_SSP1;
      interface->irq = SSP1_IRQ;
      break;
  }

  /* Initialize SSP block */
  LPC_SSP_TypeDef *reg = interface->reg;

  /* Set frame size */
  reg->CR0 = !config->frame ? CR0_DSS(8) : CR0_DSS(config->frame);

  /* Set mode for SPI interface */
  if (config->mode & 0x01) //TODO Remove magic numbers
    reg->CR0 |= CR0_CPHA;
  if (config->mode & 0x02)
    reg->CR0 |= CR0_CPOL;

  sspSetRate(interface, config->rate);
  reg->CR1 = CR1_SSE; /* Enable peripheral */

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sspDeinit(void *object)
{
  struct Ssp *interface = object;

  /* Disable UART peripheral power */
  switch (interface->channel)
  {
    case 0:
      sysPowerDisable(PWR_SSP0);
      break;
    case 1:
      sysPowerDisable(PWR_SSP1);
      break;
  }
//  gpioDeinit(&interface->csPin);
  gpioDeinit(&interface->mosiPin);
  gpioDeinit(&interface->misoPin);
  gpioDeinit(&interface->sckPin);
  /* Reset SSP descriptor */
  setDescriptor(interface->channel, 0);
}
