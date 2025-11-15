# Copyright (C) 2017 xent
# Project is distributed under the terms of the MIT License

string(REPLACE "_" ";" _platform_parts ${PLATFORM})
list(GET _platform_parts 0 _platform_soc)
string(TOLOWER ${_platform_soc} CMAKE_SYSTEM_SOC)

list(APPEND STARTUP_FILES "startup.c")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_SOC}/startup.c")
    list(APPEND STARTUP_FILES "${CMAKE_SYSTEM_SOC}/startup.c")
endif()

list(APPEND SOURCE_FILES "../pin.c")
list(APPEND SOURCE_FILES "${CONFIG_GEN_CLOCK}/clocking.c")
list(APPEND SOURCE_FILES "${CONFIG_GEN_PIN}/pin.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/system.c")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_SOC}/vectors.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/vectors.c")
endif()

if(CONFIG_PM)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/pm.c")
endif()

if(CONFIG_PLATFORM_LPC_ADC_BASE)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/adc_base.c")
endif()

if(CONFIG_PLATFORM_LPC_ADC)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_ADC}/adc.c")
    list(APPEND SOURCE_FILES "${CONFIG_GEN_ADC}/adc_common.c")
endif()

if(CONFIG_PLATFORM_LPC_ADC_DMA)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_ADC}/adc_dma.c")
endif()

if(CONFIG_PLATFORM_LPC_ADC_DMA_STREAM)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_ADC}/adc_dma_stream.c")
endif()

if(CONFIG_PLATFORM_LPC_ADC_ONESHOT)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_ADC}/adc_oneshot.c")
endif()

if(CONFIG_PLATFORM_LPC_ATIMER)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/atimer.c")
endif()

if(CONFIG_PLATFORM_LPC_BOD)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_BOD}/bod.c")
endif()

if(CONFIG_PLATFORM_LPC_CAN)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_CAN}/can.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/can_base.c")
endif()

if(CONFIG_PLATFORM_LPC_DAC_BASE)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/dac_base.c")
endif()

if(CONFIG_PLATFORM_LPC_DAC)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_DAC}/dac.c")
endif()

if(CONFIG_PLATFORM_LPC_DAC_DMA)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_DAC}/dac_dma.c")
endif()

if(CONFIG_PLATFORM_LPC_EEPROM)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_FLASH}/eeprom.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/eeprom_base.c")
endif()

if(CONFIG_PLATFORM_LPC_EMC)
    list(APPEND SOURCE_FILES "emc_sdram.c")
    list(APPEND SOURCE_FILES "emc_sram.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/emc_base.c")
endif()

if(CONFIG_PLATFORM_LPC_ETHERNET)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/ethernet.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/ethernet_base.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/ethernet_mdio.c")
endif()

if(CONFIG_PLATFORM_LPC_EVENT_ROUTER)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/event_router.c")
endif()

if(CONFIG_PLATFORM_LPC_FAST_GPIO_BUS)
    list(APPEND SOURCE_FILES "fast_gpio_bus_common.c")
    list(APPEND SOURCE_FILES "${CONFIG_GEN_PINBUS}/fast_gpio_bus.c")
endif()

if(CONFIG_PLATFORM_LPC_FLASH)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_FLASH}/flash.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/flash_base.c")
endif()

if(CONFIG_PLATFORM_LPC_GPDMA)
    list(APPEND SOURCE_FILES "gpdma_circular.c")
    list(APPEND SOURCE_FILES "gpdma_list.c")
    list(APPEND SOURCE_FILES "gpdma_memcopy.c")
    list(APPEND SOURCE_FILES "gpdma_oneshot.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/gpdma_base.c")
endif()

if(CONFIG_PLATFORM_LPC_GPPWM)
    list(APPEND SOURCE_FILES "gppwm.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/gppwm_base.c")
endif()

if(CONFIG_PLATFORM_LPC_GPTIMER_BASE)
    list(APPEND SOURCE_FILES "gptimer_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/gptimer_base.c")
endif()

if(CONFIG_PLATFORM_LPC_GPTIMER)
    list(APPEND SOURCE_FILES "gptimer.c")
endif()

if(CONFIG_PLATFORM_LPC_GPTIMER_CAPTURE)
    list(APPEND SOURCE_FILES "gptimer_capture.c")
endif()

if(CONFIG_PLATFORM_LPC_GPTIMER_COUNTER)
    list(APPEND SOURCE_FILES "gptimer_counter.c")
