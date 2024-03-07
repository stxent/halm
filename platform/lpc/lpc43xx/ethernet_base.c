/*
 * ethernet_base.c
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/lpc43xx/ethernet_base.h>
#include <halm/platform/lpc/lpc43xx/system_defs.h>
#include <halm/platform/lpc/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
enum
{
  CHANNEL_COL,
  CHANNEL_CRS,
  CHANNEL_MDC,
  CHANNEL_MDIO,
  CHANNEL_RX_CLK,
  CHANNEL_RX_DV,
  CHANNEL_RX_ER,
  CHANNEL_RXD0,
  CHANNEL_RXD1,
  CHANNEL_RXD2,
  CHANNEL_RXD3,
  CHANNEL_TX_CLK,
  CHANNEL_TX_ER,
  CHANNEL_TX_EN,
  CHANNEL_TXD0,
  CHANNEL_TXD1,
  CHANNEL_TXD2,
  CHANNEL_TXD3
};
/*----------------------------------------------------------------------------*/
static bool setInstance(struct EthernetBase *);
/*----------------------------------------------------------------------------*/
static enum Result ethInit(void *, const void *);
static void ethDeinit(void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const EthernetBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = ethInit,
    .deinit = ethDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinEntry ethernetPins[] = {
    {
        .key = PIN(PORT_0, 0), /* ENET_RXD1 */
        .channel = CHANNEL_RXD1,
        .value = 2
    }, {
        .key = PIN(PORT_0, 1), /* ENET_COL */
        .channel = CHANNEL_COL,
        .value = 2
    }, {
        .key = PIN(PORT_0, 1), /* ENET_TX_EN */
        .channel = CHANNEL_TX_EN,
        .value = 6
    }, {
        .key = PIN(PORT_1, 15), /* ENET_RXD0 */
        .channel = CHANNEL_RXD0,
        .value = 3
    }, {
        .key = PIN(PORT_1, 16), /* ENET_CRS */
        .channel = CHANNEL_CRS,
        .value = 3
    }, {
        .key = PIN(PORT_1, 16), /* ENET_RX_DV */
        .channel = CHANNEL_RX_DV,
        .value = 7
    }, {
        .key = PIN(PORT_1, 17), /* ENET_MDIO */
        .channel = CHANNEL_MDIO,
        .value = 3
    }, {
        .key = PIN(PORT_1, 18), /* ENET_TXD0 */
        .channel = CHANNEL_TXD0,
        .value = 3
    }, {
        .key = PIN(PORT_1, 19), /* ENET_TX_CLK or ENET_REF_CLK */
        .channel = CHANNEL_TX_CLK,
        .value = 0
    }, {
        .key = PIN(PORT_1, 20), /* ENET_TXD1 */
        .channel = CHANNEL_TXD1,
        .value = 3
    }, {
        .key = PIN(PORT_2, 0), /* ENET_MDC */
        .channel = CHANNEL_MDC,
        .value = 7
    }, {
        .key = PIN(PORT_4, 1), /* ENET_COL */
        .channel = CHANNEL_COL,
        .value = 7
    }, {
        .key = PIN(PORT_7, 7), /* ENET_MDC */
        .channel = CHANNEL_MDC,
        .value = 6
    }, {
        .key = PIN(PORT_9, 0), /* ENET_CRS */
        .channel = CHANNEL_CRS,
        .value = 5
    }, {
        .key = PIN(PORT_9, 1), /* ENET_RX_ER */
        .channel = CHANNEL_RX_ER,
        .value = 5
    }, {
        .key = PIN(PORT_9, 2), /* ENET_RXD3 */
        .channel = CHANNEL_RXD3,
        .value = 5
    }, {
        .key = PIN(PORT_9, 3), /* ENET_RXD2 */
        .channel = CHANNEL_RXD2,
        .value = 5
    }, {
        .key = PIN(PORT_9, 4), /* ENET_TXD2 */
        .channel = CHANNEL_TXD2,
        .value = 5
    }, {
        .key = PIN(PORT_9, 5), /* ENET_TXD3 */
        .channel = CHANNEL_TXD3,
        .value = 5
    }, {
        .key = PIN(PORT_9, 6), /* ENET_COL */
        .channel = CHANNEL_COL,
        .value = 5
    }, {
        .key = PIN(PORT_C, 0), /* ENET_RX_CLK */
        .channel = CHANNEL_RX_CLK,
        .value = 3
    }, {
        .key = PIN(PORT_C, 1), /* ENET_MDC */
        .channel = CHANNEL_MDC,
        .value = 3
    }, {
        .key = PIN(PORT_C, 2), /* ENET_TXD2 */
        .channel = CHANNEL_TXD2,
        .value = 3
    }, {
        .key = PIN(PORT_C, 3), /* ENET_TXD3 */
        .channel = CHANNEL_TXD3,
        .value = 3
    }, {
        .key = PIN(PORT_C, 4), /* ENET_TX_EN */
        .channel = CHANNEL_TX_EN,
        .value = 3
    }, {
        .key = PIN(PORT_C, 5), /* ENET_TX_ER */
        .channel = CHANNEL_TX_ER,
        .value = 3
    }, {
        .key = PIN(PORT_C, 6), /* ENET_RXD2 */
        .channel = CHANNEL_RXD2,
        .value = 3
    }, {
        .key = PIN(PORT_C, 7), /* ENET_RXD3 */
        .channel = CHANNEL_RXD3,
        .value = 3
    }, {
        .key = PIN(PORT_C, 8), /* ENET_RX_DV */
        .channel = CHANNEL_RX_DV,
        .value = 3
    }, {
        .key = PIN(PORT_C, 9), /* ENET_RX_ER */
        .channel = CHANNEL_RX_ER,
        .value = 3
    }, {
        .key = PIN(PORT_C, 14), /* ENET_TX_ER */
        .channel = CHANNEL_TX_ER,
        .value = 6
    }, {
        .key = PIN(PORT_CLK, 0), /* ENET_TX_CLK or ENET_REF_CLK */
        .channel = CHANNEL_TX_CLK,
        .value = 7
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct EthernetBase *instance = NULL;
/*----------------------------------------------------------------------------*/
static bool setInstance(struct EthernetBase *object)
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
void ETHERNET_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
void ethConfigPins(struct EthernetBase *interface,
    const struct EthernetBaseConfig *config)
{
  enum InterfaceType
  {
    INTERFACE_UNDEFINED,
    INTERFACE_RMII,
    INTERFACE_MII
  };

  static const uint32_t MIIM_MINIMAL_MASK =
      1UL << CHANNEL_MDC
      | 1UL << CHANNEL_MDIO;
  static const uint32_t MII_MINIMAL_MASK =
      1UL << CHANNEL_RX_CLK
      | 1UL << CHANNEL_TX_CLK
      | 1UL << CHANNEL_CRS
      | 1UL << CHANNEL_RXD0
      | 1UL << CHANNEL_RXD1
      | 1UL << CHANNEL_RXD2
      | 1UL << CHANNEL_RXD3
      | 1UL << CHANNEL_TX_EN
      | 1UL << CHANNEL_TXD0
      | 1UL << CHANNEL_TXD1
      | 1UL << CHANNEL_TXD2
      | 1UL << CHANNEL_TXD3;
  static const uint32_t RMII_MINIMAL_MASK =
      1UL << CHANNEL_TX_CLK
      | 1UL << CHANNEL_RX_DV
      | 1UL << CHANNEL_RXD0
      | 1UL << CHANNEL_RXD1
      | 1UL << CHANNEL_TX_EN
      | 1UL << CHANNEL_TXD0
      | 1UL << CHANNEL_TXD1;

  const PinNumber pinArray[] = {
      config->col,
      config->crs,
      config->mdc,
      config->mdio,
      config->rxclk,
      config->rxdv,
      config->rxer,
      config->rxd[0],
      config->rxd[1],
      config->rxd[2],
      config->rxd[3],
      config->txclk,
      config->txer,
      config->txen,
      config->txd[0],
      config->txd[1],
      config->txd[2],
      config->txd[3]
  };
  uint32_t used = 0;

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      const struct PinEntry * const pinEntry = pinFind(ethernetPins,
          pinArray[index], (uint8_t)index);
      assert(pinEntry != NULL);

      const struct Pin pin = pinInit(pinArray[index]);

      pinInput(pin);
      pinSetFunction(pin, pinEntry->value);

      used |= 1UL << index;
    }
  }

  enum InterfaceType type = INTERFACE_UNDEFINED;

  if ((used & MII_MINIMAL_MASK) == MII_MINIMAL_MASK)
    type = INTERFACE_MII;
  else if ((used & RMII_MINIMAL_MASK) == RMII_MINIMAL_MASK)
    type = INTERFACE_RMII;

  assert(type != INTERFACE_UNDEFINED);

  interface->miim = ((used & MIIM_MINIMAL_MASK) == MIIM_MINIMAL_MASK);
  interface->rmii = type == INTERFACE_RMII;
}
/*----------------------------------------------------------------------------*/
static enum Result ethInit(void *object, const void *configBase)
{
  const struct EthernetBaseConfig * const config = configBase;
  struct EthernetBase * const interface = object;

  if (!setInstance(interface))
    return E_BUSY;

  interface->handler = NULL;
  interface->irq = ETHERNET_IRQ;
  interface->reg = LPC_ETHERNET;

  /* Configure input and output pins */
  ethConfigPins(interface, config);

  /* Enable clocks to the register interface */
  sysClockEnable(CLK_M4_ETHERNET);
  /* Reset registers to default values */
  sysResetEnable(RST_ETHERNET);

  uint32_t creg6 = LPC_CREG->CREG6 & ~CREG6_ETHMODE_MASK;

  if (interface->rmii)
    creg6 |= CREG6_ETHMODE(CREG6_ETHMODE_RMII);
  else
    creg6 |= CREG6_ETHMODE(CREG6_ETHMODE_MII);

  LPC_CREG->CREG6 = creg6;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void ethDeinit([[maybe_unused]] void *object)
{
  sysClockDisable(CLK_M4_ETHERNET);
  instance = NULL;
}
