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

if(${CONFIG_PLATFORM_NUMICRO_PDMA})
    list(APPEND SOURCE_FILES "pdma_oneshot.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/pdma_base.c")
endif()

# if(${CONFIG_PM})
#     list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/pm.c")
# endif()

if(${CONFIG_PLATFORM_NUMICRO_SERIAL})
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/serial.c")
endif()

#if(${CONFIG_PLATFORM_NUMICRO_SERIAL_DMA})
    #list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/serial_dma.c")
#endif()

# if(${CONFIG_PLATFORM_NUMICRO_SPI})
#     list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/spi.c")
# endif()

if(${CONFIG_PLATFORM_NUMICRO_SPI_BASE})
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/spi_base.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/spi_common.c")
endif()

# if(${CONFIG_PLATFORM_NUMICRO_SPI_DMA})
#     list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/spi_dma.c")
# endif()

if(${CONFIG_PLATFORM_NUMICRO_UART_BASE})
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/uart_base.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/uart_common.c")
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_SOC}/CMakeLists.txt" OPTIONAL)
add_library(halm_platform OBJECT ${SOURCE_FILES})
