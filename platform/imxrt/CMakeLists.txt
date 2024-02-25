# Copyright (C) 2024 xent
# Project is distributed under the terms of the MIT License

string(REPLACE "_" ";" _platform_parts ${PLATFORM})
list(GET _platform_parts 0 _platform_soc)
string(TOLOWER ${_platform_soc} CMAKE_SYSTEM_SOC)

list(APPEND SOURCE_FILES "../pin.c")
list(APPEND SOURCE_FILES "startup.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/clocking.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/pin.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/startup.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/system.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/vectors.c")

if(CONFIG_PM)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/pm.c")
endif()

if(CONFIG_PLATFORM_IMXRT_SERIAL)
    list(APPEND SOURCE_FILES "serial.c")
endif()

if(CONFIG_PLATFORM_IMXRT_SERIAL_DMA)
    list(APPEND SOURCE_FILES "serial_dma.c")
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_SOC}/CMakeLists.txt" OPTIONAL)
add_library(halm_platform OBJECT ${SOURCE_FILES})
