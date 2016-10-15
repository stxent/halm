#Copyright (C) 2016 xent
#Project is distributed under the terms of the GNU General Public License v3.0

ifeq ($(CONFIG_GENERIC_GPIO_BUS),y)
  CSOURCES += generic/gpio_bus.c
endif

ifeq ($(CONFIG_GENERIC_SDIO_SPI),y)
  CSOURCES += generic/sdio_spi.c

  PROJECT_FLAGS += CONFIG_GENERIC_SDIO_SPI_CRC
endif

ifeq ($(CONFIG_GENERIC_SDCARD),y)
  CSOURCES += generic/sdcard.c
endif

ifeq ($(CONFIG_GENERIC_SOFTWARE_TIMER),y)
  CSOURCES += generic/software_timer.c
endif

ifeq ($(CONFIG_GENERIC_WORK_QUEUE),y)
  CSOURCES += generic/work_queue.c
endif
