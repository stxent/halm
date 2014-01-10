/*
 * gptimer_pwm.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/gptimer_pwm.h>
#include <platform/nxp/gptimer_pwm_defs.h>
/*----------------------------------------------------------------------------*/
#define UNPACK_FUNCTION(value)  ((value) & 0x0F)
/* Unpack timer match channel */
#define UNPACK_CHANNEL(value)   (((value) >> 4) & 0x0F)
/*----------------------------------------------------------------------------*/
static int8_t findEmptyChannel(uint8_t);
static void updateResolution(struct GpTimerPwmUnit *, uint8_t);
/*----------------------------------------------------------------------------*/
static enum result unitInit(void *, const void *);
static void unitDeinit(void *);
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *, const void *);
static void channelDeinit(void *);
static void channelSetEnabled(void *, bool);
static void channelSetDutyCycle(void *, uint8_t);
static void channelSetPeriod(void *, uint16_t);
/*----------------------------------------------------------------------------*/
static const struct EntityClass unitTable = {
    .size = sizeof(struct GpTimerPwmUnit),
    .init = unitInit,
    .deinit = unitDeinit
};
/*----------------------------------------------------------------------------*/
static const struct PwmClass channelTable = {
    .size = sizeof(struct GpTimerPwm),
    .init = channelInit,
    .deinit = channelDeinit,

    .setDutyCycle = channelSetDutyCycle,
    .setEnabled = channelSetEnabled,
    .setPeriod = channelSetPeriod
};
/*----------------------------------------------------------------------------*/
extern const struct GpioDescriptor gpTimerPwmPins[];
const struct EntityClass *GpTimerPwmUnit = &unitTable;
const struct PwmClass *GpTimerPwm = &channelTable;
/*----------------------------------------------------------------------------*/
static int8_t findEmptyChannel(uint8_t channels)
{
  int8_t pos = 4; /* Each timer has 4 match blocks */

  while (--pos >= 0 && channels & (1 << pos));
  return pos;
}
/*----------------------------------------------------------------------------*/
static void updateResolution(struct GpTimerPwmUnit *device, uint8_t channel)
{
  LPC_TIMER_Type *reg = device->parent.reg;

  /* Put the timer into a reset state and wait for internal counters to clear */
  reg->TCR |= TCR_CRES;
  while (reg->TC || reg->PC);

  /* Disable previous match channel */
  reg->MCR &= ~MCR_RESET(device->current);

  /* Initialize new match channel and enable it */
  device->current = channel;
  reg->MR[device->current] = device->resolution;
  reg->MCR |= MCR_RESET(device->current);

  /* Clear reset bit and enable counting */
  reg->TCR &= ~TCR_CRES;
}
/*----------------------------------------------------------------------------*/
static enum result unitInit(void *object, const void *configPtr)
{
  const struct GpTimerPwmUnitConfig * const config = configPtr;
  const struct GpTimerBaseConfig parentConfig = {
      .channel = config->channel,
      .input = 0
  };
  struct GpTimerPwmUnit *device = object;
  enum result res;

  const uint32_t clockFrequency = gpTimerGetClock(object);
  const uint32_t timerFrequency = config->frequency * config->resolution;
  assert(timerFrequency && timerFrequency < clockFrequency);

  /* Call base class constructor */
  if ((res = GpTimerBase->init(object, &parentConfig)) != E_OK)
    return res;

  device->resolution = config->resolution;
  device->matches = 0;
  device->current = findEmptyChannel(device->matches);

  LPC_TIMER_Type *reg = device->parent.reg;

  /* Base initialization is similar to common timer class */
  reg->MCR = 0;
  reg->PC = reg->TC = 0;
  reg->CCR = 0;
  reg->CTCR = 0;
  reg->EMR = 0;
  reg->IR = IR_MASK;
  /* PWM Control register is available only on certain parts */
  reg->PWMC = 0;

