PROJECT = hal
PROJECTDIR = .

ifneq ($(CONFIGFILE),)
	include $(CONFIGFILE)
else
	include .config
endif

ifeq ($(CROSS_COMPILE),)
	CROSS_COMPILE = arm-none-eabi-
endif

AR = $(CROSS_COMPILE)ar
CC = $(CROSS_COMPILE)gcc

ifeq ($(CONFIG_LPC11XX),y)
	CHIP = LPC11XX
	CORE_TYPE = cortex
	CORE = m0
	PLATFORM_TYPE = nxp
	PLATFORM = lpc11xx
endif

ifeq ($(CONFIG_LPC11EXX),y)
	CHIP = LPC11EXX
	CORE_TYPE = cortex
	CORE = m0
	PLATFORM_TYPE = nxp
	PLATFORM = lpc11exx
endif

ifeq ($(CONFIG_LPC13XX),y)
	CHIP = LPC13XX
	CORE_TYPE = cortex
	CORE = m3
	PLATFORM_TYPE = nxp
	PLATFORM = lpc13xx
endif

ifeq ($(CONFIG_LPC17XX),y)
	CHIP = LPC17XX
	CORE_TYPE = cortex
	CORE = m3
	PLATFORM_TYPE = nxp
	PLATFORM = lpc17xx
endif

ifeq ($(CONFIG_LPC43XX),y)
	CHIP = LPC43XX
	CORE_TYPE = cortex
	CORE = m4
	PLATFORM_TYPE = nxp
	PLATFORM = lpc43xx
	PLATFORM_FLAGS += -D__MULTICORE_NONE
endif

ifeq ($(CONFIG_OPTIMIZE_NONE),y)
	OPTIMIZE_FLAGS = -O0 -g3
endif

ifeq ($(CONFIG_OPTIMIZE_SIZE),y)
	OPTIMIZE_FLAGS = -Os
endif

ifeq ($(CONFIG_OPTIMIZE_ALL),y)
	OPTIMIZE_FLAGS = -O3
endif

INCLUDEPATH += -I./
INCLUDEPATH += $(addprefix -I$(PROJECTDIR)/, include include/common)

CFLAGS += -std=c11 -Wall -Wextra -Winline -Wcast-qual -pedantic -Wshadow
CFLAGS += -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections
CFLAGS += $(OPTIMIZE_FLAGS) -mcpu=$(CORE_TYPE)-$(CORE) -mthumb
CFLAGS += -specs=redlib.specs -D__REDLIB__ $(PLATFORM_FLAGS) -D$(CHIP)

#Common modules
CSOURCES_COMMON := $(shell find common -name "*.c")
CSOURCES += $(CSOURCES_COMMON)

#Permanent core modules
CSOURCES += core/$(CORE_TYPE)/$(CORE)/irq.c
CSOURCES += core/$(CORE_TYPE)/$(CORE)/memory.c
CSOURCES += core/$(CORE_TYPE)/$(CORE)/spinlock.c

ifeq ($(CONFIG_CORE_CORTEX_PM),y)
	CSOURCES += core/$(CORE_TYPE)/pm.c
	CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/pm.c
	CSOURCES += pm/pm.c
endif

ifeq ($(CONFIG_CORE_CORTEX_SYSTICK),y)
	CSOURCES += core/$(CORE_TYPE)/systick.c
endif

#Permanent platform modules
CSOURCES += platform/pin.c
CSOURCES += platform/$(PLATFORM_TYPE)/startup.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/clocking.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/pin.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/vectors.c

ifeq ($(CONFIG_PLATFORM_NXP_UART),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/uart_common.c
	CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/uart_base.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_SERIAL),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/serial.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_SERIAL_POLL),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/serial_poll.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_SERIAL_DMA),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/serial_dma.c
endif

OUTPUTDIR = build_$(PLATFORM)
COBJECTS = $(addprefix $(OUTPUTDIR)/, $(CSOURCES:%.c=%.o))
PROJECTFILE = $(OUTPUTDIR)/lib$(PROJECT).a

.PHONY: all clean menuconfig
.SUFFIXES:

all: $(PROJECTFILE)

$(PROJECTFILE): $(COBJECTS)
	$(AR) -r $@ $(COBJECTS)

$(OUTPUTDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(INCLUDEPATH) -MMD -MF "$(@:%.o=%.d)" -MT $@ $< -o $@

clean:
	rm -f $(COBJECTS:%.o=%.d)
	rm -f $(COBJECTS)
	rm -f $(PROJECTFILE)

menuconfig:
	kconfig-mconf kconfig

-include $(COBJECTS:%.o=%.d)
