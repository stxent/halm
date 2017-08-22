#Copyright (C) 2017 xent
#Project is distributed under the terms of the GNU General Public License v3.0

ifeq ($(CONFIG_PLATFORM_LINUX_CONSOLE),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/console.c
endif

ifeq ($(CONFIG_PLATFORM_LINUX_MMF),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/mmf.c
endif

ifeq ($(CONFIG_PLATFORM_LINUX_RTC),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/rtc.c
endif

ifeq ($(CONFIG_PLATFORM_LINUX_SERIAL),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/serial.c
endif

ifeq ($(CONFIG_PLATFORM_LINUX_UDP),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/udp.c
endif
