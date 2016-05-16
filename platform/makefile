#Copyright (C) 2016 xent
#Project is distributed under the terms of the GNU General Public License v3.0

ifeq ($(CONFIG_PLATFORM_GPIO_BUS),y)
  CSOURCES += platform/gpio_bus.c
endif

ifeq ($(CONFIG_PLATFORM_SDIO_SPI),y)
  CSOURCES += platform/sdio_spi.c
endif

ifeq ($(CONFIG_PLATFORM_SDCARD),y)
  CSOURCES += platform/sdcard.c
endif

include platform/nxp/makefile
