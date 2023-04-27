/*
 * ethernet_mdio.c
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/lpc43xx/ethernet.h>
#include <halm/platform/lpc/lpc43xx/ethernet_defs.h>
#include <halm/platform/lpc/lpc43xx/ethernet_mdio.h>
#include <halm/platform/platform_defs.h>
#include <assert.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static uint32_t frequencyToClockRange(uint32_t);
/*----------------------------------------------------------------------------*/
static enum Result mdioInit(void *, const void *);
static enum Result mdioGetParam(void *, int, void *);
static enum Result mdioSetParam(void *, int, const void *);
static size_t mdioRead(void *, void *, size_t);
static size_t mdioWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const MDIO = &(const struct InterfaceClass){
    .size = sizeof(struct MDIO),
    .init = mdioInit,
    .deinit = NULL, /* Default destructor */

    .setCallback = NULL,
    .getParam = mdioGetParam,
    .setParam = mdioSetParam,
    .read = mdioRead,
    .write = mdioWrite
};
/*----------------------------------------------------------------------------*/
static uint32_t frequencyToClockRange(uint32_t frequency)
{
  if (frequency <= 35000000)
  {
    /* 20 - 35 MHz */
    return CR_CLOCK_DIV_16;
  }
  else if (frequency <= 60000000)
  {
    /* 35 - 60 MHz */
    return CR_CLOCK_DIV_26;
  }
  else if (frequency <= 100000000)
  {
    /* 60 - 100 MHz */
    return CR_CLOCK_DIV_42;
  }
  else if (frequency <= 150000000)
  {
    /* 100 - 150 MHz */
    return CR_CLOCK_DIV_62;
  }
  else if (frequency <= 250000000)
  {
    /* 150 - 250 MHz */
    return CR_CLOCK_DIV_102;
  }
  else
  {
    /* 250 - 300 MHz */
    return CR_CLOCK_DIV_124;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result mdioInit(void *object, const void *configBase)
{
  const struct MDIOConfig * const config = configBase;
  assert(config != NULL);

  struct MDIO * const interface = object;

  interface->parent = config->parent;
  interface->address = 0;
  interface->position = 0;

  const uint32_t frequency = clockFrequency(MainClock);
  LPC_ETHERNET_Type * const reg = interface->parent->base.reg;

  reg->MAC_MII_ADDR = MAC_MII_ADDR_CR(frequencyToClockRange(frequency));

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result mdioGetParam(void *object, int parameter, void *data)
{
  const struct MDIO * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_ADDRESS:
      *(uint32_t *)data = (uint32_t)interface->address;
      return E_OK;

    case IF_POSITION:
      *(uint32_t *)data = (uint32_t)interface->position;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result mdioSetParam(void *object, int parameter, const void *data)
{
  struct MDIO * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_ADDRESS:
      interface->address = (uint8_t)(*(const uint32_t *)data);
      return E_OK;

    case IF_POSITION:
      interface->position = (uint8_t)(*(const uint32_t *)data);
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t mdioRead(void *object, void *buffer, size_t length)
{
  static const uint32_t ADDR_MASK = MAC_MII_ADDR_GR_MASK | MAC_MII_ADDR_PA_MASK
      | MAC_MII_ADDR_W;

  if (length < sizeof(uint16_t))
    return 0;

  struct MDIO * const interface = object;
  LPC_ETHERNET_Type * const reg = interface->parent->base.reg;

  reg->MAC_MII_ADDR = (reg->MAC_MII_ADDR & ~ADDR_MASK)
      | MAC_MII_ADDR_PA(interface->address)
      | MAC_MII_ADDR_GR(interface->position);

  /* Start the read cycle */
  reg->MAC_MII_ADDR |= MAC_MII_ADDR_GB;

  /* Wait until operation completed */
  while (reg->MAC_MII_ADDR & MAC_MII_ADDR_GB);

  const uint16_t value = reg->MAC_MII_DATA;
  memcpy(buffer, &value, sizeof(value));

  return sizeof(uint16_t);
}
/*----------------------------------------------------------------------------*/
static size_t mdioWrite(void *object, const void *buffer, size_t length)
{
  static const uint32_t ADDR_MASK = MAC_MII_ADDR_GR_MASK | MAC_MII_ADDR_PA_MASK;

  if (length < sizeof(uint16_t))
    return 0;

  struct MDIO * const interface = object;
  LPC_ETHERNET_Type * const reg = interface->parent->base.reg;

  reg->MAC_MII_ADDR = (reg->MAC_MII_ADDR & ~ADDR_MASK)
      | MAC_MII_ADDR_PA(interface->address)
      | MAC_MII_ADDR_GR(interface->position)
      | MAC_MII_ADDR_W;

  uint16_t value;

  /* Prepare data and start the write cycle */
  memcpy(&value, buffer, sizeof(value));
  reg->MAC_MII_DATA = value;
  reg->MAC_MII_ADDR |= MAC_MII_ADDR_GB;

  /* Wait until operation completed */
  while (reg->MAC_MII_ADDR & MAC_MII_ADDR_GB);

  return sizeof(uint16_t);
}
