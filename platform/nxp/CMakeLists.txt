# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

string(TOLOWER ${PLATFORM} CMAKE_SYSTEM_SOC)

list(APPEND SOURCE_FILES "../pin.c")
list(APPEND SOURCE_FILES "startup.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/clocking.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/pin.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/startup.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/system.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/vectors.c")

if(${CONFIG_PM})
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/pm.c")
endif()

if(${CONFIG_PLATFORM_NXP_ADC_BASE})
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/adc_base.c")
endif()

if(${CONFIG_PLATFORM_NXP_BOD})
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/bod.c")
endif()

if(${CONFIG_PLATFORM_NXP_CAN})
    list(APPEND SOURCE_FILES "${CONFIG_GEN_CAN}/can.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/can_base.c")
endif()

if(${CONFIG_PLATFORM_NXP_DAC_BASE})
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/dac_base.c")
endif()

if(${CONFIG_PLATFORM_NXP_EEPROM})
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/eeprom.c")
endif()

if(${CONFIG_PLATFORM_NXP_EMC})
    list(APPEND SOURCE_FILES "emc_sram.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/emc_base.c")
endif()

if(${CONFIG_PLATFORM_NXP_FAST_GPIO_BUS})
    list(APPEND SOURCE_FILES "fast_gpio_bus_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/fast_gpio_bus.c")
endif()

if(${CONFIG_PLATFORM_NXP_GPDMA})
    list(APPEND SOURCE_FILES "gpdma_circular.c")
    list(APPEND SOURCE_FILES "gpdma_list.c")
    list(APPEND SOURCE_FILES "gpdma_oneshot.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/gpdma_base.c")
endif()

if(${CONFIG_PLATFORM_NXP_GPPWM})
    list(APPEND SOURCE_FILES "gppwm.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/gppwm_base.c")
endif()

if(${CONFIG_PLATFORM_NXP_GPTIMER_BASE})
    list(APPEND SOURCE_FILES "gptimer_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/gptimer_base.c")
endif()

if(${CONFIG_PLATFORM_NXP_GPTIMER_CAPTURE})
    list(APPEND SOURCE_FILES "gptimer_capture.c")
endif()

if(${CONFIG_PLATFORM_NXP_GPTIMER_COUNTER})
    list(APPEND SOURCE_FILES "gptimer_counter.c")
endif()

if(${CONFIG_PLATFORM_NXP_GPTIMER_PWM})
    list(APPEND SOURCE_FILES "gptimer_pwm.c")
endif()

if(${CONFIG_PLATFORM_NXP_I2C_BASE})
    list(APPEND SOURCE_FILES "${CONFIG_GEN_I2C}/i2c_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/i2c_base.c")
endif()

if(${CONFIG_PLATFORM_NXP_I2S_BASE})
    list(APPEND SOURCE_FILES "i2s_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/i2s_base.c")
endif()

if(${CONFIG_PLATFORM_NXP_IAP})
    list(APPEND SOURCE_FILES "iap.c")
endif()

if(${CONFIG_PLATFORM_NXP_IAP_FLASH})
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/flash.c")
endif()

if(${CONFIG_PLATFORM_NXP_PININT})
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/pin_interrupt.c")
endif()

if(${CONFIG_PLATFORM_NXP_SDMMC})
    list(APPEND SOURCE_FILES "dma_sdmmc.c")
    list(APPEND SOURCE_FILES "sdmmc.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/sdmmc_base.c")
endif()

if(${CONFIG_PLATFORM_NXP_SCT_BASE})
    list(APPEND SOURCE_FILES "sct_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/sct_base.c")
endif()

if(${CONFIG_PLATFORM_NXP_SSP_BASE})
    list(APPEND SOURCE_FILES "ssp_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/ssp_base.c")
endif()

if(${CONFIG_PLATFORM_NXP_UART_BASE})
    list(APPEND SOURCE_FILES "${CONFIG_GEN_UART}/uart_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/uart_base.c")
endif()

