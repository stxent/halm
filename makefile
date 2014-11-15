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
	CORE = m0
	CORE_TYPE = cortex
	GENERATION = gen_1
	PLATFORM = lpc11xx
	PLATFORM_TYPE = nxp
endif

ifeq ($(CONFIG_LPC11EXX),y)
	CHIP = LPC11EXX
	CORE = m0
	CORE_TYPE = cortex
	GENERATION = gen_1
	PLATFORM = lpc11exx
	PLATFORM_TYPE = nxp
endif

ifeq ($(CONFIG_LPC13XX),y)
	CHIP = LPC13XX
	CORE = m3
	CORE_TYPE = cortex
	GENERATION = gen_1
	PLATFORM = lpc13xx
	PLATFORM_TYPE = nxp
endif

ifeq ($(CONFIG_LPC17XX),y)
	CHIP = LPC17XX
	CORE = m3
	CORE_TYPE = cortex
	GENERATION = gen_1
	PLATFORM = lpc17xx
	PLATFORM_TYPE = nxp

	#Platform-specific options
	PLATFORM_SYSTEM_MODULE = y
endif

ifeq ($(CONFIG_LPC43XX),y)
	CHIP = LPC43XX
	CORE = m4
	CORE_TYPE = cortex
	GENERATION = gen_1
	PLATFORM = lpc43xx
	PLATFORM_TYPE = nxp

	#Platform-specific options
	PLATFORM_FLAGS += -D__MULTICORE_NONE
	PLATFORM_SYSTEM_MODULE = y
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

ifeq ($(CONFIG_PM),y)
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

ifeq ($(PLATFORM_SYSTEM_MODULE),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/system.c
endif

ifeq ($(CONFIG_PLATFORM_GPIO_BUS),y)
	CSOURCES += platform/gpio_bus.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_ADC_BASE),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/$(GENERATION)/adc_unit.c
	CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/adc_base.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_DAC_BASE),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/dac_base.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_GPDMA),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/gpdma.c
	CSOURCES += platform/$(PLATFORM_TYPE)/gpdma_list.c
	CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/gpdma_base.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_GPPWM),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/gppwm.c
	CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/gppwm_base.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_GPTIMER_BASE),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/gptimer_common.c
	CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/gptimer_base.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_GPTIMER_CAPTURE),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/gptimer_capture.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_GPTIMER_PWM),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/gptimer_pwm.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_I2C_BASE),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/$(GENERATION)/i2c_common.c
	CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/i2c_base.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_PIN_INTERRUPT),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/pin_interrupt.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_SDMMC_BASE),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/sdmmc_base.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_SCT_BASE),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/sct_base.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_SSP_BASE),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/ssp_common.c
	CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/ssp_base.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_UART_BASE),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/$(GENERATION)/uart_common.c
	CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/uart_base.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_ADC),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/$(GENERATION)/adc.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_ADC_BURST),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/$(GENERATION)/adc_burst.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_DAC),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/$(GENERATION)/dac.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_DAC_DMA),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/$(GENERATION)/dac_dma.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_GPTIMER),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/gptimer.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_ONE_WIRE_SSP),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/one_wire_ssp.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_ONE_WIRE_UART),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/$(GENERATION)/one_wire_uart.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_I2C),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/$(GENERATION)/i2c.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_I2C_SLAVE),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/$(GENERATION)/i2c_slave.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_SCT_TIMER),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/sct_timer.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_SERIAL),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/$(GENERATION)/serial.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_SERIAL_POLL),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/$(GENERATION)/serial_poll.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_SERIAL_DMA),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/$(GENERATION)/serial_dma.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_SPI),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/$(GENERATION)/spi.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_SPI_DMA),y)
	CSOURCES += platform/$(PLATFORM_TYPE)/$(GENERATION)/spi_dma.c
endif

ifeq ($(CONFIG_PLATFORM_SDIO_SPI),y)
	CSOURCES += platform/sdio_spi.c
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
