#Copyright (C) 2014 xent
#Project is distributed under the terms of the GNU General Public License v3.0

GEN_ADC := $(patsubst "%",%,$(CONFIG_GEN_ADC))
GEN_DAC := $(patsubst "%",%,$(CONFIG_GEN_DAC))
GEN_I2C := $(patsubst "%",%,$(CONFIG_GEN_I2C))
GEN_RTC := $(patsubst "%",%,$(CONFIG_GEN_RTC))
GEN_SPI := $(patsubst "%",%,$(CONFIG_GEN_SPI))
GEN_UART := $(patsubst "%",%,$(CONFIG_GEN_UART))
GEN_USB := $(patsubst "%",%,$(CONFIG_GEN_USB))

ifeq ($(CONFIG_PM),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/pm.c

  FLAG_NAMES += CONFIG_PLATFORM_NXP_PM_PD CONFIG_PLATFORM_NXP_PM_DPD
  FLAG_NAMES += CONFIG_PLATFORM_NXP_GPTIMER_PM
  FLAG_NAMES += CONFIG_PLATFORM_NXP_I2C_PM
  FLAG_NAMES += CONFIG_PLATFORM_NXP_SERIAL_PM
endif

ifeq ($(CONFIG_USB),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_USB)/usb_device_base.c
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/usb_base.c
endif

CSOURCES += platform/pin.c
CSOURCES += platform/$(PLATFORM_TYPE)/startup.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/clocking.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/pin.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/startup.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/system.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/vectors.c

ifeq ($(CONFIG_PLATFORM_GPIO_BUS),y)
  CSOURCES += platform/gpio_bus.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_ADC_BASE),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_ADC)/adc_unit.c
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
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_I2C)/i2c_common.c
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/i2c_base.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_I2S_BASE),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/i2s_common.c
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/i2s_base.c
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
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_UART)/uart_common.c
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/uart_base.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_WAKEUP_INTERRUPT),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/wakeup_interrupt.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_WDT),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/wdt.c
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/wdt_base.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_ADC),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_ADC)/adc.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_ADC_BURST),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_ADC)/adc_burst.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_ADC_DMA),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_ADC)/adc_dma.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_DAC),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_DAC)/dac.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_DAC_DMA),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_DAC)/dac_dma.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_GPTIMER),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/gptimer.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_ONE_WIRE_SSP),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/one_wire_ssp.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_ONE_WIRE_UART),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_UART)/one_wire_uart.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_I2C),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_I2C)/i2c.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_I2C_SLAVE),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_I2C)/i2c_slave.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_I2S),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/i2s_dma.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_RTC),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_RTC)/rtc.c
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/rtc_base.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_SCT_PWM),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/sct_pwm.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_SCT_TIMER),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/sct_timer.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_SDIO_SPI),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/sdio_spi.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_SERIAL),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_UART)/serial.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_SERIAL_POLL),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_UART)/serial_poll.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_SERIAL_DMA),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_UART)/serial_dma.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_SPI),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_SPI)/spi.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_SPI_DMA),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_SPI)/spi_dma.c
endif

ifeq ($(CONFIG_PLATFORM_SDCARD),y)
  CSOURCES += platform/sdcard.c
endif
