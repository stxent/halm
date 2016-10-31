#Copyright (C) 2016 xent
#Project is distributed under the terms of the GNU General Public License v3.0

ifeq ($(CONFIG_PM),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/pm.c

  PROJECT_FLAGS += CONFIG_PLATFORM_STM_PM_STOP CONFIG_PLATFORM_STM_PM_STANDBY
endif
