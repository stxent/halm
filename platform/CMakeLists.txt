#Copyright (C) 2017 xent
#Project is distributed under the terms of the GNU General Public License v3.0

string(FIND "${CMAKE_SYSTEM_NAME}" "Generic" SYSTEM_IS_GENERIC)

if(${SYSTEM_IS_GENERIC} EQUAL 0)
    if(NOT DEFINED PLATFORM)
        message(FATAL_ERROR "PLATFORM not defined")
    endif()

    string(FIND "${PLATFORM}" "LPC" PLATFORM_IS_NXP)
    string(FIND "${PLATFORM}" "STM32" PLATFORM_IS_STM)

    if(${PLATFORM_IS_NXP} EQUAL 0)
        include("${PROJECT_SOURCE_DIR}/platform/nxp/CMakeLists.txt")
    elseif(${PLATFORM_IS_STM} EQUAL 0)
        include("${PROJECT_SOURCE_DIR}/platform/stm/CMakeLists.txt")
    endif()
else()
    string(TOLOWER ${CMAKE_SYSTEM_NAME} CONVERTED_NAME)
    include("${PROJECT_SOURCE_DIR}/platform/${CONVERTED_NAME}/CMakeLists.txt")
endif()
