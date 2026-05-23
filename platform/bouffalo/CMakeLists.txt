# Copyright (C) 2026 xent
# Project is distributed under the terms of the MIT License

string(TOLOWER ${PLATFORM} CMAKE_SYSTEM_SOC)

list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/clocking.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/l1c.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/pin.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/startup.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/startup.S")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/system.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/vectors.c")

if(CONFIG_CORE_RISCV_MACHINE_TIMER)
    list(APPEND SOURCE_FILES "machine_timer.c")
endif()

if(CONFIG_PM)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/pm.c")
endif()

if(CONFIG_PLATFORM_BOUFFALO_DMA)
    list(APPEND SOURCE_FILES "dma_circular.c")
    list(APPEND SOURCE_FILES "dma_list.c")
    list(APPEND SOURCE_FILES "dma_memcopy.c")
    list(APPEND SOURCE_FILES "dma_oneshot.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/dma_base.c")
endif()

if(CONFIG_PLATFORM_BOUFFALO_GPTIMER_BASE)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/gptimer_base.c")
endif()

if(CONFIG_PLATFORM_BOUFFALO_GPTIMER)
    list(APPEND SOURCE_FILES "gptimer.c")
endif()

if(CONFIG_PLATFORM_BOUFFALO_SPI)
    list(APPEND SOURCE_FILES "spi.c")
endif()

if(CONFIG_PLATFORM_BOUFFALO_SPI_BASE)
    list(APPEND SOURCE_FILES "spi_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/spi_base.c")
endif()

if(CONFIG_PLATFORM_BOUFFALO_SPI_DMA)
    list(APPEND SOURCE_FILES "spi_dma.c")
endif()

if(CONFIG_PLATFORM_BOUFFALO_UART_BASE)
    list(APPEND SOURCE_FILES "uart_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/uart_base.c")
endif()

if(CONFIG_PLATFORM_BOUFFALO_SERIAL)
    list(APPEND SOURCE_FILES "serial.c")
endif()

if(CONFIG_PLATFORM_BOUFFALO_SERIAL_DMA)
    list(APPEND SOURCE_FILES "serial_dma.c")
endif()

add_library(halm_platform OBJECT ${SOURCE_FILES})
