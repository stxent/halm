# Copyright (C) 2017 xent
# Project is distributed under the terms of the MIT License

string(TOLOWER ${PLATFORM} CMAKE_SYSTEM_SOC)

list(APPEND STARTUP_FILES "startup.c")
list(APPEND STARTUP_FILES "${CMAKE_SYSTEM_SOC}/startup.c")

list(APPEND SOURCE_FILES "../pin.c")
list(APPEND SOURCE_FILES "${CONFIG_GEN_PIN}/pin.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/clocking.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/system.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/vectors.c")

if(CONFIG_PM)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/pm.c")
endif()

if(CONFIG_PLATFORM_STM32_PIN_REMAP)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/pin_remap.c")
endif()

if(CONFIG_PLATFORM_STM32_ADC_BASE)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_ADC}/adc_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/adc_base.c")
endif()

if(CONFIG_PLATFORM_STM32_ADC)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_ADC}/adc.c")
endif()

if(CONFIG_PLATFORM_STM32_ADC_DMA)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_ADC}/adc_dma.c")
endif()

if(CONFIG_PLATFORM_STM32_BDMA)
    list(APPEND SOURCE_FILES "bdma_circular.c")
    list(APPEND SOURCE_FILES "bdma_oneshot.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/bdma_base.c")
endif()

if(CONFIG_PLATFORM_STM32_CAN)
    list(APPEND SOURCE_FILES "can.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/bxcan_base.c")
endif()

if(CONFIG_PLATFORM_STM32_DMA)
    list(APPEND SOURCE_FILES "dma_circular.c")
    list(APPEND SOURCE_FILES "dma_list.c")
    list(APPEND SOURCE_FILES "dma_oneshot.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/dma_base.c")
endif()

if(CONFIG_PLATFORM_STM32_EXTI)
    list(APPEND SOURCE_FILES "exti.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/exti_base.c")
endif()

if(CONFIG_PLATFORM_STM32_GPTIMER_BASE)
    list(APPEND SOURCE_FILES "gptimer_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/gptimer_base.c")
endif()

if(CONFIG_PLATFORM_STM32_GPTIMER)
    list(APPEND SOURCE_FILES "gptimer.c")
endif()

if(CONFIG_PLATFORM_STM32_I2C_BASE)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_I2C}/i2c_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/i2c_base.c")
endif()

if(CONFIG_PLATFORM_STM32_I2C)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_I2C}/i2c.c")
endif()

if(CONFIG_PLATFORM_STM32_IWDG)
    list(APPEND SOURCE_FILES "iwdg.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/iwdg_base.c")
endif()

if(CONFIG_PLATFORM_STM32_SDIO)
    list(APPEND SOURCE_FILES "sdio.c")
    list(APPEND SOURCE_FILES "sdio_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/sdio_base.c")
endif()

if(CONFIG_PLATFORM_STM32_SPI_BASE)
    list(APPEND SOURCE_FILES "spi_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/spi_base.c")
endif()

if(CONFIG_PLATFORM_STM32_SPI)
    list(APPEND SOURCE_FILES "spi.c")
endif()

if(CONFIG_PLATFORM_STM32_UART_BASE)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_UART}/uart_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/uart_base.c")
endif()

if(CONFIG_PLATFORM_STM32_SERIAL)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_UART}/serial.c")
endif()

if(CONFIG_PLATFORM_STM32_SERIAL_DMA)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_UART}/serial_dma.c")
endif()

if(CONFIG_PLATFORM_USB)
    list(APPEND SOURCE_FILES "${CONFIG_GEN_USB}/usb_device.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/usb_base.c")
endif()

if(CONFIG_PLATFORM_STM32_DMA)
    list(APPEND SOURCE_FILES "dma_memcopy.c")
elseif(CONFIG_PLATFORM_STM32_BDMA)
    list(APPEND SOURCE_FILES "bdma_memcopy.c")
endif()

add_library(halm_platform OBJECT ${STARTUP_FILES} ${SOURCE_FILES})
set_source_files_properties(${STARTUP_FILES} PROPERTIES COMPILE_OPTIONS "${FLAGS_STARTUP}")
