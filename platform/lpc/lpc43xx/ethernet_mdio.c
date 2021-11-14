/*
 * ethernet_mdio.c
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/lpc43xx/ethernet.h>
#include <halm/platform/lpc/lpc43xx/ethernet_defs.h>
#include <halm/platform/lpc/lpc43xx/ethernet_mdio.h>
#include <halm/platform/platform_defs.h>
#include <assert.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static enum Result mdioInit(void *, const void *);
static void mdioDeinit(void *);
static void mdioSetCallback(void *, void (*)(void *), void *);
static enum Result mdioGetParam(void *, int, void *);
static enum Result mdioSetParam(void *, int, const void *);
static size_t mdioRead(void *, void *, size_t);
static size_t mdioWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const MDIO = &(const struct InterfaceClass){
    .size = sizeof(struct MDIO),
    .init = mdioInit,
    .deinit = mdioDeinit,

    .setCallback = mdioSetCallback,
    .getParam = mdioGetParam,
    .setParam = mdioSetParam,
    .read = mdioRead,
    .write = mdioWrite
};
/*----------------------------------------------------------------------------*/
static enum Result mdioInit(void *object, const void *configBase)
{
  const struct MDIOConfig * const config = configBase;
  assert(config);

  struct MDIO * const interface = object;

  interface->parent = config->parent;
  interface->address = 0;
  interface->position = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void mdioDeinit(void *object __attribute__((unused)))
{
}
/*----------------------------------------------------------------------------*/
// TODO Update template
static void mdioSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  (void)object;
  (void)callback;
  (void)argument;
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
  (void)length;

  static const uint32_t MII_RD_TOUT = 0x00050000;

  struct MDIO * const interface = object;
  LPC_ETHERNET_Type * const reg = interface->parent->base.reg;
  unsigned int tout;
  uint16_t val;

  while (reg->MAC_MII_ADDR & MAC_MII_ADDR_GB); // Check GMII busy bit
  reg->MAC_MII_ADDR = MAC_MII_ADDR_PA(interface->address)
      | MAC_MII_ADDR_GR(interface->position);
  reg->MAC_MII_ADDR |= MAC_MII_ADDR_GB; // Start PHY Read Cycle

  /* Wait until operation completed */
  for (tout = 0; tout < MII_RD_TOUT; tout++) {
    if ((reg->MAC_MII_ADDR & MAC_MII_ADDR_GB) == 0) {
       break;
    }
  }

  if (tout == MII_RD_TOUT)
    return 0;

  val = reg->MAC_MII_DATA;
  memcpy(buffer, &val, sizeof(val));

  return sizeof(val);
}
/*----------------------------------------------------------------------------*/
static size_t mdioWrite(void *object, const void *buffer, size_t length)
{
  (void)length;

  static const uint32_t MII_WR_TOUT = 0x00050000;

  struct MDIO * const interface = object;
  LPC_ETHERNET_Type * const reg = interface->parent->base.reg;
  unsigned int tout;
  uint16_t val;

  memcpy(&val, buffer, sizeof(val));

  while (reg->MAC_MII_ADDR & MAC_MII_ADDR_GB); // Check GMII busy bit
  reg->MAC_MII_ADDR = MAC_MII_ADDR_PA(interface->address)
      | MAC_MII_ADDR_GR(interface->position)
      | MAC_MII_ADDR_W;
  reg->MAC_MII_DATA = val;
  reg->MAC_MII_ADDR |= MAC_MII_ADDR_GB; // Start PHY Write Cycle

  /* Wait until operation completed */
  for (tout = 0; tout < MII_WR_TOUT; tout++) {
    if ((reg->MAC_MII_ADDR & MAC_MII_ADDR_GB) == 0) {
       break;
    }
  }

  if (tout == MII_WR_TOUT)
    return 0;

  return sizeof(val);
}
