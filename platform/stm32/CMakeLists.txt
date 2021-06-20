# Copyright (C) 2017 xent
# Project is distributed under the terms of the MIT License

string(TOLOWER ${PLATFORM} CMAKE_SYSTEM_SOC)

list(APPEND SOURCE_FILES "../pin.c")
list(APPEND SOURCE_FILES "startup.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/clocking.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/pin.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/pin_remap.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/startup.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/system.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/vectors.c")

if(${CONFIG_PM})
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/pm.c")
endif()

if(${CONFIG_PLATFORM_STM32_DMA})
    list(APPEND SOURCE_FILES "dma_circular.c")
    list(APPEND SOURCE_FILES "dma_oneshot.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/dma_base.c")
endif()

if(${CONFIG_PLATFORM_STM32_CAN})
    list(APPEND SOURCE_FILES "can.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/bxcan_base.c")
endif()

if(${CONFIG_PLATFORM_STM32_EXTI})
    list(APPEND SOURCE_FILES "exti.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/exti_base.c")
endif()

if(${CONFIG_PLATFORM_STM32_GPTIMER_BASE})
    list(APPEND SOURCE_FILES "gptimer_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/gptimer_base.c")
endif()

if(${CONFIG_PLATFORM_STM32_GPTIMER})
    list(APPEND SOURCE_FILES "gptimer.c")
endif()

if(${CONFIG_PLATFORM_STM32_SPI_BASE})
    list(APPEND SOURCE_FILES "spi_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/spi_base.c")
endif()

if(${CONFIG_PLATFORM_STM32_SPI})
    list(APPEND SOURCE_FILES "spi.c")
endif()

if(${CONFIG_PLATFORM_STM32_UART_BASE})
    list(APPEND SOURCE_FILES "${CONFIG_GEN_UART}/uart_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/uart_base.c")
endif()

if(${CONFIG_PLATFORM_STM32_SERIAL})
    list(APPEND SOURCE_FILES "${CONFIG_GEN_UART}/serial.c")
endif()

if(${CONFIG_PLATFORM_STM32_SERIAL_DMA})
    list(APPEND SOURCE_FILES "${CONFIG_GEN_UART}/serial_dma.c")
endif()

if(${CONFIG_PLATFORM_USB})
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/usb_base.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/usb_device.c")
endif()

add_library(halm_platform OBJECT ${SOURCE_FILES})