endif()

if(CONFIG_PLATFORM_LPC_GPTIMER_PWM)
    list(APPEND SOURCE_FILES "gptimer_pwm.c")
endif()

if(CONFIG_PLATFORM_LPC_I2C_BASE)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_I2C}/i2c_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/i2c_base.c")
endif()

if(CONFIG_PLATFORM_LPC_I2C)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_I2C}/i2c.c")
endif()

if(CONFIG_PLATFORM_LPC_I2C_SLAVE)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_I2C}/i2c_slave.c")
endif()

if(CONFIG_PLATFORM_LPC_I2S_BASE)
    list(APPEND SOURCE_FILES "i2s_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/i2s_base.c")
endif()

if(CONFIG_PLATFORM_LPC_I2S_DMA)
    list(APPEND SOURCE_FILES "i2s_dma.c")
endif()

if(CONFIG_PLATFORM_LPC_IAP)
    list(APPEND SOURCE_FILES "iap.c")
endif()

if(CONFIG_PLATFORM_LPC_PININT_BASE)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/pin_int_base.c")
endif()

if(CONFIG_PLATFORM_LPC_PININT)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_PININT}/pin_int.c")
endif()

if(CONFIG_PLATFORM_LPC_ONE_WIRE_SSP)
    list(APPEND SOURCE_FILES "one_wire_ssp.c")
endif()

if(CONFIG_PLATFORM_LPC_ONE_WIRE_UART)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_UART}/one_wire_uart.c")
endif()

if(CONFIG_PLATFORM_LPC_RIT)
    list(APPEND SOURCE_FILES "rit.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/rit_base.c")
endif()

if(CONFIG_PLATFORM_LPC_RTC)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_RTC}/rtc.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/rtc_base.c")
endif()

if(CONFIG_PLATFORM_LPC_SDMMC)
    list(APPEND SOURCE_FILES "dma_sdmmc.c")
    list(APPEND SOURCE_FILES "sdmmc.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/sdmmc_base.c")
endif()

if(CONFIG_PLATFORM_LPC_SCT_BASE)
    list(APPEND SOURCE_FILES "sct_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/sct_base.c")
endif()

if(CONFIG_PLATFORM_LPC_SCT_PWM)
    list(APPEND SOURCE_FILES "sct_pwm.c")
endif()

if(CONFIG_PLATFORM_LPC_SCT_TIMER)
    list(APPEND SOURCE_FILES "sct_timer.c")
endif()

if(CONFIG_PLATFORM_LPC_SERIAL)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_UART}/serial.c")
endif()

if(CONFIG_PLATFORM_LPC_SERIAL_POLL)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_UART}/serial_poll.c")
endif()

if(CONFIG_PLATFORM_LPC_SERIAL_DMA)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_UART}/serial_dma.c")
endif()

if(CONFIG_PLATFORM_LPC_SGPIO_BASE)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/sgpio_base.c")
endif()

if(CONFIG_PLATFORM_LPC_SPI)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_SPI}/spi.c")
endif()

if(CONFIG_PLATFORM_LPC_SPI_DMA)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_SPI}/spi_dma.c")
endif()

if(CONFIG_PLATFORM_LPC_SPIFI)
    list(APPEND SOURCE_FILES "spifi.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/spifi_base.c")
endif()

if(CONFIG_PLATFORM_LPC_SSP_BASE)
    list(APPEND SOURCE_FILES "ssp_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/ssp_base.c")
endif()

if(CONFIG_PLATFORM_LPC_UART_BASE)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_UART}/uart_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/uart_base.c")
endif()

if(CONFIG_PLATFORM_LPC_WAKEUPINT)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_PININT}/wakeup_int.c")
endif()

if(CONFIG_PLATFORM_LPC_WDT)
    list(APPEND SOURCE_FILES "wdt.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/wdt_base.c")
endif()

if(CONFIG_PLATFORM_LPC_WWDT)
    list(APPEND SOURCE_FILES "wwdt.c")
endif()

if(CONFIG_PLATFORM_USB)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/usb_base.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/usb_device.c")
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_SOC}/CMakeLists.txt" OPTIONAL)
add_library(halm_platform OBJECT ${STARTUP_FILES} ${SOURCE_FILES})
set_source_files_properties(${STARTUP_FILES} PROPERTIES COMPILE_OPTIONS "${FLAGS_STARTUP}")
