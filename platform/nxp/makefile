#Copyright (C) 2014 xent
#Project is distributed under the terms of the GNU General Public License v3.0

BUILD_FLAGS += GEN_ADC GEN_CAN GEN_DAC GEN_I2C GEN_RTC GEN_SPI GEN_UART

ifeq ($(CONFIG_PM),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/pm.c

  PROJECT_FLAGS += CONFIG_PLATFORM_NXP_PM_DEEPSLEEP CONFIG_PLATFORM_NXP_PM_PD
  PROJECT_FLAGS += CONFIG_PLATFORM_NXP_CAN_PM
  PROJECT_FLAGS += CONFIG_PLATFORM_NXP_GPTIMER_PM
  PROJECT_FLAGS += CONFIG_PLATFORM_NXP_I2C_PM
  PROJECT_FLAGS += CONFIG_PLATFORM_NXP_SSP_PM
  PROJECT_FLAGS += CONFIG_PLATFORM_NXP_UART_PM
endif

CSOURCES += platform/pin.c
CSOURCES += platform/$(PLATFORM_TYPE)/startup.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/clocking.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/pin.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/startup.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/system.c
CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/vectors.c

ifeq ($(CONFIG_PLATFORM_NXP_FAST_GPIO_BUS),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/fast_gpio_bus_common.c
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/fast_gpio_bus.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_ADC_BASE),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_ADC)/adc_unit.c
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/adc_base.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_CAN),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_CAN)/can.c
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/can_base.c

  PROJECT_FLAGS += CONFIG_PLATFORM_NXP_CAN_TSEG1 CONFIG_PLATFORM_NXP_CAN_TSEG2
endif

ifeq ($(CONFIG_PLATFORM_NXP_DAC_BASE),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/dac_base.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_EEPROM),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/eeprom.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_EMC),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/emc_sram.c
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/emc_base.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_GPDMA),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/gpdma.c
  CSOURCES += platform/$(PLATFORM_TYPE)/gpdma_circular.c
  CSOURCES += platform/$(PLATFORM_TYPE)/gpdma_list.c
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/gpdma_base.c

  PROJECT_FLAGS += CONFIG_PLATFORM_NXP_GPDMA_PRIORITY
  PROJECT_FLAGS += CONFIG_PLATFORM_NXP_GPDMA_SYNC
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

ifeq ($(CONFIG_PLATFORM_NXP_GPTIMER_COUNTER),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/gptimer_counter.c
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

ifeq ($(CONFIG_PLATFORM_NXP_IAP),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/iap.c
endif

ifeq ($(CONFIG_PLATFORM_NXP_IAP_FLASH),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/flash.c
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

ifeq ($(CONFIG_PLATFORM_NXP_ADC_BUS),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(GEN_ADC)/adc_bus.c
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

  PROJECT_FLAGS += CONFIG_PLATFORM_NXP_ONE_WIRE_SSP_SEARCH
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

ifeq ($(CONFIG_PLATFORM_NXP_I2S_DMA),y)
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

ifeq ($(CONFIG_PLATFORM_USB),y)
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/usb_base.c
  CSOURCES += platform/$(PLATFORM_TYPE)/$(PLATFORM)/usb_device.c

  PROJECT_FLAGS += CONFIG_PLATFORM_USB_DEVICE_EP_REQUESTS
  PROJECT_FLAGS += CONFIG_PLATFORM_USB_DEVICE_POOL_SIZE

  ifeq ($(CONFIG_PLATFORM_USB_DMA),y)
    PROJECT_FLAGS += CONFIG_PLATFORM_USB_DMA
  endif

  ifeq ($(CONFIG_PLATFORM_USB_HS),y)
    PROJECT_FLAGS += CONFIG_PLATFORM_USB_HS
  endif
endif
