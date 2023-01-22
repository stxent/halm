# Copyright (C) 2023 xent
# Project is distributed under the terms of the MIT License

string(REPLACE "_" ";" _platform_parts ${PLATFORM})
list(GET _platform_parts 0 _platform_soc)
string(TOLOWER ${_platform_soc} CMAKE_SYSTEM_SOC)

list(APPEND SOURCE_FILES "../pin.c")
list(APPEND SOURCE_FILES "startup.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/clocking.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/pin.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/system.c")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_SOC}/startup.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/startup.c")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_SOC}/vectors.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/vectors.c")
endif()

if(${CONFIG_PLATFORM_NUMICRO_ADC_BASE})
    list(APPEND SOURCE_FILES "adc_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/adc_base.c")
endif()

if(${CONFIG_PLATFORM_NUMICRO_ADC})
    list(APPEND SOURCE_FILES "adc.c")
endif()

if(${CONFIG_PLATFORM_NUMICRO_ADC_DMA})
    list(APPEND SOURCE_FILES "adc_dma.c")
endif()

if(${CONFIG_PLATFORM_NUMICRO_ADC_ONESHOT})
    # list(APPEND SOURCE_FILES "adc_oneshot.c")
endif()

if(${CONFIG_PLATFORM_NUMICRO_GPTIMER_BASE})
    list(APPEND SOURCE_FILES "gptimer_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/gptimer_base.c")
endif()

if(${CONFIG_PLATFORM_NUMICRO_GPTIMER})
    list(APPEND SOURCE_FILES "gptimer.c")
endif()

if(${CONFIG_PLATFORM_NUMICRO_PDMA})
    list(APPEND SOURCE_FILES "pdma_circular.c")
    list(APPEND SOURCE_FILES "pdma_list.c")
    list(APPEND SOURCE_FILES "pdma_oneshot.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/pdma_base.c")
endif()

if(${CONFIG_PLATFORM_NUMICRO_PININT})
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/pin_int.c")
endif()

# if(${CONFIG_PM})
#     list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/pm.c")
# endif()

if(${CONFIG_PLATFORM_NUMICRO_SERIAL})
    list(APPEND SOURCE_FILES "serial.c")
endif()

if(${CONFIG_PLATFORM_NUMICRO_SERIAL_DMA})
    # list(APPEND SOURCE_FILES "serial_dma_v1.c")
    list(APPEND SOURCE_FILES "serial_dma.c")
endif()

if(${CONFIG_PLATFORM_NUMICRO_SPI})
    list(APPEND SOURCE_FILES "spi.c")
endif()

if(${CONFIG_PLATFORM_NUMICRO_SPI_BASE})
    list(APPEND SOURCE_FILES "spi_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/spi_base.c")
endif()

if(${CONFIG_PLATFORM_NUMICRO_SPI_DMA})
    list(APPEND SOURCE_FILES "spi_dma.c")
endif()

if(${CONFIG_PLATFORM_NUMICRO_UART_BASE})
    list(APPEND SOURCE_FILES "uart_common.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/uart_base.c")
endif()

if(CONFIG_PLATFORM_USB)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/usb_base.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/usb_device.c")
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_SOC}/CMakeLists.txt" OPTIONAL)
add_library(halm_platform OBJECT ${SOURCE_FILES})
