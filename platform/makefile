#Copyright (C) 2014 xent
#Project is distributed under the terms of the GNU General Public License v3.0

ifeq ($(CONFIG_PM),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/pm.c
  FLAG_NAMES += CONFIG_PLATFORM_NXP_PM_PD CONFIG_PLATFORM_NXP_PM_DPD
endif

CSOURCES += platform/pin.c
CSOURCES += platform/$(PLATFORM_TYPE)/startup.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/clocking.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/pin.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/system.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/vectors.c

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

ifeq ($(CONFIG_PLATFORM_NXP_SDMMC),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/dma_sdmmc.c
  CSOURCES += platform/$(PLATFORM_TYPE)/sdmmc.c
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

ifeq ($(CONFIG_PLATFORM_NXP_WDT),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/wdt.c
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/wdt_base.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_ADC),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GENERATION)/adc.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_ADC_BURST),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GENERATION)/adc_burst.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_ADC_DMA),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GENERATION)/adc_dma.c
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

ifeq ($(CONFIG_PLATFORM_NXP_SDIO_SPI),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/sdio_spi.c
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

ifeq ($(CONFIG_PLATFORM_SDCARD),y)
  CSOURCES += platform/sdcard.c
endif
