/*
 * gptimer_pwm.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/gptimer_defs.h>
#include <platform/nxp/gptimer_pwm.h>
/*----------------------------------------------------------------------------*/
/* Unpack function */
#define UNPACK_FUNCTION(value)          ((value) & 0x0F)
/* Unpack match channel */
#define UNPACK_CHANNEL(value)           (((value) >> 4) & 0x0F)
/*----------------------------------------------------------------------------*/
struct GpTimerPwmChannel
{
  struct Pwm parent;

  struct GpTimerPwm *controller;
  uint32_t *reg;
  struct Gpio pin;
  uint8_t channel; /* Match channel */
};
/*----------------------------------------------------------------------------*/
extern const struct GpioDescriptor gpTimerPwmPins[];
/*----------------------------------------------------------------------------*/
static inline uint32_t *calcMatchChannel(LPC_TMR_Type *, uint8_t);
static int8_t findEmptyChannel(uint8_t);
static void updateResolution(struct GpTimerPwm *, uint8_t);
/*----------------------------------------------------------------------------*/
static enum result controllerInit(void *, const void *);
static void controllerDeinit(void *);
static void *controllerCreate(void *, gpio_t, uint8_t);
/*----------------------------------------------------------------------------*/
static void channelControl(void *, bool);
static void channelSetDutyCycle(void *, uint8_t);
static void channelSetPeriod(void *, uint16_t);
/*----------------------------------------------------------------------------*/
static const struct PwmControllerClass controllerTable = {
    .size = sizeof(struct GpTimerPwm),
    .init = controllerInit,
    .deinit = controllerDeinit,

    .create = controllerCreate
};

static const struct PwmClass channelTable = {
    .size = sizeof(struct GpTimerPwmChannel),
    .init = 0,
    .deinit = 0,

    .control = channelControl,
    .setDutyCycle = channelSetDutyCycle,
    .setPeriod = channelSetPeriod
};
/*----------------------------------------------------------------------------*/
const struct PwmControllerClass *GpTimerPwm = &controllerTable;
const struct PwmClass *GpTimerPwmChannel = &channelTable;
/*----------------------------------------------------------------------------*/
static inline uint32_t *calcMatchChannel(LPC_TMR_Type *timer,
    uint8_t channel)
{
  return (uint32_t *)&timer->MR0 + channel;
}
/*----------------------------------------------------------------------------*/
static int8_t findEmptyChannel(uint8_t channels)
{
  int8_t pos = 4; /* Each timer has 4 match blocks */

  while (--pos >= 0 && channels & (1 << pos));
  return pos;
}
/*----------------------------------------------------------------------------*/
static void updateResolution(struct GpTimerPwm *device, uint8_t channel)
{
  LPC_TMR_Type *reg = device->timer->parent.reg;

  /* Put timer in reset state */
  reg->TCR |= TCR_CRES;
  while (reg->TC || reg->PC);

  /* Disable previous match channel */
  reg->MCR &= ~MCR_RESET(device->current);

  device->current = channel;
  *calcMatchChannel(reg, device->current) = device->resolution;
  /* Enable new match channel */
  reg->MCR |= MCR_RESET(device->current);
  /* Enable timer */
  reg->TCR &= ~TCR_CRES;
}
/*----------------------------------------------------------------------------*/
static void channelControl(void *object, bool state)
{
  struct GpTimerPwmChannel *pwm = object;
  LPC_TMR_Type *reg = pwm->controller->timer->parent.reg;

  if (!state)
  {
    reg->PWMC &= ~PWMC_ENABLE(pwm->channel);
    reg->EMR &= ~EMR_EXTERNAL_MATCH(pwm->channel);
  }
  else
    reg->PWMC |= PWMC_ENABLE(pwm->channel);
}
/*----------------------------------------------------------------------------*/
static void channelSetDutyCycle(void *object, uint8_t percentage)
{
  struct GpTimerPwmChannel *pwm = object;

  if (percentage)
  {
    *pwm->reg = percentage == 100 ? 0 : (uint32_t)pwm->controller->resolution
        * (uint32_t)(100 - percentage) / 100;
  }
  else
    *pwm->reg = (uint32_t)pwm->controller->resolution + 1;
}
/*----------------------------------------------------------------------------*/
static void channelSetPeriod(void *object, uint16_t period)
{
  struct GpTimerPwmChannel *pwm = object;

  if (period == pwm->controller->resolution)
    *pwm->reg = 0;
  else
    *pwm->reg = !period ? (uint32_t)pwm->controller->resolution + 1 : period;
}
/*----------------------------------------------------------------------------*/
static void *controllerCreate(void *object, gpio_t output, uint8_t percentage)
{
  const struct GpioDescriptor *pin;
  struct GpTimerPwm *device = object;
  struct GpTimerPwmChannel *pwm;

  if (!(pin = gpioFind(gpTimerPwmPins, output, device->timer->parent.channel)))
    return 0;

  int8_t freeChannel = findEmptyChannel(device->matches
      | (1 << UNPACK_CHANNEL(pin->value)));
  if (freeChannel == -1)
    return 0; /* There are no free match channels left */

  if (!(pwm = init(GpTimerPwmChannel, 0)))
    return 0;

  /* Initialize PWM specific registers in Timer/Counter block */
  LPC_TMR_Type *reg = device->timer->parent.reg;

  pwm->controller = device;
  pwm->channel = UNPACK_CHANNEL(pin->value);
  pwm->reg = calcMatchChannel(reg, pwm->channel);

  device->matches |= 1 << UNPACK_CHANNEL(pin->value);
  updateResolution(device, (uint8_t)freeChannel);

  /* Initialize match output pin */
  pwm->pin = gpioInit(output);
  gpioSetFunction(pwm->pin, UNPACK_FUNCTION(pin->value));

  pwmSetDutyCycle(pwm, percentage);

  /* Enable selected PWM channel */
  reg->PWMC |= PWMC_ENABLE(pwm->channel);

  return pwm;
}
/*----------------------------------------------------------------------------*/
static void controllerDeinit(void *object)
{
  struct GpTimerPwm *device = object;

  deinit(device->timer); /* Delete GpTimer object */
}
/*----------------------------------------------------------------------------*/
static enum result controllerInit(void *object, const void *configPtr)
{
  const struct GpTimerPwmConfig * const config = configPtr;
  struct GpTimerPwm *device = object;
  struct GpTimerConfig timerConfig = {
      .channel = config->channel,
      .frequency = config->frequency * (uint32_t)config->resolution
  };

  /* Create parent GpTimer object */
  if (!(device->timer = init(GpTimer, &timerConfig)))
    return E_MEMORY;

  device->resolution = config->resolution;
  device->matches = 0;
  device->current = findEmptyChannel(device->matches);

  ((LPC_TMR_Type *)device->timer->parent.reg)->PWMC = 0;

  return E_OK;
}
