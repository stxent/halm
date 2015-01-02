#Copyright (C) 2014 xent
#Project is distributed under the terms of the GNU General Public License v3.0

CSOURCES += core/$(CORE_TYPE)/$(CORE)/irq.c
CSOURCES += core/$(CORE_TYPE)/$(CORE)/memory.c
CSOURCES += core/$(CORE_TYPE)/$(CORE)/spinlock.c

ifeq ($(CONFIG_PM),y)
  CSOURCES += core/$(CORE_TYPE)/pm.c
endif

ifeq ($(CONFIG_CORE_CORTEX_SYSTICK),y)
  CSOURCES += core/$(CORE_TYPE)/systick.c
endif
