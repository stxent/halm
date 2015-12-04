#Copyright (C) 2014 xent
#Project is distributed under the terms of the GNU General Public License v3.0

PROJECT := halm
PROJECT_DIR := $(shell pwd)

CONFIG_FILE ?= .config
CROSS_COMPILE ?= arm-none-eabi-

-include $(CONFIG_FILE)
BUILD_FLAGS += CORE CORE_TYPE PLATFORM PLATFORM_TYPE

#Nested makefiles
include core/makefile
include platform/makefile
include pm/makefile
include usb/makefile

#Expand build flags
define process-flag
  $(1) := $$(CONFIG_$(1):"%"=%)
endef

$(foreach entry,$(BUILD_FLAGS),$(eval $(call process-flag,$(entry))))

#Process build flags
ifeq ($(CORE_TYPE),cortex)
  AR := $(CROSS_COMPILE)ar
  CC := $(CROSS_COMPILE)gcc
  CPU_FLAGS += -mcpu=cortex-$(CORE) -mthumb
  CPU_FLAGS += -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections
  ifeq ($(CONFIG_FPU),y)
    CPU_FLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
  endif
else ifneq ($(MAKECMDGOALS),menuconfig)
  $(error Target architecture is undefined)
endif

ifneq ($(CONFIG_ASSERTIONS),y)
  OPT_FLAGS += -DNDEBUG
endif

ifeq ($(CONFIG_OPTIMIZATIONS),"full")
  OPT_FLAGS += -O3
else ifeq ($(CONFIG_OPTIMIZATIONS),"size")
  OPT_FLAGS += -Os
else ifeq ($(CONFIG_OPTIMIZATIONS),"none")
  OPT_FLAGS += -O0 -g3
else ifeq ($(CONFIG_OPTIMIZATIONS),"debug")
  OPT_FLAGS += -Og -g3
else
  OPT_FLAGS += $(CONFIG_OPTIMIZATIONS)
endif

#Configure common paths and libraries
INCLUDE_PATH += -Iinclude
OUTPUT_DIR := build_$(PLATFORM)
OPTION_FILE := $(OUTPUT_DIR)/.options

#External libraries
XCORE_PATH ?= $(PROJECT_DIR)/../xcore
INCLUDE_PATH += -I"$(XCORE_PATH)/include"

#Configure compiler options
CFLAGS += -std=c11 -Wall -Wextra -Winline -pedantic -Wshadow
CFLAGS += $(OPT_FLAGS) $(CPU_FLAGS) @$(OPTION_FILE)
CFLAGS += -D$(shell echo $(PLATFORM) | tr a-z A-Z)

#Process auxiliary project options
define append-flag
  ifeq ($$($(1)),y)
    OPTION_STRING += -D$(1)
  else ifneq ($$($(1)),)
    OPTION_STRING += -D$(1)=$$($(1))
  endif
endef

$(foreach entry,$(PROJECT_FLAGS),$(eval $(call append-flag,$(entry))))

#Configure targets
LIBRARY_FILE += $(OUTPUT_DIR)/lib$(PROJECT).a
TARGETS += $(LIBRARY_FILE)

COBJECTS = $(CSOURCES:%.c=$(OUTPUT_DIR)/%.o)

#Define default targets
.PHONY: all clean menuconfig
.SUFFIXES:
.DEFAULT_GOAL = all

all: $(TARGETS)

$(LIBRARY_FILE): $(OPTION_FILE) $(COBJECTS)
	$(AR) -r $@ $(filter-out $(OPTION_FILE),$^)

$(OUTPUT_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(INCLUDE_PATH) -MMD -MF $(@:%.o=%.d) -MT $@ $< -o $@

$(OPTION_FILE): $(CONFIG_FILE)
	@mkdir -p $(@D)
	@echo '$(OPTION_STRING)' > $@

clean:
	rm -f $(COBJECTS:%.o=%.d) $(COBJECTS)
	rm -f $(TARGETS)
	rm -f $(OPTION_FILE)

menuconfig:
	kconfig-mconf kconfig

ifneq ($(MAKECMDGOALS),clean)
  -include $(COBJECTS:%.o=%.d)
endif
