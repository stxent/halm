#Copyright (C) 2016 xent
#Project is distributed under the terms of the GNU General Public License v3.0

CSOURCES += platform/pin.c
CSOURCES += platform/$(PLATFORM_TYPE)/startup.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/clocking.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/pin.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/startup.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/system.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/vectors.c

ifeq ($(CONFIG_PM),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/pm.c
endif

ifeq ($(CONFIG_PLATFORM_STM_UART_BASE),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/uart_common.c
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/uart_base.c
endif

ifeq ($(CONFIG_PLATFORM_STM_SERIAL),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/serial.c
endif
