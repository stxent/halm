/*
 * bod.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/bod.h>
#include <halm/platform/lpc/lpc17xx/system_defs.h>
#include <halm/platform/platform_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(struct Bod *);
/*----------------------------------------------------------------------------*/
static enum Result bodInit(void *, const void *);
static void bodEnable(void *);
static void bodDisable(void *);
static void bodSetCallback(void *, void (*)(void *), void *);

#ifndef CONFIG_PLATFORM_LPC_BOD_NO_DEINIT
static void bodDeinit(void *);
#else
#  define bodDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterruptClass * const Bod = &(const struct InterruptClass){
    .size = sizeof(struct Bod),
    .init = bodInit,
    .deinit = bodDeinit,

    .enable = bodEnable,
    .disable = bodDisable,
    .setCallback = bodSetCallback
};
/*----------------------------------------------------------------------------*/
static struct Bod *instance = NULL;
/*----------------------------------------------------------------------------*/
static bool setInstance(struct Bod *object)
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
void BOD_ISR(void)
{
  instance->callback(instance->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result bodInit(void *object, const void *configBase)
{
  const struct BodConfig * const config = configBase;
  assert(config != NULL);

  struct Bod * const bod = object;

  if (setInstance(bod))
  {
    bod->callback = NULL;
    bod->enabled = false;

    uint32_t pcon = LPC_SC->PCON & ~(PCON_BODRPM | PCON_BOGD | PCON_BORD);

    if (config->resetLevel == BOD_RESET_DISABLED)
      pcon |= PCON_BORD;

#ifdef CONFIG_PLATFORM_LPC_PM_DISABLE_BOD
    pcon |= PCON_BODRPM;
#endif

    /* Disable reset before Global Disable bit modification */
    LPC_SC->PCON |= PCON_BORD;
    /* Apply new settings */
    LPC_SC->PCON = pcon;

    irqSetPriority(BOD_IRQ, config->priority);
    return E_OK;
  }
  else
    return E_BUSY;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_BOD_NO_DEINIT
static void bodDeinit(void *)
{
  /* Disable interrupts */
  irqDisable(BOD_IRQ);

  /* Fully disable BOD circuitry */
  LPC_SC->PCON |= PCON_BORD;
  LPC_SC->PCON |= PCON_BODRPM | PCON_BOGD;

  instance = NULL;
}
#endif
/*----------------------------------------------------------------------------*/
static void bodEnable(void *object)
{
  struct Bod * const bod = object;

  bod->enabled = true;

  if (bod->callback != NULL)
  {
    irqClearPending(BOD_IRQ);
    irqEnable(BOD_IRQ);
  }
}
/*----------------------------------------------------------------------------*/
static void bodDisable(void *object)
{
  struct Bod * const bod = object;

  bod->enabled = false;
  irqDisable(BOD_IRQ);
}
/*----------------------------------------------------------------------------*/
static void bodSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Bod * const bod = object;

  bod->callbackArgument = argument;
  bod->callback = callback;

  if (bod->enabled && bod->callback != NULL)
  {
    irqClearPending(BOD_IRQ);
    irqEnable(BOD_IRQ);
  }
  else
    irqDisable(BOD_IRQ);
}
