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
#define UNPACK_FUNCTION(value)  ((value) & 0x0F)
/* Unpack match channel */
#define UNPACK_CHANNEL(value)   (((value) >> 4) & 0x0F)
/*----------------------------------------------------------------------------*/
static inline uint32_t *calcMatchChannel(LPC_TIMER_Type *, uint8_t);
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
static inline uint32_t *calcMatchChannel(LPC_TIMER_Type *timer,
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
static void updateResolution(struct GpTimerPwmUnit *device, uint8_t channel)
{
  LPC_TIMER_Type *reg = device->parent.reg;

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
static enum result unitInit(void *object, const void *configPtr)
{
  const struct GpTimerPwmUnitConfig * const config = configPtr;
  struct GpTimerPwmUnit *device = object;
  const struct GpTimerBaseConfig parentConfig = {
      .channel = config->channel,
      .input = 0
  };
  enum result res;

  assert(config->frequency && config->resolution);

  /* Call GpTimerBase class constructor */
  if ((res = GpTimerBase->init(object, &parentConfig)) != E_OK)
    return res;

  device->resolution = config->resolution;
  device->matches = 0;
  device->current = findEmptyChannel(device->matches);

  LPC_TIMER_Type *reg = device->parent.reg;

  reg->MCR = 0; /* Reset control register */
  reg->PC = reg->TC = 0; /* Reset internal counters */
  reg->IR = IR_MASK; /* Clear pending interrupts */
  reg->PWMC = 0; /* Disable all PWM channels */

  /* Configure prescaler */
  reg->PR = gpTimerGetClock(object)
      / (config->frequency * (uint32_t)config->resolution) - 1;
  /* Enable timer/counter */
  reg->TCR = TCR_CEN;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void unitDeinit(void *object)
{
  struct GpTimerPwmUnit *device = object;

  /* TODO Recursive delete of all ancestors */

  /* Disable counter */
  ((LPC_TIMER_Type *)device->parent.reg)->TCR &= ~TCR_CEN;
  /* Call GpTimerBase destructor */
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
    return 0;

  int8_t freeChannel = findEmptyChannel(config->parent->matches
      | (1 << UNPACK_CHANNEL(pinDescriptor->value)));
  if (freeChannel == -1)
    return 0; /* There are no free match channels left */

  pwm->unit = config->parent;
  pwm->channel = UNPACK_CHANNEL(pinDescriptor->value);

  /* Initialize PWM specific registers in Timer/Counter block */
  LPC_TIMER_Type *reg = pwm->unit->parent.reg;

  pwm->reg = calcMatchChannel(reg, pwm->channel);
  pwm->unit->matches |= 1 << UNPACK_CHANNEL(pinDescriptor->value);
  updateResolution(pwm->unit, (uint8_t)freeChannel);

  /* Initialize match output pin */
  struct Gpio pin = gpioInit(config->pin);
  gpioOutput(pin, 0);
  gpioSetFunction(pin, UNPACK_FUNCTION(pinDescriptor->value));

  /* Set initial duty cycle */
  /* Call function directly because of unfinished object construction */
  channelSetDutyCycle(pwm, config->value);

  /* Enable selected PWM channel */
  reg->PWMC |= PWMC_ENABLE(pwm->channel);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  struct GpTimerPwm *pwm = object;
  LPC_TIMER_Type *reg = pwm->unit->parent.reg;

  /* Disable PWM channel */
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
    *pwm->reg = percentage == 100 ? 0 : (uint32_t)pwm->unit->resolution
        * (uint32_t)(100 - percentage) / 100;
  }
  else
    *pwm->reg = (uint32_t)pwm->unit->resolution + 1;
}
/*----------------------------------------------------------------------------*/
static void channelSetPeriod(void *object, uint16_t period)
{
  struct GpTimerPwm *pwm = object;

  if (period == pwm->unit->resolution)
    *pwm->reg = 0;
  else
    *pwm->reg = !period ? (uint32_t)pwm->unit->resolution + 1 : period;
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