if(${CONFIG_PLATFORM_NXP_WAKEUPINT})
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/wakeup_interrupt.c")
endif()

if(${CONFIG_PLATFORM_NXP_WDT})
    list(APPEND SOURCE_FILES "wdt.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/wdt_base.c")
endif()

if(${CONFIG_PLATFORM_NXP_ADC_BUS})
    list(APPEND SOURCE_FILES "${CONFIG_GEN_ADC}/adc_bus.c")
endif()

if(${CONFIG_PLATFORM_NXP_ADC_DMA})
    list(APPEND SOURCE_FILES "${CONFIG_GEN_ADC}/adc_dma.c")
endif()

if(${CONFIG_PLATFORM_NXP_ADC_ONESHOT})
    list(APPEND SOURCE_FILES "${CONFIG_GEN_ADC}/adc_oneshot.c")
endif()

if(${CONFIG_PLATFORM_NXP_DAC})
    list(APPEND SOURCE_FILES "${CONFIG_GEN_DAC}/dac.c")
endif()

if(${CONFIG_PLATFORM_NXP_DAC_DMA})
    list(APPEND SOURCE_FILES "${CONFIG_GEN_DAC}/dac_dma.c")
endif()

if(${CONFIG_PLATFORM_NXP_GPTIMER})
    list(APPEND SOURCE_FILES "gptimer.c")
endif()

if(${CONFIG_PLATFORM_NXP_ONE_WIRE_SSP})
    list(APPEND SOURCE_FILES "one_wire_ssp.c")
endif()

if(${CONFIG_PLATFORM_NXP_ONE_WIRE_UART})
    list(APPEND SOURCE_FILES "${CONFIG_GEN_UART}/one_wire_uart.c")
endif()

if(${CONFIG_PLATFORM_NXP_I2C})
    list(APPEND SOURCE_FILES "${CONFIG_GEN_I2C}/i2c.c")
endif()

if(${CONFIG_PLATFORM_NXP_I2C_SLAVE})
    list(APPEND SOURCE_FILES "${CONFIG_GEN_I2C}/i2c_slave.c")
endif()

if(${CONFIG_PLATFORM_NXP_I2S_DMA})
    list(APPEND SOURCE_FILES "i2s_dma.c")
endif()

if(${CONFIG_PLATFORM_NXP_RIT})
    list(APPEND SOURCE_FILES "rit.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/rit_base.c")
endif()

if(${CONFIG_PLATFORM_NXP_RTC})
    list(APPEND SOURCE_FILES "${CONFIG_GEN_RTC}/rtc.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/rtc_base.c")
endif()

if(${CONFIG_PLATFORM_NXP_SCT_PWM})
    list(APPEND SOURCE_FILES "sct_pwm.c")
endif()

if(${CONFIG_PLATFORM_NXP_SCT_TIMER})
    list(APPEND SOURCE_FILES "sct_timer.c")
endif()

if(${CONFIG_PLATFORM_NXP_SERIAL})
    list(APPEND SOURCE_FILES "${CONFIG_GEN_UART}/serial.c")
endif()

if(${CONFIG_PLATFORM_NXP_SERIAL_POLL})
    list(APPEND SOURCE_FILES "${CONFIG_GEN_UART}/serial_poll.c")
endif()

if(${CONFIG_PLATFORM_NXP_SERIAL_DMA})
    list(APPEND SOURCE_FILES "${CONFIG_GEN_UART}/serial_dma.c")
endif()

if(${CONFIG_PLATFORM_NXP_SPI})
    list(APPEND SOURCE_FILES "${CONFIG_GEN_SPI}/spi.c")
endif()

if(${CONFIG_PLATFORM_NXP_SPI_DMA})
    list(APPEND SOURCE_FILES "${CONFIG_GEN_SPI}/spi_dma.c")
endif()

if(${CONFIG_PLATFORM_USB})
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/usb_base.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/usb_device.c")
endif()

add_library(halm_platform OBJECT ${SOURCE_FILES})