  /* Configure prescaler */
  reg->PR = timerFrequency / clockFrequency - 1;
  /* Enable timer */
  reg->TCR = TCR_CEN;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void unitDeinit(void *object)
{
  struct GpTimerPwmUnit *device = object;

  ((LPC_TIMER_Type *)device->parent.reg)->TCR &= ~TCR_CEN;
  GpTimerBase->deinit(object);
}
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *object, const void *configPtr)
{
  const struct GpTimerPwmConfig * const config = configPtr;
  const struct GpioDescriptor *pinDescriptor;
  struct GpTimerPwm *pwm = object;

  pinDescriptor = gpioFind(gpTimerPwmPins, config->pin,
      config->parent->parent.channel);
  if (!pinDescriptor)
    return E_VALUE;

  /* Check if there is a free match channel */
  int8_t freeChannel = findEmptyChannel(config->parent->matches
      | (1 << UNPACK_CHANNEL(pinDescriptor->value)));
  if (freeChannel == -1)
    return E_BUSY;

  pwm->unit = config->parent;
  pwm->channel = UNPACK_CHANNEL(pinDescriptor->value);

  LPC_TIMER_Type *reg = pwm->unit->parent.reg;

  pwm->value = (uint32_t *)(reg->MR + pwm->channel);
  pwm->unit->matches |= 1 << UNPACK_CHANNEL(pinDescriptor->value);
  updateResolution(pwm->unit, (uint8_t)freeChannel);

  /* Initialize match output pin */
  struct Gpio pin = gpioInit(config->pin);
  gpioOutput(pin, 0);
  gpioSetFunction(pin, UNPACK_FUNCTION(pinDescriptor->value));

  /* Call function directly because of unfinished object construction */
  channelSetDutyCycle(pwm, config->value);

  /* Enable PWM channel */
  reg->PWMC |= PWMC_ENABLE(pwm->channel);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  struct GpTimerPwm *pwm = object;
  LPC_TIMER_Type *reg = pwm->unit->parent.reg;

  reg->PWMC &= ~PWMC_ENABLE(pwm->channel);
}
/*----------------------------------------------------------------------------*/
static void channelSetEnabled(void *object, bool state)
{
  struct GpTimerPwm *pwm = object;
  LPC_TIMER_Type *reg = pwm->unit->parent.reg;

  if (!state)
  {
    reg->PWMC &= ~PWMC_ENABLE(pwm->channel);
    /* Clear match value to avoid undefined output level */
    reg->EMR &= ~EMR_EXTERNAL_MATCH(pwm->channel);
  }
  else
    reg->PWMC |= PWMC_ENABLE(pwm->channel);
}
/*----------------------------------------------------------------------------*/
static void channelSetDutyCycle(void *object, uint8_t percentage)
{
  struct GpTimerPwm *pwm = object;

  if (percentage)
  {
    /*
     * If a match register is set to zero, than output pin goes high
     * and will stay in this state continuously.
     */
    *pwm->value = percentage == 100 ? 0 : (uint32_t)pwm->unit->resolution
        * (uint32_t)(100 - percentage) / 100;
  }
  else
  {
    /*
     * Set match register to a value greater than resolution
     * to obtain low level on output pin during all cycle.
     */
    *pwm->value = (uint32_t)pwm->unit->resolution + 1;
  }
}
/*----------------------------------------------------------------------------*/
static void channelSetPeriod(void *object, uint16_t period)
{
  struct GpTimerPwm *pwm = object;

  if (period == pwm->unit->resolution)
    *pwm->value = 0;
  else
    *pwm->value = !period ? (uint32_t)pwm->unit->resolution + 1 : period;
}
/*----------------------------------------------------------------------------*/
/**
 * Create GpTimerPwm object, associated with unit.
 * @param unit Pointer to GpTimerPwmUnit object.
 * @param output pin used as output for pulse width modulated signal.
 * @param value Initial duty cycle value in percents.
 * @return Pointer to new Pwm object on success or zero on error.
 */
void *gpTimerPwmCreate(void *unit, uint8_t value, gpio_t pin)
{
  const struct GpTimerPwmConfig channelConfig = {
      .parent = unit,
      .pin = pin,
      .value = value
  };

  return init(GpTimerPwm, &channelConfig);
}
