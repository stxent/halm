#Copyright (C) 2014 xent
#Project is distributed under the terms of the GNU General Public License v3.0

PROJECT := halm
PROJECTDIR := $(shell pwd)

CONFIG_FILE ?= .config
CROSS_COMPILE ?= arm-none-eabi-

-include $(CONFIG_FILE)
OPTION_NAMES += CORE CORE_TYPE PLATFORM PLATFORM_TYPE

#Nested makefiles
include core/makefile
include platform/makefile
include pm/makefile
include usb/makefile

#Process configuration options
define process-option
  $(1) := $$(CONFIG_$(1):"%"=%)
endef

$(foreach entry,$(OPTION_NAMES),$(eval $(call process-option,$(entry))))

#Determine build flags
ifeq ($(CORE_TYPE),cortex)
  AR := $(CROSS_COMPILE)ar
  CC := $(CROSS_COMPILE)gcc
  CPU_FLAGS += -mcpu=cortex-$(CORE) -mthumb
  CPU_FLAGS += -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections
else ifneq ($(MAKECMDGOALS),menuconfig)
  $(error Target architecture is undefined)
endif

ifeq ($(CONFIG_OPTIMIZATIONS),"full")
  OPT_FLAGS := -O3 -DNDEBUG
else ifeq ($(CONFIG_OPTIMIZATIONS),"size")
  OPT_FLAGS := -Os -DNDEBUG
else ifeq ($(CONFIG_OPTIMIZATIONS),"none")
  OPT_FLAGS := -O0 -g3
else
  OPT_FLAGS := $(CONFIG_OPTIMIZATIONS)
endif

#Configure common paths and libraries
INCLUDEPATH += -Iinclude
OUTPUTDIR = build_$(PLATFORM)

#External libraries
XCORE_PATH ?= $(PROJECTDIR)/../xcore
INCLUDEPATH += -I"$(XCORE_PATH)/include"

#Configure compiler options
CFLAGS += -std=c11 -Wall -Wextra -Winline -pedantic -Wshadow
CFLAGS += $(OPT_FLAGS) $(CPU_FLAGS) $(CONFIG_FLAGS)
CFLAGS += -D$(shell echo $(PLATFORM) | tr a-z A-Z)

#Process project options
define append-flag
  ifeq ($$($(1)),y)
    CONFIG_FLAGS += -D$(1)
  else ifneq ($$($(1)),)
    CONFIG_FLAGS += -D$(1)=$$($(1))
  endif
endef

$(foreach entry,$(FLAG_NAMES),$(eval $(call append-flag,$(entry))))

#Configure targets
LIBRARY_FILE += $(OUTPUTDIR)/lib$(PROJECT).a
TARGETS += $(LIBRARY_FILE)

COBJECTS = $(CSOURCES:%.c=$(OUTPUTDIR)/%.o)

#Define default targets
.PHONY: all clean menuconfig
.SUFFIXES:
.DEFAULT_GOAL = all

all: $(TARGETS)

$(LIBRARY_FILE): $(COBJECTS)
	$(AR) -r $@ $^

$(OUTPUTDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(INCLUDEPATH) -MMD -MF $(@:%.o=%.d) -MT $@ $< -o $@

clean:
	rm -f $(COBJECTS:%.o=%.d) $(COBJECTS)
	rm -f $(TARGETS)

menuconfig:
	kconfig-mconf kconfig

ifneq ($(MAKECMDGOALS),clean)
  -include $(COBJECTS:%.o=%.d)
endif
